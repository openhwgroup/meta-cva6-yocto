SRCREV_machine = "548da672c895a591adea1cab71ad57a80eb976c3"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI = "git://github.com/openhwgroup/linux;protocol=https;branch=linux-5.10.y;name=machine; \
           git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-5.10;destsuffix=${KMETA}"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

SRC_URI += "file://0001-cva6-genesys2-add-32-and-64-bits-dts.patch"

SRC_URI:append:cv32a6imac-genesys2 = " file://cv32a6-fragment.cfg \
                                   file://cv32a6/defconfig \
                                 "
SRC_URI:append:cv32a6ima-genesys2 = " file://cv32a6-fragment.cfg \
                                   file://cv32a6/defconfig \
                                 "
SRC_URI:append:cv64a6gc-genesys2 = " file://cv64a6/defconfig \
                                 "

COMPATIBLE_MACHINE:append:cv64a6gc-genesys2 = "|cv64a6gc-genesys2"
COMPATIBLE_MACHINE:append:cv32a6ima-genesys2 = "|cv32a6ima-genesys2"
COMPATIBLE_MACHINE:append:cv32a6imac-genesys2 = "|cv32a6imac-genesys2"
LINUX_VERSION = "5.10.7"
