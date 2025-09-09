print("[USING CUSTOM SCRIPT]", __file__)
#!/usr/bin/env python3
import argparse
import os
import sys
from loguru import logger
import queue
import threading
from functools import partial
from types import SimpleNamespace
import numpy as np

import json
import cv2
import time

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from common.tracker.byte_tracker import BYTETracker
from common.hailo_inference import HailoInfer
from common.toolbox import init_input_source, get_labels, load_json_file, preprocess, visualize, FrameRateTracker
from object_detection_post_process import inference_result_handler
from my_consumer_same_process import on_frame_dict


import argparse
from pathlib import Path

try:
    # 너의 유틸에 이 이름이 있다면 그대로 사용
    from toolbox import default_preprocess, get_labels, load_json_file
except Exception:
    try:
        # 혹시 이름이 다르면 여기서 대체 (예: preprocess_image)
        from toolbox import preprocess_image as default_preprocess
        from toolbox import get_labels, load_json_file
    except Exception:
        # toolbox_4가 없거나 함수명이 다르면 common.toolbox에서 라벨/설정만 가져오고,
        # 전처리는 아래의 fallback으로 처리
        from common.toolbox import get_labels, load_json_file

        def default_preprocess(img_rgb, dst_w, dst_h):
            """
            Fallback: 모델 입력 크기에 맞춰 resize만 수행.
            - 입력: RGB(HWC, uint8)
            - 출력: (dst_h, dst_w, 3) RGB, uint8, contiguous
            """
            out = cv2.resize(img_rgb, (dst_w, dst_h), interpolation=cv2.INTER_LINEAR)
            return np.ascontiguousarray(out, dtype=np.uint8)

# 기본 경로(원하면 환경변수로 덮어쓰기 가능)
_DEFAULT_NET    = os.environ.get("HAILO_HEF", os.path.expanduser("~/hailo-8/hailo-8/best.hef"))
_DEFAULT_LABELS = os.environ.get("HAILO_LABELS", os.path.expanduser("~/hailo-8/hailo-8/drone.txt"))
# config_4.json을 쓴다면 파일명 맞춰주세요. (없으면 config.json로 바꿔도 됨)
_DEFAULT_CONFIG = os.environ.get("HAILO_CONFIG", str(Path(__file__).with_name("config.json")))

# 내부 싱글톤
__MODEL  = None
__LABELS = None
__CONFIG = None

def _ensure_model():
    """모델/라벨/설정을 1회만 로드."""
    global __MODEL, __LABELS, __CONFIG
    if __MODEL is None:
        __LABELS = get_labels(_DEFAULT_LABELS)                          # :contentReference[oaicite:5]{index=5}
        __CONFIG = load_json_file(_DEFAULT_CONFIG)                       # :contentReference[oaicite:6]{index=6}
        __MODEL  = HailoInfer(_DEFAULT_NET, batch_size=1)
    return __MODEL, __LABELS, __CONFIG

def _close_model():
    global __MODEL
    try:
        if __MODEL is not None:
            __MODEL.close()
    finally:
        __MODEL = None

def example(image_bgr):
    """
    외부 코드에서 import하여 바로 쓰는 단일 함수.
    입력: OpenCV BGR 이미지 한 장 (np.ndarray).
    반환: list[dict] — object_label, start/end, accuracy 포함.
    """
    model, labels, cfg = _ensure_model()

    # 1) 전처리
    h, w, _ = image_bgr.shape
    inp_h, inp_w, _ = model.get_input_shape()  # (H, W, C)
    img_rgb = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2RGB)
    pre = default_preprocess(img_rgb, inp_w, inp_h)                      # :contentReference[oaicite:7]{index=7}

    # 2) 추론 (콜백으로 결과 수집)
    q = queue.Queue()

    def _cb(completion_info, bindings_list):
        if completion_info.exception:
            q.put(completion_info.exception)
            return
        # outputs → (단일 배치 가정) 첫 결과만 꺼냄
        b = bindings_list[0]
        if len(b._output_names) == 1:
            out = b.output().get_buffer()
        else:
            out = {name: np.expand_dims(b.output(name).get_buffer(), axis=0)
                   for name in b._output_names}
        q.put(out)

    model.run([pre], _cb)  # batch=1

    out = q.get()  # 블록 대기
    if isinstance(out, Exception):
        raise out

    # 3) 후처리 → 픽셀 좌표의 list[dict] (class_id, accuracy, bbox) 얻기
    dets = inference_result_handler(image_bgr, out, labels, cfg,
                                    tracker=None, return_detection=True)  # :contentReference[oaicite:8]{index=8}

    # 4) 외부에서 쓰기 쉬운 형식으로 변환
    result = []
    for d in dets:
        cid = int(d.get("class_id", 0))
        lbl = labels[cid] if 0 <= cid < len(labels) else str(cid)
        b   = d["bbox"]
        result.append({
            "object_label": lbl,
            "start": {"x": int(b["x1"]), "y": int(b["y1"])},
            "end":   {"x": int(b["x2"]), "y": int(b["y2"])},
            "accuracy": float(d.get("accuracy", d.get("score", 0.0)))
        })
    return result
