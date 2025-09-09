Hailo App단 메뉴얼입니다.

로컬 PC Ubuntu에서 object_detection.py 예제 코드 생성
Hailo-Application-Code-Examples/runtime/hailo-8/python/object_detection/object_detection.py at main · hailo-ai/Hailo-Application-Code-Examples · GitHub
에 접속 후 git clone 진행

경로 
Hailo-Application-Code-Examples/runtime/hailo-8/python/object_detection/
object_detection.py, object_detection_post_process.py, config.json 
등등을 Custom해서 작업을 진행

Hailo-8 + RPI5 최신 버전으로 패키지 다운받기

hailo홈페이지에 접속하여 회원가입 후
https://hailo.ai/developer-zone/software-downloads/ 로 접속 
필요한 HailoRT 패키지와 device driver인 HailoRT PCIe driver등 3개를 다운로드를 Windows에서 다운을 진행 window powershell을 이용해서 scp 명령어로 windows에서 RPi5로 다운로드한 파일을 전송

scp <command_options> <linux_user>@<ip_addresss>:<directory> <내_윈도우_컴퓨터_디렉토리>

내 Rpi의 디렉토리 ~/Desktop/P/max6675라는 폴더 자체를 내 랩탑 바탕화면에 있는 test 폴더로 보내고 싶을 때

scp -r numakers@192.168.137.107:~/Desktop/P/max6675 C:\Users\김정빈\Desktop\test

파일을 윈도우에서 리눅스 서버측으로 보낼때
scp "C:\Downloads\hailort_4.22.0_arm64.deb"           pi@<PI_IP>:/home/pi/hailo_pkgs/
scp "C:\Downloads\hailort-pcie-driver_4.22.0_all.deb" pi@<PI_IP>:/home/pi/hailo_pkgs/
scp "C:\Downloads\hailort-4.22.0-cp310-cp310-linux_aarch64.whl" pi@<PI_IP>:/home/pi/hailo_pkgs/

기존 충돌 패키지 제거(있다면)
라즈베리파이 기본 저장소에 python3-hailort (4.20.0) 같은 오래된 패키지가 깔려 있으면 충돌 발생

sudo apt remove -y 'python3-hailort' 'hailort' 'hailo-dkms' || true
sudo dpkg -r hailort-pcie-driver || true
sudo modprobe -r hailo_pci || true

커널 헤더/DKMS 설치
sudo apt update
sudo apt install -y dkms raspberrypi-kernel-headers

HailoRT 사용자공간 라이브러리 설치(.deb)
sudo apt install -y ./hailort_4.22.0_arm64.deb

PCIe 드라이버 설치(.deb)
sudo apt install -y ./hailort-pcie-driver_4.22.0_all.deb

#DKMS가 자동으로 모듈 빌드 → 설치 로그가 나옵니다.

sudo depmod -a
sudo modprobe hailo_pci     # 모듈 수동 로드(다음 부팅부터는 자동)

설치/로딩 확인
lsmod | grep -i hailo
modinfo hailo_pci | egrep 'version|filename'

#→ version: 4.22.0 로 나와야 함

Python 바인딩 설치(.whl)
#(원하는 경로에 venv가 없다면 생성)

python3 -m venv ~/hailo-8/hailo-8/.venv

#활성화

source ~/hailo-8/hailo-8/.venv/bin/activate

#최신 pip로

pip install -U pip

#휠 설치

pip install ~/hailo_pkgs/hailort-4.22.0-cp310-cp310-linux_aarch64.whl

버전/동작 확인
#라이브러리 버전

hailo --version

#→ HailoRT v4.22.0

#드라이버·라이브러리 매칭(장치 연결 상태에서)

hailortcli fw-control identify

#Python 바인딩 간단 확인(venv 안)

python - <<'PY'
import hailo_platform.pyhailort as ph
print("VDevice ok:", hasattr(ph, "VDevice"))
PY

RPI5에 HailoRT 버전 4.20.0 으로 다운그레이드
1) 최초 증상 (예제 실행 시)
python -u object_detection.py -n ~/hailo-8/hailo-8/yolov8s.hef -i camera -l ~/hailo-8/hailo-8/labels_from_pt.txt -b 1 --show-fps -r hd

