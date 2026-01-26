LICENSE = "GPL-2.0-or-later"

inherit deploy

# TODO: This file should be built from source.
do_install() {
    install -m 755 ${THISDIR}/files/u-boot.itb ${D}/
}

do_deploy() {
    install -m 755 ${D}/* ${DEPLOYDIR}/
}

addtask deploy after do_install

FILES:${PN} := "u-boot.itb"
