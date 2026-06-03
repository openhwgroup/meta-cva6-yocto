SUMMARY = "CVA6 ZCU111 platform validation utilities"
LICENSE = "MIT & Apache-2.0"
LIC_FILES_CHKSUM = " \
    file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302 \
    file://coremark/LICENSE.md;md5=0a18b17ae63deaa8a595035f668aebe1 \
"

SRC_URI = " \
    file://mmio-test.c \
    file://irq-test.c \
    file://aes-gcm-test.c \
    file://memstress.c \
    file://run-reference-benchmarks \
    file://coremark/ \
    file://dhrystone/ \
"

S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -O2 -Wall -Wextra -o mmio-test mmio-test.c
    ${CC} ${CFLAGS} ${LDFLAGS} -O2 -Wall -Wextra -o irq-test irq-test.c
    ${CC} ${CFLAGS} ${LDFLAGS} -O2 -Wall -Wextra -o aes-gcm-test aes-gcm-test.c
    ${CC} ${CFLAGS} ${LDFLAGS} -O2 -g -Wall -Wextra -o memstress memstress.c

    ${CC} ${CFLAGS} ${LDFLAGS} -O3 \
        -Icoremark -Icoremark/posix \
        -DPERFORMANCE_RUN=1 \
        -DITERATIONS=0 \
        -DMULTITHREAD=1 -DUSE_PTHREAD=0 -DUSE_FORK=0 -DUSE_SOCKET=0 \
        -DFLAGS_STR='"-O3 -DPERFORMANCE_RUN=1 -DITERATIONS=0 -DMULTITHREAD=1 -DUSE_PTHREAD=0 -DUSE_FORK=0 -DUSE_SOCKET=0 -lrt"' \
        -o coremark-bin \
        coremark/core_main.c \
        coremark/core_list_join.c \
        coremark/core_matrix.c \
        coremark/core_state.c \
        coremark/core_util.c \
        coremark/posix/core_portme.c \
        -lrt

    cp dhrystone/dhry_1.c dhry_1.build.c
    sed -i '/extern char     \*malloc ();/d' dhry_1.build.c
    sed -i '/extern  int     times ();/d' dhry_1.build.c
    ${CC} ${CFLAGS} ${LDFLAGS} -std=gnu89 -O3 \
        -DHZ=100 -include string.h -include stdlib.h \
        -Idhrystone \
        -o dhrystone-bench \
        dhry_1.build.c \
        dhrystone/dhry_2.c
}

do_install() {
    install -d ${D}/root
    install -m 0755 mmio-test ${D}/root/mmio-test
    install -m 0755 irq-test ${D}/root/irq-test
    install -m 0755 aes-gcm-test ${D}/root/aes-gcm-test
    install -m 0755 memstress ${D}/root/memstress
    install -m 0755 coremark-bin ${D}/root/coremark
    install -m 0755 dhrystone-bench ${D}/root/dhrystone
    install -m 0755 run-reference-benchmarks ${D}/root/run-reference-benchmarks
}

FILES:${PN} += " \
    /root/mmio-test \
    /root/irq-test \
    /root/aes-gcm-test \
    /root/memstress \
    /root/coremark \
    /root/dhrystone \
    /root/run-reference-benchmarks \
"