# ===== end of public API =====

def _autoparse_detections(result, labels, W, H, cls_thresh=1e-5):
    """
    다양한 YOLO/Hailo 결과를 탐지 리스트로 변환.
    지원:
      - dict: boxes/scores/labels | classes 조합
      - dict: (여러 스케일) head별 cls/box/obj 분리된 경우도 합침
      - ndarray: (N,85)/(N,84) 또는 (N,6)/(N,7)
    반환: list[{"class_id","score","bbox":{"x1","y1","x2","y2"}}]
    """
    dets = []

    def _to2d(val):
        # ragged 안전 변환
        try:
            a = np.asarray(val)
        except Exception:
            return None
        if getattr(a, "dtype", None) == object:
            try:
                a = np.stack([np.asarray(x) for x in val])
            except Exception:
                return None
        if getattr(a, "ndim", 0) < 2:
            return None
        try:
            return a.reshape(-1, a.shape[-1])
        except Exception:
            return None

    def _add(ci, sc, box):
        x1, y1, x2, y2 = map(float, box)
        norm_like = (0.0 <= min(x1, y1, x2, y2) <= 1.2) and (max(x1, y1, x2, y2) <= 1.2)
        if norm_like:
            x1, y1, x2, y2 = x1*W, y1*H, x2*W, y2*H
        x1 = int(np.clip(round(x1), 0, W-1))
        y1 = int(np.clip(round(y1), 0, H-1))
        x2 = int(np.clip(round(x2), 0, W-1))
        y2 = int(np.clip(round(y2), 0, H-1))
        if x2 - x1 > 1 and y2 - y1 > 1 and sc >= cls_thresh:
            dets.append({
                "class_id": int(ci),
                "score": float(sc),
                "bbox": {"x1": x1, "y1": y1, "x2": x2, "y2": y2}
            })

    # ---------------- dict 형태 ----------------
    if isinstance(result, dict):
        # 키 이름 전처리
        low = {k.lower(): k for k in result.keys()}

        # 1) boxes + (scores) + (labels|classes)
        if any(k in low for k in ["boxes", "bboxes", "bbox", "detections"]):
            bkey = low.get("boxes") or low.get("bboxes") or low.get("bbox") or low.get("detections")
            boxes = _to2d(result[bkey])
            if boxes is None:
                try:
                    boxes = np.asarray(result[bkey], dtype=float).reshape(-1, 4)
                except Exception:
                    boxes = None
            if boxes is not None:
                scores = None; clsids = None
                if "scores" in low:  scores = np.asarray(result[low["scores"]]).reshape(-1)
                if "labels" in low:  clsids = np.asarray(result[low["labels"]]).reshape(-1)
                if "classes" in low:
                    cls_mat = np.asarray(result[low["classes"]])
                    cls_mat = cls_mat.reshape(-1, cls_mat.shape[-1])
                    tmp_scores = np.max(cls_mat, axis=-1)
                    tmp_clsids = np.argmax(cls_mat, axis=-1)
                    if scores is None: scores = tmp_scores
                    if clsids is None: clsids = tmp_clsids
                # boxes가 (N,6/7) 형태면 잘라내기
                if boxes.shape[-1] >= 6 and (scores is None or clsids is None):
                    if scores is None: scores = boxes[:, 4]
                    if clsids is None: clsids = boxes[:, 5]
                    boxes = boxes[:, :4]
                if scores is None: scores = np.ones((boxes.shape[0],), dtype=float)
                if clsids is None: clsids = np.zeros((boxes.shape[0],), dtype=int)
                n = min(len(boxes), len(scores), len(clsids))
                for b, sc, ci in zip(boxes[:n], scores[:n], clsids[:n]):
                    _add(ci, sc, b)
                return dets

        # 2) head가 분리되어 있는 경우(여러 스케일)
        #    후보 모으기: 마지막 축이 4인 것(박스), 80/84/85인 것(클래스), 1/5인 것(obj)
        box_cands = []
        cls_cands = []
        obj_cands = []
        for k, v in result.items():
            a = _to2d(v)
            if a is None: 
                continue
            F = a.shape[-1]
            lk = k.lower()
            if F == 4 or any(t in lk for t in ["box", "bbox", "reg"]):
                box_cands.append(a)
            elif F in (80, 84, 85) or any(t in lk for t in ["cls", "class"]):
                cls_cands.append(a)
            elif F in (1, 5) or "obj" in lk or "object" in lk:
                obj_cands.append(a)

        # 스케일별로 길이를 맞춰 가장 긴 것에 맞춰 결합
        if box_cands and cls_cands:
            boxes = np.concatenate(box_cands, axis=0)
            cls_mat = np.concatenate(cls_cands, axis=0)
            m = min(len(boxes), len(cls_mat))
            boxes = boxes[:m]; cls_mat = cls_mat[:m]
            if cls_mat.shape[-1] in (84, 85):
                # YOLOv8: xywh + obj + 80 class 형태로도 들어오는 모델 대응
                if cls_mat.shape[-1] == 85 and boxes.shape[-1] != 4:
                    # boxes가 xywh이면 xyxy로 변환할 때 같이 처리하므로 여기선 보류
                    pass
            # cls 점수/클래스
            if cls_mat.shape[-1] >= 2:
                clsids = np.argmax(cls_mat, axis=-1)
                scores = np.max(cls_mat, axis=-1)
            else:
                clsids = np.zeros((m,), dtype=int)
                scores = cls_mat.reshape(-1)

            # obj가 있으면 곱하기
            if obj_cands:
                obj = np.concatenate(obj_cands, axis=0).reshape(-1)[:m]
                scores = scores * obj

            # boxes가 xywh이면 xyxy로 변환
            if boxes.shape[-1] == 4:
                # 추정: 값 범위로 xywh/xyxy 판단 어려우니 두 경우 모두 시도
                #  (xywh일 때가 더 흔하므로 우선 xywh→xyxy 변환)
                x, y, w, h = boxes.T
                xyxy = np.stack([x - w/2, y - h/2, x + w/2, y + h/2], axis=1)
            else:
                # 마지막 축이 6/7 등인 케이스는 앞 4개를 좌표로 가정
                xyxy = boxes[:, :4]

            for b, sc, ci in zip(xyxy, scores, clsids):
                _add(ci, sc, b)
            return dets

    # ---------------- ndarray 형태 ----------------
    a = _to2d(result)
    if a is not None:
        F = a.shape[-1]
        if F in (85, 84):
            xywh = a[:, :4]
            has_obj = (F == 85)
            obj = a[:, 4] if has_obj else 1.0
            cls_mat = a[:, 5:] if has_obj else a[:, 4:]
            clsids = np.argmax(cls_mat, axis=-1)
            scores = (obj * np.max(cls_mat, axis=-1)) if has_obj else np.max(cls_mat, axis=-1)
            x, y, w, h = xywh.T
            xyxy = np.stack([x - w/2, y - h/2, x + w/2, y + h/2], axis=1)
            for b, sc, ci in zip(xyxy, scores, clsids):
                _add(ci, sc, b)
            return dets
        if F in (6, 7):
            xyxy = a[:, :4]
            scores = a[:, 4]
            clsids = a[:, 5].astype(int)
            for b, sc, ci in zip(xyxy, scores, clsids):
                _add(ci, sc, b)
            return dets

    return dets

