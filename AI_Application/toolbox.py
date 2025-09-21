from typing import List, Generator, Optional, Tuple, Dict, Callable, Any
from pathlib import Path
from loguru import logger
import json
import os
import sys
import numpy as np
import queue
import cv2
import time

IMAGE_EXTENSIONS: Tuple[str, ...] = (".jpg", ".png", ".bmp", ".jpeg")
CAMERA_RESOLUTION_MAP = {
    "sd": (640, 480),
    "hd": (1280, 720),
    "fhd": (1920, 1080),
}
# Allow override via env; fallback to 0
CAMERA_INDEX = int(os.getenv("CAMERA_INDEX", "0"))


# ──────────────────────────────────────────────────────────────────────────────
# Small utilities
# ──────────────────────────────────────────────────────────────────────────────
class SimpleFPS:
    def __init__(self, alpha=0.9):
        self.alpha = alpha
        self._fps = None
        self._t_last = None

    def tick(self):
        t = time.perf_counter()
        if self._t_last is not None:
            inst = 1.0 / max(t - self._t_last, 1e-9)
            self._fps = inst if self._fps is None else (self.alpha * self._fps + (1 - self.alpha) * inst)
        self._t_last = t

    def value(self):
        return 0.0 if self._fps is None else self._fps

    def draw(self, frame, xy=(10, 24)):
        if self._fps is not None:
            cv2.putText(frame, f"FPS: {self._fps:5.1f}", xy, cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2, cv2.LINE_AA)


def load_json_file(path: str) -> Dict[str, Any]:
    if not os.path.isfile(path):
        raise FileNotFoundError(f"File not found: {path}")
    with open(path, "r", encoding="utf-8") as f:
        try:
            return json.load(f)
        except json.JSONDecodeError as e:
            raise json.JSONDecodeError(f"Invalid JSON format in '{path}': {e.msg}", e.doc, e.pos)


def is_valid_camera_index(index: int) -> bool:
    cap = cv2.VideoCapture(index, cv2.CAP_V4L2)
    ok = cap.isOpened()
    cap.release()
    return ok


def list_available_cameras(max_index=5):
    available = []
    for i in range(max_index + 1):
        cap = cv2.VideoCapture(i, cv2.CAP_V4L2)
        if cap.isOpened():
            available.append(i)
        cap.release()
    return available


# ──────────────────────────────────────────────────────────────────────────────
# Input source init (camera / video / images)
# ──────────────────────────────────────────────────────────────────────────────

def _get_camera_native_resolution() -> Tuple[int, int]:
    tmp = cv2.VideoCapture(CAMERA_INDEX, cv2.CAP_V4L2)
    if not tmp.isOpened():
        return (640, 480)
    tmp.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*"YUYV"))
    w = int(tmp.get(cv2.CAP_PROP_FRAME_WIDTH) or 640)
    h = int(tmp.get(cv2.CAP_PROP_FRAME_HEIGHT) or 480)
    tmp.release()
    return (w, h)


def init_input_source(input_path: str, batch_size: int, resolution: Optional[str]):
    """Return (cap, images). One of them is None.
    - cap: cv2.VideoCapture opened with V4L2 + YUYV (low latency)
    - images: list of BGR np.ndarray loaded by OpenCV
    """
    cap = None
    images = None

    if input_path == "camera":
        if not is_valid_camera_index(CAMERA_INDEX):
            logger.error(f"CAMERA_INDEX {CAMERA_INDEX} not found.")
            logger.warning(f"Available camera indices: {list_available_cameras()}")
            sys.exit(1)

        if not resolution:
            CAMERA_CAP_WIDTH, CAMERA_CAP_HEIGHT = _get_camera_native_resolution()
        else:
            req_w, req_h = CAMERA_RESOLUTION_MAP.get(resolution, (640, 480))
            CAMERA_CAP_WIDTH = min(640, int(req_w))
            CAMERA_CAP_HEIGHT = min(480, int(req_h))

        cap = cv2.VideoCapture(CAMERA_INDEX, cv2.CAP_V4L2)
        cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*"YUYV"))  # enforce YUYV
        cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, CAMERA_CAP_WIDTH)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, CAMERA_CAP_HEIGHT)
        cap.set(cv2.CAP_PROP_FPS, 30)

        if not cap.isOpened():
            logger.error("Failed to open camera with V4L2/YUYV.")
            sys.exit(1)

        actual_w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        actual_h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        actual_fps = int(cap.get(cv2.CAP_PROP_FPS))
        fourcc = int(cap.get(cv2.CAP_PROP_FOURCC))
        fourcc_str = "".join([chr((fourcc >> 8 * i) & 0xFF) for i in range(4)])
        logger.info(f"Camera opened: {actual_w}x{actual_h} @ {actual_fps} fps, FOURCC={fourcc_str}")

    elif any(input_path.lower().endswith(sfx) for sfx in [".mp4", ".avi", ".mov", ".mkv"]):
        if not os.path.exists(input_path):
            logger.error(f"File not found: {input_path}")
            sys.exit(1)
        cap = cv2.VideoCapture(input_path)

    else:
        images = load_images_opencv(input_path)
        try:
            validate_images(images, batch_size)
        except ValueError as e:
            logger.error(e)
            sys.exit(1)

    return cap, images