[HailoRT] [error] CHECK failed - Driver version (4.20.0) is different from library version (4.22.0)
[HailoRT] [error] Driver version mismatch, status HAILO_INVALID_DRIVER_VERSION(76)
...
hailo_platform.pyhailort.pyhailort.HailoRTException: ... (HAILO_INVALID_DRIVER_VERSION)

2) 진단시 필요 명령
(A) 파이썬 바인딩 버전/ 유무 확인

source ~/hailo-8/hailo-8/.venv/bin/activate
python -c "import importlib.metadata as m; print('hailo_platform =', m.version('hailo_platform'))"

결과
importlib.metadata.PackageNotFoundError: No package metadata was found for hailo_platform
venv 안에 바인딩이 없다는 의미(시스템 패키지에만 존재).

(B) 커널 모듈 로드 상태
lsmod | grep -i hailo || true
결과: hailo_pci 로드됨.

(C) 커널 모듈 버전/경로
modinfo hailo_pci 2>/dev/null | egrep '^(version|filename)' || \
modinfo hailo       2>/dev/null | egrep '^(version|filename)'

결과 예:
filename: /lib/modules/.../drivers/media/pci/hailo/hailo_pci.ko.xz
version:  4.20.0

(D) 커널/ DKMS 현황
uname -r
dkms status | grep -i hailo || true
결과: 커널 버전 표시, dkms는 처음엔 “command not found”.

(E) 장치 식별(여기도 버전 불일치 그대로 표출)
hailortcli fw-control identify
결과
[HailoRT] [error] CHECK failed - Driver version (4.20.0) is different from library version (4.22.0)
...

(F) apt 저장소에 있는 버전 확인
apt-cache policy hailort hailo-dkms python3-hailort | sed -n '1,200p'

당시 출력 요지:

hailort: Installed 4.22.0
python3-hailort: Installed 4.20.0-1
hailo-dkms: (none) / 후보는 4.19.0-1 등
즉 라이브러리(hailort) 4.22.0 vs 드라이버/파이썬 바인딩 4.20.0로 불일치.

3) 4.22.0로 맞추려다 실패했던 과정(네트워크/저장소 이슈)
3-1) 4.22.0으로 정렬 시도
sudo apt install -y 'hailo-dkms=4.22.0*' 'python3-hailort=4.22.0*' 'hailort=4.22.0*'
결과
E: Version '4.22.0*' for 'hailo-dkms' was not found
E: Version '4.22.0*' for 'python3-hailort' was not found

3-2) Hailo APT repo 추가했지만 DNS 불가
echo 'deb [trusted=yes] https://repository.hailo.ai/apt-repo/ stable main' | \
  sudo tee /etc/apt/sources.list.d/hailo.list
sudo apt update
결과
Could not resolve 'repository.hailo.ai'

3-3) DNS 점검 시도
getent hosts repository.hailo.ai || nslookup repository.hailo.ai 1.1.1.1
# nslookup 없어서
sudo apt install -y dnsutils
nslookup repository.hailo.ai 1.1.1.1
결과: no servers could be reached (네트워크/DNS 문제로 업데이트 실패)

→ 그래서 4.22.0 정렬 포기하고, 4.20.0으로 다운그레이드하는 쪽으로 선회.

4) 4.20.0 다운그레이드 때 썼던 명령들/오류
4-1) hailort만 4.20.0으로 내리기 시도
sudo apt install -y 'hailort=4.20.0-1'
오류
The following packages will be DOWNGRADED: hailort
E: Packages were downgraded and -y was used without --allow-downgrades.

4-2) 실제로 성공한 명령
sudo apt install -y --allow-downgrades 'hailort=4.20.0-1'
그 후 확인
hailo --version
# → HailoRT v4.20.0
(참고) python3-hailort는 이미 4.20.0-1 상태였고, 커널 모듈도 modinfo hailo_pci가 4.20.0이었기 때문에, 세 가지(드라이버/라이브러리/파이썬 바인딩)가 4.20.0으로 정렬됨.

