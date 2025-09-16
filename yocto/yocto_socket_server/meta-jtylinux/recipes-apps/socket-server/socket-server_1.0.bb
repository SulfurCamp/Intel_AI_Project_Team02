SUMMARY = "Jeong's Custom socket server binary"
LICENSE = "CLOSED"
SRC_URI = "file://socket-server file://idpasswd.txt"

S = "${WORKDIR}"

do_install() {
    install -d ${D}/home/root
    install -m 0755 ${WORKDIR}/socket-server ${D}/home/root/socket-server
    install -m 0644 ${WORKDIR}/idpasswd.txt ${D}/home/root/idpasswd.txt
}

FILES:${PN} += "/home/root/socket-server /home/root/idpasswd.txt"

