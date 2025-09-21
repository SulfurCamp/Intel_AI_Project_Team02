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
from object_detection_post_process import draw_detections
from my_consumer_same_process import on_frame_dict


import argparse
from pathlib import Path

# 큐가 가득 차면 가장 오래된 항목 하나 버리고 새 항목을 넣는 put
def _offer_drop_old(q, item):
    import queue as _q
    try:
        q.put(item, block=False)
    except _q.Full:
        try:
            _ = q.get_nowait()  # 오래된 프레임 하나 버림
        except _q.Empty:
            pass
        try:
            q.put(item, block=False)
        except _q.Full:
            pass

# --- debug flags (safe defaults) ---
try:
    RAW_PRINT_EVERY
except NameError:
    RAW_PRINT_EVERY = 0
try:
    RAW_PROBE
except NameError:
    RAW_PROBE = False

# --- lightweight debug printer ---
def _print_raw(arr, tag="[raw]", every=0, frame_id=None):
    if not every or every <= 0:
        return
    import numpy as np
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
        __LABELS = get_labels(_DEFAULT_LABELS)
        __CONFIG = load_json_file(_DEFAULT_CONFIG)
        __MODEL  = HailoInfer(_DEFAULT_NET, batch_size=1)
    return __MODEL, __LABELS, __CONFIG

def _close_model():
    global __MODEL
    try:
        if __MODEL is not None:
            __MODEL.close()
    finally:
        __MODEL = None

