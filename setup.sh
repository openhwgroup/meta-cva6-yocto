#!/bin/bash
# Bootstrapper for buildbot slave

DIR="build"
MACHINE="cv32a6-genesys2"
CONFFILE="conf/auto.conf"
BITBAKEIMAGE="core-image-full-cmdline"

# make sure sstate is there
#echo "Creating sstate directory"
#mkdir -p ~/sstate/$MACHINE

# fix permissions set by buildbot
#echo "Fixing permissions for buildbot"
#umask -S u=rwx,g=rx,o=rx
#chmod -R 755 .

# Reconfigure dash on debian-like systems
which aptitude > /dev/null 2>&1
ret=$?
if [ "$(readlink /bin/sh)" = "dash" -a "$ret" = "0" ]; then
  sudo aptitude install expect -y
  expect -c 'spawn sudo dpkg-reconfigure -freadline dash; send "n\n"; interact;'
elif [ "${0##*/}" = "dash" ]; then
  echo "dash as default shell is not supported"
  return
fi
# bootstrap OE
echo "Init OE"
export BASH_SOURCE="openembedded-core/oe-init-build-env"
. ./openembedded-core/oe-init-build-env $DIR

# Symlink the cache
#echo "Setup symlink for sstate"
#ln -s ~/sstate/${MACHINE} sstate-cache

# add the missing layers
echo "Adding layers"
bitbake-layers add-layer ../meta-openembedded/meta-oe
bitbake-layers add-layer ../meta-openembedded/meta-python
bitbake-layers add-layer ../meta-openembedded/meta-multimedia
bitbake-layers add-layer ../meta-openembedded/meta-networking
bitbake-layers add-layer ../meta-riscv
bitbake-layers add-layer ../meta-cva6-yocto

# fix the configuration
echo "Creating auto.conf"

if [ -e $CONFFILE ]; then
    rm -rf $CONFFILE
fi
cat <<EOF > $CONFFILE
MACHINE ?= "${MACHINE}"
# rootfs for debugging
#IMAGE_GEN_DEBUGFS = "1"
#IMAGE_FSTYPES_DEBUGFS = "tar.gz"
EXTRA_IMAGE_FEATURES:append = " tools-debug"
EXTRA_IMAGE_FEATURES:append = " debug-tweaks"
EXTRA_IMAGE_FEATURES:append = " ssh-server-dropbear"
EXTRA_IMAGE_FEATURES:append = " package-management"
PACKAGECONFIG:append:pn-qemu-native = " sdl"
PACKAGECONFIG:append:pn-nativesdk-qemu = " sdl"
USER_CLASSES ?= "buildstats buildhistory buildstats-summary image-prelink"

require conf/distro/include/no-static-libs.inc
require conf/distro/include/yocto-uninative.inc
require conf/distro/include/security_flags.inc

INHERIT += "uninative"

DISTRO_FEATURES:append = " largefile multiarch pam "
#DISTRO_FEATURES_BACKFILL_CONSIDERED += "sysvinit"
#VIRTUAL-RUNTIME_init_manager = "systemd"
INIT_MANAGER = "mdev-busybox"
HOSTTOOLS_NONFATAL:append = " ssh"
SYSVINIT_ENABLED_GETTYS = ""
IMAGE_INSTALL:append = " vitetris"
IMAGE_INSTALL:append = " pacman4console"
EOF

echo "To build an image run"
echo "---------------------------------------------------"
echo "MACHINE=cv64a6-genesys2 bitbake core-image-minimal"
echo "---------------------------------------------------"
echo ""
echo "Buildable machine info"
echo "---------------------------------------------------"
echo "* cv64a6-genesys2: The OpenHW CV64A6 SoC on Genesys2"
echo "* cv32a6-genesys2: The OpenHW CV32A6 SoC on Genesys2"
echo "---------------------------------------------------"

# start build
#echo "Starting build"
#bitbake $BITBAKEIMAGE