5) venv 관련 이슈(패키징 특이점)
venv 내부에서는 hailo_platform가 안 보임 → 시스템 패키지를 보도록 새 venv 생성
deactivate 2>/dev/null || true
python3 -m venv --system-site-packages ~/hailo-8/hailo-8/.venv
source ~/hailo-8/hailo-8/.venv/bin/activate
import hailo_platform.pyhailort as ph 했을 때 hasattr(ph,"VDevice")가 False인데,
import hailo_platform.pyhailort._pyhailort as c; hasattr(c,"VDevice")는 True였던 “re-export 누락” 이슈가 있었음.
이를 우회하려고 .pth/sitecustomize로 _pyhailort 내용을 pyhailort에 재-내보내는(attach) 스니펫을 시도했으나,
.pth 로드 타이밍에 ModuleNotFoundError: No module named 'hailo_platform' 같은 부작용 로그가 잠깐 찍혔음(환경 로딩 순서 영향).

결론적으로는 시스템 패키지(=python3-hailort 4.20.0-1)가 보이는 venv에서 실행은 가능했어요.

6) 요약 체크리스트
증상: Driver 4.20.0 vs Library 4.22.0 불일치 → HAILO_INVALID_DRIVER_VERSION(76)\
확인
modinfo hailo_pci → 4.20.0
apt-cache policy → hailort 4.22.0 / python3-hailort 4.20.0-1
hailo --version → v4.22.0 (초기)
4.22.0 정렬 실패 이유: repository.hailo.ai DNS 불가
조치: sudo apt install --allow-downgrades 'hailort=4.20.0-1'
결과: hailo --version → v4.20.0 (드라이버/라이브러리/바인딩 모두 4.20으로 정렬)
venv: --system-site-packages 사용

Hailo-8+RPi5 로컬 HDMI

0) 증상 요약 (VNC + Wayland에서)
예제 실행(예: cv2.imshow 쓰는 스크립트) 시 터미널에 다음이 뜸:

qt.qpa.xcb: could not connect to display
qt.qpa.plugin: Could not load the Qt platform plugin "xcb" in ".../cv2/qt/plugins" even though it was found.
This application failed to start because no Qt platform plugin could be initialized.
Available platform plugins are: xcb.
Aborted
원인: 라즈베리파이 OS(Bookworm)는 기본이 Wayland인데, 우리가 띄우는 OpenCV/Qt GUI가 X11(xcb) 백엔드를 필요로 함. VNC(가상 세션) 환경에서는 DISPLAY 붙잡기가 잘 안됨.

1) HDMI 연결 + X11 세션으로 전환 (권장 경로)
1-1) Wayland → X11 전환
GUI에서:
sudo raspi-config → Advanced Options → Wayland → X11 선택 → 재부팅
(또는 GUI “Raspberry Pi Configuration” 앱에서 Wayland 끄고 X11로 전환해도 됨)
1-2) X11 기본 도구 설치 (테스트용)
sudo apt update
sudo apt install -y x11-apps xbitmaps mesa-utils libxcb-xinerama0
1-3) X11 작동 확인
모니터/키보드가 꽂힌 로컬 세션(HDMI) 에서 터미널 열고:
echo $XDG_SESSION_TYPE    # → x11 이어야 정상
xeyes &                   # 눈알 창 떠야 함
glxgears &                # 60FPS 근처로 프레임 나옴
종료: 해당 창에서 Ctrl+C 또는 창의 X 버튼
재시작 도중 “X connection to :0 broken (explicit kill or server shutdown)”는 정상적일 수 있음(디스플레이 매니저 재시작 시).
1-4) (원격 SSH에서 띄우려는 경우) 디스플레이 붙이기
로컬(HDMI) 세션에서 한 번:
xhost +local:         # 같은 머신의 로컬 프로세스에게 X 접근 허용
그 다음 SSH에서:
export DISPLAY=:0
export QT_X11_NO_MITSHM=1
unset WAYLAND_DISPLAY
# (옵션) OpenGL 간접렌더 강제 해제
export LIBGL_ALWAYS_INDIRECT=0

