/****************************************************************************
 * boards/arm/samd2l2/apc3/src/sam_badc.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>
#include <errno.h>

#include <nuttx/config.h>
#include <arch/board/board.h>
#include <nuttx/board.h>

#include <nuttx/analog/adc.h>

#include "chip.h"
#include "sam_adc.h"

#include "apc3.h"

#ifdef CONFIG_ADC

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_adc_register
 *
 * Description:
 *   Initialize ADC and register the ADC driver.
 *
 ****************************************************************************/

void sam_adc_register(void)
{
  struct adc_dev_s *adc;
  int ret;

  ret = OK;

  adc = sam_adcinitialize(BOARD_ADC_GCLKGEN);
  if (adc == NULL)
  {
    aerr("ERROR: BRINGUP: Failed to get ADC interface\n\r");
    return;
  }

  /* Register the ADC driver */
  ret = adc_register("/dev/adc0", adc);
  if (ret < 0)
  {
    aerr("ERROR: BRINGUP: adc_register failed: %d\n\r",
         ret);
  }
}


#endif /* CONFIG_ADC */
