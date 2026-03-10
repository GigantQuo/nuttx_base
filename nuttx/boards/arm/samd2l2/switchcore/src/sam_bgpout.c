/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_bgpout.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

#include <nuttx/ioexpander/gpout.h>

#include "sam_gpout.h"
#include "sam_port.h"

#if defined(CONFIG_DEV_GPOUT) && defined(CONFIG_SAMD2L2_GPOUT)

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

const uint32_t outputs_pull[BOARD_NGPIOOUT] =
{
  OUT_0,
  OUT_1,
  OUT_2,
  OUT_3,
  OUT_4,
  OUT_5
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpout_register
 *
 * Arguments:
 *  int minor - number of registering driver
 *
 * Description:
 *   Register GPOUT driver
 *
 ****************************************************************************/

void sam_gpout_register(int minor)
{
  int ret = OK;
  struct gpout_dev_s *gpout;

  gpioinfo("GPOUT initializing\n\r");

  gpout = sam_gpout_initialize(outputs_pull);
  if (gpout == NULL)
  {
    _err("ERROR: Failed to initialize gpoutputs\n\r");
    return;
  }
  ret = gpout_register(gpout, minor);
  if (ret < 0)
  {
    _err("ERROR: Failed to register GPOUT%d driver: %d\n",
         (unsigned int)minor, (unsigned int)ret);
    sam_gpout_uninitialize(gpout);
    return;
  }

  gpioinfo("GPOUT%d registering is done\n\r", (unsigned int)minor);
}


#endif /* CONFIG_DEV_GPOUT && CONFIG_SAMD2L2_GPOUT */