def parse_args() -> argparse.Namespace:
    """
    Parse command-line arguments for the detection application.

    Returns:
        argparse.Namespace: Parsed CLI arguments.
    """
    parser = argparse.ArgumentParser(description="Run object detection with optional tracking and performance measurement.")

    parser.add_argument(
        "-n", "--net",
        type=str,
        default="yolov8n.hef",
        help="Path to the network in HEF format."
    )

    parser.add_argument(
        "-i", "--input",
        type=str,
        default="bus.jpg",
        help="Path to the input (image, video, or folder)."
    )

    parser.add_argument(
        "-b", "--batch_size",
        type=int,
        default=1,
        help="Number of images per batch."
    )

    parser.add_argument(
        "-l", "--labels",
        type=str,
        default=str(Path(__file__).parent.parent / "common" / "coco.txt"),
        help="Path to label file (e.g., coco.txt). If not set, default COCO labels will be used."
    )

    parser.add_argument(
        "-s", "--save_stream_output",
        action="store_true",
        help="Save the visualized stream output to disk."
    )

    parser.add_argument(
        "-o", "--output-dir",
        type=str,
        default=None,
        help="Directory to save result images or video."
    )

    parser.add_argument(
        "-r", "--resolution",
        type=str,
        choices=["sd", "hd", "fhd"],
        default="sd",
        help="(Camera only) Input resolution: 'sd' (640x480), 'hd' (1280x720), or 'fhd' (1920x1080)."
    )

    parser.add_argument(
        "--track",
        action="store_true",
        help="Enable object tracking across frames."
    )

    parser.add_argument(
        "--show-fps",
        action="store_true",
        help="Enable FPS measurement and display."
    )

    parser.add_argument(
        "--camera-preview",
        action="store_true",
        help="OpenCV 카메라 미리보기 전용 모드(추론 없이 화면 출력)."
    )

    parser.add_argument(
        "--camera-index",
        type=int,
        default=None,
        help="카메라 인덱스 강제 지정(미지정 시 환경변수 CAMERA_INDEX 또는 0)."
    ) 

    args = parser.parse_args()

    # Validate paths
    if not os.path.exists(args.net):
        raise FileNotFoundError(f"Network file not found: {args.net}")
    if not os.path.exists(args.labels):
        raise FileNotFoundError(f"Labels file not found: {args.labels}")

    if args.output_dir is None:
        args.output_dir = os.path.join(os.getcwd(), "output")
    os.makedirs(args.output_dir, exist_ok=True)

    return args


