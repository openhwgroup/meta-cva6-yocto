LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=751419260aa954499f7abaabaa882bbe"

SRC_URI = "git://github.com/YoctoForBeaglebone/pacman4console;protocol=https;branch=master \
           file://0001-Makfile-correction.patch \
           file://0001-Differentiate-build-path-and-execute-path.patch \
           "

PV = "1.0+git${SRCPV}"
SRCREV = "ddc229c3478b43b572cef4fc09bb1580f00a1e74"

S = "${WORKDIR}/git"
W = "${WORKDIR}"

DEPENDS = "ncurses"
RDEPENDS:${PN} = "ncurses-libncurses"

do_compile () {
	prefix=${W}/usr/local/ make install
}

do_install () {
    mkdir -p ${D}/usr/local/bin
    mkdir -p ${D}/usr/local/share/pacman/Levels
    install -m 0755 ${W}/usr/local/bin/*  ${D}/usr/local/bin/
    install -m 0644 ${W}/usr/local/share/pacman/Levels/*  ${D}/usr/local/share/pacman/Levels
}

FILES:${PN} += "/usr"
