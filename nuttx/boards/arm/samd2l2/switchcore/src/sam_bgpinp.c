/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_bgpinp.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

#include <nuttx/ioexpander/gpinp.h>

#include "sam_gpinp.h"
#include "sam_port.h"

#if defined(CONFIG_DEV_GPINP) && defined(CONFIG_SAMD2L2_GPINP)

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

const uint32_t inputs_pull[BOARD_NGPIOIN] =
{
  INP_0,
  INP_1,
  INP_2,
  INP_3,
  INP_4,
  INP_5,
  INP_6,
  INP_7,
  INP_8,
  INP_9,
  INP_10,
  INP_11,
  INP_12,
  INP_13
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpinp_register
 *
 * Arguments:
 *  int minor - number of registering driver
 *
 * Description:
 *   Register GPINP driver
 *
 ****************************************************************************/

void sam_gpinp_register(int minor)
{
  int ret = OK;
  struct gpinp_dev_s *gpinp;

  gpioinfo("GPINP initializing\n\r");

  gpinp = sam_gpinp_initialize(inputs_pull);
  if (gpinp == NULL)
  {
    _err("ERROR: Failed to initialize gpinputs\n\r");
    return;
  }
  ret = gpinp_register(gpinp, minor);
  if (ret < 0)
  {
    _err("ERROR: Failed to register GPINP%d driver: %d\n",
        (unsigned int)minor, (unsigned int)ret);
    sam_gpinp_uninitialize(gpinp);
    return;
  }

  gpioinfo("GPINP%d registering is done\n\r", (unsigned int)minor);
}


#endif /* CONFIG_DEV_GPINP && CONFIG_SAMD2L2_GPINP */
