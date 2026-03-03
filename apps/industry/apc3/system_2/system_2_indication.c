/****************************************************************************
 * apps/industry/apc3/system_2/system_2_indication.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <arch/board/board.h>

#include "system_2.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

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
 * Name: SYS2_indication
 *
 * Description:
 *   Install the indication
 *
 * Arguments:
 *   mask   - the mask of selected outputs
 *   value  - the output value to be written
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_indication(const char mask,
                    const char value)
{
  char ind_value[1];
  char check[sizeof(ind_value)];
  char *devpath;
  int ret;
  int fd;

  ret = OK;
  devpath = "/dev/gpout1";

  /* Buffers initialization */
  memset(ind_value, 0x00, sizeof(ind_value));
  memset(check, 0x00, sizeof(check));

  /* Enter the task critical section */
  ret = SYS2_enter_critical_section();
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to enter critical section: %d\n\r",
    ret);
    return ret;
  }

  /* Open the driver that drives the
   * indication outputs
   */
  fd = open(devpath, O_RDWR);
  if (fd < 0)
  {
    ret = fd;
    gpioerr("ERROR: SYSTEM_2: Failed to open /dev/gpout1: %d\n\r",
            ret);
    SYS2_leave_critical_section();
    return ret;
  }

  /* Read the current indication value */
  ret = read(fd, ind_value, sizeof(ind_value));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_2: Failed to read /dev/gpout1: %d\n\r",
            ret);
    CLOSE(fd);
    return ret;
  }

  /* Apply the mask to the readed value:
   * power ok - 0x1,
   * alarm    - 0x2
   */
  ind_value[0] &= ~mask;
  /* Set the alarm output */
  ind_value[0] |= value;

  /* Write the indication value */
  ret = write(fd, ind_value, sizeof(ind_value));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_2: Failed to write /dev/gpout1: %d\n\r",
            ret);
    CLOSE(fd);
    return ret;
  }

  /* Perform check-after-write */
  ret = read(fd, check, sizeof(check));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_2: Failed to read /dev/gpout1: %d\n\r",
            ret);
    CLOSE(fd);
    return ret;
  }

  if (check[0] != ind_value[0])
  {
    ret = -EPWROUT;
  }

  CLOSE(fd);
  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