def run_inference_pipeline(net, input, batch_size, labels, output_dir,
          save_stream_output=False, resolution="sd",
          enable_tracking=False, show_fps=False) -> None:
    """
    Initialize queues, HailoAsyncInference instance, and run the inference.
    """
    labels = get_labels(labels)
    # config_data = load_json_file("config.json")
    CONFIG_PATH = Path(__file__).parent / "config.json"
    config_data = load_json_file(str(CONFIG_PATH))

    # Initialize input source from string: "camera", video file, or image folder.
    cap, images = init_input_source(input, batch_size, resolution)
    tracker = None
    fps_tracker = None
    if show_fps:
        fps_tracker = FrameRateTracker()

    if enable_tracking:
        # load tracker config from config_data
        tracker_config = config_data.get("visualization_params", {}).get("tracker", {})
        tracker = BYTETracker(SimpleNamespace(**tracker_config))

    input_queue = queue.Queue()
    output_queue = queue.Queue()

    frame_counter = {"idx": 0}

    post_process_callback_fn = inference_result_handler

    hailo_inference = HailoInfer(net, batch_size)
    height, width, _ = hailo_inference.get_input_shape()

    viz_callback = lambda frame, result, *rest: inference_result_handler(
        frame, result, labels, config_data, tracker
    )

    preprocess_thread = threading.Thread(
        target=preprocess, args=(images, cap, batch_size, input_queue, width, height)
    )
    postprocess_thread = threading.Thread(
        target=visualize, args=(output_queue, cap, save_stream_output,
                                output_dir, viz_callback, fps_tracker)
    )
    # infer_thread 생성부
    infer_thread = threading.Thread(
        target=infer, args=(hailo_inference, input_queue, output_queue),
        kwargs=dict(
            post_process_callback_fn=post_process_callback_fn,
            frame_counter=frame_counter,
            labels=labels,
            config_data=config_data,     # ★ 추가
            tracker=tracker,             # ★ 추가(없으면 None도 OK)
            input_is_norm=True           # 후처리 bbox가 0~1이면 True
       )
    )

    preprocess_thread.start()
    postprocess_thread.start()
    infer_thread.start()

    if show_fps:
        fps_tracker.start()

    preprocess_thread.join()
    infer_thread.join()
    output_queue.put(None)  # Signal process thread to exit
    postprocess_thread.join()

    if show_fps:
        logger.debug(fps_tracker.frame_rate_summary())

    logger.info('Inference was successful!')

