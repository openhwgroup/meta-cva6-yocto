do_deploy() {
    for fw in fw_jump fw_payload; do
        for file in ${D}/share/opensbi/*/${RISCV_SBI_PLAT}/firmware/${fw}.*; do
            if [ -f "$file" ]; then
                install -m 755 "$file" ${DEPLOYDIR}/
            fi
        done
    done
}

EXTRA_OEMAKE:append:cv32a6-genesys2 = " PLATFORM_RISCV_ISA=rv32ima PLATFORM_RISCV_XLEN=32"
EXTRA_OEMAKE:append:cv32a6-agilex7 = " PLATFORM_RISCV_ISA=rv32ima PLATFORM_RISCV_XLEN=32"
EXTRA_OEMAKE:append:cv64a6-zcu111 = " FW_JUMP=y"
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-rename-ariane-to-cva6.patch \
            file://0001-Reverting-Makefile-Don-t-specify-mabi-or-march.patch \
            "
SRC_URI:append:cv32a6-agilex7 = "file://opensbi.patch"
SRC_URI:append:cv64a6-agilex7 = "file://opensbi.patch"
