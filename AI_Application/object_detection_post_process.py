# object_detection_post_process.py  (DROP-IN for Hailo NMS output)
# - Supports Hailo on-chip NMS: (1,5,N)/(5,N)/(N,5) [x1,y1,x2,y2,score]
# - Handles resize vs letterbox scaling back to pixel coords
# - Keeps optional YOLO-XYWH / DFL code paths for future reuse (not used for current HEF)

import cv2
import numpy as np
from common.toolbox import id_to_color

# -----------------------------
# Center cache for DFL decoder
# -----------------------------
_CENTER_CACHE = {}
def _get_dfl_centers(net_size: int) -> np.ndarray:
    c = _CENTER_CACHE.get(net_size)
    if c is not None:
        return c
    centers = []
    for s in (8, 16, 32):
        h, w = net_size // s, net_size // s
        ys, xs = np.meshgrid(np.arange(h), np.arange(w), indexing='ij')
        cx = (xs.reshape(-1) + 0.5) * s
        cy = (ys.reshape(-1) + 0.5) * s
        centers.append(np.stack([cx, cy, np.full_like(cx, s)], axis=1))
    c = np.concatenate(centers, axis=0).astype(np.float32)
    _CENTER_CACHE[net_size] = c
    return c


# -----------------------------
# Lightweight debug printer
# -----------------------------
try:
    RAW_PRINT_EVERY
except NameError:
    RAW_PRINT_EVERY = 0

def _print_raw(arr, tag="[raw]", every=0, frame_id=None):
    if not every or every <= 0:
        return
    key = f"__cnt_{tag}"
    c = getattr(_print_raw, key, 0) + 1
    setattr(_print_raw, key, c)
    if c % int(every) != 0:
        return
    a = np.asarray(arr)
    try:
        msg = f"{tag} shape={a.shape} dtype={a.dtype}"
        if frame_id is not None:
            msg = f"{msg} frame={frame_id}"
        print(msg)
    except Exception as e:
        print(f"{tag} print error: {e}")


# -----------------------------
# NMS (class-agnostic)
# -----------------------------
def nms_xyxy_agnostic(boxes, scores, iou_th=0.45, max_det=100):
    boxes  = np.asarray(boxes, float)
    scores = np.asarray(scores, float)
    order = scores.argsort()[::-1]
    keep = []
    while order.size and len(keep) < max_det:
        i = order[0]
        keep.append(i)
        if order.size == 1:
            break
        xx1 = np.maximum(boxes[i,0], boxes[order[1:],0])
        yy1 = np.maximum(boxes[i,1], boxes[order[1:],1])
        xx2 = np.minimum(boxes[i,2], boxes[order[1:],2])
        yy2 = np.minimum(boxes[i,3], boxes[order[1:],3])
        w = np.maximum(0.0, xx2-xx1)
        h = np.maximum(0.0, yy2-yy1)
        inter = w*h
        area_i = (boxes[i,2]-boxes[i,0])*(boxes[i,3]-boxes[i,1]) + 1e-5
        area_o = (boxes[order[1:],2]-boxes[order[1:],0])*(boxes[order[1:],3]-boxes[order[1:],1]) + 1e-5
        iou = inter / (area_i + area_o - inter + 1e-5)
        order = order[1:][iou < iou_th]
    return np.asarray(keep, int)


# -----------------------------
# Small utils
# -----------------------------
def _xyxy_to_xywh(boxes):
    b = np.asarray(boxes, dtype=np.float32)
    cx = (b[:, 0] + b[:, 2]) * 0.5
    cy = (b[:, 1] + b[:, 3]) * 0.5
    w  =  b[:, 2] - b[:, 0]
    h  =  b[:, 3] - b[:, 1]
    return np.stack([cx, cy, w, h], axis=1)

def _xywh_to_xyxy(xywh):
    a = np.asarray(xywh, dtype=np.float32)
    if a.ndim == 1:
        cx, cy, w, h = a
        return np.array([cx - w*0.5, cy - h*0.5, cx + w*0.5, cy + h*0.5], dtype=np.float32)
    x1 = a[:, 0] - a[:, 2] * 0.5
    y1 = a[:, 1] - a[:, 3] * 0.5
    x2 = a[:, 0] + a[:, 2] * 0.5
    y2 = a[:, 1] + a[:, 3] * 0.5
    return np.stack([x1, y1, x2, y2], axis=1)

def _as_prob(x):
    x = np.asarray(x, dtype=np.float32)
    xmin, xmax = float(np.nanmin(x)), float(np.nanmax(x))
    if xmin >= 0.0 and xmax <= 255.0 and xmax >= 2.0:
        return np.clip(x / 255.0, 0.0, 1.0)
    x = np.clip(x, -50.0, 50.0)
    return 1.0 / (1.0 + np.exp(-x))

