# meta-cva6-yocto

This README file contains information on the contents of the meta-cva6-yocto layer.

This layer is compatible with honister.

## Dependencies

This layer depends on:

* URI: https://github.com/openembedded/openembedded-core
  * branch: master
  * revision: HEAD
* URI: https://github.com/openembedded/bitbake
  * branch: master
  * revision: HEAD
* URI: https://github.com/riscv/meta-riscv
  * branch: master  
  * revision: HEAD

## System requirements

The system requirements for yocto are described [here](https://docs.yoctoproject.org/3.4/ref-manual/system-requirements.html).

## Adding the meta-cva6-yocto layer to your build

First install the repo tool
```text
mkdir ${HOME}/bin
curl https://storage.googleapis.com/git-repo-downloads/repo > ${HOME}/bin/repo
chmod a+x ${HOME}/bin/repo
PATH=${PATH}:~/bin
```

### Create workspace
```text
mkdir cva6-yocto && cd cva6-yocto
repo init -u https://github.com/openhwgroup/meta-cva6-yocto -b main -m tools/manifests/cva6-yocto.xml
repo sync
repo start work --all
```

### Setup Build Environment
```text
. ./meta-cva6-yocto/setup.sh
```

## Available Machines

This layer is available for:

* **cv64a6-genesys2**: The Digilent FPGA board with the bitstream generated from the [cva6 corev-apu](https://github.com/openhwgroup/cva6) with target default target *cv64a6_imafdc_sv39*.
* **cv32a6-genesys2**: Same board with the bitstream generated from the [cva6 corev-apu](https://github.com/openhwgroup/cva6) with target *cv32a6_ima_sv32_fpga*.
* **cv64a6-agilex7**: The [Altera FPGA board](https://www.altera.com/products/devkit/a1jui0000061rlpmay/agilex-7-fpga-f-series-development-kit-p-tile-and-e-tile-rev) with the bitstream generated from the [cva6 corev-apu](https://github.com/openhwgroup/cva6) with target default target *cv64a6_imafdc_sv39*.
* **cv32a6-agilex7**: Same board with the bitstream generated from the [cva6 corev-apu](https://github.com/openhwgroup/cva6) with target *cv32a6_ima_sv32_fpga*.
* **cv64a6-zcu111**: The AMD ZCU111 board with the validated CVA6 PL DDR4 bitstream. This machine is intended first for JTAG-loaded OpenSBI, Linux `Image`, DTB, and rootfs cpio validation, not SD-card boot.

## Build and flash your image

To generate a console-only image for the cv32a6-genesys2:
```text
MACHINE=cv32a6-genesys2 bitbake core-image-minimal
```
Image files will be located in build/tmp-glibc/deploy/images/cv32a6-genesys2.

To generate the ZCU111 validation image:

```text
MACHINE=cv64a6-zcu111 bitbake cva6-zcu111-image
```

Image files will be located in `build/tmp-glibc/deploy/images/cv64a6-zcu111`.
The first ZCU111 flow uses JTAG loading, matching the existing CVA6 FPGA tests:

```text
fw_jump.elf        -> 0x80000000
Image              -> 0x80200000
cv64a6_zcu111.dtb  -> 0x82200000
rootfs cpio.gz     -> loaded as initrd
```

The expected hardware validation commands after boot are:

```text
/root/mmio-test
/root/irq-test
/root/aes-gcm-test
```

### Flashing in Genesys II
Warning, you need to find the correct device image and fill it in the dd's of= parameter:

```text
gunzip -c build/tmp-glibc/deploy/images/cv32a6-genesys2/core-image-minimal-cv32a6-genesys2.wic.gz | sudo dd of=/dev/sd$ bs=1M iflag=fullblock oflag=direct conv=fsync status=progress
```
The login is "root", no password is needed.

### Flashing in Agilex7
```text
cd agilex7-sd
make format-sd SDDEVICE=/dev/sdX
make flash-sd  SDDEVICE=/dev/sdX MACHINE=cv32a6-agilex7
```

*NOTE: You may need to remove and insert the SD card between format and flash commands.

The login is "root", no password is needed.

## Yocto SDK

### SDK creation

To produce the SDK for your target, you need to launch the following command : 

```text
MACHINE=cv32a6-genesys2 bitbake core-image-minimal -c populate_sdk_ext
```

This will create the SDK Installer in build/tmp-glibc/deploy/sdk/oecore-x86_64-riscv32nf-toolchain-ext-nodistro.0.sh file.

To install it, simply run
```text
./build/tmp-glibc/deploy/sdk/oecore-x86_64-riscv32nf-toolchain-ext-nodistro.0.sh
```

You can install the SDK anywhere you want as long as you have writing rights.


### SDK usage

To use the SDK, you need to source the environment-setup file in the SDK install path :

```text
source $(SDK_INSTALL_PATH)/environment-setup-riscv32nf-oe-linux
```

Your environment variables should have changed (e.g. **PATH**, **CC**, **LD** and more). This allow you to manually build software.
The *devtool* command is also available. It will allow build, test, package an integrate software in the Yocto build system.

## Tests done

The core-image-minimal has been tested for the 32 and 64bits version of corev-apu with release [3ddf797](https://github.com/openhwgroup/cva6/tree/3ddf797e95923fd11113c8e443046105dfbf8843).

TFTP boot from u-boot is working.
