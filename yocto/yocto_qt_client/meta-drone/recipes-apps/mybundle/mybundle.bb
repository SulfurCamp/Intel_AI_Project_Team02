SUMMARY = "Bundle AiotClient (no host blobs)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://mydir/"
S = "${WORKDIR}/mydir"

DEPENDS += "chrpath-native"
RDEPENDS:${PN} += "qtbase qtbase-plugins qtwayland qtwayland-plugins qtmultimedia qtsvg sqlite3"
RDEPENDS:${PN} += "qtdatavis3d"
inherit chrpath

do_install() {
    appdir="${D}/opt/mybundle/Intel_AI_Qt"
    srcdir="${S}/Intel_AI_Qt"

    install -d "${appdir}"

    # 실행파일(ARM 빌드된 것만)
    [ -f "${srcdir}/AiotClient" ] && install -m 0755 "${srcdir}/AiotClient" "${appdir}/"

    # 데이터는 소유권 보존 없이 복사
    [ -d "${srcdir}/Images" ] && cp -R --no-preserve=ownership "${srcdir}/Images" "${appdir}/"
    [ -d "${srcdir}/mp3" ]    && cp -R --no-preserve=ownership "${srcdir}/mp3"    "${appdir}/"
    [ -f "${srcdir}/aiot.db" ] && install -m 0644 "${srcdir}/aiot.db" "${appdir}/"

    # 혹시 남아있을 수 있는 호스트 산출물 폴더는 방지
    rm -rf "${appdir}/lib" "${appdir}/plugins" "${appdir}/AioClient_AppDir" "${appdir}/build" || true

    # 소유권/퍼미션 표준화
    chown -R root:root "${D}/opt/mybundle"
    find "${D}/opt/mybundle" -type d -exec chmod 0755 {} \;
    find "${D}/opt/mybundle" -type f -exec chmod 0644 {} \;
    [ -f "${appdir}/AiotClient" ] && chmod 0755 "${appdir}/AiotClient"
}


FILES:${PN} += "/opt/mybundle/Intel_AI_Qt/*"
