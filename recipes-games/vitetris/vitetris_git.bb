LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://licence.txt;md5=811d34e4d649b52ab374744994338607"

SRC_URI = "git://github.com/vicgeralds/vitetris;protocol=https;branch=master \
           file://0001-Correct-install-command.patch \
           "

PV = "1.0+git${SRCPV}"
SRCREV = "d725f51fc626b386feb17da623241bf662f72b41"

W = "${WORKDIR}"
S = "${WORKDIR}/git"
B = "${WORKDIR}/build"
I = "${WORKDIR}/install"

do_configure () {
	CC="${CC}" ${S}/configure --prefix=${W}
}

do_compile () {
    make -C ${S} install
}

do_install () {
    mkdir -p ${D}/usr/local/bin
    install -m 0755 ${W}/bin/tetris  ${D}/usr/local/bin/tetris
    echo TOTO ${FILES_${PN}}
}

FILES:${PN} += "/usr"
