do_deploy() {
    install -m 755 ${D}/share/opensbi/*/${RISCV_SBI_PLAT}/firmware/fw_payload.* ${DEPLOYDIR}/
}

EXTRA_OEMAKE:append:cv32a6-genesys2 = " PLATFORM_RISCV_ISA=rv32ima PLATFORM_RISCV_XLEN=32"
