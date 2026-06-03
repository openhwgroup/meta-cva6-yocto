SUMMARY = "CVA6 ZCU111 initramfs (bundled into the kernel Image)"
LICENSE = "MIT"

inherit core-image

# This image is an input to linux-yocto:do_bundle_initramfs. The machine-level
# OpenSBI dependency belongs to bootable images, not to this kernel input,
# otherwise BitBake creates opensbi -> kernel deploy -> initramfs -> opensbi.
EXTRA_IMAGEDEPENDS:remove = "opensbi"

# Passwordless root for bring-up.
IMAGE_FEATURES += "debug-tweaks"

# Same validation userland as the main image, runnable straight from RAM.
# This is a DEDICATED image (not cva6-zcu111-image) so it can be referenced by
# INITRAMFS_IMAGE without creating a circular dependency on the kernel.
IMAGE_INSTALL:append = " \
    busybox \
    pacman4console \
    vitetris \
    zcu111-tests \
"

# Produce a cpio for bundling into the kernel Image.
IMAGE_FSTYPES = "cpio.gz"
IMAGE_LINGUAS = ""

ROOTFS_POSTPROCESS_COMMAND += "zcu111_initramfs_console_setup; "

zcu111_initramfs_console_setup() {
    install -d ${IMAGE_ROOTFS}/dev

    if [ ! -e ${IMAGE_ROOTFS}/dev/console ]; then
        mknod -m 600 ${IMAGE_ROOTFS}/dev/console c 5 1
    fi
    if [ ! -e ${IMAGE_ROOTFS}/dev/null ]; then
        mknod -m 666 ${IMAGE_ROOTFS}/dev/null c 1 3
    fi
    if [ ! -e ${IMAGE_ROOTFS}/dev/ttyS0 ]; then
        mknod -m 660 ${IMAGE_ROOTFS}/dev/ttyS0 c 4 64
    fi

    sed -i '/^\/dev\/root[[:space:]]/d' ${IMAGE_ROOTFS}/etc/fstab

    if ! grep -q '^devtmpfs[[:space:]]\+/dev[[:space:]]' ${IMAGE_ROOTFS}/etc/fstab; then
        echo 'devtmpfs             /dev                 devtmpfs   mode=0755              0  0' >> ${IMAGE_ROOTFS}/etc/fstab
    fi

    sed -i '/^console::respawn:/d' ${IMAGE_ROOTFS}/etc/inittab
    sed -i '/^ttyS0::respawn:/d' ${IMAGE_ROOTFS}/etc/inittab
    echo 'ttyS0::respawn:/sbin/getty -L 115200 ttyS0 vt100' >> ${IMAGE_ROOTFS}/etc/inittab

    sed -i 's#^root:[^:]*:#root:$6$jzdpfl79qOQhgzR.$dDcC3FQhzKXBH9484SWTf1QdS1QQLLVGPdcU/6NP4Rhqqw0grKeknEYTDgx.haiRmGzNr1Dy6BMvtf2NEfrGR1:#' ${IMAGE_ROOTFS}/etc/shadow
}