# ── OpenCV 카메라 프리뷰(추론 없이 화면만 띄우기) ─────────────────────────────
def _resolve_camera_index(idx_arg):
    if idx_arg is not None:
        return int(idx_arg)
    env = os.environ.get("CAMERA_INDEX", "0")
    try:
        return int(env)
    except Exception:
        return 0

def _set_cam_resolution(cap, resolution: str):
    # sd: 640x480, hd: 1280x720, fhd: 1920x1080
    sizes = {"sd": (640, 480), "hd": (1280, 720), "fhd": (1920, 1080)}
    w, h = sizes.get(resolution, (640, 480))
    # V4L2 권장
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,  w)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, h)
    return w, h

def run_camera_preview(resolution="sd", camera_index=None):
    cam_idx = _resolve_camera_index(camera_index)
    # CAP_V4L2로 열기
    cap = cv2.VideoCapture(cam_idx, cv2.CAP_V4L2)
    if not cap.isOpened():
        raise RuntimeError(f"카메라를 열 수 없습니다: /dev/video{cam_idx}")

    w, h = _set_cam_resolution(cap, resolution)
    cv2.namedWindow("RPI5 Camera", cv2.WINDOW_NORMAL)

    # 간단 FPS
    t0, frames = time.time(), 0
    try:
        while True:
            ok, frame = cap.read()
            if not ok:
                print("[Camera] 프레임을 읽지 못했습니다."); break
            frames += 1
            # FPS 표시
            dt = time.time() - t0
            fps = frames / dt if dt > 0 else 0.0
            cv2.putText(frame, f"{w}x{h}  FPS:{fps:5.1f}",
                        (10, 24), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255,255,255), 2, cv2.LINE_AA)
            cv2.imshow("RPI5 Camera", frame)
            # q 키로 종료
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        cap.release()
        cv2.destroyAllWindows()
# ──────────────────────────────────────────────────────────────────────────────


def infer(hailo_inference, input_queue, output_queue, *,
          post_process_callback_fn, frame_counter, labels, config_data=None, tracker=None, input_is_norm=True):
    """
    Main inference loop that pulls data from the input queue, runs asynchronous
    inference, and pushes results to the output queue.

    Each item in the input queue is expected to be a tuple:
        (input_batch, preprocessed_batch)
        - input_batch: Original frames (used for visualization or tracking)
        - preprocessed_batch: Model-ready frames (e.g., resized, normalized)

    Args:
        hailo_inference (HailoInfer): The inference engine to run model predictions.
        input_queue (queue.Queue): Provides (input_batch, preprocessed_batch) tuples.
        output_queue (queue.Queue): Collects (input_frame, result) tuples for visualization.

    Returns:
        None
    """
    while True:
        next_batch = input_queue.get()
        if not next_batch:
            break  # Stop signal received

        input_batch, preprocessed_batch = next_batch

        # Prepare the callback for handling the inference result
        inference_callback_fn = partial(
            inference_callback,
            input_batch=input_batch,
            output_queue=output_queue,
            post_process_callback_fn=post_process_callback_fn,
            frame_counter=frame_counter,
            labels=labels,  # 추가
            config_data=config_data,
            tracker=tracker,
            input_is_norm=input_is_norm
        )

        # Run async inference
        try:
            hailo_inference.run(preprocessed_batch, inference_callback_fn)
        except Exception as e:
            logger.exception("infer failed: %s", e)
            output_queue.put(None)
            break 

    # Release resources and context
    hailo_inference.close()

