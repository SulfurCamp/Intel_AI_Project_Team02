#app_three_threads.py
import sys
import socket
import time
import threading
import cv2
import serial
from dataclasses import dataclass, asdict
from flask import Flask, Response, render_template_string
from ultralytics import YOLO

# ===== 추가: RealSense =====
import numpy as np
import pyrealsense2 as rs  # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<< CHANGED

# ===== 설정 =====
# CAM_INDEX   = 0  # 웹캠용이었음 → 불필요
WIDTH, HEIGHT, FPS = 1280, 720, 30
MODEL_PATH  = "yolov8n.pt"   # 필요 시 바꿔도 됨
WANT_CLASS  = None           # COCO 사람만: 0, 전체: None

# RealSense 옵션 (필요시 조절)
RS_USE_DEPTH   = False       # True 로 바꾸면 깊이 스트림도 켭니다
RS_ALIGN_DEPTH = False       # True 로 바꾸면 컬러에 깊이를 정렬합니다
RS_SERIAL      = None        # 특정 장치 지정 시 시리얼 문자열

SERIAL_PORT = "/dev/ttyUSB0" # Windows 예: "COM6"
SERIAL_BAUD = 115200
SERIAL_TIMEOUT = 0.05
UART_RATE_HZ = 20

HOST, PORT = "0.0.0.0", 8000

BUF_SIZE = 100
NAME_SIZE = 20
ARR_CNT = 5  # 프로토콜 유지용. (파싱 예비)

CHAT_HOST = "192.168.0.47"
#CHAT_HOST = "127.0.0.1"
CHAT_PORT = 5000
CHAT_NAME = "RASPBERRYPI"

RS_USE_DEPTH   = True        # 깊이 스트림 켭니다
RS_ALIGN_DEPTH = True        # 컬러 기준으로 깊이 정렬

# ===== 공유 상태 =====
latest_frame = None
frame_lock   = threading.Lock()

latest_result = None
latest_depth = None
result_cv = threading.Condition()  # 새 결과 알림
stop_event = threading.Event()

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

# ===== 유틸(그리기) =====
def draw_grid(frame):
    h, w = frame.shape[:2]
    w3, h3 = w // 3, h // 3
    color = (0, 255, 255)
    t = 1
    cv2.line(frame, (w3, 0), (w3, h), color, t, cv2.LINE_AA)
    cv2.line(frame, (2*w3, 0), (2*w3, h), color, t, cv2.LINE_AA)
    cv2.line(frame, (0, h3), (w, h3), color, t, cv2.LINE_AA)
    cv2.line(frame, (0, 2*h3), (w, 2*h3), color, t, cv2.LINE_AA)
    # 번호
    for r in range(3):
        for c in range(3):
            cx = int((c + 0.5) * w3)
            cy = int((r + 0.5) * h3)
            idx = r * 3 + c + 1
            cv2.putText(frame, str(idx), (cx - 10, cy + 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0,255,0), 2, cv2.LINE_AA)
            
def draw_cross(frame):
    h, w = frame.shape[:2]
    cx, cy = w // 2, h // 2
    cv2.drawMarker(frame, (cx, cy), (255, 0, 0),
                   markerType=cv2.MARKER_CROSS, markerSize=20, thickness=2, line_type=cv2.LINE_AA)

# ===== 추론 스레드 =====
def select_target(result, want_cls=None):
    if result.boxes is None or len(result.boxes) == 0:
        return None
    xyxy = result.boxes.xyxy.cpu().numpy()
    cls  = result.boxes.cls.cpu().numpy() if result.boxes.cls is not None else None
    best_i, best_area = -1, -1.0
    for i, (x1,y1,x2,y2) in enumerate(xyxy):
        if want_cls is not None and cls is not None and int(cls[i]) != int(want_cls):
            continue
        area = max(0.0, x2-x1) * max(0.0, y2-y1)
        if area > best_area:
            best_area, best_i = area, i
    if best_i < 0: return None
    return tuple(map(float, xyxy[best_i]))

def send_loop(sock: socket.socket, stop_event: threading.Event, name: str) -> None:
    """
    latest_result가 변경될 때만 서버로 전송.
    메시지가 '[' 로 시작하지 않으면 [ALLMSG] prefix를 붙여 보냄.
    """
    print("Send loop: only on latest_result changes")
    last_sent = None

    try:
        while not stop_event.is_set():
            with result_cv:
                result_cv.wait_for(
                    lambda: stop_event.is_set() or (latest_result is not None and latest_result != last_sent),
                    timeout=1.0
                )
                cur = latest_result

            if stop_event.is_set():
                break
            if cur is None or cur == last_sent:
                continue

            payload = f"{str(cur)}@{str(latest_depth)}\n"

            name_msg = payload if payload.startswith('[') else f"[DBCLIENT]{payload}"

            try:
                sock.sendall(name_msg.encode("utf-8"))
                last_sent = cur
            except OSError:
                stop_event.set()
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
    """
    서버로부터의 수신 데이터를 그대로 표준출력에 씀.
    서버가 연결을 닫으면 stop_event를 set하고 종료.
    """
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

