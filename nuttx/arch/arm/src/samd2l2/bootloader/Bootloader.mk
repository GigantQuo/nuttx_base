############################################################################
# arch/arm/src/samd2l2/bootloader/Bootloader.mk
############################################################################

BOOTLOADER_INCLUDE_DIR = $(BOOTLOADER_DIR)
BOOTLOADER_BIN = $(call CONVERT_PATH,$(TOPDIR)$(DELIM)bootloader.bin)
BOOTLOADER_BIN_RAW = $(BOOTLOADER_BIN:.elf=.bin)

BOOTLOADER_SCRIPT_LOCAL = $(BOOTLOADER_DIR)$(DELIM)flash.ld

BOOTLOADER_CSRCS = $(BOOTLOADER_DIR)$(DELIM)sam_bootloader.c

BOOTLOADER_COBJS = $(BOOTLOADER_CSRCS:.c=$(OBJEXT))

BOOTLOADER_CFLAGS = $(filter-out -L% -l% -T%,$(CFLAGS)) \
                    ${INCDIR_PREFIX}$(BOOTLOADER_INCLUDE_DIR)
BOOTLOADER_CPPFLAGS = $(CPPFLAGS) -I$(TOPDIR)$(DELIM)include

BOOTLOADER_LDFLAGS = --print-memory-usage --entry=__start
BOOTLOADER_LDFLAGS += -nostdlib --gc-sections --cref
BOOTLOADER_LDFLAGS += -Map=$(TOPDIR)$(DELIM)bootloader.map

.PHONY: bootloader bootloader_clean

# Bootloader build rules
bootloader: $(BOOTLOADER_COBJS) $(BOOTLOADER_SCRIPT_LOCAL)
	$(Q) echo "LD: bootloader"
	$(Q) $(LD) $(BOOTLOADER_LDFLAGS) -T $(BOOTLOADER_SCRIPT_LOCAL) \
		-o $(BOOTLOADER_BIN) $(BOOTLOADER_COBJS)
	$(Q) echo "Bootloader built: $(BOOTLOADER_BIN)"

	$(Q) $(NM) $(BOOTLOADER_BIN) | \
	grep -v '\(compiled\)\|\(\$(OBJEXT)$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
	sort > $(TOPDIR)$(DELIM)System.bootloader.map


# Separate rules for C and assembly files
$(BOOTLOADER_COBJS): $(BOOTLOADER_DIR)$(DELIM)%$(OBJEXT): $(BOOTLOADER_DIR)$(DELIM)%.c
	$(Q) echo "CC: bootloader/$(notdir $<)"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) -c $(BOOTLOADER_CFLAGS) $(BOOTLOADER_CPPFLAGS) -o $@ $<

bootloader_clean:
	$(Q) echo "Cleaning bootloader..."
	$(Q) $(call DELFILE, $(BOOTLOADER_COBJS))
	$(Q) $(call DELFILE, $(BOOTLOADER_BIN))
	$(Q) $(call DELFILE, $(BOOTLOADER_BIN_RAW))
	$(Q) $(call DELFILE, $(TOPDIR)$(DELIM)bootloader.map)
	$(Q) $(call DELFILE, $(TOPDIR)$(DELIM)System.bootloader.map)

-include Make.dep