def _to_pixel_xyxy(box, W, H, normalized=True):
    """
    다양한 bbox 포맷을 안전하게 픽셀 좌표 (x1,y1,x2,y2)로 변환.
    - dict: {x,y,w,h} 또는 {x1,y1,x2,y2} / {x_min,y_min,x_max,y_max}
    - list/tuple: [x,y,w,h] 또는 [x1,y1,x2,y2]
    normalized=True 이면 0~1 → 픽셀로 스케일링
    """
    def clamp(v, lo, hi): return max(lo, min(hi, v))

    x1=y1=x2=y2=None

    if isinstance(box, dict):
        if all(k in box for k in ("x","y","w","h")):
            x1, y1, w, h = box["x"], box["y"], box["w"], box["h"]
            if normalized:
                x1, y1, w, h = x1*W, y1*H, w*W, h*H
            x2, y2 = x1 + w, y1 + h
        elif all(k in box for k in ("x1","y1","x2","y2")):
            x1, y1, x2, y2 = box["x1"], box["y1"], box["x2"], box["y2"]
            if normalized:
                x1, y1, x2, y2 = x1*W, y1*H, x2*W, y2*H
        elif all(k in box for k in ("x_min","y_min","x_max","y_max")):
            x1, y1, x2, y2 = box["x_min"], box["y_min"], box["x_max"], box["y_max"]
            if normalized:
                x1, y1, x2, y2 = x1*W, y1*H, x2*W, y2*H
    elif isinstance(box, (list, tuple)) and len(box) == 4:
        a,b,c,d = box
        # heuristics: 값이 1.0 이하이면 정규화된 것으로 가정
        guess_norm = normalized if normalized is not None else (max(a,b,c,d) <= 1.0)
        if guess_norm:
            # [x,y,w,h] 또는 [x1,y1,x2,y2] 둘 다 안전 처리
            if c <= 1.0 and d <= 1.0:  # [x,y,w,h] normalized
                x1,y1,w,h = a*W, b*H, c*W, d*H
                x2,y2 = x1+w, y1+h
            else:                       # [x1,y1,x2,y2] normalized (희박)
                x1,y1,x2,y2 = a*W, b*H, c*W, d*H
        else:
            # 픽셀 스페이스
            if c < W and d < H:        # [x,y,w,h] pixels
                x1,y1,w,h = a,b,c,d
                x2,y2 = x1+w, y1+h
            else:                      # [x1,y1,x2,y2] pixels
                x1,y1,x2,y2 = a,b,c,d

    if None in (x1,y1,x2,y2):
        # 포맷을 못 알아먹었으면 안전하게 0,0,0,0
        x1=y1=x2=y2=0

    x1 = int(round(clamp(x1, 0, W-1)))
    y1 = int(round(clamp(y1, 0, H-1)))
    x2 = int(round(clamp(x2, 0, W-1)))
    y2 = int(round(clamp(y2, 0, H-1)))
    # 정규화/폭/높이 환산 오류 방지
    x1, x2 = min(x1,x2), max(x1,x2)
    y1, y2 = min(y1,y2), max(y1,y2)
    return x1,y1,x2,y2


def detections_to_dict(detections, labels, img_w, img_h, input_is_norm=True):
    js = []
    for det in (detections or []):
        cid = int(det.get("class_id", -1)) if isinstance(det, dict) else -1
        label = labels[cid] if 0 <= cid < len(labels) else f"class_{cid}"
        score = float(det.get("score", 0.0)) if isinstance(det, dict) else 0.0

        # bbox 후보 찾기
        bbox = None
        if isinstance(det, dict):
            if "bbox" in det: bbox = det["bbox"]
            elif "box" in det: bbox = det["box"]
            elif all(k in det for k in ("x","y","w","h")): bbox = {"x":det["x"],"y":det["y"],"w":det["w"],"h":det["h"]}
            elif all(k in det for k in ("x1","y1","x2","y2")): bbox = {"x1":det["x1"],"y1":det["y1"],"x2":det["x2"],"y2":det["y2"]}
            elif all(k in det for k in ("x_min","y_min","x_max","y_max")): bbox = {"x_min":det["x_min"],"y_min":det["y_min"],"x_max":det["x_max"],"y_max":det["y_max"]}

        x1,y1,x2,y2 = _to_pixel_xyxy(bbox or [0,0,0,0], img_w, img_h, normalized=input_is_norm)

        js.append({
            "object_label": label,
            "score": round(score, 3),
            "start": {"x": x1, "y": y1},
            "end":   {"x": x2, "y": y2},
        })
    return js

