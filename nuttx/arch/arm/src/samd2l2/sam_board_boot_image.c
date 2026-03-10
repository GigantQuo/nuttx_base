/****************************************************************************
 * arch/arm/src/samd2l2/sam_boardctl/sam_boot_image.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <debug.h>
#include <fcntl.h>

#include <sys/boardctl.h>
#include <nuttx/irq.h>
#include <nuttx/cache.h>
#include <arch/barriers.h>

#include "nvic.h"
#include "arm_internal.h"
#include "sam_config.h"

#ifdef CONFIG_BOARDCTL_BOOT_IMAGE
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure represents the first two entries on NVIC vector table */

struct arm_vector_table
{
  uint32_t spr;   /* Stack pointer on reset */
  uint32_t reset; /* Pointer to reset exception handler */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline void cleanup_arm_nvic(void);
static inline void systick_disable(void);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name:  cleanup_arm_nvic
 *
 * Description:
 *   Acknowledge and disable all interrupts in NVIC
 *
 * Input Parameters:
 *   None
 *
 *  Returned Value:
 *    None
 *
 ****************************************************************************/

static inline void cleanup_arm_nvic(void)
{
  int i;

  /* Deprecate the interrupts */
  irqstate_t flags = enter_critical_section();

  /* Deprecate all interrupts in NVIC */
  for (i = 0; i < SAM_IRQ_NVIC_COUNT; i++)
  {

    if (i < 32)
    {
      /* Disable irq */
      putreg32(1 << (i & 0x1f), NVIC_ICER(i));

      /* Clear pending */
      putreg32(1 << (i & 0x1f), NVIC_ICPR(i));
    }
  }

  /* Reset priorities */
  for (i = 0; i < (SAM_IRQ_NVIC_COUNT/4); i++)
  {
    putreg32(0, ARMV6M_NVIC_IPR(i));
  }

  leave_critical_section(flags);
}

/****************************************************************************
 * Name:  systick_disable
 *
 * Description:
 *   Disable the SysTick system timer
 *
 * Input Parameters:
 *   None
 *
 *  Returned Value:
 *    None
 *
 ****************************************************************************/

static inline void systick_disable(void)
{
  /* Disable SysTick */
  putreg32(0, ARMV6M_SYSTICK_CSR);

  /* Reset the reload value*/
  putreg32(0, ARMV6M_SYSTICK_RVR);

  /* Reset the current value */
  putreg32(0, ARMV6M_SYSTICK_CVR);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_boot_image
 *
 * Description:
 *   This entry point is called by bootloader to jump to application image.
 *
 ****************************************************************************/

int board_boot_image(const char *path, uint32_t hdr_size)
{
  static struct arm_vector_table vt;
  struct file file;
  ssize_t bytes;
  int ret;

  ret = file_open(&file, path, O_RDONLY | O_CLOEXEC);
  if (ret < 0)
  {
    _err("ERROR: BOOTLOADER: Failed to open %s with: %d",
         path,
         ret);
    return ret;
  }

  bytes = file_pread(&file, &vt, sizeof(vt), hdr_size);
  if (bytes != sizeof(vt))
  {
    _err("ERROR: BOOTLOADER: Failed to read ARM vector table: %d",
         bytes);
    return bytes < 0 ? bytes : -1;
  }

  systick_disable();

  cleanup_arm_nvic();

  /* Set main and process stack pointers */

  printf("Alive\n\r");

  __asm__ __volatile__
  (
    "msr msp, %0\n"
    "msr control, %1\n"
    "isb\n"
    "mov pc, %2\n"
    :
    : "r" (vt.spr), "r" (0), "r" (vt.reset)
  );

  return 0;
}

#endif /* CONFIG_BOARDCTL_BOOT_IMAGE */