def _stable_softmax(z, axis=-1):
    z = np.asarray(z, dtype=np.float32)
    z = z - np.max(z, axis=axis, keepdims=True)
    z = np.clip(z, -50.0, 50.0)
    e = np.exp(z)
    denom = np.sum(e, axis=axis, keepdims=True) + 1e-9
    return e / denom

def _sigmoid(x):
    x = np.asarray(x, dtype=np.float32)
    x = np.clip(x, -50.0, 50.0)
    return 1.0 / (1.0 + np.exp(-x))


def denorm_and_unletterbox_xyxy(box, net_size, h0, w0):
    """box: [x1,y1,x2,y2] in 0..1 or 0..net_size → pixel coords with letterbox undo."""
    b = np.array(box, dtype=np.float32).copy()
    if not np.all(np.isfinite(b)):
        return None
    if b.max() <= 1.0 + 1e-6:
        b *= float(net_size)

    r  = min(net_size / float(h0), net_size / float(w0))
    dw = (net_size - w0 * r) / 2.0
    dh = (net_size - h0 * r) / 2.0

    x1 = (b[0] - dw) / r
    y1 = (b[1] - dh) / r
    x2 = (b[2] - dw) / r
    y2 = (b[3] - dh) / r

    if not all(np.isfinite([x1,y1,x2,y2])):
        return None
    if (x2 - x1) <= 1 or (y2 - y1) <= 1:
        return None

    x1 = float(np.clip(x1, 0, w0))
    x2 = float(np.clip(x2, 0, w0))
    y1 = float(np.clip(y1, 0, h0))
    y2 = float(np.clip(y2, 0, h0))
    if (x2 - x1) <= 1 or (y2 - y1) <= 1:
        return None
    return [x1, y1, x2, y2]


# -----------------------------
# YOLO xywh head decoder
# -----------------------------
def decode_yolo_xywh_with_head(arr, img_size, *,
                               score_th=0.35, obj_th=0.25, class_min=0.35,
                               head="auto", drone_idx=None, force_single_class=False):
    """
    arr: (N, C).  [x,y,w,h, obj, cls...] or [x,y,w,h, cls..., obj]
    """
    a = np.asarray(arr, np.float32)
    assert a.ndim == 2 and a.shape[1] >= 6, f"bad shape {a.shape}"
    C = a.shape[1]
    K = C - 5

    # decide head layout
    if head == "yolo_xywh_obj_cls":
        ix_obj = 4;  sl_cls = slice(5, 5+K)
    elif head == "yolo_xywh_cls_obj":
        ix_obj = -1; sl_cls = slice(4, 4+K)
    else:
        o4 = _as_prob(a[:, 4])
        ol = _as_prob(a[:, -1])
        def score(o):
            m, s = float(np.mean(o)), float(np.std(o))
            pen = 0.0 if 0.05 <= m <= 0.8 else 0.3
            return s - pen
        if score(o4) >= score(ol):
            ix_obj = 4;  sl_cls = slice(5, 5+K)
        else:
            ix_obj = -1; sl_cls = slice(4, 4+K)

    xywh = a[:, :4]
    if float(np.nanmax(np.abs(xywh))) <= 1.5:
        xywh = xywh * float(img_size)
    boxes = _xywh_to_xyxy(xywh)

    obj_p = _as_prob(a[:, ix_obj])
    cls_p = _as_prob(a[:, sl_cls])  # [N, K]

    if (drone_idx is not None) and (0 <= int(drone_idx) < K):
        di = int(drone_idx)
        cls_score = cls_p[:, di]
        class_ids = (np.zeros_like(cls_score, dtype=np.int32) if force_single_class
                     else np.full(cls_score.shape, di, dtype=np.int32))
    else:
        max_idx   = np.argmax(cls_p, axis=1)
        cls_score = cls_p[np.arange(cls_p.shape[0]), max_idx]
        class_ids = (np.zeros_like(max_idx, dtype=np.int32) if force_single_class
                     else max_idx.astype(np.int32))

    keep = (obj_p >= float(obj_th)) & (cls_score >= float(class_min))
    if not np.any(keep):
        return np.empty((0,4), np.float32), np.array([], np.float32), np.array([], np.int32)

    scores = obj_p * cls_score
    keep = keep & (scores >= float(score_th))
    if not np.any(keep):
        return np.empty((0,4), np.float32), np.array([], np.float32), np.array([], np.int32)

    boxes   = boxes[keep]
    scores  = scores[keep]
    classes = class_ids[keep]

    boxes[:, [0,2]] = np.clip(boxes[:, [0,2]], 0, float(img_size))
    boxes[:, [1,3]] = np.clip(boxes[:, [1,3]], 0, float(img_size))
    return boxes.astype(np.float32), scores.astype(np.float32), classes.astype(np.int32)


