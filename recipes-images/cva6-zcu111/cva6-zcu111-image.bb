SUMMARY = "CVA6 ZCU111 validation image"
LICENSE = "MIT"

inherit core-image

IMAGE_FEATURES += "debug-tweaks"

ROOTFS_POSTPROCESS_COMMAND += "zcu111_root_password_setup; "

zcu111_root_password_setup() {
    sed -i 's#^root:[^:]*:#root:$6$jzdpfl79qOQhgzR.$dDcC3FQhzKXBH9484SWTf1QdS1QQLLVGPdcU/6NP4Rhqqw0grKeknEYTDgx.haiRmGzNr1Dy6BMvtf2NEfrGR1:#' ${IMAGE_ROOTFS}/etc/shadow
}

IMAGE_INSTALL:append = " \
    busybox \
    pacman4console \
    vitetris \
    zcu111-tests \
"
