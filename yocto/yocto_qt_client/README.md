# meta-drone (Raspberry Pi 4 · Qt5/Wayland · Yocto kirkstone)
- 라즈베리파이 4용 Qt5 Wayland GUI 이미지를 Yocto로 빌드
- `mybundle` 레시피가 `/opt/mybundle/Intel_AI_Qt/.` 경로로 어플리케이션을 설치한다.
- QSQLITE를 사용하도록 Qt/SQLite 구성도 포함한다.

# 레이어 추가
```bash
mkdir -p ~/yocto_ws && cd ~/yocto_ws
```
```bash
git clone -b kirkstone https://git.yoctoproject.org/poky
git clone -b kirkstone https://git.openembedded.org/meta-openembedded
git clone -b kirkstone https://github.com/meta-qt5/meta-qt5.git
git clone -b kirkstone https://github.com/agherzan/meta-raspberrypi.git
```

# `meta-drone/conf/layer.conf`

```bash
# 이 레이어가 검색 대상임을 BBPATH에 추가
BBPATH .= ":${LAYERDIR}"

# 이 레이어가 제공하는 레시피 검색 패턴
# recipes-*/<하위>/파일.bb, .bbappend 를 모두 포함
BBFILES += "${LAYERDIR}/recipes-*/*/*.bb ${LAYERDIR}/recipes-*/*/*.bbappend"

# 컬렉션명 등록과 우선순위 설정
BBFILE_COLLECTIONS += "meta-drone"
BBFILE_PATTERN_meta-drone = "^${LAYERDIR}/"
BBFILE_PRIORITY_meta-drone = "10"          # 숫자 클수록 우선

# 지원하는 Yocto 시리즈(브랜치) 선언
LAYERSERIES_COMPAT_meta-drone = "kirkstone"
```

# `poky/build/conf/local.conf`

```bash
# 타깃 머신(RPi4 32-bit). 64-bit면 raspberrypi4-64 사용
MACHINE = "raspberrypi4"

# VC4 KMS 드라이버 사용
RPI_USE_VC4GRAPHICS = "1"
GPU_MEM = "128"

# 배포 기능: Wayland, PulseAudio, WiFi, BT 활성화
DISTRO_FEATURES:append = " wayland pulseaudio wifi bluetooth"

# 이미지 추가 기능: SSH 서버와 디버그 편의 기능
EXTRA_IMAGE_FEATURES += " ssh-server-openssh"
EXTRA_IMAGE_FEATURES += " debug-tweaks"

# config.txt 추가 항목 (meta-raspberrypi가 반영)
RPI_EXTRA_CONFIG += "gpu_mem=128\n"
RPI_EXTRA_CONFIG += "dtoverlay=vc4-kms-dsi-7inch\n"
RPI_EXTRA_CONFIG += "dtoverlay=vc4-kms-v3d\n"
RPI_EXTRA_CONFIG += "dtparam=i2c_arm=on\n"

# 중복 가능성: 위에서 이미 wayland/opengl 추가됨
DISTRO_FEATURES:append = " wayland opengl"

# QtBase 빌드 시 백엔드 선택
# Wayland EGL, EGLFS, KMS 사용. X11은 제거
PACKAGECONFIG:append:pn-qtbase = " wayland-egl eglfs kms"
PACKAGECONFIG:remove:pn-qtbase = " xcb"
```

# `poky/build/conf/bblayers.conf`

```bash
# POKY_BBLAYERS_CONF_VERSION is increased each time build/conf/bblayers.conf
# changes incompatibly
# bblayers.conf 포맷 버전
POKY_BBLAYERS_CONF_VERSION = "2"

# 최상위 디렉터리
BBPATH = "${TOPDIR}"
BBFILES ?= ""

# 사용 레이어 목록. 경로는 절대경로 추천
BBLAYERS ?= " \
  /home/ubuntu/yocto_ws/poky/meta \
  /home/ubuntu/yocto_ws/poky/meta-poky \
  /home/ubuntu/yocto_ws/poky/meta-yocto-bsp \
  /home/ubuntu/yocto_ws/meta-openembedded/meta-oe \
  /home/ubuntu/yocto_ws/meta-openembedded/meta-networking \
  /home/ubuntu/yocto_ws/meta-openembedded/meta-python \
  /home/ubuntu/yocto_ws/meta-openembedded/meta-multimedia \
  /home/ubuntu/yocto_ws/meta-qt5 \
  /home/ubuntu/yocto_ws/meta-raspberrypi \
  /home/ubuntu/yocto_ws/meta-drone \
  "

```

