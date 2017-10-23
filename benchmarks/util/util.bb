DESCRIPTION = "utils"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://README.txt;md5=edc6f24823f445db0da9ce539827fa55"
PR="r0"
SRC_URI = "file://util.tar.bz2;name=tarball"

S="${WORKDIR}"
#export CC="${STAGING_DIR_NATIVE}/usr/bin/aarch64-fsl-linux/aarch64-fsl-linux-gcc"

do_compile() {
	cd ${S}
	make time_usleep
}

do_install() {
	mkdir -p ${D}/usr/
	mkdir -p ${D}/usr/sbin/
	cp ${S}/time_usleep ${D}/usr/sbin/
}
