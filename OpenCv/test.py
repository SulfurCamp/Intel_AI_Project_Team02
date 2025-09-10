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
MODEL_PATH = "/home/ydj9072/opencv/runs/detect/drone_detector_yolov8s_dataset35/weights/best.pt"      # ← YOLOv8s 가중치 파일
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

CHAT_HOST = "192.168.0.47"
CHAT_PORT = 5000
CHAT_NAME = "RASPBERRYPI"

# ===== 공유 상태 =====
latest_frame = None
frame_lock   = threading.Lock()

latest_result   = None        # 선택 박스 중심의 3x3 그리드 문자
latest_depth    = None        # 선택 박스 중심의 깊이(m)
latest_is_drone = False       # 선택 박스가 'drone' 라벨인지
latest_label    = None        # 선택된 타깃의 라벨명 (소켓 전송용)

result_cv = threading.Condition()
stop_event = threading.Event()

sec = 1.0
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
    latest_result가 있을 때만 서버로 전송.
    포맷: "<그리드문자>@<1|2>@<라벨명>\n"
          - 1: depth 없음/0, 2: depth 있음
    """
    print("Send loop: only on latest_result changes")
    next_ts = time.monotonic()

    try:
        while not stop_event.is_set():
            cur    = latest_result
            depth  = latest_depth
            label  = latest_label or ""

            if cur is not None:
                tag = "1" if (depth in (None, 0)) else "2"
                payload  = f"{str(cur)}@{tag}@{label}\n"

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

# ===== 추론 스레드 (★ YOLOv8s CUDA 기반) =====
def inference_thread():
    global latest_frame, latest_result, latest_depth, latest_is_drone, latest_label

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
        # 일부 환경에선 .to() 없이 device 인자로만 지정해도 동작
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
            # NOTE: conf=CONF_TH 로 1차 필터 (NMS 내부에서 적용)
            #       device=device_str 로 CUDA 사용 (가능 시)
            try:
                result = model(frame, conf=CONF_TH, verbose=False, device=device_str)[0]
            except Exception as e:
                print(f"[INF] YOLO inference error: {e}")
                result = None

            # ---- 결과 파싱 → (area, conf, label, x1,y1,x2,y2) 리스트 ----
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
                    cand.append((area, c, lbl, x1, y1, x2, y2))

            # ---- ★ 면적 최대 박스 1개 선택 (동률 시 conf 큰 것) ★ ----
            best = max(cand, key=lambda t: (t[0], t[1])) if cand else None

            # ---- 오버레이: 선택 박스만 그린다 ----
            vis = frame.copy()
            if best:
                area, acc, lbl, x1, y1, x2, y2 = best
                is_drone = (lbl.lower() == "drone")
                color = (0, 255, 0) if is_drone else (255, 0, 0)  # drone=초록, 기타=파랑
                cv2.rectangle(vis, (x1, y1), (x2, y2), color, 2)
                cv2.putText(
                    vis, f"{lbl} {acc:.2f} {area}",
                    (x1, max(0, y1 - 6)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2
                )

                # ---- 결과/깊이 갱신 (선택 박스 기준) ----
                bx, by = (x1 + x2)//2, (y1 + y2)//2
                cv2.circle(vis, (bx, by), 5, (0, 255, 255), -1)

                latest_result   = value_from_xy(bx, by, w, h) or "?"
                latest_is_drone = is_drone
                latest_label    = lbl

                if depth_frame is not None:
                    bx_c = int(max(0, min(w-1, bx)))
                    by_c = int(max(0, min(h-1, by)))
                    depth_val = depth_frame.get_distance(bx_c, by_c)  # meters
                    with frame_lock:
                        latest_depth = float(depth_val)
                else:
                    with frame_lock:
                        latest_depth = None
            else:
                latest_result   = None
                latest_is_drone = False
                latest_label    = None
                with frame_lock:
                    latest_depth = None

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
                print(f"[UART] Open failed: {e}; retry in 1s")
                time.sleep(1.0)
                continue

        cur   = latest_result
        depth = latest_depth

        if cur is not None:
            tag = "1" if (depth in (None, 0)) else "2"
            payload = f"{str(cur)}@{tag}\n"
        else:
            payload = f"{R}\n"  # 검출 없을 때 메시지

        try:
            ser.write(payload.encode("ascii"))
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