def load_images_opencv(images_path: str) -> List[np.ndarray]:
    path = Path(images_path)
    imgs: List[np.ndarray] = []
    if path.is_file() and path.suffix.lower() in IMAGE_EXTENSIONS:
        img = cv2.imread(str(path))
        if img is not None:
            imgs.append(img)
    elif path.is_dir():
        for p in sorted(path.glob("*")):
            if p.suffix.lower() in IMAGE_EXTENSIONS:
                im = cv2.imread(str(p))
                if im is not None:
                    imgs.append(im)
    return imgs


def load_input_images(images_path: str):
    from PIL import Image
    path = Path(images_path)
    imgs = []
    if path.is_file() and path.suffix.lower() in IMAGE_EXTENSIONS:
        imgs.append(Image.open(path))
    elif path.is_dir():
        imgs.extend([Image.open(p) for p in sorted(path.glob("*")) if p.suffix.lower() in IMAGE_EXTENSIONS])
    return imgs


def validate_images(images: List[np.ndarray], batch_size: int) -> None:
    if not images:
        raise ValueError("No valid images found in the specified path.")
    if len(images) % batch_size != 0:
        raise ValueError("The number of input images must be divisible by batch size.")


def divide_list_to_batches(images_list: List[np.ndarray], batch_size: int) -> Generator[List[np.ndarray], None, None]:
    for i in range(0, len(images_list), batch_size):
        yield images_list[i : i + batch_size]


def generate_color(class_id: int) -> tuple:
    np.random.seed(class_id)
    return tuple(np.random.randint(0, 255, size=3).tolist())


def get_labels(labels_path: str) -> list:
    with open(labels_path, "r", encoding="utf-8") as f:
        return f.read().splitlines()


def id_to_color(idx):
    np.random.seed(idx)
    return np.random.randint(0, 255, size=3, dtype=np.uint8)


# ──────────────────────────────────────────────────────────────────────────────
# Preprocess (NHWC · UINT8 · RGB) — matches HEF input
# ──────────────────────────────────────────────────────────────────────────────

def preprocess(
    images: List[np.ndarray],
    cap: Optional[cv2.VideoCapture],
    batch_size: int,
    input_queue: queue.Queue,
    width: int,
    height: int,
    preprocess_fn: Optional[Callable[[np.ndarray, int, int], np.ndarray]] = None,
) -> None:
    """Pushes (frames_bgr, frames_rgb_preprocessed) batches into queue, then sentinel None."""
    preprocess_fn = preprocess_fn or default_preprocess

    if cap is None:
        preprocess_images(images, batch_size, input_queue, width, height, preprocess_fn)
    else:
        preprocess_from_cap(cap, batch_size, input_queue, width, height, preprocess_fn)

    # Signal end of stream
    try:
        input_queue.put(None, block=False)
    except queue.Full:
        try:
            _ = input_queue.get_nowait()
        except queue.Empty:
            pass
        input_queue.put(None, block=False)


def preprocess_from_cap(
    cap: cv2.VideoCapture,
    batch_size: int,
    input_queue: queue.Queue,
    width: int,
    height: int,
    preprocess_fn: Callable[[np.ndarray, int, int], np.ndarray],
) -> None:
    frames_bgr: List[np.ndarray] = []
    frames_rgb_pre: List[np.ndarray] = []

    while True:
        ok, frame_bgr = cap.read()
        if not ok:
            break

        frames_bgr.append(frame_bgr)
        rgb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
        pre = preprocess_fn(rgb, width, height)  # NHWC, uint8
        frames_rgb_pre.append(pre)

        if len(frames_bgr) == batch_size:
            item = (frames_bgr, frames_rgb_pre)
            try:
                input_queue.put(item, block=False)
            except queue.Full:
                try:
                    _ = input_queue.get_nowait()
                except queue.Empty:
                    pass
                input_queue.put(item, block=False)
            frames_bgr, frames_rgb_pre = [], []


def preprocess_images(
    images: List[np.ndarray],
    batch_size: int,
    input_queue: queue.Queue,
    width: int,
    height: int,
    preprocess_fn: Callable[[np.ndarray, int, int], np.ndarray],
) -> None:
    for batch in divide_list_to_batches(images, batch_size):
        # Convert BGR→RGB for each input file to match HEF expectations
        rgbs = [cv2.cvtColor(img, cv2.COLOR_BGR2RGB) for img in batch]
        pres = [preprocess_fn(im, width, height) for im in rgbs]
        item = (batch, pres)  # keep original BGR frames for viz
        try:
            input_queue.put(item, block=False)
        except queue.Full:
            try:
                _ = input_queue.get_nowait()
            except queue.Empty:
                pass
            input_queue.put(item, block=False)


# ──────────────────────────────────────────────────────────────────────────────
# Two reference preprocessors
# ──────────────────────────────────────────────────────────────────────────────

