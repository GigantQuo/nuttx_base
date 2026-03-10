/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_bgpout.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>

#include <nuttx/config.h>
#include <arch/board/board.h>
#include <nuttx/board.h>

#include <nuttx/ioexpander/gpout.h>

#include "sam_gpout.h"
#include "sam_port.h"

#include "apc3.h"

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

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
const uint32_t outputs_pull0[BOARD_NGPOUT0] =
{
  VDD3V3_EN,
  VDD1V8_EN,
  VDD1V5_EN,
  VDD1V02_EN_0,
  VDD1V02_EN_1,
  VDD0V9_EN,
  VDD5V0_EN,
  PWR1_ON,
  PWR2_ON,
  CONTROL
};
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
const uint32_t outputs_pull0[BOARD_NGPOUT0] =
{
  VDD3V3_EN,
  VDD1V8_EN,
  VDD1V5_EN,
  VDD1V02_EN_0,
  VDD1V02_EN_1,
  VDD5V0_EN,
  VDD3V3_0_EN,
  VDD3V3_1_EN,
  VDD1V0_EN_0,
  VDD1V0_EN_1
};
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
const uint32_t outputs_pull0[BOARD_NGPOUT0] =
{
  VDD3V3_EN,
  VDD1V8_EN,
  VDD1V5_EN,
  VDD1V02_EN_0,
  VDD0V9_EN,
  VDD5V0_EN,
  PWR1_ON,
  PWR2_ON,
  CONTROL
};
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
const uint32_t outputs_pull0[BOARD_NGPOUT0] =
{
  VDD3V3_EN,
  VDD1V8_EN,
  VDD1V5_EN,
  VDD1V02_EN_0,
  VDD5V0_EN,
  VDD3V3_0_EN,
  VDD1V0_EN_0,
};
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
const uint32_t outputs_pull1[BOARD_NGPOUT1] =
{
  POWER_GOOD,
  ALARM,
  PW_OK_SW0,
  PW_OK_SW1,
  PW_OK_TCA
};
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
const uint32_t outputs_pull1[BOARD_NGPOUT1] =
{
  POWER_GOOD,
  ALARM,
  PW_OK_SW0,
  PW_OK_SW1
};
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
const uint32_t outputs_pull1[BOARD_NGPOUT1] =
{
  POWER_GOOD,
  ALARM,
  PW_OK_SW0,
  PW_OK_TCA
};
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
const uint32_t outputs_pull1[BOARD_NGPOUT1] =
{
  POWER_GOOD,
  ALARM,
  PW_OK_SW0
};
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

const uint32_t outputs_pull2[BOARD_NGPOUT2] =
{
  LED_BUTTON_1,
  LED_BUTTON_2
};

const uint32_t outputs_pull3[BOARD_NGPOUT3] =
{
  RESET_SYSTEM
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

void sam_gpout_register(unsigned int minor)
{
  struct gpout_dev_s *gpout;
  const uint32_t *outs[NOUT_DRVRS];
  uint32_t sizes[NOUT_DRVRS];
  int ret;
  unsigned int i;

  ret = OK;

  outs[0] = outputs_pull0;
  outs[1] = outputs_pull1;
  outs[2] = outputs_pull2;
  outs[3] = outputs_pull3;

  sizes[0] = BOARD_NGPOUT0;
  sizes[1] = BOARD_NGPOUT1;
  sizes[2] = BOARD_NGPOUT2;
  sizes[3] = BOARD_NGPOUT3;

  gpioinfo("GPOUT initializing\n\r");

  for (i = 0; i < NOUT_DRVRS; i++)
  {
    gpout = sam_gpout_initialize(outs[i],
                                 sizes[i]);
    if (gpout == NULL)
    {
      gpioerr("ERROR: BRINGUP: Failed to initialize gpout%d\n\r",
              i);
      return;
    }

    ret = gpout_register(gpout, minor++);
    if (ret < 0)
    {
      gpioerr("ERROR: BRINGUP: Failed to register GPOUT%d driver: %d\n",
              minor, (unsigned int)ret);

      sam_gpout_uninitialize(gpout);
      gpout = NULL;
      return;
    }

    gpout = NULL;
    gpioinfo("GPOUT%d registering is done\n\r",
             minor);
  }
}


#endif /* CONFIG_DEV_GPOUT && CONFIG_SAMD2L2_GPOUT */
