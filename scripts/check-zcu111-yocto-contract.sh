#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"

test -f "${ROOT}/conf/machine/cv64a6-zcu111.conf"
test -f "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111.dts"
test -f "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111/defconfig"
test -f "${ROOT}/recipes-images/cva6-zcu111/cva6-zcu111-image.bb"
test -f "${ROOT}/recipes-cva6/zcu111-tests/zcu111-tests.bb"

grep -Fq 'fw_jump' "${ROOT}/recipes-bsp/opensbi/opensbi_%.bbappend"
grep -Fq 'MACHINEOVERRIDES =. "cv64a6-zcu111:"' "${ROOT}/conf/machine/cv64a6-zcu111.conf"
grep -Fq 'KERNEL_IMAGETYPE = "Image"' "${ROOT}/conf/machine/cv64a6-zcu111.conf"
grep -Fq 'IMAGE_FSTYPES += "cpio.gz ext4"' "${ROOT}/conf/machine/cv64a6-zcu111.conf"
grep -Fq 'RISCV_SBI_FDT ?= "cv64a6_zcu111.dtb"' "${ROOT}/conf/machine/cv64a6-zcu111.conf"

grep -Fq 'model = "CVA6 ZCU111 DDR4";' "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111.dts"
grep -Fq 'memory@80000000' "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111.dts"
grep -Fq 'serial@10000000' "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111.dts"
grep -Fq 'pl-peripheral@50000000' "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111.dts"
grep -Fq 'aes-gcm@50001000' "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111.dts"

grep -Fq 'CONFIG_UIO=y' "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111/defconfig"
grep -Fq 'CONFIG_UIO_PDRV_GENIRQ=y' "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111/defconfig"
grep -Fq 'CONFIG_SERIAL_8250_CONSOLE=y' "${ROOT}/recipes-kernel/linux/linux-yocto/cv64a6-zcu111/defconfig"

echo "PASS: ZCU111 Yocto contract present"
