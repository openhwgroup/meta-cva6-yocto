FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append:cv64a6gc-genesys2 = " file://cv64a6-genesys2-fragment.cfg"
SRC_URI:append:cv32a6ima-genesys2 = " file://cv32a6-genesys2-fragment.cfg"
SRC_URI:append:cv32a6imac-genesys2 = " file://cv32a6-genesys2-fragment.cfg"