2) 우리가 코드에서 바꿨던 부분(멈춤/무응답 대응)
OpenCV 창을 풀스크린로 열면 라즈베리파이/Qt 조합에서 응답없음이 나와서 일반 윈도우로 전환했음.
# 예전 (문제): 풀스크린
# cv2.namedWindow("Output", cv2.WND_PROP_FULLSCREEN)
# cv2.setWindowProperty("Output", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
# 변경 (권장): 일반 윈도우 + 크기 지정
cv2.namedWindow("Output", cv2.WINDOW_NORMAL)
cv2.resizeWindow("Output", 1280, 720)
또한 렌더루프에서 반드시 매 프레임 cv2.waitKey(1) 호출이 있어야 이벤트루프가 돌면서 “응답없음”이 안 뜹니다.

3) 실행 예 (카메라 프리뷰/추론 없는 경로 확인)
# (SSH에서 띄우면 위의 DISPLAY / xhost 설정 먼저)
python3 object_detection.py \
  -n ~/hailo-8/hailo-8/yolov8s.hef \
  -i camera \
  -l ~/hailo-8/hailo-8/coco80_labels.txt \
  -b 1 --show-fps -r hd
-r 옵션은 sd|hd|fhd만 유효합니다.
(실수로 hd192 등 넣었을 때: invalid choice 에러 봤었음)
종료는 창 활성화 후 q 키.

4) 문제/에러와 해결 요약
4-1) qt.qpa.xcb: could not connect to display
원인: DISPLAY 못 붙음/Wayland 세션.
해결: X11로 전환(위 1-1), DISPLAY=:0, xhost +local:, QT_X11_NO_MITSHM=1 설정.
4-2) Could not load the Qt platform plugin "xcb"...
원인: X11 플러그인 의존 패키지/세션 문제.
해결: sudo apt install -y libxcb-xinerama0, X11 세션 보장.
4-3) 창이 [Not Responding] / 멈춤
원인: 풀스크린 + 이벤트 루프 미처리
해결: WINDOW_NORMAL + resizeWindow, 루프에서 cv2.waitKey(1) 유지.
4-4) argument -r/--resolution: invalid choice: 'hd192'
해결: -r는 sd|hd|fhd만 허용.
4-5) 카메라 장치 인덱스 에러
... open VIDEOIO(V4L2:/dev/video0): can't open camera by index
Camera index out of range
해결: 사용 가능한 인덱스 출력 코드로 확인(우리가 이미 도구 함수 넣어둠).
혹은 직접 확인:
v4l2-ctl --list-devices  # (v4l-utils 설치 필요: sudo apt install v4l-utils)
4-6) QObject::killTimer.../QObject::~QObject...
설명: Qt 타이머가 다른 스레드에서 정지될 때 나오는 경고. 종료 시점 로그로 무시 가능.

5) 세션/디스플레이 상태 점검에 썼던 명령들
# 지금 세션 타입 확인
echo $XDG_SESSION_TYPE         # 기대값: x11
# 어떤 디스플레이 서버가 떠 있는지
pgrep -af 'Xorg|Xwayland|wayfire|weston|lxsession|lightdm'
# 디스플레이 매니저 재시작 (로그인 화면까지 리셋)
sudo systemctl restart lightdm        # 또는: sudo systemctl restart display-manager
# X 테스트 앱
xeyes &
glxgears &
# SSH에서 X 붙이기(로컬에서 한 번)
xhost +local:
# SSH 세션에서
export DISPLAY=:0
export QT_X11_NO_MITSHM=1
unset WAYLAND_DISPLAY

6) 자주 쓰는 환경변수(한 번에 세팅)
cat <<'ENV' >> ~/.bashrc
# X11 attach helpers
export DISPLAY=:0
export QT_X11_NO_MITSHM=1
unset WAYLAND_DISPLAY
export LIBGL_ALWAYS_INDIRECT=0
ENV
source ~/.bashrc
이렇게 하면 “VNC + Wayland”에서 안 뜨던 OpenCV 창을 HDMI + X11 환경에서 안정적으로 띄우고, 추후 SSH에서도 디스플레이에 붙여 실행할 수 있어요.