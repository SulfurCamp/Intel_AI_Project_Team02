SUMMARY = "Jeong's Custom image for Socket Server"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit core-image

IMAGE_INSTALL += "\
    libgcc \
    openssh \
    openssh-sftp-server \
    openssl \
    socket-server \
"
