/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_bgpint.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

#include <nuttx/ioexpander/gpint.h>

#include "sam_pinmap.h"
#include "sam_gpint.h"
#include "sam_port.h"

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

const uint32_t int_pull[BOARD_NGPIOIN] =
{
  INT_0,
  INT_1
};

const uint32_t irq_pull[BOARD_NGPIOIN] =
{
  INT_0_EXTINT,
  INT_1_EXTINT
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
 *  int minor - number of registering driver
 *
 * Description:
 *   Register GPINT driver
 *
 ****************************************************************************/

void sam_gpint_register(int minor)
{
  int ret = OK;
  struct gpint_dev_s *gpint;

  gpioinfo("GPINT initializing\n\r");

  gpint = sam_gpint_initialize(int_pull, irq_pull);
  /*if (gpint == NULL)
  {
    _err("ERROR: Failed to initialize gpinterrupts\n\r");
    return;
  }*/
  ret = gpint_register(gpint, minor);
  if (ret < 0)
  {
    _err("ERROR: Failed to register GPINT%d driver: %d\n",
         (unsigned int)minor, (unsigned int)ret);
    sam_gpint_uninitialize(gpint);
    return;
  }

  gpioinfo("GPINT%d registering is done\n\r", (unsigned int)minor);
}


#endif /* CONFIG_DEV_GPINT && CONFIG_SAMD2L2_GPINT */
