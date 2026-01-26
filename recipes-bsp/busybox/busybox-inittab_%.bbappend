SERIAL_CONSOLES:cv32a6-agilex7 := ""
SERIAL_CONSOLES:cv64a6-agilex7 := ""

# workarround for blocking /dev/ttyS0
do_install:append:cv32a6-agilex7 () {
        echo "console::respawn:/sbin/getty -L console 0 vt100" >> ${D}/${sysconfdir}/inittab
} 

do_install:append:cv64a6-agilex7 () {
        echo "console::respawn:/sbin/getty -L console 0 vt100" >> ${D}/${sysconfdir}/inittab
} 
