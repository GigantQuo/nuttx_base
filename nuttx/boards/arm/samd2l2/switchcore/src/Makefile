############################################################################
# boards/arm/samd2l2/switchcore/src/Makefile
############################################################################

include $(TOPDIR)/Make.defs

CSRCS = sam_boot.c sam_bringup.c

ifeq ($(CONFIG_BOARDCTL),y)
CSRCS += sam_appinit.c
endif

ifeq ($(CONFIG_DEV_GPIO),y)
CSRCS += sam_gpio.c
endif

ifeq ($(CONFIG_I2C_DRIVER),y)
CSRCS += sam_i2c_bmaster.c
endif

ifeq ($(CONFIG_I2C_SLAVE_DRIVER),y)
CSRCS += sam_i2c_bslave.c
endif

ifeq ($(CONFIG_SENSORS_SHT4X),y)
CSRCS += sam_sht4x.c
endif

ifeq ($(CONFIG_SENSORS_LPS25H),y)
CSRCS += sam_lps22h.c
endif

ifeq ($(CONFIG_SENSORS_MAX662),y)
CSRCS += sam_max662.c
endif

ifeq ($(CONFIG_USBDEV),y)
CSRCS += sam_usb.c
endif

ifeq ($(CONFIG_SAMD2L2_WDT),y)
CSRCS += sam_watchdog.c
endif

ifeq ($(CONFIG_DEV_GPINP),y)
CSRCS += sam_bgpinp.c
endif

ifeq ($(CONFIG_DEV_GPINT),y)
CSRCS += sam_bgpint.c
endif

ifeq ($(CONFIG_DEV_GPOUT),y)
CSRCS += sam_bgpout.c
endif

include $(TOPDIR)/boards/Board.mk