def _pick_nms_stream(outputs_dict: dict):
    """여러 출력 중 NMS 스트림을 우선 선택. 없으면 첫 항목."""
    keys = list(outputs_dict.keys())
    if not keys:
        return None
    nms_keys = [k for k in keys if "nms" in k.lower()]
    key = nms_keys[0] if nms_keys else keys[0]
    return outputs_dict[key]

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
    pre = default_preprocess(img_rgb, inp_w, inp_h)

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
            outputs = {name: np.expand_dims(b.output(name).get_buffer(), axis=0)
                       for name in b._output_names}
            out = _pick_nms_stream(outputs)
        q.put(out)

    model.run([pre], _cb)  # batch=1

    out = q.get()  # 블록 대기
    if isinstance(out, Exception):
        raise out

    # --- raw 수신 여부 간단 체크 ---
    try:
        buf = out
        arr = np.asarray(buf)
        if arr.ndim == 3 and arr.shape[0] == 1:
            arr = arr[0]                    # (1, N, C) → (N, C)
        print(f"[raw] received=YES shape={arr.shape} dtype={arr.dtype}")
    except Exception as e:
        print(f"[raw] received=NO reason={e!r}")
    # --------------------------------

    # 3) 후처리 → 픽셀 좌표의 list[dict] (class_id, accuracy, bbox) 얻기
    dets = inference_result_handler(image_bgr, out, labels, cfg,
                                    tracker=None, return_detection=True)

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

    input_queue = queue.Queue(maxsize=2)
    output_queue = queue.Queue(maxsize=2)

    frame_counter = {"idx": 0}

    post_process_callback_fn = inference_result_handler

    hailo_inference = HailoInfer(net, batch_size)
    height, width, _ = hailo_inference.get_input_shape()


    def viz_callback(frame, _unused, det_dicts=None):
        # det_dicts: inference_callback에서 만든 '친화적 리스트'가 들어옵니다.
        # draw_detections는 postproc dict 형태를 받으므로 변환:
        if isinstance(det_dicts, list):
            boxes, scores, classes = [], [], []
            for d in det_dicts:
                # object_detection_fix13.py의 detections_to_dict() 형식 가정
                x1, y1 = d["start"]["x"], d["start"]["y"]
                x2, y2 = d["end"]["x"],   d["end"]["y"]
                boxes.append([x1, y1, x2, y2])
                scores.append(float(d.get("score", d.get("accuracy", 0.0))))
                classes.append(int(d.get("class_id", 0)))
            det_pp = {
                "detection_boxes": boxes,
                "detection_scores": scores,
                "detection_classes": classes,
                "num_detections": len(boxes),
            }
            return draw_detections(det_pp, frame, labels, tracker=tracker)
        return frame

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
            input_is_norm=True           # 후처리 bbox가 0~1이면 True (postproc에서 픽셀로 변환하므로 False로 넘김)
       )
    )

    preprocess_thread.start()
    postprocess_thread.start()
    infer_thread.start()

    if show_fps:
        fps_tracker.start()

    preprocess_thread.join()
    infer_thread.join()
    _offer_drop_old(output_queue, None)  # Signal process thread to exit (non-blocking)
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
          post_process_callback_fn, frame_counter, labels,
          config_data=None, tracker=None, input_is_norm=True):
    """
    Single-inflight async inference loop:
    - Queue에서 배치를 받음 (None이면 종료)
    - run() 제출
    - 콜백 끝날 때까지 대기 → 한 번에 하나만 돌도록 제어
    - 종료 시 sentinel(None) 한 번만 넣고 닫기
    """
    import threading

    inflight_done = threading.Event()
    inflight_done.set()  # 시작 시 "비어있음"

    def _cb_wrapper(*cb_args, **cb_kwargs):
        try:
            return inference_callback(*cb_args, **cb_kwargs)
        finally:
            inflight_done.set()  # 콜백이 어떤 일이 있어도 해제

    try:
        while True:
            next_batch = input_queue.get()
            if next_batch is None:   # 종료 신호
                break

            input_batch, preprocessed_batch = next_batch

            inference_callback_fn = partial(
                _cb_wrapper,
                input_batch=input_batch,
                output_queue=output_queue,
                post_process_callback_fn=post_process_callback_fn,
                frame_counter=frame_counter,
                labels=labels,
                config_data=config_data,
                tracker=tracker,
                input_is_norm=input_is_norm
            )

            try:
                inflight_done.clear()
                hailo_inference.run(preprocessed_batch, inference_callback_fn)
            except Exception as e:
                logger.exception("infer run() failed: %s", e)
                inflight_done.set()
                continue

            # 콜백 완료까지 대기 → 다음 잡은 그 다음에
            inflight_done.wait()

    finally:
        inflight_done.wait(timeout=2.0)
        try:
            # 시각화 스레드 종료 신호 "한 번만"
            try:
                output_queue.put_nowait(None)
            except Exception:
                pass
        finally:
            try:
                hailo_inference.close()
            except Exception as e:
                logger.warning("hailo_inference.close() warning: %s", e)


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
        return

    for i, bindings in enumerate(bindings_list):
        # 0) 결과 꺼내기 (여러 스트림이면 NMS 스트림만 선택)
        if len(bindings._output_names) == 1:
            result_for_pp = bindings.output().get_buffer()
        else:
            outputs = {
                name: np.expand_dims(bindings.output(name).get_buffer(), axis=0)
                for name in bindings._output_names
            }
            result_for_pp = _pick_nms_stream(outputs)

        frame = input_batch[i]
        H, W = frame.shape[:2]

        # 첫 프레임에 결과 구조 찍기
        if frame_counter["idx"] == 0:
            try:
                a = np.asarray(result_for_pp, dtype=object)
                print("[result] ndim:", getattr(a, "ndim", None),
                      "shape:", getattr(a, "shape", None))
            except Exception as e:
                print("[result] inspect failed:", e)

        # 1) ── 후처리 호출: "검출 리스트"를 직접 반환받기 ───────────────
        #    handler 시그니처: (frame, result, labels, config_data, tracker=None, return_detection=False)
        detections = []
        try:
            detections = post_process_callback_fn(frame, result_for_pp, labels, config_data, tracker, True)
        except TypeError:
            detections = post_process_callback_fn(frame, result_for_pp, labels, config_data, True)
        except Exception as e:
            logger.debug(f"post_process returned error: {e}")
            detections = []

        # 타입 보정: {"detections":[...]} 형태 지원
        if isinstance(detections, dict) and isinstance(detections.get("detections"), list):
            detections = detections["detections"]
        elif not isinstance(detections, list):
            detections = []

        # 2) 프레임 id
        fid = frame_counter["idx"]
        frame_counter["idx"] += 1

        # 3) dict payload 작성 (postproc이 픽셀 좌표를 주므로 False)
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
        _offer_drop_old(output_queue, (frame, None, det_dicts))

        # 첫 페이로드에서 샘플 하나만 출력
        if frame_counter["idx"] == 1:
            print("[debug] sample det:", detections[0] if detections else None)



# ───────────────────────────────────────────────────────────────────
# Single-shot / streaming runner USING ONLY example()
# ───────────────────────────────────────────────────────────────────

def _print_json(dets):
    # example()의 반환(list[dict])을 한 줄 JSON으로 출력
    print(json.dumps(dets, ensure_ascii=False, separators=(",", ":")))

def _maybe_draw(frame_bgr, dets, labels, show=False, win_name="Detections"):
    if not show:
        return
    # draw를 원하면 example() 결과를 postproc dict로 변환해서 그리기
    from object_detection_post_process import draw_detections
    from common.toolbox import id_to_color  # 이미 의존 있음

    boxes, scores, classes = [], [], []
    for d in dets:
        x1, y1 = d["start"]["x"], d["start"]["y"]
        x2, y2 = d["end"]["x"],   d["end"]["y"]
        boxes.append([x1, y1, x2, y2])
        scores.append(float(d.get("accuracy", d.get("score", 0.0))))
        # 단일 클래스이지만 혹시 class_id가 들어오면 반영
        classes.append(int(d.get("class_id", 0)))

    det_pp = {
        "detection_boxes": boxes,
        "detection_scores": scores,
        "detection_classes": classes,
        "num_detections": len(boxes),
    }
    out = draw_detections(det_pp, frame_bgr.copy(), labels, tracker=None)
    cv2.imshow(win_name, out)

