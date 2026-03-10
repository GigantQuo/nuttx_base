############################################################################
# apps/industry/apc3/Makefile
############################################################################

include $(APPDIR)/Make.defs

# NSH Library

PROGNAME  = $(CONFIG_INDUSTRY_APC3_PROGNAME)
PRIORITY  = $(CONFIG_INDUSTRY_APC3_PRIORITY)
STACKSIZE = $(CONFIG_INDUSTRY_APC3_STACKSIZE)
MODULE    = $(CONFIG_INDUSTRY_APC3)

# APC3 Source Files
CSRCS   = apc3_main.c

ifeq ($(CONFIG_INDUSTRY_APC3_SYSTEM_0),y)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/industry/apc3/system_0
CSRCS += system_0/system_0_main.c
endif # CONFIG_INDUSTRY_APC3_SYSTEM_0

ifeq ($(CONFIG_INDUSTRY_APC3_SYSTEM_1),y)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/industry/apc3/system_1
CSRCS += system_1/system_1_main.c
endif # CONFIG_INDUSTRY_APC3_SYSTEM_1

ifeq ($(CONFIG_INDUSTRY_APC3_SYSTEM_2),y)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/industry/apc3/system_2
CSRCS += system_2/system_2_main.c
CSRCS += system_2/system_2_alarm_thrd.c
CSRCS += system_2/system_2_gphw.c
CSRCS += system_2/system_2_define_PU.c
CSRCS += system_2/system_2_shutdown.c
CSRCS += system_2/system_2_pwrup.c
CSRCS += system_2/system_2_acc_script.c
CSRCS += system_2/system_2_main_script.c
CSRCS += system_2/system_2_indication.c
CSRCS += system_2/system_2_blocks.c
CSRCS += system_2/system_2_rstrq_handler.c
CSRCS += system_2/system_2_critical_section.c
endif # CONFIG_INDUSTRY_APC3_SYSTEM_2

ifeq ($(CONFIG_INDUSTRY_APC3_SYSTEM_3),y)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/industry/apc3/system_3
CSRCS += system_3/system_3_main.c
CSRCS += system_3/system_3_signal_handler.c
endif # CONFIG_INDUSTRY_APC3_SYSTEM_3

ifeq ($(CONFIG_INDUSTRY_APC3_SYSTEM_4),y)
CFLAGS += ${INCDIR_PREFIX}$(APPDIR)/industry/apc3/system_4
CSRCS += system_4/system_4_main.c
CSRCS += system_4/system_4_stby_handler.c
endif # CONFIG_INDUSTRY_APC3_SYSTEM_4

include $(APPDIR)/Application.mk