# -----------------------------
# DFL decoder (optional path)
# -----------------------------
def _decode_yolov8_dfl_core(dfl_64, cls_1, img_size, strides, score_th,
                            reg_max=16, model_cfg=None):
    # dfl_64: [N,64], cls_1: [N,1]
    assert dfl_64.shape[1] == 64 and cls_1.shape[1] == 1
    N = dfl_64.shape[0]
    dfl = dfl_64.reshape(N, 4, reg_max)  # [N,4,16]

    x = np.arange(reg_max, dtype=np.float32)
    # softmax stable
    z = dfl - np.max(dfl, axis=2, keepdims=True)
    z = np.clip(z, -50.0, 50.0)
    e = np.exp(z); denom = np.sum(e, axis=2, keepdims=True) + 1e-9
    prob = e / denom                          # [N,4,16]
    dist = (prob @ x).astype(np.float32)      # [N,4]

    # expected anchors count for strides (8,16,32)
    N_expect = ((img_size//8)*(img_size//8) +
                (img_size//16)*(img_size//16) +
                (img_size//32)*(img_size//32))
    if N != N_expect:
        return np.empty((0,4), np.float32), np.array([], np.float32), np.array([], np.int32)

    centers = _get_dfl_centers(img_size)      # [N,3]: cx, cy, stride
    cx, cy, s = centers[:,0], centers[:,1], centers[:,2]

    l, t, r, b = dist[:,0]*s, dist[:,1]*s, dist[:,2]*s, dist[:,3]*s
    x1 = cx - l; y1 = cy - t; x2 = cx + r; y2 = cy + b

    finite = np.isfinite(x1) & np.isfinite(y1) & np.isfinite(x2) & np.isfinite(y2)
    pos    = (x2 > x1) & (y2 > y1)
    valid  = finite & pos
    if not np.any(valid):
        return np.empty((0,4), np.float32), np.array([], np.float32), np.array([], np.int32)

    boxes = np.stack([x1, y1, x2, y2], axis=1)[valid]

    # class score (logit → prob) with temperature / bias
    mcfg = model_cfg or {}
    T = float(mcfg.get("cls_temperature", 8.0))
    cls_bias = float(mcfg.get("cls_bias", 0.0))

    cls_raw = cls_1.squeeze(-1).astype(np.float32)
    if np.all(cls_raw >= -1e-6) and np.all(cls_raw <= 1.0 + 1e-6):
        scores_all = np.clip(cls_raw, 0.0, 1.0)
    else:
        if cls_bias != 0.0:
            cls_raw = cls_raw - cls_bias
        if T > 1.0:
            cls_raw = cls_raw / T
        cls_raw = np.clip(cls_raw, -12.0, 12.0)
        scores_all = 1.0 / (1.0 + np.exp(-cls_raw))
    scores = scores_all[valid]

    keep = scores >= float(score_th)
    if not np.any(keep):
        return np.empty((0,4), np.float32), np.array([], np.float32), np.array([], np.int32)

    boxes  = boxes[keep]
    scores = scores[keep]
    class_ids = np.zeros_like(scores, dtype=np.int32)  # single-class
    boxes[:, [0,2]] = np.clip(boxes[:, [0,2]], 0, float(img_size))
    boxes[:, [1,3]] = np.clip(boxes[:, [1,3]], 0, float(img_size))
    return boxes.astype(np.float32), scores.astype(np.float32), class_ids


def decode_yolov8_dfl_singlehead_auto(logits, img_size: int, num_classes: int = 1,
                                      reg_max: int = 16, strides=(8,16,32),
                                      score_th=0.6, model_cfg=None):
    """Channels [64 DFL | 1 CLS] vs [1 CLS | 64 DFL] auto-select."""
    assert logits.ndim == 2 and logits.shape[1] == (4*reg_max + num_classes), f"bad logits shape {logits.shape}"
    dfl1 = logits[:, :4*reg_max];     cls1 = logits[:, 4*reg_max:]
    b1, s1, c1 = _decode_yolov8_dfl_core(dfl1, cls1, img_size, strides, score_th, reg_max, model_cfg)

    dfl2 = logits[:, num_classes:num_classes+4*reg_max];  cls2 = logits[:, :num_classes]
    b2, s2, c2 = _decode_yolov8_dfl_core(dfl2, cls2, img_size, strides, score_th, reg_max, model_cfg)

    if b2.shape[0] > b1.shape[0] or (b2.shape[0] == b1.shape[0] and float(s2.sum()) > float(s1.sum())):
        return b2, s2, c2
    return b1, s1, c1


# -----------------------------
# Main APIs
# -----------------------------
def inference_result_handler(original_frame, infer_results, labels, config_data, tracker=None, return_detection=False):
    """
    infer_results: pass the SINGLE output array for Hailo NMS, e.g.
      out = infer_results["model/yolov8_nms_postprocess"]  # (1,5,N)
    """
    det_raw = extract_detections(original_frame, infer_results, config_data)

    if return_detection:
        H, W = original_frame.shape[:2]
        det_list = []

        if isinstance(det_raw, dict):
            boxes   = (det_raw.get("detection_boxes")
                       or det_raw.get("boxes") or det_raw.get("bboxes") or det_raw.get("bbox"))
            classes = (det_raw.get("detection_classes")
                       or det_raw.get("classes") or det_raw.get("class_ids") or det_raw.get("labels"))
            scores  = (det_raw.get("detection_scores")
                       or det_raw.get("scores") or det_raw.get("conf") or det_raw.get("confidences"))
            if boxes is None:
                return []
            boxes = np.asarray(boxes)
            if boxes.ndim >= 2 and boxes.shape[-1] >= 4:
                boxes = boxes.reshape(-1, boxes.shape[-1])[:, :4]
            else:
                return []

            N = boxes.shape[0]
            classes = np.zeros((N,), dtype=int)   if classes is None else np.asarray(classes).reshape(-1)[:N]
            scores  = np.ones((N,), dtype=float)  if scores  is None else np.asarray(scores).reshape(-1)[:N]

            x1 = np.clip(boxes[:,0].round().astype(int), 0, W-1)
            y1 = np.clip(boxes[:,1].round().astype(int), 0, H-1)
            x2 = np.clip(boxes[:,2].round().astype(int), 0, W-1)
            y2 = np.clip(boxes[:,3].round().astype(int), 0, H-1)

            w = x2 - x1; h = y2 - y1
            keep = (w > 1) & (h > 1) & (scores > 0.0)

            for xx1, yy1, xx2, yy2, sc, ci in zip(x1[keep], y1[keep], x2[keep], y2[keep], scores[keep], classes[keep]):
                det_list.append({
                    "class_id": int(ci),
                    "accuracy": float(sc),
                    "score": float(sc),
                    "bbox": {"x1": int(xx1), "y1": int(yy1), "x2": int(xx2), "y2": int(yy2)}
                })
            return det_list

        if isinstance(det_raw, list):
            for d in det_raw:
                if not isinstance(d, dict):
                    continue
                if "bbox" in d:
                    b = d["bbox"]
                    x1, y1, x2, y2 = b.get("x1"), b.get("y1"), b.get("x2"), b.get("y2")
                    if None in (x1, y1, x2, y2):
                        continue
                    sc = float(d.get("accuracy", d.get("score", 0.0)))
                    det_list.append({
                        "class_id": int(d.get("class_id", 0)),
                        "accuracy": sc,
                        "score": sc,
                        "bbox": {
                            "x1": int(round(x1)), "y1": int(round(y1)),
                            "x2": int(round(x2)), "y2": int(round(y2)),
                        }
                    })
            return det_list

        return []

    return draw_detections(det_raw, original_frame, labels, tracker=tracker)


def extract_detections(image: np.ndarray, detections, config_data) -> dict:
    """Return dict: detection_boxes/classes/scores/num_detections (+ optional detections xywh list)"""
    # ----- config & thresholds -----
    model_cfg = (config_data.get("model") or {})
    pp        = (config_data.get("postprocess_params") or {})
    vis       = (config_data.get("visualization_params") or {})

    net_size  = int(model_cfg.get("input_size", 320))
    pre_mode  = str(model_cfg.get("preprocess_mode", "letterbox")).lower()  # "letterbox" | "resize"

    min_area_ratio = float(model_cfg.get("min_box_area_ratio", 0.0))
    max_area_ratio = float(model_cfg.get("max_box_area_ratio", 1.0))
    drop_border    = float(model_cfg.get("drop_border_ratio", 0.0))

    score_th = float(pp.get("score_threshold", vis.get("score_thres", 0.50)))
    iou_th   = float(pp.get("nms_iou_thresh",   0.45))
    max_det  = int(pp.get("max_detections",     vis.get("max_boxes_to_draw", 10)))
    pre_topk = int(pp.get("pre_nms_topk",       0))

    # for YOLO-xywh branch defaults
    obj_th   = float(pp.get("obj_threshold", 0.25))
    cls_min  = float(pp.get("class_min", 0.35))
    head_opt = str(pp.get("head_layout", "auto"))
    drone_idx = pp.get("drone_class_index", None)
    force_sc  = bool(pp.get("force_single_class", True))

    H, W = image.shape[:2]

    # net → pixel with keep indices
    def _to_pixel_boxes(boxes_net):
        boxes_pix = []
        keep_idx  = []
        sx, sy = W / float(net_size), H / float(net_size)
        for i, b in enumerate(np.asarray(boxes_net, dtype=np.float32)):
            if pre_mode == "resize":
                d = [b[0]*sx, b[1]*sy, b[2]*sx, b[3]*sy]
            else:
                d = denorm_and_unletterbox_xyxy(b, net_size=net_size, h0=H, w0=W)
                if d is None:
                    continue
            x1, y1, x2, y2 = d
            if (x2 - x1) > 1 and (y2 - y1) > 1:
                boxes_pix.append([x1, y1, x2, y2])
                keep_idx.append(i)
        if not boxes_pix:
            return np.empty((0,4), np.float32), np.array([], dtype=int)
        return np.asarray(boxes_pix, np.float32), np.asarray(keep_idx, dtype=int)

    # try ndarray
    try:
        arr_any = np.asarray(detections)
    except Exception:
        arr_any = None

    # =========================================
    # PATH 1) Hailo on-chip NMS : (1,5,N)/(5,N)/(N,5)
    # =========================================
    if arr_any is not None:
        a = np.asarray(arr_any)
        if a.ndim == 3 and a.shape[0] == 1:
            a = a[0]  # (1,5,N) → (5,N)
        if a.ndim == 2 and (a.shape[0] == 5 or a.shape[1] == 5):
            mat = a if a.shape[0] == 5 else a.T  # (5,N)

            # 좌표 순서 인식 (설정 우선, 없으면 auto)
            nms_order = str(pp.get("nms_order", pp.get("nms_layout", "auto"))).lower()

            def _convert(order_tag: str):
                """order_tag in {'xyxy','yxyx'} → (boxes_pix, scores_aligned)"""
                if order_tag == "yxyx":
                    y1_, x1_, y2_, x2_, scr = [mat[i] for i in range(5)]
                    boxes_net = np.stack([x1_, y1_, x2_, y2_], axis=1).astype(np.float32)
                else:  # "xyxy"
                    x1_, y1_, x2_, y2_, scr = [mat[i] for i in range(5)]
                    boxes_net = np.stack([x1_, y1_, x2_, y2_], axis=1).astype(np.float32)

                scr = np.asarray(scr, np.float32).reshape(-1)

                # 1차 점수 컷
                keep = scr >= float(score_th)
                if not np.any(keep):
                    return np.empty((0, 4), np.float32), np.empty((0,), np.float32)

                boxes_kept = boxes_net[keep]
                scr_kept   = scr[keep]

                # 네트 좌표 → 픽셀 좌표 변환 (invalid drop 포함)
                boxes_pix, keep_idx = _to_pixel_boxes(boxes_kept)
                if boxes_pix.size == 0:
                    return np.empty((0, 4), np.float32), np.empty((0,), np.float32)

                # 점수를 살아남은 박스에 정렬
                if keep_idx.size:
                    scr_aligned = scr_kept[keep_idx]
                else:
                    scr_aligned = np.empty((0,), np.float32)

                # 최종 안전 장치: 길이 동기화
                m = min(boxes_pix.shape[0], scr_aligned.shape[0])
                return boxes_pix[:m], scr_aligned[:m]

            if nms_order in ("xyxy", "yxyx"):
                boxes_pix, scores = _convert(nms_order)
            else:
                # auto: 두 가설 비교(유효 개수↑, 평균 면적↑가 더 그럴듯)
                b_xy, s_xy = _convert("xyxy")
                b_yx, s_yx = _convert("yxyx")

                def _valid_score(b):
                    if b.size == 0:
                        return (0, 0.0)
                    areas = (b[:, 2] - b[:, 0]) * (b[:, 3] - b[:, 1])
                    return (b.shape[0], float(np.mean(areas)))

                boxes_pix, scores = (b_yx, s_yx) if _valid_score(b_yx) > _valid_score(b_xy) else (b_xy, s_xy)

            if boxes_pix.size == 0:
                return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}

            # 단일 클래스 가정
            n = boxes_pix.shape[0]
            classes = np.zeros((n,), np.int32)

            # area/border filters (인덱스 기반으로 동기화)
            areas = (boxes_pix[:, 2] - boxes_pix[:, 0]) * (boxes_pix[:, 3] - boxes_pix[:, 1])
            mask = np.ones((n,), dtype=bool)
            if min_area_ratio > 0.0:
                mask &= (areas >= min_area_ratio * (W * H))
            if max_area_ratio < 1.0:
                mask &= (areas <= max_area_ratio * (W * H))
            if drop_border > 0.0:
                bx1, by1, bx2, by2 = boxes_pix[:, 0], boxes_pix[:, 1], boxes_pix[:, 2], boxes_pix[:, 3]
                edge = (bx1 <= drop_border * W) | (bx2 >= (1 - drop_border) * W) | \
                       (by1 <= drop_border * H) | (by2 >= (1 - drop_border) * H)
                mask &= ~edge

            idx = np.nonzero(mask)[0]
            if idx.size == 0:
                return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}
            boxes_pix = boxes_pix[idx]
            scores    = scores[idx]
            classes   = classes[idx]

            # 칩 내부에서 이미 NMS 완료 → Top-K만 컷
            if max_det and boxes_pix.shape[0] > max_det:
                order = np.argsort(-scores)[:max_det]
                boxes_pix, scores, classes = boxes_pix[order], scores[order], classes[order]

            boxes_pix = boxes_pix.round().astype(int)
            scores    = np.clip(scores, 0.0, 1.0).astype(float)

            return {
                'detection_boxes': boxes_pix.tolist(),
                'detection_classes': classes.tolist(),
                'detection_scores': scores.tolist(),
                'num_detections': int(boxes_pix.shape[0])
            }

    # =========================================
    # PATH 2) DFL single-head (64 + 1)  → not used for current HEF
    # =========================================
    if arr_any is not None and arr_any.ndim == 2 and arr_any.shape[1] == 65:
        arr = arr_any.astype(np.float32)

        dq = (model_cfg.get("output_dequant") or {})
        if dq:
            scale = float(dq.get("scale", 1.0)); zp = float(dq.get("zp", 0.0))
            arr = (arr - zp) * scale

        dfl1, cls1 = arr[:, :64],  arr[:, 64:65]
        dfl2, cls2 = arr[:, 1:65], arr[:, :1]

        b1, s1, c1 = _decode_yolov8_dfl_core(dfl1, cls1, net_size, (8,16,32), score_th, 16, model_cfg)
        b2, s2, c2 = _decode_yolov8_dfl_core(dfl2, cls2, net_size, (8,16,32), score_th, 16, model_cfg)
        if b2.shape[0] > b1.shape[0] or (b2.shape[0] == b1.shape[0] and float(s2.sum()) > float(s1.sum())):
            boxes_net, scores, classes = b2, s2, c2
        else:
            boxes_net, scores, classes = b1, s1, c1

        if boxes_net.size == 0:
            return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}

        boxes_pix, keep = _to_pixel_boxes(boxes_net)
        if boxes_pix.size == 0:
            return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}
        scores  = scores[keep]
        classes = classes[keep]

        # filters + pre-NMS topk
        areas = (boxes_pix[:,2] - boxes_pix[:,0]) * (boxes_pix[:,3] - boxes_pix[:,1])
        mask = np.ones((boxes_pix.shape[0],), dtype=bool)
        if min_area_ratio > 0.0:
            mask &= (areas >= min_area_ratio * (W * H))
        if max_area_ratio < 1.0:
            mask &= (areas <= max_area_ratio * (W * H))
        if drop_border > 0.0:
            bx1, by1, bx2, by2 = boxes_pix[:,0], boxes_pix[:,1], boxes_pix[:,2], boxes_pix[:,3]
            edge = (bx1 <= drop_border * W) | (bx2 >= (1 - drop_border) * W) | (by1 <= drop_border * H) | (by2 >= (1 - drop_border) * H)
            mask &= ~edge
        if not np.any(mask):
            return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}
        boxes_pix, scores, classes = boxes_pix[mask], scores[mask], classes[mask]

        if pre_topk and boxes_pix.shape[0] > pre_topk:
            order = np.argsort(-scores)[:pre_topk]
            boxes_pix, scores, classes = boxes_pix[order], scores[order], classes[order]

        keep_idx = nms_xyxy_agnostic(boxes_pix, scores, iou_th=iou_th, max_det=max_det)
        if keep_idx.size == 0:
            return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}

        boxes_pix = boxes_pix[keep_idx].round().astype(int)
        scores    = np.clip(scores[keep_idx], 0.0, 1.0).astype(float)
        classes   = np.zeros((len(scores),), dtype=int)  # 1-class

        xywh = _xyxy_to_xywh(boxes_pix)
        det_xywh = [
            {"class_id": int(ci), "confidence": float(sc),
             "x": float(cx), "y": float(cy), "w": float(w), "h": float(h)}
            for (cx, cy, w, h), sc, ci in zip(xywh, scores, classes)
        ]
        return {
            'detection_boxes': boxes_pix.tolist(),
            'detection_classes': classes.tolist(),
            'detection_scores': scores.tolist(),
            'num_detections': int(keep_idx.size),
            "detections": det_xywh
        }

    # =========================================
    # PATH 3) YOLO xywh (obj/cls) → not used for current HEF
    # =========================================
    if arr_any is not None and arr_any.ndim == 2 and arr_any.shape[1] >= 6:
        arr = arr_any.astype(np.float32)
        boxes_net, scores, classes = decode_yolo_xywh_with_head(
            arr, img_size=net_size,
            score_th=score_th, obj_th=obj_th, class_min=cls_min,
            head=head_opt, drone_idx=drone_idx, force_single_class=force_sc
        )
        if boxes_net.size == 0:
            return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}

        boxes_pix, keep = _to_pixel_boxes(boxes_net)
        if boxes_pix.size == 0:
            return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}
        scores  = np.asarray(scores,  np.float32)[keep]
        classes = np.asarray(classes, np.int32)[keep]

        areas = (boxes_pix[:,2]-boxes_pix[:,0])*(boxes_pix[:,3]-boxes_pix[:,1])
        mask = np.ones((boxes_pix.shape[0],), dtype=bool)
        if min_area_ratio > 0.0:
            mask &= (areas >= min_area_ratio*(W*H))
        if max_area_ratio < 1.0:
            mask &= (areas <= max_area_ratio*(W*H))
        if drop_border > 0.0:
            bx1, by1, bx2, by2 = boxes_pix[:,0], boxes_pix[:,1], boxes_pix[:,2], boxes_pix[:,3]
            edge = (bx1<=drop_border*W)|(bx2>=(1-drop_border)*W)|(by1<=drop_border*H)|(by2>=(1-drop_border)*H)
            mask &= ~edge
        if not np.any(mask):
            return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}
        boxes_pix, scores, classes = boxes_pix[mask], scores[mask], classes[mask]

        if pre_topk and boxes_pix.shape[0] > pre_topk:
            order = np.argsort(-scores)[:pre_topk]
            boxes_pix, scores, classes = boxes_pix[order], scores[order], classes[order]

        keep_idx = nms_xyxy_agnostic(boxes_pix, scores, iou_th=iou_th, max_det=max_det)
        if keep_idx.size == 0:
            return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}
        boxes_pix = boxes_pix[keep_idx].round().astype(int)
        scores    = np.clip(scores[keep_idx], 0.0, 1.0).astype(float)
        classes   = (np.zeros((len(scores),), dtype=int) if force_sc else classes[keep_idx].astype(int))

        xywh = _xyxy_to_xywh(boxes_pix)
        det_xywh = [
            {"class_id": int(ci), "confidence": float(sc),
             "x": float(cx), "y": float(cy), "w": float(w), "h": float(h)}
            for (cx, cy, w, h), sc, ci in zip(xywh, scores, classes)
        ]
        return {
            'detection_boxes': boxes_pix.tolist(),
            'detection_classes': classes.tolist(),
            'detection_scores': scores.tolist(),
            'num_detections': int(keep_idx.size),
            "detections": det_xywh
        }

    # =========================================
    # PATH 4) Legacy list-of-lists
    # =========================================
    all_boxes, all_scores, all_classes = [], [], []
    raw = detections if isinstance(detections, list) else []
    try:
        for class_id, detection in enumerate(raw or []):
            for det in detection:
                bbox, score = det[:4], float(det[4])
                score = max(0.0, min(1.0, score))
                if score < score_th:
                    continue
                d = denorm_and_unletterbox_xyxy(bbox, net_size=net_size, h0=H, w0=W)
                if d is None:
                    continue
                x1, y1, x2, y2 = d
                if (x2-x1) > 1 and (y2-y1) > 1:
                    all_boxes.append([x1,y1,x2,y2])
                    all_scores.append(score)
                    all_classes.append(class_id)
    except Exception:
        return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}

    if not all_boxes:
        return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}

    boxes   = np.asarray(all_boxes, dtype=float)
    scores  = np.asarray(all_scores, dtype=float)
    classes = np.asarray(all_classes, dtype=int)

    keep_idx = nms_xyxy_agnostic(boxes, scores, iou_th=iou_th, max_det=max_det)
    if keep_idx.size == 0:
        return {'detection_boxes': [], 'detection_classes': [], 'detection_scores': [], 'num_detections': 0}

    boxes   = boxes[keep_idx].round().astype(int)
    scores  = np.clip(scores[keep_idx], 0.0, 1.0).astype(float)
    classes = classes[keep_idx].astype(int)

    return {
        'detection_boxes': boxes.tolist(),
        'detection_classes': classes.tolist(),
        'detection_scores': scores.tolist(),
        'num_detections': int(keep_idx.size)
    }


