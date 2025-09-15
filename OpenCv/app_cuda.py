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
MODEL_PATH = "best.pt"      # ← YOLOv8 가중치 파일
WIDTH, HEIGHT, FPS = 1280, 720, 30

# RealSense 옵션
RS_USE_DEPTH   = True
RS_ALIGN_DEPTH = True
RS_SERIAL      = None  # 특정 시리얼 사용시 문자열 입력

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

# ---- 카메라 FOV(도) : 기종/해상도에 맞게 조정 (D435 컬러 대략값 예시)
FOV_X_DEG = 69.4
FOV_Y_DEG = 42.5
FOV_X_RAD = np.deg2rad(FOV_X_DEG)
FOV_Y_RAD = np.deg2rad(FOV_Y_DEG)

# ---- 부호 토글(설치 방향 튜닝용)
# IMU에서 읽은 델타에 곱해줌
SIGN_YAW_IMU      = +1
SIGN_PITCH_IMU    = +1   # ← 위로 올릴 때 dpitch 양수
# 델타각 → 픽셀 쉬프트 부호(화면 좌표 기준)
SIGN_YAW_TO_PIX   = -1   # +yaw(우회전) → 화면 x 감소(좌)
SIGN_PITCH_TO_PIX = -1   # +pitch(업)    → 화면 y 감소(위)

# ---- 디버그 스위치
DEBUG_IMU  = True   # IMU yaw/pitch/roll 주기적 출력
DEBUG_SIGN = True   # dyaw/dpitch 및 쉬프트/투영 좌표/age 출력
GYRO_SCALE = 1.0    # 만약 자이로가 deg/s면 np.deg2rad(1.0)로 변경

# ===== 공유 상태 =====
latest_frame = None
frame_lock   = threading.Lock()

latest_result   = None        # 3x3 그리드 문자(Q/W/E/A/S/D/Z/X/C)
latest_depth    = None        # m
latest_is_drone = False
latest_label    = None

latest_cx = None              # 중심 x
latest_cy = None              # 중심 y

result_cv = threading.Condition()
stop_event = threading.Event()

sec = 0.2
R = "R"

# ===== 3x3 키맵 =====
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

# ===== 유틸 =====
def _valid_depth(z):
    return (z is not None) and isinstance(z, (int, float)) and (z > 0.0) and (not np.isnan(z))

def depth_to_bgr(z):
    if (z is None) or (not isinstance(z, (int, float))) or np.isnan(z) or z <= 0.0:
        return (0, 0, 255)      # 빨강
    if 0.5 <= z <= 1.0:
        return (0, 255, 0)      # 초록
    if z >= 1.0:
        return (255, 0, 0)      # 파랑
    return (0, 0, 255)

# ===== IMU 상태 / 보정 =====
imu_lock = threading.Lock()
imu_yaw = 0.0
imu_pitch = 0.0
imu_roll = 0.0

# IMU 가용성(저수준 센서에서 최근 프레임 수신 여부)
imu_available = False
imu_last_frame_ts = 0.0

# UART 직후 기준 자세 스냅샷(라디안)
last_uart_cmd = None
last_uart_yaw = 0.0
last_uart_pitch = 0.0
last_uart_ts = 0.0

# 스냅샷 조건(움직임 키만)
MOVING_KEYS = set(list("QWEADZXC"))
prev_move_cmd = None
MIN_SNAPSHOT_INTERVAL = 0.25  # 같은 방향 연속 스냅샷 최소 간격(초)

def _wrap_pi(a):
    # -pi ~ pi
    return (a + np.pi) % (2*np.pi) - np.pi

def complementary_update(gyro, accel, dt, alpha=0.98):
    """
    gyro: (gx, gy, gz) [rad/s]
    accel: (ax, ay, az) [m/s^2]
    yaw: 자이로 적분, pitch/roll: 가속도로 드리프트 보정 (보완필터)
    - D435i에서 정지 중 g ≈ (0, -9.8, 0) → 평평하면 pitch=0, roll=0 되도록 구성
    """
    global imu_yaw, imu_pitch, imu_roll

    gx, gy, gz = gyro
    ax, ay, az = accel

    # 1) 자이로 적분
    imu_yaw   = _wrap_pi(imu_yaw   + gz * dt)  # z축 회전
    imu_pitch = _wrap_pi(imu_pitch + gx * dt)  # x축 회전
    imu_roll  = _wrap_pi(imu_roll  + gy * dt)  # y축 회전

    # 2) 가속도 기반 기울기 (중력 벡터 정규화)
    g = np.sqrt(ax*ax + ay*ay + az*az)
    if g > 1e-3:
        axn, ayn, azn = ax/g, ay/g, az/g
        # 핵심 수정: 평평(ay≈-g)일 때 pitch=0, roll=0
        pitch_acc = np.arctan2( azn, -ayn )  # 업 틸트(+az) → +pitch
        roll_acc  = np.arctan2( axn, -ayn )  # 우측 기울임(+ax) → +roll
        imu_pitch = alpha*imu_pitch + (1-alpha)*pitch_acc
        imu_roll  = alpha*imu_roll  + (1-alpha)*roll_acc

    return imu_yaw, imu_pitch, imu_roll

