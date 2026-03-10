/****************************************************************************
 * arch/arm/src/samd2l2/bootloader/bootloader.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <assert.h>
#include <debug.h>
#include <stdbool.h>

#include "chip.h"
#include "arm_internal.h"
#include "ram_vectors.h"
#include "nvic.h"

#include "sam_config.h"

#include "sam_bootloader.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_LFS_SIZE
#  define CONFIG_LFS_SIZE 253952
#endif

#ifndef CONFIG_SECTION_START
#  define CONFIG_SECTION_START 0x00002000
#endif

#define IDLE_STACK ((uint32_t)_ebss + CONFIG_IDLETHREAD_STACKSIZE)

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static void start(void);

static inline void bootloader(void);
static inline uint32_t find_file_entry(const char* filename);
static inline bool compare_name(const uint32_t offset,
                                const char *name);
static inline void jump_to_kernel(const uint32_t entry_point);


/****************************************************************************
 * Public Data
 ****************************************************************************/

/* The v6m vector table consists of an array of function pointers, with the
 * first slot (vector zero) used to hold the initial stack pointer.
 *
 * As all exceptions (interrupts) are routed via exception_common, we just
 * need to fill this array with pointers to it.
 *
 * Note that the [ ... ] designated initializer is a GCC extension.
 */

const void * const _vectors[] locate_data(".vectors")
aligned_data(VECTAB_ALIGN) =
{
  /* Initial stack */

  (void *)IDLE_STACK,

  /* Reset exception handler */

  start,

  /* Vectors 2 - n point directly at the generic handler */

  [2 ... ARMV6M_PERIPHERAL_INTERRUPTS] = &exception_common
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: start
 *
 * Description:
 *   True entry point nedeed to prepare a registers.
 *
 ****************************************************************************/

static void start(void)
{
  /* Zero lr to mark the end of backtrace */

  asm volatile ("mov lr, %0\n\t"
  "bx      %1\n\t"
  :
  : "r"(0), "r"(__start));
}

/****************************************************************************
 * Name: bootloader
 *
 * Description:
 *   Find the nuttx entry point in the dedicated section in the littlefs
 *    and jump to it.
 *
 ****************************************************************************/

static inline void bootloader(void)
{
  uint32_t kernel_entry;

  kernel_entry = find_file_entry("nuttx.bin");

  if (kernel_entry)
  {
    jump_to_kernel(kernel_entry);
  }

  /* No kernel loaded into the flash */
}

/****************************************************************************
 * Name: find_file_entry
 *
 * Description:
 *   Find the nuttx entry point.
 *
 * Arguments:
 *   filename - the filename of nuttx binary e.g. nuttx.bin
 *
 * Returned value:
 *   0                - if there is no kernel
 *   pointer to entry - if it is OK
 *
 ****************************************************************************/

static inline uint32_t find_file_entry(const char* filename)
{
  uint32_t ret;
  uint32_t off;
  uint32_t next_off;

  ret = 0;
  off = 0;

  /* Go under all LFS blocks */
  while (off < CONFIG_LFS_SIZE)
  {
    uint32_t tag;
    uint32_t next_tag;
    uint8_t type;
    uint32_t size;

    tag = getreg32(off);
    next_tag = getreg32(off + 4);

    /* The note type:
     * MSB 0 - write,
     * MSB 1 - continue.
     */
    type = (tag >> 24) & 0x7F;
    size = tag & 0xFFF;


    /* If it is a directory */
    if ((tag & 0x80000000) == 0 && type == 0x02)
    {
      /* Entry structure:
       * [CTZ head]
       * [CTZ size]
       * [File name]
       */

      /* Continue tag and next_tag */
      uint32_t entry_off = off + 8;
      /* After CTZ head (4) and CTZ size (4) */
      uint32_t name_off = entry_off + 8;

      /* Compare the filename */
      if (compare_name(name_off, filename) == 0)
      {
        /* We have foud it */
        uint32_t ctz_head;
        uint32_t entry_point;

        /* The fisrt block of the file */
        ctz_head = getreg32(entry_off);

        /* A data begining with first blok for nuttx.bin file
         * Entry point offset __start = 0x2000 (flash.ld script)
         */
        entry_point = CONFIG_SECTION_START + (ctz_head * 256);

        /* Check */
        if ((*(uint16_t*)entry_point & 0xFF00) == 0x2100)
        {
          ret = entry_point;
          break;
        }
      }
    }

    /* Next tag run */
    if (next_tag & 0x80000000)
    {
      /* Align for 4 */
      off += 8 + ((size + 3) & ~3);
    }
    else
    {
      /* Next hop by next pointer */
      next_off = next_tag & 0x00FFFFFF;

      /* The end of the chain */
      if (next_off == 0) break;

      off = next_off * 256;
    }
  }

  return ret;
}

/****************************************************************************
 * Name: compare_name
 *
 * Description:
 *   Compare the data under offset with filename
 *
 * Arguments:
 *   offset - the flash memory offset
 *   name   - the filenames
 *
 * Returned value:
 *   0 - if there are no hit
 *   1 - if there are hit
 *
 ****************************************************************************/

static inline bool compare_name(const uint32_t offset,
                                const char *name)
{
  uint32_t i;

  i = 0;

  while (name[i])
  {
    if (getreg8(offset + i) != name[i]) return false;
  }

  return ((getreg8(offset + i) == 0) ? true : false);
}

/****************************************************************************
 * Name: find_file_entry
 *
 * Description:
 *   Find the nuttx entry point.
 *
 * Arguments:
 *   entry_point - the entry point to nuttx.bin
 *
 ****************************************************************************/

static inline void jump_to_kernel(const uint32_t entry_point)
{
  /* Direct jump using assembly to ensure no compiler interference.
   * This is the most reliable way to transfer control.
   */

  __asm volatile
  (
    "bx %0\n"          /* Branch and exchange to entry point */
    :
    : "r"(entry_point)
    : "memory"         /* Memory clobber - everything may change */
  );

  /* Never reached */
  while (1);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: __start
 *
 * Description:
 *   This is the reset entry point.
 *
 ****************************************************************************/

void __start(void)
{
  const uint32_t *src;
  uint32_t *dest;

  /* Disable all interrupts */
  putreg32(0xFFFFFFFF, ARMV6M_NVIC_ICER);

  /* Initialize the bss section */
  for (dest = (uint32_t *)_sbss; dest < (uint32_t *)_ebss; )
  {
    *dest++ = 0;
  }

  /* Initialize the data section */
  for (src = (const uint32_t *)_eronly,
    dest = (uint32_t *)_sdata; dest < (uint32_t *)_edata;
  )
  {
    *dest++ = *src++;
  }

  bootloader();

  /* Will never be executed */
  while (1);
}

/****************************************************************************
 * Name: exception_common
 *
 * Description:
 *   Defautl dummy handler for unexpected interrupts.
 *
 ****************************************************************************/

void exception_common(void)
{
  while (1);
}