def inference_callback(
    completion_info,
    bindings_list: list,
    input_batch: list,
    output_queue: queue.Queue,
    *,
    post_process_callback_fn,   # 그대로
    frame_counter,              # 그대로
    labels,                     # 그대로
    config_data=None,
    tracker=None,
    input_is_norm=True          # 그대로
) -> None:
    if completion_info.exception:
        logger.error(f'Inference error: {completion_info.exception}')
        output_queue.put(None)  # 시각화 스레드가 멈추지 않도록
        return

    for i, bindings in enumerate(bindings_list):
        # 0) 결과 꺼내기
        if len(bindings._output_names) == 1:
            result = bindings.output().get_buffer()
        else:
            result = {
                name: np.expand_dims(bindings.output(name).get_buffer(), axis=0)
                for name in bindings._output_names
            }

        frame = input_batch[i]
        H, W = frame.shape[:2]

        # 첫 프레임에 결과 구조 찍기
        if frame_counter["idx"] == 0:
            import numpy as np
            try:
                if isinstance(result, dict):
                    print("[result] keys:", list(result.keys()))
                    for k, v in list(result.items())[:3]:
                        a = np.asarray(v, dtype=object)
                        print("  ", k, "ndim:", getattr(a, "ndim", None),
                              "shape:", getattr(a, "shape", None))
                else:
                    a = np.asarray(result, dtype=object)
                    print("[result] ndim:", getattr(a, "ndim", None),
                          "shape:", getattr(a, "shape", None))
            except Exception as e:
                print("[result] inspect failed:", e)

        # 1) ── 후처리 호출: "검출 리스트"를 직접 반환받기 ───────────────
        #    handler 시그니처: (frame, result, labels, config_data, tracker=None, return_detection=False)
        detections = []
        try:
            # tracker까지 받는 구현
            detections = post_process_callback_fn(frame, result, labels, config_data, tracker, True)
        except TypeError:
            # tracker 인자를 안 받는 구현
            detections = post_process_callback_fn(frame, result, labels, config_data, True)
        except Exception as e:
            logger.debug(f"post_process returned error: {e}")
            detections = []

        # 타입 보정: {"detections":[...]} 형태 지원
        if isinstance(detections, dict) and isinstance(detections.get("detections"), list):
            detections = detections["detections"]
        elif not isinstance(detections, list):
            detections = []

        # 리스트가 비면 결과 텐서에서 자동 추출(폴백)
        if not detections:
            try:
                detections = _autoparse_detections(result, labels, W, H)
            except Exception as e:
                logger.debug(f"autoparse failed: {e}")
                detections = []
        # ────────────────────────────────────────────────────────────────

        # 2) 프레임 id
        fid = frame_counter["idx"]
        frame_counter["idx"] += 1

        # 3) dict payload 작성 (핸들러가 픽셀 좌표를 주므로 False)
        det_dicts = detections_to_dict(detections, labels, W, H, input_is_norm=False)
        payload = {
            "frame_id": fid,
            "image_size": {"w": W, "h": H},
            "detections": det_dicts,
        }

        # 4) dict 그대로 콜백
        try:
            on_frame_dict(payload)
        except Exception as e:
            logger.error(f"on_frame_dict failed: {e}")

        # 5) 시각화 스레드로 프레임 넘기기
        output_queue.put((frame, result))

        # 첫 페이로드에서 샘플 하나만 출력
        if frame_counter["idx"] == 1:
            print("[debug] sample det:", detections[0] if detections else None)



def main() -> None:
    """
    Main function to run the script.
    """
    args = parse_args()
    if args.camera_preview:
        run_camera_preview(resolution=args.resolution, camera_index=args.camera_index)
        return
    run_inference_pipeline(args.net, args.input, args.batch_size, args.labels,
          args.output_dir, args.save_stream_output, args.resolution,
          args.track, args.show_fps)




if __name__ == "__main__":
    main()