# ---- 저수준 Motion Sensor 기반 IMU 스레드 ----
def imu_thread():
    """
    Motion Module 저수준 센서 API 사용:
      - 장치의 Motion Sensor를 직접 open/start
      - 콜백으로 최신 gyro/accel 저장
      - 내부 루프에서 dt 적분 + 보완필터 적용
    """
    global imu_yaw, imu_pitch, imu_roll
    global imu_available, imu_last_frame_ts

    ctx = rs.context()
    if len(ctx.devices) == 0:
        print("[IMU] no device")
        return

    motion_sensor = None
    device = None
    for dev in ctx.devices:
        for s in dev.query_sensors():
            try:
                if s.is_motion_sensor():
                    motion_sensor = s
                    device = dev
                    break
            except Exception:
                pass
        if motion_sensor:
            break

    if motion_sensor is None:
        print("[IMU] no motion sensor on any device")
        imu_available = False
        return

    dev_name = device.get_info(rs.camera_info.name)
    dev_sn   = device.get_info(rs.camera_info.serial_number)
    print(f"[IMU] Using device: {dev_name} (SN: {dev_sn})")
    print("[IMU] Sensor:", motion_sensor.get_info(rs.camera_info.name))

    def pick_profile(profiles, stream_type, fmt=rs.format.motion_xyz32f, desired_fps=None):
        cand = [p for p in profiles if p.stream_type() == stream_type and p.format() == fmt]
        if not cand:
            return None
        if desired_fps is None:
            return cand[0]
        return min(cand, key=lambda p: abs(p.fps() - desired_fps))

    profiles = motion_sensor.get_stream_profiles()
    gyro_prof  = pick_profile(profiles, rs.stream.gyro,  rs.format.motion_xyz32f, 200)
    accel_prof = pick_profile(profiles, rs.stream.accel, rs.format.motion_xyz32f, 63)
    if gyro_prof is None or accel_prof is None:
        print("[IMU] no suitable gyro/accel profiles")
        imu_available = False
        return

    print(f"[IMU] Selected profiles: Gyro {gyro_prof.fps()}Hz, Accel {accel_prof.fps()}Hz")

    latest_gyro = [0.0, 0.0, 0.0]
    latest_acc  = [0.0, 0.0, 0.0]
    have_gyro = False
    have_acc  = False

    def on_motion(frame):
        nonlocal latest_gyro, latest_acc, have_gyro, have_acc
        global imu_last_frame_ts
        if not frame.is_motion_frame():
            return
        md = frame.as_motion_frame().get_motion_data()
        st = frame.get_profile().stream_type()
        if st == rs.stream.gyro:
            latest_gyro = [md.x, md.y, md.z]
            have_gyro = True
        elif st == rs.stream.accel:
            latest_acc  = [md.x, md.y, md.z]
            have_acc = True
        imu_last_frame_ts = time.monotonic()

    try:
        motion_sensor.open([gyro_prof, accel_prof])
    except Exception as e:
        print("[IMU] sensor.open failed:", e)
        imu_available = False
        return

    try:
        motion_sensor.start(on_motion)
    except Exception as e:
        print("[IMU] sensor.start failed:", e)
        try:
            motion_sensor.close()
        except:
            pass
        imu_available = False
        return

    print("[IMU] streaming...")
    last_debug = 0.0
    prev_t = time.monotonic()

    try:
        while not stop_event.is_set():
            now = time.monotonic()
            dt = max(1e-4, now - prev_t)
            prev_t = now

            # 최근 1초 내 프레임 유무로 가용성 판단
            if (now - imu_last_frame_ts) < 1.0:
                if not imu_available:
                    print("[IMU] available = True")
                imu_available = True
            else:
                if imu_available:
                    print("[IMU] available = False (no motion frames)")
                imu_available = False

            if imu_available and have_gyro and have_acc:
                gx, gy, gz = latest_gyro
                ax, ay, az = latest_acc
                gx *= GYRO_SCALE; gy *= GYRO_SCALE; gz *= GYRO_SCALE
                with imu_lock:
                    complementary_update((gx, gy, gz), (ax, ay, az), dt)

            if DEBUG_IMU and (now - last_debug) > 0.25:
                last_debug = now
                with imu_lock:
                    print(f"[IMU] yaw={imu_yaw:+.4f} pitch={imu_pitch:+.4f} roll={imu_roll:+.4f}")

            time.sleep(0.003)  # ~300Hz 루프

    finally:
        try:
            motion_sensor.stop()
        except:
            pass
        try:
            motion_sensor.close()
        except:
            pass
        print("[IMU] thread exit")