def inference_thread():
    global latest_frame, latest_result, latest_depth

    pipeline = rs.pipeline()
    config   = rs.config()
    if RS_SERIAL:
        config.enable_device(RS_SERIAL)

    config.enable_stream(rs.stream.color, WIDTH, HEIGHT, rs.format.bgr8, FPS)
    if RS_USE_DEPTH:
        config.enable_stream(rs.stream.depth, WIDTH, HEIGHT, rs.format.z16, FPS)

    profile = pipeline.start(config)

    align = rs.align(rs.stream.color) if (RS_USE_DEPTH and RS_ALIGN_DEPTH) else None
    depth_scale = None
    if RS_USE_DEPTH:
        depth_sensor = profile.get_device().first_depth_sensor()
        depth_scale = depth_sensor.get_depth_scale()

    model = YOLO(MODEL_PATH)

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
            cx, cy = w // 2, h // 2

            # --- YOLO ---
            target_bbox = None
            for r in model(frame, stream=True):
                target_bbox = select_target(r, WANT_CLASS)
                if target_bbox is not None:
                    break

            # ---- 화면용 오버레이 ----
            tmp = frame.copy()
            draw_grid(tmp)
            draw_cross(tmp)
            with frame_lock:
                latest_frame = tmp

            # ---- 좌표 결정 (디텍션 없으면 화면 중앙) ----
            if target_bbox is not None:
                x1, y1, x2, y2 = map(int, target_bbox)
                bx, by = (x1 + x2)//2, (y1 + y2)//2
                latest_result = value_from_xy(bx, by, w, h) or "?"
                cv2.rectangle(frame, (x1,y1), (x2,y2), (0,255,0), 2)
                cv2.circle(frame, (bx,by), 5, (0,0,255), -1)
            else:
                bx, by = cx, cy  # 감지 없으면 중앙 픽셀
                latest_result = None  # 또는 유지하고 싶으면 삭제

            # ---- 깊이 읽기 ----
            if depth_frame is not None:
                # 좌표 클램프(경계 밖 접근 방지)
                bx_clamped = int(max(0, min(w-1, bx)))
                by_clamped = int(max(0, min(h-1, by)))
                depth_val = depth_frame.get_distance(bx_clamped, by_clamped)  # 미터
                with frame_lock:
                    latest_depth = float(depth_val)  # 0.0일 수도 있음(유효치 없는 경우)

            with frame_lock:
                latest_frame = frame

            with result_cv:
                result_cv.notify_all()
    finally:
        try: pipeline.stop()
        except: pass
        print("[INF] Inference thread exit")

# ===== Flask(MJPEG) 스레드 =====
app = Flask(__name__)

INDEX_HTML = """
<!doctype html>
<title>Stream (YOLO + Grid + Cross)</title>
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
    # reloader 끄기(카메라 2번 열림 방지)
    app.run(host=HOST, port=PORT, threaded=True, use_reloader=False)

# ===== UART 스레드 =====
def uart_thread():
    ser = None
    period = 1.0 / max(1, UART_RATE_HZ)
    while not stop_event.is_set():
        # 연결 없으면 재시도
        if ser is None:
            try:
                ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=SERIAL_TIMEOUT)
                print(f"[UART] Opened {SERIAL_PORT} @ {SERIAL_BAUD}")
            except Exception as e:
                print(f"[UART] Open failed: {e}; retry in 1s")
                time.sleep(1.0)
                continue

        # 새 결과가 오길 기다림(또는 타임아웃 주기 전송)
        with result_cv:
            result_cv.wait(timeout=period)
            res = latest_result

        if res is None:
            continue

        # NOTE: 기존 코드가 res를 객체로 가정합니다. 현재 latest_result는 문자열(예: 'Q')이므로
        # 아래 라인은 필요시 프로토콜에 맞게 바꾸세요.
        # line = f"VAL:{res.value},dx:{res.dx},dy:{res.dy},ndx:{res.ndx:.3f},ndy:{res.ndy:.3f}\n"
        line = f"VAL:{res}\n"
        try:
            ser.write(line.encode("ascii"))
        except Exception as e:
            print(f"[UART] Write failed: {e}; reopening...")
            try: ser.close()
            except: pass
            ser = None

    # 종료 정리
    try:
        if ser and ser.is_open:
            ser.close()
    except:
        pass
    print("[UART] Thread exit")

# ===== 소켓 클라이언트 스레드 (추가) =====
def socket_client_thread():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except OSError as e:
        print(f"socket() error: {e}", file=sys.stderr)
        sys.exit(1)

    try:
        sock.connect((CHAT_HOST, CHAT_PORT))
    except OSError as e:
        print(f"connect() error: {e}", file=sys.stderr)
        sys.exit(1)

    # 접속 즉시 인증(또는 초기 메시지) 전송: "[<name>:PASSWD]"
    first_msg = f"[{CHAT_NAME}:PASSWD]".encode("utf-8")
    try:
        sock.sendall(first_msg)
    except OSError as e:
        print(f"send() error: {e}", file=sys.stderr)
        sys.exit(1)

    stop_event = threading.Event()

    # 수신 스레드 시작
    t_recv = threading.Thread(target=recv_loop, args=(sock, stop_event), daemon=True)
    t_recv.start()

    # 송신 스레드
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
    th_uart = threading.Thread(target=uart_thread,    name="uart",  daemon=True)
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
