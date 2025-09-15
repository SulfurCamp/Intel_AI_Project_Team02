import sys
import socket
import time
import threading
import cv2
import serial
from dataclasses import dataclass, asdict
from flask import Flask, Response, render_template_string

# ==== YOLOv8 (CUDA 우선) ====
import torch
from ultralytics import YOLO

# ===== RealSense =====
import numpy as np
import pyrealsense2 as rs

# ===== 설정 =====
CONF_TH = 0.5
#MODEL_PATH = "/home/ydj9072/opencv/runs/detect/drone_detector_yolov8s_dataset36/weights/best.pt"      # ← YOLOv8s 가중치 파일
#MODEL_PATH = "/home/ydj9072/opencv/best.pt"
MODEL_PATH = "/home/ydj9072/best.pt"
WIDTH, HEIGHT, FPS = 1280, 720, 30

# RealSense 옵션
RS_USE_DEPTH   = True
RS_ALIGN_DEPTH = True
RS_SERIAL      = None

# UART / 소켓
SERIAL_PORT = "/dev/ttyACM0"
BAUDRATE = 115200
SERIAL_TIMEOUT = 0.05
UART_RATE_HZ = 20

HOST, PORT = "0.0.0.0", 8000

BUF_SIZE = 100
NAME_SIZE = 20

CHAT_HOST = "192.168.0.172"
CHAT_PORT = 5000
CHAT_NAME = "RASPBERRYPI"

# ===== 공유 상태 =====
latest_frame = None
frame_lock   = threading.Lock()

latest_result   = None        # 선택 박스 중심의 3x3 그리드 문자
latest_depth    = None        # 선택 박스 중심의 깊이(m)
latest_is_drone = False       # 선택 박스가 'drone' 라벨인지
latest_label    = None        # 선택된 타깃의 라벨명 (소켓 전송용)
latest_cx       = None        # ★ 중심 X (pixel)
latest_cy       = None        # ★ 중심 Y (pixel)

result_cv = threading.Condition()
stop_event = threading.Event()

sec = 0.2
R = "R"  # 검출 없을 때 UART로 보낼 대체 문자

# ===== 3x3 키맵 & 좌표→문자 =====
KEYMAP = [
    ['Q','W','E'],
    ['A','S','D'],
    ['Z','X','C'],
]
def value_from_xy(x, y, w, h):
    if not (0 <= x < w and 0 <= y < h):
        return None
    col = min(2, int(3 * x / w))
    row = min(2, int(3 * y / h))
    return KEYMAP[row][col]

# ===== 소켓 보조 =====
def send_loop(sock: socket.socket, stop_event: threading.Event, name: str, interval: float = sec) -> None:
    """
    latest_* 값으로 서버 전송.
    포맷: "<cx>@<cy>@<depth>@<label>\\n"
    - depth가 유효하지 않으면 0.000으로 보냄
    - label이 없으면 빈 문자열
    """
    print("Send loop: sending cx@cy@depth@label")
    next_ts = time.monotonic()

    try:
        while not stop_event.is_set():
            cx    = latest_cx
            cy    = latest_cy
            depth = latest_depth
            label = latest_label or ""

            if (cx is not None) and (cy is not None):
                z = depth if (depth is not None and depth > 0.0 and not (isinstance(depth, float) and np.isnan(depth))) else 0.0
                payload  = f"{int(cx)}@{int(cy)}@{z:.3f}@{label}\n"

                name_msg = payload if payload.startswith('[') else f"[QTCLIENT]{payload}"
                try:
                    sock.sendall(name_msg.encode("utf-8"))
                except OSError:
                    stop_event.set()
                    break

            next_ts += interval
            sleep_for = max(0.0, next_ts - time.monotonic())
            if stop_event.wait(sleep_for):
                break
    finally:
        try:
            sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        try:
            sock.close()
        except OSError:
            pass