# ===== 보정식 (IMU Δ각 기반) =====
def apply_adjustment(cx, cy, cmd, w, h, depth_z=None):
    """
    IMU 기반 보정:
      - Δyaw, Δpitch = (현재 IMU) - (UART 시점 IMU)
      - 깊이 있으면 3D 회전 투영, 없으면 FOV 근사 쉬프트
    안전장치: IMU 미가용/기준 미설정/NaN/폭주 시 원좌표 폴백
    """
    if cx is None or cy is None or w <= 0 or h <= 0:
        return None, None

    # IMU 미가용이면 보정 OFF
    if not imu_available:
        if DEBUG_SIGN:
            print("[SIGN] IMU unavailable -> no correction")
        return int(cx), int(cy)

    # UART 기준이 아직 없다면 보정 없이 반환
    if last_uart_ts == 0.0:
        if DEBUG_SIGN:
            print("[SIGN] baseline not set yet (last_uart_ts==0)")
        return int(cx), int(cy)

    # Δ각 (라디안) + 부호 토글 + age
    with imu_lock:
        dyaw_raw   = _wrap_pi(imu_yaw   - last_uart_yaw)
        dpitch_raw = _wrap_pi(imu_pitch - last_uart_pitch)
        now = time.monotonic()
        age = now - last_uart_ts

    dyaw   = SIGN_YAW_IMU   * dyaw_raw
    dpitch = SIGN_PITCH_IMU * dpitch_raw

    if DEBUG_SIGN:
        print(f"[SIGN] age={age:.3f}s dyaw={dyaw:+.4f} rad, dpitch={dpitch:+.4f} rad (cmd={last_uart_cmd})")

    # ---------- 1) 깊이 기반 3D 투영 ----------
    if depth_z is not None and depth_z > 0.0 and np.isfinite(depth_z):
        # fx, fy 계산: FOV가 비정상(0/NaN)일 경우 폴백
        if not (np.isfinite(FOV_X_RAD) and FOV_X_RAD > 1e-6 and
                np.isfinite(FOV_Y_RAD) and FOV_Y_RAD > 1e-6):
            return int(cx), int(cy)

        fx = (w/2.0) / np.tan(FOV_X_RAD/2.0)
        fy = (h/2.0) / np.tan(FOV_Y_RAD/2.0)
        if not (np.isfinite(fx) and np.isfinite(fy) and fx > 0 and fy > 0):
            return int(cx), int(cy)

        K  = np.array([[fx, 0, w/2.0],
                       [0, fy, h/2.0],
                       [0,  0,   1.0]], dtype=float)
        try:
            Kinv = np.linalg.inv(K)
        except np.linalg.LinAlgError:
            return int(cx), int(cy)

        hp1  = np.array([cx, cy, 1.0], dtype=float)
        if not np.all(np.isfinite(hp1)):
            return int(cx), int(cy)

        ray  = Kinv @ hp1
        if not np.all(np.isfinite(ray)) or abs(ray[2]) < 1e-6:
            return int(cx), int(cy)

        X    = ray * (depth_z / ray[2])
        if not np.all(np.isfinite(X)):
            return int(cx), int(cy)

        # 회전 행렬
        cyw, syw = np.cos(dyaw),   np.sin(dyaw)
        cpt, spt = np.cos(dpitch), np.sin(dpitch)
        R_yaw   = np.array([[ cyw, 0, syw],
                            [ 0,   1,  0 ],
                            [-syw, 0, cyw]], dtype=float)
        R_pitch = np.array([[1,  0,   0 ],
                            [0, cpt,-spt],
                            [0, spt, cpt]], dtype=float)
        R = R_yaw @ R_pitch
        if not np.all(np.isfinite(R)):
            return int(cx), int(cy)

        Xp = R @ X
        if not np.all(np.isfinite(Xp)):
            return int(cx), int(cy)

        hp2 = K @ Xp
        if not np.all(np.isfinite(hp2)) or abs(hp2[2]) < 1e-6:
            return int(cx), int(cy)

        u2 = hp2[0] / hp2[2]
        v2 = hp2[1] / hp2[2]
        if not (np.isfinite(u2) and np.isfinite(v2)):
            return int(cx), int(cy)

        u2_i = int(max(0, min(w-1, round(u2))))
        v2_i = int(max(0, min(h-1, round(v2))))

        if DEBUG_SIGN:
            print(f"[PROJ] u2={u2:.1f}, v2={v2:.1f} -> ({u2_i},{v2_i}) from ({cx},{cy}), z={depth_z:.3f}")

        return u2_i, v2_i

    # ---------- 2) FOV 근사 쉬프트 (깊이 없음) ----------
    if not (np.isfinite(FOV_X_RAD) and FOV_X_RAD > 1e-6 and
            np.isfinite(FOV_Y_RAD) and FOV_Y_RAD > 1e-6):
        return int(cx), int(cy)

    shift_x = SIGN_YAW_TO_PIX   * dyaw   * (w / FOV_X_RAD)
    shift_y = SIGN_PITCH_TO_PIX * dpitch * (h / FOV_Y_RAD)

    if not (np.isfinite(shift_x) and np.isfinite(shift_y)):
        return int(cx), int(cy)

    u2 = int(max(0, min(w-1, round(cx + shift_x))))
    v2 = int(max(0, min(h-1, round(cy + shift_y))))

    if DEBUG_SIGN:
        print(f"[SHIFT] shift_x={shift_x:+.2f}px, shift_y={shift_y:+.2f}px -> ({u2},{v2}) from ({cx},{cy})")

    return u2, v2

