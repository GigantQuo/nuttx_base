/****************************************************************************
 * boards/arm/samd2l2/qpc3/src/sam_bgpinp.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>

#include <nuttx/config.h>
#include <arch/board/board.h>
#include <nuttx/board.h>

#include <nuttx/ioexpander/gpinp.h>

#include "sam_gpinp.h"
#include "sam_port.h"

#include "apc3.h"

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

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
const uint32_t inputs_pull0[BOARD_NGPINP0] =
{
  PWR1_PRESENT_CLAMP,
  PWR1_AC_OK_CLAMP,
  PWR1_ALERT_CLAMP,
  PWR1_PW_OK_CLAMP,
  PWR2_PRESENT_CLAMP,
  PWR2_AC_OK_CLAMP,
  PWR2_ALERT_CLAMP,
  PWR2_PW_OK_CLAMP,
  PWR_AC_OK,
  PWR_BAT_LOW
};
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
const uint32_t inputs_pull0[BOARD_NGPINP0] =
{
  PWR_PRE0,
  PWR_PG0,
  PWR_PRE1,
  PWR_PG1,
  PWR_AC_OK,
  PWR_BAT_LOW
};

#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

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

void sam_gpinp_register(unsigned int minor)
{
  int ret = OK;
  struct gpinp_dev_s *gpinp;
  const uint32_t *inps[NINP_DRVRS];
  uint32_t sizes[NINP_DRVRS];
  uint8_t i;

  inps[0] = inputs_pull0;
  sizes[0] = BOARD_NGPINP0;

  gpioinfo("GPINP initializing\n\r");

  for (i = 0; i < NINP_DRVRS; i++)
  {
    gpinp = sam_gpinp_initialize(inps[i],
                                 sizes[i]);
    if (gpinp == NULL)
    {
      gpioerr("ERROR: BRINGUP: Failed to initialize gpinp%d\n\r",
              i);
      return;
    }
    ret = gpinp_register(gpinp, minor++);
    if (ret < 0)
    {
      gpioerr("ERROR: BRINGUP: Failed to register GPINP%d driver: %d\n",
              minor, (unsigned int)ret);

      sam_gpinp_uninitialize(gpinp);
      gpinp = NULL;
      return;
    }

    gpioinfo("GPINP%d registering is done\n\r",
            minor);
  }
}


#endif /* CONFIG_DEV_GPINP && CONFIG_SAMD2L2_GPINP */
