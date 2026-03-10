/****************************************************************************
 * apps/industry/apc3/system_2/system_2_gphw.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <nuttx/ioexpander/gpout.h>

#include <arch/board/board.h>

#include "system_2.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) || defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
#define BROKEN_PIN (BOARD_GPOUT0_1V02_0)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
#define BROKEN_PIN (BOARD_GPOUT0_3V3_0)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
#define BROKEN_PIN (0xFF) /* Broken pin is not used */

#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */
/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_gpout_hw
 *
 * Description:
 *   Perform hardware gpout driver operations.
 *
 * Arguments:
 *   fd  - the opened file descriptor
 *   bit - the bit position
 *   val - the value to be written
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_gpout_hw(const int fd,
    const char bit,
    bool val)
{
    struct bitval_s upbit;
    bool check;
    int ret;

    ret = OK;

    /* bitval_s initialization */
    upbit.val = &val;
    upbit.bit = bit;

    /* Perform one-bit write */
    ret = ioctl(fd, GPOUT_BIT_WRITE, &upbit);
    if (ret < 0) {
        gpioerr("ERROR: SYSTEM_2: Failed to ioctl /dev/gpout: %d\n\r",
            ret);
        return ret;
    }

    if (bit == BROKEN_PIN) /* PB16 is broken */
    {
        return ret;
    }

    /* Aftercheck:
     * Perform one-bit read
     */

    /* bitval_s reinitialization */
    // upbit.val = &check;

    /* Wait until the value has been installed */
    // usleep(5);

    // ret = ioctl(fd, GPOUT_BIT_READ, &upbit);
    // if (ret < 0)
    //{
    //   gpioerr("ERROR: SYSTEM_2: Failed to ioctl /dev/gpout: %d\n\r",
    //           ret);
    //   return ret;
    // }

    // if (check != val)
    //{
    //   ret = -EPWROUT;
    // }

    return ret;
}

#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
