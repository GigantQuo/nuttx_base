/****************************************************************************
 * boards/arm/samd2l2/apc3/src/sam_bgpint.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>

#include <nuttx/config.h>
#include <arch/board/board.h>
#include <nuttx/board.h>

#include <nuttx/ioexpander/gpint.h>

#include "sam_pinmap.h"
#include "sam_gpint.h"
#include "sam_port.h"

#include "apc3.h"

#if defined(CONFIG_DEV_GPINT) && defined(CONFIG_SAMD2L2_GPINT)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

const uint32_t int_pull0[BOARD_NGPINT0] =
{
  BUTTON_1,
  BUTTON_2
};
const uint32_t irq_pull0[BOARD_NGPINT0] =
{
  BUTTON_1_EXTINT,
  BUTTON_2_EXTINT
};

const uint32_t int_pull1[BOARD_NGPINT1] =
{
  STANDBY
};
const uint32_t irq_pull1[BOARD_NGPINT1] =
{
  STANDBY_EXTINT
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpint_register
 *
 * Arguments:
 *  minor - number of registering driver
 *
 * Description:
 *   Register GPINT driver
 *
 ****************************************************************************/

void sam_gpint_register(unsigned int minor)
{
  struct gpint_dev_s *gpint;
  const uint32_t *ints[NINT_DRVRS];
  const uint32_t *irqs[NINT_DRVRS];
  uint32_t sizes[NINT_DRVRS];
  int ret;
  int i;

  ret = OK;

  ints[0] = int_pull0;
  ints[1] = int_pull1;

  irqs[0] = irq_pull0;
  irqs[1] = irq_pull1;

  sizes[0] = BOARD_NGPINT0;
  sizes[1] = BOARD_NGPINT1;

  gpioinfo("GPINT initializing\n\r");

  for (i = 0; i < NINT_DRVRS; i++)
  {
    gpint = sam_gpint_initialize(ints[i],
                                 sizes[i],
                                 irqs[i]);
    if (gpint == NULL)
    {
      _err("ERROR: Failed to initialize gpint%d\n\r",
           i);
      return;
    }

    ret = gpint_register(gpint, minor++);
    if (ret < 0)
    {
      _err("ERROR: Failed to register GPINT%d driver: %d\n",
           minor, (unsigned int)ret);

      sam_gpint_uninitialize(gpint);
      gpint = NULL;
      return;
    }

    gpint = NULL;
    gpioinfo("GPINT%d registering is done\n\r",
             minor);
  }
}


#endif /* CONFIG_DEV_GPINT && CONFIG_SAMD2L2_GPINT */