# -----------------------------
# Drawing
# -----------------------------
def draw_detection(image: np.ndarray, box: list, labels: list, score: float, color: tuple, track=False):
    xmin, ymin, xmax, ymax = map(int, box)
    cv2.rectangle(image, (xmin, ymin), (xmax, ymax), color, 2)
    font = cv2.FONT_HERSHEY_SIMPLEX

    top_text = f"{labels[0]}: {score:.1f}%" if not track or len(labels) == 2 else f"{score:.1f}%"
    bottom_text = None
    if track:
        bottom_text = labels[1] if len(labels) == 2 else labels[0]

    text_color = (255, 255, 255)
    border_color = (0, 0, 0)

    cv2.putText(image, top_text, (xmin + 4, ymin + 20), font, 0.5, border_color, 2, cv2.LINE_AA)
    cv2.putText(image, top_text, (xmin + 4, ymin + 20), font, 0.5, text_color, 1, cv2.LINE_AA)

    if bottom_text:
        pos = (xmax - 50, ymax - 6)
        cv2.putText(image, bottom_text, pos, font, 0.5, border_color, 2, cv2.LINE_AA)
        cv2.putText(image, bottom_text, pos, font, 0.5, text_color, 1, cv2.LINE_AA)


def compute_iou(boxA, boxB):
    xA, yA = max(boxA[0], boxB[0]), max(boxA[1], boxB[1])
    xB, yB = min(boxA[2], boxB[2]), min(boxA[3], boxB[3])
    inter = max(0, xB - xA) * max(0, yB - yA)
    areaA = max(1e-5, (boxA[2] - boxA[0]) * (boxA[3] - boxA[1]))
    areaB = max(1e-5, (boxB[2] - boxB[0]) * (boxB[3] - boxB[1]))
    return inter / (areaA + areaB - inter + 1e-5)