def default_preprocess(image_rgb: np.ndarray, model_w: int, model_h: int) -> np.ndarray:
    """Letterbox to (model_h, model_w); returns NHWC uint8 RGB (contiguous).
    - image_rgb: RGB uint8
    """
    h, w = image_rgb.shape[:2]
    scale = min(model_w / w, model_h / h)
    nw, nh = int(w * scale), int(h * scale)
    resized = cv2.resize(image_rgb, (nw, nh), interpolation=cv2.INTER_LINEAR)

    out = np.full((model_h, model_w, 3), 114, dtype=np.uint8)
    x0 = (model_w - nw) // 2
    y0 = (model_h - nh) // 2
    out[y0 : y0 + nh, x0 : x0 + nw] = resized
    return np.ascontiguousarray(out, dtype=np.uint8)


def resize_preprocess(image_rgb: np.ndarray, model_w: int, model_h: int) -> np.ndarray:
    """Pure resize (no padding). Useful if preprocess_mode == 'resize'."""
    out = cv2.resize(image_rgb, (model_w, model_h), interpolation=cv2.INTER_LINEAR)
    return np.ascontiguousarray(out, dtype=np.uint8)


# ──────────────────────────────────────────────────────────────────────────────
# Visualization
# ──────────────────────────────────────────────────────────────────────────────

def visualize(
    output_queue: queue.Queue,
    cap: Optional[cv2.VideoCapture],
    save_stream_output: bool,
    output_dir: str,
    callback: Callable[..., np.ndarray],  # accepts (frame, infer[, dets])
    fps_tracker: Optional["FrameRateTracker"] = None,
    side_by_side: bool = False,
) -> None:
    """Consumes output_queue and displays/saves frames.
    Queue item formats supported:
      (frame, infer)  # legacy
      (frame, infer, dets)  # preferred
      None  # termination
    """
    internal_fps = SimpleFPS() if fps_tracker is None else None
    image_id = 0
    writer = None

    if cap is not None:
        cv2.namedWindow("Output", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("Output", 1280, 720)
        if save_stream_output:
            base_w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
            base_h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            frame_w = base_w * 2 if side_by_side else base_w
            frame_h = base_h
            os.makedirs(output_dir, exist_ok=True)
            out_path = os.path.join(output_dir, "output.avi")
            writer = cv2.VideoWriter(out_path, cv2.VideoWriter_fourcc(*"XVID"), cap.get(cv2.CAP_PROP_FPS) or 30.0, (frame_w, frame_h))

    while True:
        item = output_queue.get()
        if item is None:
            try:
                if writer is not None:
                    writer.release()
            except Exception:
                pass
            try:
                if cap is not None:
                    cap.release()
            except Exception:
                pass
            try:
                cv2.destroyAllWindows()
            except Exception:
                pass
            output_queue.task_done()
            break

        out_frame = None
        try:
            if not isinstance(item, tuple):
                output_queue.task_done()
                continue

            if len(item) == 3:
                frame, infer, extra = item
                out_frame = callback(frame, infer, extra)
            elif len(item) == 2:
                frame, infer = item
                out_frame = callback(frame, infer)
            else:
                output_queue.task_done()
                continue

            fps_val = None
            if fps_tracker is not None:
                try:
                    fps_tracker.increment()
                    fps_val = fps_tracker.fps
                except Exception:
                    fps_val = None
            else:
                internal_fps.tick()
                fps_val = internal_fps.value()

            if fps_val and out_frame is not None:
                cv2.putText(out_frame, f"FPS: {fps_val:5.1f}", (16, 32), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1, cv2.LINE_AA)

            if out_frame is not None:
                if cap is not None:
                    cv2.imshow("Output", out_frame)
                    if writer is not None:
                        writer.write(out_frame)
                else:
                    os.makedirs(output_dir, exist_ok=True)
                    cv2.imwrite(os.path.join(output_dir, f"output_{image_id}.png"), out_frame)
                    image_id += 1
                if cv2.waitKey(1) & 0xFF == ord("q"):
                    try:
                        if writer is not None:
                            writer.release()
                    except Exception:
                        pass
                    try:
                        if cap is not None:
                            cap.release()
                    except Exception:
                        pass
                    try:
                        cv2.destroyAllWindows()
                    except Exception:
                        pass
                    output_queue.task_done()
                    break
        finally:
            try:
                output_queue.task_done()
            except Exception:
                pass


# ──────────────────────────────────────────────────────────────────────────────
# FPS tracker for the outer pipeline
# ──────────────────────────────────────────────────────────────────────────────
class FrameRateTracker:
    def __init__(self):
        self._count = 0
        self._start_time = None

    def start(self) -> None:
        self._start_time = time.time()

    def increment(self, n: int = 1) -> None:
        self._count += n

    @property
    def count(self) -> int:
        return self._count

    @property
    def elapsed(self) -> float:
        if self._start_time is None:
            return 0.0
        return time.time() - self._start_time

    @property
    def fps(self) -> float:
        el = self.elapsed
        return self._count / el if el > 0 else 0.0

    def frame_rate_summary(self) -> str:
        return f"Processed {self.count} frames at {self.fps:.2f} FPS"