# `mybundle.bb`

```bash
SUMMARY = "Bundle AiotClient (no host blobs)"
LICENSE = "MIT"
# 라이선스 파일 체크섬. MIT 공통 라이선스 사용
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# 소스는 레이어 내 files 디렉터리(로컬 복사물). 폴더 통째로 포함
SRC_URI = "file://mydir/"
S = "${WORKDIR}/mydir"

# 빌드 타임 도구 의존성: chrpath-native(호스트에서 실행)
DEPENDS += "chrpath-native"

# 런타임 의존성: 타깃에서 실행 시 필요한 패키지들
RDEPENDS:${PN} += "qtbase qtbase-plugins qtwayland qtwayland-plugins qtmultimedia qtsvg sqlite3"
RDEPENDS:${PN} += "qtdatavis3d"

# chrpath 클래스 상속: RPATH 수정 등의 QA 대응용
inherit chrpath

do_install() {
    appdir="${D}/opt/mybundle/Intel_AI_Qt"
    srcdir="${S}/Intel_AI_Qt"

    # 설치 대상 디렉터리 생성
    install -d "${appdir}"

    # 메인 실행파일 설치(있을 때만)
    [ -f "${srcdir}/AiotClient" ] && install -m 0755 "${srcdir}/AiotClient" "${appdir}/"

    # 리소스 복사(소유권 유지하지 않음)
    [ -d "${srcdir}/Images" ] && cp -R --no-preserve=ownership "${srcdir}/Images" "${appdir}/"
    [ -d "${srcdir}/mp3" ]    && cp -R --no-preserve=ownership "${srcdir}/mp3"    "${appdir}/"
    [ -f "${srcdir}/aiot.db" ] && install -m 0644 "${srcdir}/aiot.db" "${appdir}/"

    # 필요 없는 디렉터리 제거
    rm -rf "${appdir}/lib" "${appdir}/plugins" "${appdir}/AioClient_AppDir" "${appdir}/build" || true

    # 권한 및 소유권 정리
    chown -R root:root "${D}/opt/mybundle"
    find "${D}/opt/mybundle" -type d -exec chmod 0755 {} \;
    find "${D}/opt/mybundle" -type f -exec chmod 0644 {} \;
    [ -f "${appdir}/AiotClient" ] && chmod 0755 "${appdir}/AiotClient"
}

# 패키지에 포함할 파일 경로 지정
FILES:${PN} += "/opt/mybundle/Intel_AI_Qt/*"

# 권장 추가:
# - 실행 중 라이브러리 검색 문제 시 qt.conf 또는 QT_PLUGIN_PATH 준비
# - chrpath로 실행파일의 RPATH 점검 필요 시:
#   do_install:append() { chrpath -d ${appdir}/AiotClient || true; }
```

# `drone-qt5-image.bb`

```bash
SUMMARY = "RPi4 Qt5 Wayland image for Drone GUI"
LICENSE = "MIT"

# 코어 이미지 기능 상속
inherit core-image

# core-image-weston 기본 구성 포함
require recipes-graphics/images/core-image-weston.bb

# 배포 기능 추가(이미지 전반에 적용)
DISTRO_FEATURES:append = " wayland opengl pulseaudio"

# 이미지에 포함할 패키지들
IMAGE_INSTALL:append = " \
  weston weston-init libinput evtest \
  qtbase qtbase-plugins qtdeclarative qtquickcontrols2 \
  qtwayland qtwayland-plugins qtmultimedia qtsvg \
  sqlite3 \
  gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
  alsa-utils pulseaudio \
  connman connman-client wpa-supplicant linux-firmware-bcm43455 \
  tzdata chrony \
  qtdatavis3d \
  mybundle \
"
```

# `qtbase_%.bbappend`

```bash
# 모든 버전의 qtbase에 sqlite SQL 드라이버를 추가
PACKAGECONFIG:append:pn-qtbase = " sql-sqlite"
```

# 빌드
```bash
cd ~/yocto_ws/poky

source oe-init-build-env

bitbake drone-qt5-image

cd ./tmp/deploy/images/raspberrypi4

bzcat drone-qt5-image-raspberrypi4*.wic.bz2 | sudo dd of=/dev/sdc bs=4M status=progress conv=fsync
```

# 실행
```sh
./AiotClient -platform wayland
```