def find_best_matching_detection_index(track_box, detection_boxes):
    best_iou, best_idx = 0, -1
    for i, det_box in enumerate(detection_boxes):
        iou = compute_iou(track_box, det_box)
        if iou > best_iou:
            best_iou = iou; best_idx = i
    return best_idx if best_idx != -1 else None


def draw_detections(detections: dict, img_out: np.ndarray, labels, tracker=None):
    boxes = detections.get("detection_boxes", []) or []
    scores = detections.get("detection_scores", []) or []
    classes = detections.get("detection_classes", []) or []
    num_detections = int(detections.get("num_detections", len(boxes) or 0))

    boxes = np.asarray(boxes, dtype=float)
    scores = np.asarray(scores, dtype=float)
    classes = np.asarray(classes, dtype=int)

    if boxes.ndim != 2 or boxes.shape[0] == 0:
        return img_out

    num_detections = min(num_detections, boxes.shape[0], scores.shape[0], classes.shape[0])

    def safe_label(ci: int) -> str:
        ci = int(ci)
        if ci < 0 or ci >= len(labels):
            ci = 0
        return labels[ci], ci

    if tracker:
        dets_for_tracker = []
        for i in range(num_detections):
            x1, y1, x2, y2 = boxes[i]
            sc = float(np.clip(scores[i], 0.0, 1.0))
            dets_for_tracker.append([x1, y1, x2, y2, sc])
        if not dets_for_tracker:
            return img_out

        online_targets = tracker.update(np.array(dets_for_tracker, dtype=float))
        for track in online_targets:
            x1, y1, x2, y2 = track.tlbr
            best_idx = find_best_matching_detection_index(track.tlbr, boxes[:num_detections])
            if best_idx is None:
                label_text, cid = labels[0], 0
                sc = float(np.clip(track.score, 0.0, 1.0))
            else:
                cid_raw = int(classes[best_idx])
                label_text, cid = safe_label(cid_raw)
                sc = float(np.clip(scores[best_idx], 0.0, 1.0))
            color = tuple(id_to_color(cid).tolist())
            draw_detection(img_out, [x1, y1, x2, y2], [label_text, f"ID {track.track_id}"], sc * 100.0, color, track=True)
        return img_out

    # tracker 없음
    for i in range(num_detections):
        x1, y1, x2, y2 = boxes[i]
        sc = float(np.clip(scores[i], 0.0, 1.0))
        label_text, cid = safe_label(int(classes[i]))
        color = tuple(id_to_color(cid).tolist())
        draw_detection(img_out, [x1, y1, x2, y2], [label_text], sc * 100.0, color)
    return img_out
