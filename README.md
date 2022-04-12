This README file contains information on the contents of the meta-cva6 layer.

Please see the corresponding sections below for details.

Dependencies
============

https://github.com/riscv/meta-riscv

Patches
=======

Please submit any patches against the meta-cva6 layer to the xxxx mailing list (xxxx@zzzz.org)
and cc: the maintainer:

Maintainer: Kevin Eyssartier <kevin.eyssartier@thalesgroup.com>

Table of Contents
=================

  I. Adding the meta-cva6 layer to your build
 II. Build and flash your image
III. Flashing to SD card
 IV. Tests done
  V. Todo


I. Adding the meta-cva6 layer to your build
===========================================

Run ```bitbake-layers add-layer meta-cva6```

II. Build and flash your image
==============================

Run 'MACHINE=cva6-genesys2 bitbake core-image-minimal'

Warning, you need to find the correct device image and fill it in the dd's of= parameter:

```gunzip -c build/tmp-glibc/deploy/images/cva6-genesys2/core-image-minimal-cva6-genesys2.wic.gz | sudo dd of=/dev/sd$ bs=1M iflag=fullblock oflag=direct conv=fsync```

The login is "root", no password is needed.

IV. Tests done
==============

The core-image-minimal has been tested on the cv64a6 genesys2 board with release [3ddf797](https://github.com/openhwgroup/cva6/tree/3ddf797e95923fd11113c8e443046105dfbf8843).

V. Todo
=======

- Correction to the u-boot low-risc. TFTP image loading is failing.
- Accelerate the Linux boot time. Suspicion of slow low-risc driver link availability, which blocks depending systemD services.
- Accelerate Linux login time.
