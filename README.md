# meta-cva6-yocto

This README file contains information on the contents of the meta-cva6-yocto layer.

This layer is compatible with scarthgap.

## Dependencies

This layer depends on:

* URI: https://git.openembedded.org/meta-openembedded
  * branch: scarthgap
  * revision: HEAD
* URI: https://git.yoctoproject.org/git/poky
  * branch: scrathgap
  * revision: HEAD

## System requirements

The system requirements for yocto are described [here](https://docs.yoctoproject.org/scarthgap/ref-manual/system-requirements.html).

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
repo init -u https://github.com/openhwgroup/meta-cva6-yocto -b scarthgap -m tools/manifests/cva6-yocto.xml
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

## Build and flash your image

To generate a console-only image for the cv32a6-genesys2:
```text
MACHINE=cv32a6-genesys2 bitbake core-image-minimal
```
Image files will be located in build/tmp/deploy/images/cv32a6-genesys2.

### Flashing in Genesys II
Warning, you need to find the correct device image and fill it in the dd's of= parameter:

```text
gunzip -c build/tmp/deploy/images/cv32a6-genesys2/core-image-minimal-cv32a6-genesys2.wic.gz | sudo dd of=/dev/sd$ bs=1M iflag=fullblock oflag=direct conv=fsync status=progress
```

Or use bmaptool if you have it:

```text
sudo bmaptool copy build/tmp/deploy/images/cv32a6-genesys2/core-image-minimal-cv32a6-genesys2.wic.gz /dev/sd$
```
The login is "root", no password is needed.

### Flashing in Agilex7
```text
gunzip -c build/tmp/deploy/images/cv32a6-agilex7/core-image-minimal-cv32a6-agilex7.wic.gz | sudo dd of=/dev/sd$ bs=1M iflag=fullblock oflag=direct conv=fsync status=progress
```

Or use bmaptool if you have it:

```text
sudo bmaptool copy build/tmp/deploy/images/cv32a6-agilex7/core-image-minimal-cv32a6-agilex7.wic.gz /dev/sd$
```
The login is "root", no password is needed.

## Yocto SDK

### SDK creation

To produce the SDK for your target, you need to launch the following command : 

```text
MACHINE=cv32a6-genesys2 bitbake core-image-minimal -c populate_sdk_ext
```

This will create the SDK Installer in build/tmp/deploy/sdk/poky-glibc-x86_64-core-image-minimal-riscv32nf-cv32a6-genesys2-toolchain-ext-5.0.11.sh file.

To install it, simply run
```text
./build/tmp/deploy/sdk/poky-glibc-x86_64-core-image-minimal-riscv32nf-cv32a6-genesys2-toolchain-ext-5.0.11.sh

```

You can install the SDK anywhere you want as long as you have writing rights.


### SDK usage

To use the SDK, you need to source the environment-setup file in the SDK install path :

```text
source $(SDK_INSTALL_PATH)/environment-setup-riscv32nf-poky-linux
```

Your environment variables should have changed (e.g. **PATH**, **CC**, **LD** and more). This allow you to manually build software.
The *devtool* command is also available. It will allow build, test, package an integrate software in the Yocto build system.

