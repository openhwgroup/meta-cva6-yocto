require recipes-bsp/u-boot/u-boot-common.inc
require recipes-bsp/u-boot/u-boot.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI = "git://github.com/openhwgroup/u-boot.git;protocol=https;branch=cva6 \
           file://tftp-mmc-boot.txt \
           file://devtool-fragment.cfg \
           file://0004-CV64A6-add-ext4-and-fat-functionalities.patch \
           file://0005-CV64A6-add-fitImage-support.patch \
           file://0006-CV32A6-FIT-FAT-EXT4.patch \
           file://0007-cv32a6ima-cv32a6imac-cv64a6gc.patch \
           "
SRCREV = "d99b8954d516e1fb742b74b9519c52cc6863fe50"
SRC_URI[sha256sum] = "d8947969a5834b333afa2a8de9d079a74eb73675d3a7fe56e6a6edd5efae18f9"

DEPENDS:append = " u-boot-tools-native"

# Overwrite this for your server
TFTP_SERVER_IP ?= "127.0.0.1"

do_configure:prepend() {
    sed -i -e 's,@SERVERIP@,${TFTP_SERVER_IP},g' ${WORKDIR}/tftp-mmc-boot.txt
    mkimage -O linux -T script -C none -n "U-Boot boot script" \
        -d ${WORKDIR}/tftp-mmc-boot.txt ${WORKDIR}/${UBOOT_ENV_BINARY}
}

TOOLCHAIN = "gcc"