def recv_loop(sock: socket.socket, stop_event: threading.Event) -> None:
    try:
        while not stop_event.is_set():
            try:
                data = sock.recv(NAME_SIZE + BUF_SIZE)
            except OSError:
                stop_event.set()
                break

            if not data:
                stop_event.set()
                break

            try:
                text = data.decode("utf-8", errors="replace")
            except Exception:
                text = data.decode("latin1", errors="replace")

            sys.stdout.write(text)
            sys.stdout.flush()
    finally:
        stop_event.set()
        try:
            sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        try:
            sock.close()
        except OSError:
            pass

# ===== 유틸: 안전한 깊이값 비교용 =====
def _valid_depth(z):
    # RealSense에서 미측정은 0.0인 경우가 많음. NaN도 배제.
    return (z is not None) and (z > 0.0) and (not np.isnan(z))

def depth_to_bgr(z):
    """
    규칙:
      - z가 None, NaN, <= 0.0 이면 빨강 (미측정/0)
      - 0.5 ~ 1.0 이면 초록
      - 1.0 이상이면 파랑
      - 0 < z < 0.5 은 명시 없어서 빨강 처리
    반환: (B, G, R)
    """
    if (z is None) or (not isinstance(z, (int, float))) or np.isnan(z) or z <= 0.0:
        return (0, 0, 255)      # 빨강
    if 0.5 <= z <= 1.0:
        return (0, 255, 0)      # 초록
    if z >= 1.0:
        return (255, 0, 0)      # 파랑
    return (0, 0, 255)          # 0 < z < 0.5 → 빨강

