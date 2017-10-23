DESCRIPTION = "2mm from PolyBench"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://README.txt;md5=b448c56b7adbc0913280c4e8fec37940"
PR="r0"
SRC_URI = "file://2mm.tar.bz2;name=tarball"

S="${WORKDIR}"
#export CC="${STAGING_DIR_NATIVE}/usr/bin/aarch64-fsl-linux/aarch64-fsl-linux-gcc"

do_compile() {
	cd ${S}
	make
}

do_install() {
	mkdir -p ${D}/usr/
	mkdir -p ${D}/usr/sbin/
	cp ${S}/2mm ${D}/usr/sbin/
}
