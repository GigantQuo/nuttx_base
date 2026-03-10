/****************************************************************************
 * apps/industry/apc3/system_1/system_1.h
 ****************************************************************************/

#ifndef __APPS_INDUSTRY_APC3_SYSTEM_1_H
#define __APPS_INDUSTRY_APC3_SYSTEM_1_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include "industry/apc3.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_1)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Main Power Status command:
 * send the next gpinp data (for UPxx01R PU - _S series):
 *    PWR1_PRESENT_CLAMP,
 *    PWR1_AC_OK_CLAMP,
 *    PWR1_ALERT_CLAMP,
 *    PWR1_PW_OK_CLAMP,
 *    PWR2_PRESENT_CLAMP,
 *    PWR2_AC_OK_CLAMP,
 *    PWR2_ALERT_CLAMP,
 *    PWR2_PW_OK_CLAMP; - (1 byte 8/8)
 * send the next gpinp data (for BCF-150S12 PU - _FS series):
 *    PWR_PRE0,
 *    PWR_PRE1,
 *    PWR_PG0,
 *    PWR_PG1. - (1 byte 4/8)
 */
#define SYSTEM_1_MNPWRST_CMD        (0x01)

/* Battery Power Voltage Status command:
 * send the voltage of battery in the handred millivolts:
 *    VBAT voltage, - (1 byte 8/8)
 */
#define SYSTEM_1_VBPWRST_CMD        (0x02)

/* Battery Power Status command:
 * send the next gpinp data (BCF-150 PU):
 *    PWR_AC_OK,
 *    PWR_BAT_LOW; - (1 byte 2/8)
*/
#define SYSTEM_1_BSPWRST_CMD        (0x03)

/* Turn the LEDs on command:
 * turn the next gpout on:
 *    LED_BUTTON_1,
 *    LED_BUTTON_2.
 */
#define SYSTEM_1_LEDSONN_CMD        (0x08)

/* Turn the LEDs off command:
 * turn the next gpout off:
 *    LED_BUTTON_1,
 *    LED_BUTTON_2.
 */
#define SYSTEM_1_LEDSOFF_CMD        (0x09)

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
 * Name: system_1_start
 *
 * Description:
 *   The Arlan PonCat3 I2C Slave Interface start function.
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

int system_1_start(int argc, char *argv[]);


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_1 */
#endif /* __APPS_INDUSTRY_APC3_SYSTEM_1_H */