def _is_image(path: str) -> bool:
    ext = os.path.splitext(path)[1].lower()
    return ext in [".jpg", ".jpeg", ".png", ".bmp", ".tif", ".tiff"]

def _is_video(path: str) -> bool:
    ext = os.path.splitext(path)[1].lower()
    return ext in [".mp4", ".avi", ".mov", ".mkv", ".m4v"]

def _ensure_defaults_from_args(args):
    """
    example()은 모듈 전역의 _DEFAULT_* 를 읽어 초기화하므로,
    스크립트 인자로 받은 경로를 전역 기본값에 주입한다.
    """
    global _DEFAULT_NET, _DEFAULT_LABELS, _DEFAULT_CONFIG, __MODEL
    _DEFAULT_NET    = args.net
    _DEFAULT_LABELS = args.labels
    # config는 parse_args에서 기본값이 파일 옆 'config.json'이므로 그대로 둠
    # 만약 별도 경로를 쓰고 싶으면 환경변수 HAILO_CONFIG로 넘겨도 됨.
    __MODEL = None  # 첫 호출 시 안전 재초기화

def main() -> None:
    """
    Single 실행(이미지 1장) 또는 스트리밍(카메라/비디오) 모드.
    추론은 오직 example(image_bgr) 로만 수행한다.
    출력은 한 프레임당 한 줄 JSON.
    옵션으로 미리보기도 가능(--show-fps 또는 --save-stream-output 사용 시).
    """
    args = parse_args()

    if args.camera_preview:
        # 기존 프리뷰 기능은 그대로 유지
        run_camera_preview(resolution=args.resolution, camera_index=args.camera_index)
        return

    # example()이 읽을 기본 경로 주입
    _ensure_defaults_from_args(args)

    # 라벨 로딩 (draw 옵션을 위해)
    labels = get_labels(args.labels)

    show = (args.show_fps or args.save_stream_output)

    # 1) 카메라
    if args.input.lower() == "camera":
        cam_idx = args.camera_index if args.camera_index is not None else int(os.environ.get("CAMERA_INDEX", "0"))
        cap = cv2.VideoCapture(cam_idx, cv2.CAP_V4L2)
        if not cap.isOpened():
            raise RuntimeError(f"카메라를 열 수 없습니다: /dev/video{cam_idx}")

        # 해상도 설정(간단 버전)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        cap.set(cv2.CAP_PROP_FPS, 30)

        try:
            while True:
                ok, frame = cap.read()
                if not ok:
                    break
                dets = example(frame)      # 🔹 오직 example()만 호출
                _print_json(dets)
                _maybe_draw(frame, dets, labels, show=show)
                if show and (cv2.waitKey(1) & 0xFF == ord('q')):
                    break
        finally:
            cap.release()
            if show:
                cv2.destroyAllWindows()
        return

    # 2) 이미지 파일/폴더
    if os.path.isdir(args.input):
        imgs = []
        for p in sorted(os.listdir(args.input)):
            fp = os.path.join(args.input, p)
            if os.path.isfile(fp) and _is_image(fp):
                imgs.append(fp)
        if not imgs:
            raise FileNotFoundError(f"이미지 폴더에 유효한 파일이 없습니다: {args.input}")

        for fp in imgs:
            img = cv2.imread(fp)
            if img is None:
                continue
            dets = example(img)           # 🔹 example()만 호출
            _print_json(dets)
            _maybe_draw(img, dets, labels, show=show)
            if show and (cv2.waitKey(1) & 0xFF == ord('q')):
                break
        if show:
            cv2.destroyAllWindows()
        return

    # 3) 단일 이미지
    if _is_image(args.input):
        img = cv2.imread(args.input)
        if img is None:
            raise FileNotFoundError(f"이미지를 읽을 수 없습니다: {args.input}")
        dets = example(img)               # 🔹 example()만 호출
        _print_json(dets)
        _maybe_draw(img, dets, labels, show=show)
        if show:
            cv2.waitKey(0)
            cv2.destroyAllWindows()
        return

    # 4) 비디오 파일
    if _is_video(args.input):
        cap = cv2.VideoCapture(args.input)
        if not cap.isOpened():
            raise FileNotFoundError(f"비디오를 열 수 없습니다: {args.input}")
        try:
            while True:
                ok, frame = cap.read()
                if not ok:
                    break
                dets = example(frame)      # 🔹 example()만 호출
                _print_json(dets)
                _maybe_draw(frame, dets, labels, show=show)
                if show and (cv2.waitKey(1) & 0xFF == ord('q')):
                    break
        finally:
            cap.release()
            if show:
                cv2.destroyAllWindows()
        return

    raise ValueError(f"--input 인자가 지원되지 않습니다: {args.input!r}")


if __name__ == "__main__":
    main()

