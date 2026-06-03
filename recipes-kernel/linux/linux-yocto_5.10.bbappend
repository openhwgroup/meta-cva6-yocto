SRCREV_machine:cv32a6-genesys2 ?= "548da672c895a591adea1cab71ad57a80eb976c3"
SRCREV_machine:cv64a6-genesys2 ?= "548da672c895a591adea1cab71ad57a80eb976c3"
SRCREV_machine:cv32a6-agilex7 ?= "548da672c895a591adea1cab71ad57a80eb976c3"
SRCREV_machine:cv64a6-agilex7 ?= "548da672c895a591adea1cab71ad57a80eb976c3"
SRCREV_machine:cv64a6-zcu111 ?= "548da672c895a591adea1cab71ad57a80eb976c3"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI = "git://github.com/openhwgroup/linux;protocol=https;branch=linux-5.10.y;name=machine; \
           git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-5.10;destsuffix=${KMETA}"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"


SRC_URI:append:cv32a6-genesys2 = " file://0001-cva6-genesys2-add-32-and-64-bits-dts.patch \
                                   file://cv32a6-fragment.cfg \
                                   file://cv32a6/defconfig \
                                 "
SRC_URI:append:cv64a6-genesys2 = " file://0001-cva6-genesys2-add-32-and-64-bits-dts.patch \
                                   file://cv64a6/defconfig \
                                 "
SRC_URI:append:cv32a6-agilex7 = " file://cv32a6-fragment.cfg \
                                  file://cv32a6-agilex7/defconfig \
                                  file://0002-cva6-agilex7-add-dts.patch \
                                  file://0003-cva6-agilex7-uart.patch \
                                  file://0004-cva6-agilex7-mmc.patch \
                                 "
SRC_URI:append:cv64a6-agilex7 = " file://cv64a6-agilex7/defconfig \
                                  file://0002-cva6-agilex7-add-dts.patch \
                                  file://0003-cva6-agilex7-uart.patch \
                                  file://0004-cva6-agilex7-mmc.patch \
                                 "
SRC_URI:append:cv64a6-zcu111 = " file://cv64a6-zcu111.dts \
                                  file://cv64a6-zcu111/defconfig \
                                "

COMPATIBLE_MACHINE:append:cv64a6-genesys2 = "|cv64a6-genesys2"
COMPATIBLE_MACHINE:append:cv32a6-genesys2 = "|cv32a6-genesys2"
COMPATIBLE_MACHINE:append:cv32a6-agilex7 = "|cv32a6-agilex7"
COMPATIBLE_MACHINE:append:cv64a6-agilex7 = "|cv64a6-agilex7"
COMPATIBLE_MACHINE:append:cv64a6-zcu111 = "|cv64a6-zcu111"
LINUX_VERSION = "5.10.7"

do_configure:append:cv64a6-zcu111() {
    install -d ${S}/arch/riscv/boot/dts/openhwgroup
    install -m 0644 ${WORKDIR}/cv64a6-zcu111.dts ${S}/arch/riscv/boot/dts/openhwgroup/cv64a6_zcu111.dts
    grep -Fq 'cv64a6_zcu111.dtb' ${S}/arch/riscv/boot/dts/openhwgroup/Makefile || \
        echo 'dtb-$(CONFIG_SOC_CVA6) += cv64a6_zcu111.dtb' >> ${S}/arch/riscv/boot/dts/openhwgroup/Makefile
}
