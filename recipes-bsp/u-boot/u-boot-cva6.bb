require recipes-bsp/u-boot/u-boot-common.inc
require recipes-bsp/u-boot/u-boot.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI = "git://source.denx.de/u-boot/u-boot.git;protocol=https;branch=master; \
           file://0001-cva6-Add-Genesys-2-support.patch \
           file://0002-Support-agilex-13.patch \
           file://0003-cva6-agilex7-updated-Kconfig-for-current-u-boot.patch \
           file://0004-cva6-agilex7-moved-constants-from-code-to-config.patch \
           file://0005-configs-agilex7-disable-RISCV_ISA_F-for-32bit-defcon.patch \
          "

SRCREV = "34820924edbc4ec7803eb89d9852f4b870fa760a"

SRC_URI:append:cv32a6-genesys2 = "file://devtool-fragment.cfg"
SRC_URI:append:cv64a6-genesys2 = "file://devtool-fragment.cfg"

SRC_URI:append:cv32a6-agilex7 = "file://devtool-fragment-agilex.cfg \
                                 file://0008-agilex7_config.patch \
                                 "

SRC_URI:append:cv64a6-agilex7 = "file://devtool-fragment-agilex.cfg \
                                 file://0008-agilex7_config.patch \
                                 "

DEPENDS += "bc-native dtc-native gnutls-native python3-pyelftools-native u-boot-tools-native"

