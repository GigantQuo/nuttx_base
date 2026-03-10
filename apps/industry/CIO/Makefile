############################################################################
# apps/industry/CIO/Makefile
############################################################################

include $(APPDIR)/Make.defs

# NSH Library

PROGNAME  = $(CONFIG_INDUSTRY_CIO_PROGNAME)
PRIORITY  = $(CONFIG_INDUSTRY_CIO_PRIORITY)
STACKSIZE = $(CONFIG_INDUSTRY_CIO_STACKSIZE)
MODULE    = $(CONFIG_INDUSTRY_CIO)

# APC3 Source Files
CSRCS   = CIO_main.c

CSRCS   += consoleIOForNuttx.c

include $(APPDIR)/Application.mk
