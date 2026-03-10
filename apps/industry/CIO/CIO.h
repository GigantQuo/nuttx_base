/****************************************************************************
 * apps/industry/CIO/CIO.h
 ****************************************************************************/

#ifndef __APPS_INDUSTRY_CIO_H
#define __APPS_INDUSTRY_CIO_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/compiler.h>
#include <nuttx/config.h>

#include "industry/apc3.h"

#if defined(CONFIG_INDUSTRY_CIO)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CIO_MAX_ADC_CHANNEL_NUM 16

/* ADC units to voltage conversion definition */
#define ADC2MVOLT(adc) ((((adc) * 1650) / 65536) * 2)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: cio_main
 *
 * Description:
 *   Standart console input output
 *
 * Arguments:
 *   argc - the volume of argc
 *   argv - any input array
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int cio_main(int argc, FAR char* argv[]);

#endif /* CONFIG_INDUSTRY_CIO */
#endif /* __APPS_INDUSTRY_CIO_H */
