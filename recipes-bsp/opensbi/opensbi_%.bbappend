EXTRA_OEMAKE:append:cv32a6-genesys2 = " PLATFORM_RISCV_ISA=rv32ima_zicsr_zifencei PLATFORM_RISCV_XLEN=32"
EXTRA_OEMAKE:append:cv32a6-agilex7 = " PLATFORM_RISCV_ISA=rv32ima_zicsr_zifencei PLATFORM_RISCV_XLEN=32"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append:cv32a6-agilex7 = " file://opensbi.patch"
SRC_URI:append:cv64a6-agilex7 = " file://opensbi.patch"
