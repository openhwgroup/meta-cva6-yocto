FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://common_fpga.cfg"

SRC_URI:append:cv32a6-genesys2 = "\
    file://0001-cva6-genesys2-add-32-and-64-bits-dts.patch \
    file://0002-cva6-genesys2-added-Makefile-for-dtb-files.patch \
    file://0001-Incorporate-lowrisc-drivers-for-latest-kernel-releas.patch \
    file://genesys2.cfg \
    file://lowrisc-network.cfg \
"

SRC_URI:append:cv64a6-genesys2 = "\
    file://0001-cva6-genesys2-add-32-and-64-bits-dts.patch \
    file://0002-cva6-genesys2-added-Makefile-for-dtb-files.patch \
    file://0001-Incorporate-lowrisc-drivers-for-latest-kernel-releas.patch \
    file://genesys2.cfg \
    file://lowrisc-network.cfg \
"

SRC_URI:append:cv32a6-agilex7 = "\
    file://0002-cva6-agilex7-add-dts.patch \
    file://0003-cva6-agilex7-uart.patch \
    file://0004-cva6-agilex7-mmc.patch \
    file://agilex7.cfg \
"

SRC_URI:append:cv64a6-agilex7 = "\
    file://0002-cva6-agilex7-add-dts.patch \
    file://0003-cva6-agilex7-uart.patch \
    file://0004-cva6-agilex7-mmc.patch \
    file://agilex7.cfg \
"

KBUILD_DEFCONFIG:cv64a6-genesys2 = "defconfig"
KBUILD_DEFCONFIG:cv64a6-agilex7 = "defconfig"

KBUILD_DEFCONFIG:cv32a6-genesys2 = "rv32_defconfig"
KBUILD_DEFCONFIG:cv32a6-agilex7 = "rv32_defconfig"

KCONFIG_MODE = "--alldefconfig"

COMPATIBLE_MACHINE:append:cv64a6-genesys2 = "|cv64a6-genesys2"
COMPATIBLE_MACHINE:append:cv32a6-genesys2 = "|cv32a6-genesys2"

COMPATIBLE_MACHINE:append:cv32a6-agilex7 = "|cv32a6-agilex7"
COMPATIBLE_MACHINE:append:cv64a6-agilex7 = "|cv64a6-agilex7"
