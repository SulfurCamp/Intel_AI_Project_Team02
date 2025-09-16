SUMMARY = "RPi4 Qt5 Wayland image for Drone GUI"
LICENSE = "MIT"

inherit core-image
require recipes-graphics/images/core-image-weston.bb

DISTRO_FEATURES:append = " wayland opengl pulseaudio"

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