# ===== 추론 스레드 (★ YOLOv8s CUDA 기반) =====
def inference_thread():
    global latest_frame, latest_result, latest_depth, latest_is_drone, latest_label, latest_cx, latest_cy

    # ----- RealSense 파이프라인 설정 -----
    pipeline = rs.pipeline()
    config   = rs.config()
    if RS_SERIAL:
        config.enable_device(RS_SERIAL)

    config.enable_stream(rs.stream.color, WIDTH, HEIGHT, rs.format.bgr8, FPS)
    if RS_USE_DEPTH:
        config.enable_stream(rs.stream.depth, WIDTH, HEIGHT, rs.format.z16, FPS)

    profile = pipeline.start(config)
    align = rs.align(rs.stream.color) if (RS_USE_DEPTH and RS_ALIGN_DEPTH) else None

    # ----- YOLOv8s 로드 (CUDA 우선) -----
    device_str = "cuda:0" if torch.cuda.is_available() else "cpu"
    try:
        torch.backends.cudnn.benchmark = True
    except Exception:
        pass
    model = YOLO(MODEL_PATH)
    try:
        model.to(device_str)
    except Exception:
        pass

    names = model.names  # 클래스 id → 라벨명 매핑

    try:
        while not stop_event.is_set():
            frames = pipeline.wait_for_frames()
            if align is not None:
                frames = align.process(frames)

            color_frame = frames.get_color_frame()
            depth_frame = frames.get_depth_frame() if RS_USE_DEPTH else None
            if not color_frame:
                time.sleep(0.001)
                continue

            frame = np.asanyarray(color_frame.get_data())
            h, w = frame.shape[:2]

            # ---- YOLOv8s 추론 ----
            try:
                result = model(frame, conf=CONF_TH, verbose=False, device=device_str)[0]
            except Exception as e:
                print(f"[INF] YOLO inference error: {e}")
                result = None

            # ---- 결과 파싱 + 중심 깊이采樣 (depth가 가장 가까운 박스를 고름) ----
            # cand 항목: (has_valid_depth, depth, conf, area, lbl, x1,y1,x2,y2, bx,by)
            cand = []
            if result is not None and result.boxes is not None and len(result.boxes) > 0:
                boxes = result.boxes
                xyxy = boxes.xyxy.cpu().numpy()
                confs = boxes.conf.cpu().numpy()
                clses = boxes.cls.cpu().numpy().astype(int)

                for i in range(xyxy.shape[0]):
                    x1, y1, x2, y2 = xyxy[i]
                    c  = float(confs[i])
                    if c < CONF_TH:
                        continue
                    cls_id = int(clses[i])
                    lbl = names.get(cls_id, str(cls_id)) if isinstance(names, dict) else (
                          names[cls_id] if isinstance(names, list) and 0 <= cls_id < len(names) else str(cls_id))

                    # 경계 보정
                    x1 = int(max(0, min(w-1, x1))); x2 = int(max(0, min(w-1, x2)))
                    y1 = int(max(0, min(h-1, y1))); y2 = int(max(0, min(h-1, y2)))
                    if x2 <= x1 or y2 <= y1:
                        continue

                    area = (x2 - x1) * (y2 - y1)
                    bx, by = (x1 + x2)//2, (y1 + y2)//2

                    z = None
                    if depth_frame is not None:
                        bx_c = int(max(0, min(w-1, bx)))
                        by_c = int(max(0, min(h-1, by)))
                        z = float(depth_frame.get_distance(bx_c, by_c))  # meters

                    has_z = _valid_depth(z)
                    cand.append( (has_z, z if has_z else float('inf'), -c, -area, lbl, x1, y1, x2, y2, bx, by) )

            # ---- ★ 깊이 최우선 선택 로직 ★ ----
            best = None
            if cand:
                has_any_valid_depth = any(item[0] for item in cand)
                if has_any_valid_depth:
                    subset = [it for it in cand if it[0]]
                    best = min(subset, key=lambda t: (t[1], t[2], t[3]))
                else:
                    best = min(cand, key=lambda t: (t[2], t[3]))

            # ---- 오버레이 & 상태 갱신 ----
            vis = frame.copy()
            if best:
                has_z, z, nconf, narea, lbl, x1, y1, x2, y2, bx, by = best
                conf = -nconf
                area = -narea
                is_drone = (lbl.lower() == "drone")
                color = depth_to_bgr(z if _valid_depth(z) else 0.0)

                cv2.rectangle(vis, (x1, y1), (x2, y2), color, 2)
                depth_text = f"Z={z:.3f}m" if _valid_depth(z) else "Z=?"
                cv2.putText(
                    vis, f"{lbl} {conf:.2f} {depth_text}",
                    (x1, max(0, y1 - 6)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2
                )
                cv2.circle(vis, (bx, by), 5, (0, 255, 255), -1)

                latest_result   = value_from_xy(bx, by, w, h) or "?"
                latest_is_drone = is_drone
                latest_label    = lbl
                with frame_lock:
                    latest_depth = float(z) if _valid_depth(z) else 0.0
                    latest_cx    = int(bx)   # ★ 중심 X
                    latest_cy    = int(by)   # ★ 중심 Y
            else:
                latest_result   = None
                latest_is_drone = False
                latest_label    = None
                with frame_lock:
                    latest_depth = None
                    latest_cx    = None
                    latest_cy    = None

            # ---- 공유 프레임 업데이트 ----
            with frame_lock:
                latest_frame = vis

            with result_cv:
                result_cv.notify_all()

    finally:
        try:
            pipeline.stop()
        except Exception:
            pass
        print("[INF] Inference thread exit")

# ===== Flask(MJPEG) =====
app = Flask(__name__)
INDEX_HTML = """
<!doctype html>
<title>Stream</title>
<style>body{font-family:system-ui;margin:20px}img{max-width:100%;height:auto;border-radius:8px}</style>
<h3>RealSense Stream</h3>
<img src="/stream.mjpg">
"""
@app.route("/")
def index():
    return render_template_string(INDEX_HTML)

def mjpeg_gen():
    while not stop_event.is_set():
        with frame_lock:
            frame = None if latest_frame is None else latest_frame.copy()
        if frame is None:
            time.sleep(0.01); continue
        ok, buf = cv2.imencode(".jpg", frame, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
        if not ok:
            continue
        yield (b"--frame\r\nContent-Type: image/jpeg\r\n\r\n" + buf.tobytes() + b"\r\n")

@app.route("/stream.mjpg")
def stream():
    return Response(mjpeg_gen(), mimetype="multipart/x-mixed-replace; boundary=frame")

def flask_thread():
    app.run(host=HOST, port=PORT, threaded=True, use_reloader=False)

def depth_to_tag(z: float) -> str:
    """
    요청 매핑:
      - 0.0 ~ 0.5      -> '1'
      - 0.8 ~ 1.1      -> '1'  (1이 두 구간을 커버)
      - (1.1 ~ 1.4)    -> '2'  (1.1 초과 ~ 1.4 미만)
      - [1.4 ~ 1.7)    -> '3'
      - [2.0 ~ 2.3]    -> '4'
      - > 2.3          -> '5'
      - 그 외/미측정   -> '0'
    """
    if not _valid_depth(z):
        return "0"
    if 0.0 <= z <= 0.5:
        return "1"
    # 2~5번 구간
    if 0.5 < z < 1.3:
        return "2"
    if 1.3 <= z < 1.6:
        return "3"
    if 1.6 <= z <= 1.9:
        return "4"
    if z > 1.9:
        return "5"
    # 명시되지 않은 빈 구간은 0
    return "0"

# ===== UART 스레드 =====
def uart_thread(interval=None):
    ser = None
    period = 1.0 / max(1, UART_RATE_HZ)

    while not stop_event.is_set():
        if ser is None:
            try:
                ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=SERIAL_TIMEOUT)
                print(f"[UART] Opened {SERIAL_PORT} @ {BAUDRATE}")
            except Exception as e:
                time.sleep(1.0)
                continue

        cur   = latest_result
        depth = latest_depth

        if cur is not None:
            tag = depth_to_tag(depth)
            payload = f"{str(cur)}@{tag}\n"
            try:
                ser.write(payload.encode("ascii"))
                print(payload)
            except Exception as e:
                print(f"[UART] Write failed: {e}; reopening...")
                try: ser.close()
                except: pass
                ser = None

        time.sleep(period)

    try:
        if ser and ser.is_open:
            ser.close()
    except:
        pass
    print("[UART] Thread exit")

# ===== 소켓 클라이언트 =====
def socket_client_thread():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except OSError as e:
        print(f"socket() error: {e}", file=sys.stderr)
        return

    try:
        sock.connect((CHAT_HOST, CHAT_PORT))
    except OSError as e:
        print(f"connect() error: {e}", file=sys.stderr)
        return

    first_msg = f"[{CHAT_NAME}:PASSWD]".encode("utf-8")
    try:
        sock.sendall(first_msg)
    except OSError as e:
        print(f"send() error: {e}", file=sys.stderr)
        return

    t_recv = threading.Thread(target=recv_loop, args=(sock, stop_event), daemon=True)
    t_recv.start()

    t_send = threading.Thread(target=send_loop, args=(sock, stop_event, CHAT_NAME), daemon=True)
    t_send.start()

    try:
        t_send.join()
    except KeyboardInterrupt:
        stop_event.set()

    t_recv.join(timeout=1.0)

# ===== 실행 =====
if __name__ == "__main__":
    th_inf  = threading.Thread(target=inference_thread, name="infer", daemon=True)
    th_uart = threading.Thread(target=uart_thread, name="uart", kwargs={"interval": sec} , daemon=True)
    th_sock = threading.Thread(target=socket_client_thread, name="sock", daemon=True)

    th_inf.start()
    th_uart.start()
    th_sock.start()

    try:
        app.run(host=HOST, port=PORT, threaded=True, use_reloader=False)
    except KeyboardInterrupt:
        print("\n[MAIN] Ctrl+C -> stopping...")
    finally:
        stop_event.set()
        with result_cv:
            result_cv.notify_all()
        th_inf.join(timeout=2.0)
        th_uart.join(timeout=2.0)
        print("[MAIN] Done.")