# ===== 소켓 보조 =====
def send_loop(sock: socket.socket, stop_event: threading.Event, name: str, interval: float = sec) -> None:
    """
    소켓 전송 포맷:
      "[QTCLIENT]adj_x@adj_y@depth@label\n"
    - IMU 델타로 보정 좌표 계산(깊이 있으면 3D, 없으면 FOV 근사)
    - IMU 미가용/기준 미설정시는 원좌표
    """
    print("Send loop: sending adj_x@adj_y@depth@label (IMU-based)")
    next_ts = time.monotonic()

    try:
        while not stop_event.is_set():
            with frame_lock:
                cx    = latest_cx
                cy    = latest_cy
                depth = latest_depth
                w = WIDTH; h = HEIGHT
                label = latest_label or ""
            cmd = last_uart_cmd  # 참고만

            if (cx is not None) and (cy is not None) and label:
                depth_val = depth if _valid_depth(depth) else 0.0
                adj_x, adj_y = apply_adjustment(cx, cy, cmd, w, h, depth_val if depth_val>0 else None)
                if adj_x is None or adj_y is None:
                    adj_x, adj_y = int(cx), int(cy)  # 폴백

                payload = f"{adj_x}@{adj_y}@{depth_val:.3f}@{label}\n"
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

# ===== 추론 스레드 =====
def inference_thread():
    global latest_frame, latest_result, latest_depth, latest_is_drone, latest_label
    global latest_cx, latest_cy

    pipeline = rs.pipeline()
    config   = rs.config()
    if RS_SERIAL:
        config.enable_device(RS_SERIAL)

    config.enable_stream(rs.stream.color, WIDTH, HEIGHT, rs.format.bgr8, FPS)
    if RS_USE_DEPTH:
        config.enable_stream(rs.stream.depth, WIDTH, HEIGHT, rs.format.z16, FPS)

    profile = pipeline.start(config)
    align = rs.align(rs.stream.color) if (RS_USE_DEPTH and RS_ALIGN_DEPTH) else None

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

    names = model.names

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

            try:
                result = model(frame, conf=CONF_TH, verbose=False, device=device_str)[0]
            except Exception as e:
                print(f"[INF] YOLO inference error: {e}")
                result = None

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
                        z = float(depth_frame.get_distance(bx_c, by_c))

                    has_z = _valid_depth(z)
                    cand.append( (has_z, z if has_z else float('inf'), -c, -area, lbl, x1, y1, x2, y2, bx, by) )

            best = None
            if cand:
                has_any_valid_depth = any(item[0] for item in cand)
                if has_any_valid_depth:
                    subset = [it for it in cand if it[0]]
                    best = min(subset, key=lambda t: (t[1], t[2], t[3]))
                else:
                    best = min(cand, key=lambda t: (t[2], t[3]))

            vis = frame.copy()
            if best:
                has_z, z, nconf, narea, lbl, x1, y1, x2, y2, bx, by = best
                conf = -nconf
                color = depth_to_bgr(z if _valid_depth(z) else 0.0)

                cv2.rectangle(vis, (x1, y1), (x2, y2), color, 2)
                depth_text = f"Z={z:.3f}m" if _valid_depth(z) else "Z=?"
                cv2.putText(
                    vis, f"{lbl} {conf:.2f} {depth_text}",
                    (x1, max(0, y1 - 6)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2
                )
                # 중심 좌표 (노란 점만)
                cv2.circle(vis, (bx, by), 5, (0, 255, 255), -1)

                latest_result   = value_from_xy(bx, by, w, h) or "?"
                latest_is_drone = (lbl.lower() == "drone")
                latest_label    = lbl
                with frame_lock:
                    latest_cx    = int(bx)
                    latest_cy    = int(by)
                    latest_depth = float(z) if _valid_depth(z) else 0.0
            else:
                latest_result   = None
                latest_is_drone = False
                latest_label    = None
                with frame_lock:
                    latest_cx    = None
                    latest_cy    = None
                    latest_depth = None

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
    global last_uart_cmd, last_uart_yaw, last_uart_pitch, last_uart_ts, prev_move_cmd

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
            # --- depth 구간별 태그 결정 ---
            tag = "0"
            if depth is None or depth <= 0:
                tag = "0"    # 측정 불가
            elif 0.0 <= depth < 0.5:
                tag = "1"
            elif 0.5 <= depth < 0.8:
                tag = "2"
            elif 0.8 <= depth < 1.1:
                tag = "3"
            elif 1.1 <= depth < 1.4:
                tag = "4"
            else:
                tag = "5"
            
            payload = f"{str(cur)}@{tag}\n"
            
            try:
                ser.write(payload.encode("ascii"))
                print(payload, end="")

                cmd = (str(cur) or "").upper()[:1]
                now = time.monotonic()

                # ★ (A) 첫 전송 이후 '무조건' 1회 초기 스냅샷
                if last_uart_ts == 0.0:
                    with imu_lock:
                        last_uart_cmd   = cmd
                        last_uart_yaw   = imu_yaw
                        last_uart_pitch = imu_pitch
                        last_uart_ts    = now
                    prev_move_cmd = cmd
                    print(f"[SNAP-INIT] cmd={cmd} yaw={last_uart_yaw:+.4f} pitch={last_uart_pitch:+.4f}")

                # ★ (B) 그 다음부터는 '움직임 키'이고, 동일키 연속은 최소 간격 후에만
                elif (cmd in MOVING_KEYS) and ((prev_move_cmd != cmd) or (now - last_uart_ts > MIN_SNAPSHOT_INTERVAL)):
                    with imu_lock:
                        last_uart_cmd   = cmd
                        last_uart_yaw   = imu_yaw
                        last_uart_pitch = imu_pitch
                        last_uart_ts    = now
                    prev_move_cmd = cmd
                    print(f"[SNAP] cmd={cmd} yaw={last_uart_yaw:+.4f} pitch={last_uart_pitch:+.4f}")

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
    # IMU 스레드 시작 (저수준 Motion Module)
    th_imu = threading.Thread(target=imu_thread, name="imu", daemon=True)
    th_imu.start()

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

        # 순서 중요: 센서 사용하는 스레드부터 정상 종료 대기
        try:
            th_uart.join(timeout=2.0)
        except: pass
        try:
            th_inf.join(timeout=2.0)
        except: pass
        try:
            th_sock.join(timeout=2.0)   # ← 추가
        except: pass
        try:
            th_imu.join(timeout=3.0)    # ← ★ 추가: IMU 콜백 멈추고 장치 닫히도록
        except: pass

        print("[MAIN] Done.")