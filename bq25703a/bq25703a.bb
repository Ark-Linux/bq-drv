LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=3775480a712fc46a69647678acb234cb"

# FILESPATH =+ "${WORKSPACE}/:"
SRC_URI  = "file://bq25703a_drv/"


S = "${WORKDIR}/bq25703a_drv"
do_compile () {
  ${CC} gpio_config.c bq25703a_drv.c -o bq25703a_drv -lpthread
  
}

do_configure () {
}

