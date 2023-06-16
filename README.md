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

* cv64a6-genesys2: The Digilent FPGA board with the bitstream generated from the [cva6 corev-apu](https://github.com/openhwgroup/cva6) with target default target cv64a6_imafdc_sv39.
* cv32a6-genesys2: Same board with the bitstream generated from the [cva6 corev-apu](https://github.com/openhwgroup/cva6) with target cv32a6_ima_sv32_fpga.

## Build and flash your image

To generate a console-only image for the cv32a6-genesys2:
```text
MACHINE=cv32a6-genesys2 bitbake core-image-minimal
```
Image files will be located in build/tmp-glibc/deploy/images/cv32a6-genesys2.

Warning, you need to find the correct device image and fill it in the dd's of= parameter:

```text
gunzip -c build/tmp-glibc/deploy/images/cv32a6-genesys2/core-image-minimal-cv32a6-genesys2.wic.gz | sudo dd of=/dev/sd$ bs=1M iflag=fullblock oflag=direct conv=fsync status=progress
```

The login is "root", no password is needed.

## Tests done

The core-image-minimal has been tested for the 32 and 64bits version of corev-apu with release [3ddf797](https://github.com/openhwgroup/cva6/tree/3ddf797e95923fd11113c8e443046105dfbf8843).

TFTP boot from u-boot is working.
