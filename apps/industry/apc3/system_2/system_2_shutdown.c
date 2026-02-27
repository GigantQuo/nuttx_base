/****************************************************************************
 * apps/industry/apc3/system_2/system_2_shutdown.c
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

#include <sys/types.h>
#include <sys/ioctl.h>

#include <nuttx/ioexpander/gpout.h>

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
 * Name: SYS2_shutdown
 *
 * Description:
 *   Shut down the selected block
 *
 * Arguments:
 *   block - the mask of selected blocks
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_shutdown(const uint32_t block)
{
  char *devpath;
  uint8_t i;
  int ret;
  int fd;

  ret = OK;

  if (block == 0x0)
  {
    /* No changing */
    return ret;
  }

  devpath = "/dev/gpout0";

  /* Enter the task critical section */
  ret = SYS2_enter_critical_section();
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to enter critical section: %d\n\r",
    ret);
    return ret;
  }

  /* Open the driver that drives the power outputs */
  fd = open(devpath, O_RDWR);
  if (fd < 0)
  {
    ret = fd;
    gpioerr("ERROR: SYSTEM_2: Failed to open /dev/gpout0: %d\n\r",
            ret);
    SYS2_leave_critical_section();
    return ret;
  }

  /* Perform the turning off the
   * Power DC-DC drivers
   */

  /* Shutdown:          Put on reset:
   * 3.3V,              SW1,
   * 1,8V,              SW2,
   * 1,5V,              TCA
   * 1,0V_0 (1.02V_0),
   * 1,0V_1 (1.02V_1),
   * 0.9V   (None)
   */
  if (block & SYSTEM_2_BLOCK_1)
  {
    /* Reset operations */
    int fd_rst;

    devpath = "/dev/gpout1";

    /* Open the driver that drives the reset outputs */
    fd_rst = open(devpath, O_RDWR);
    if (fd_rst < 0)
    {
      ret = fd_rst;
      gpioerr("ERROR: SYSTEM_2: Failed to open /dev/gpout1: %d\n\r",
              ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }

    for (i = 0; i < SYSTEM_2_RST_NOUTS; i++)
    {
      /* 0 (false) - put on reset */
      ret = SYS2_gpout_hw(fd_rst, reset_pull[i], false);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed to perform hardware reset: %d\n\r",
             ret);
        close(fd_rst);
        close(fd);
        SYS2_leave_critical_section();
        return ret;
      }
    }

    close(fd_rst);

    /* Clear this unused variables */
    UNUSED(fd_rst);

    /* Power operations */
    for (i = 0; i < SYSTEM_2_B1_NOUTS; i++)
    {
      /* 0 (false) - turn OFF */
      ret = SYS2_gpout_hw(fd, block_1_pull[i], false);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed to turn OFF block 1: %d\n\r",
            ret);
        close(fd);
        SYS2_leave_critical_section();
        return ret;
      }
    }
  }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  /* Shutdown:    Put on reset:
   * 1,0V_0,      (No change)
   * 1,0V_1,
   * 3.3V_0,
   * 3.3V_1
   */
  if (block & SYSTEM_2_BLOCK_2)
  {
    for (i = 0; i < SYSTEM_2_B2_NOUTS; i++)
    {
      /* 0 (false) - turn OFF */
      ret = SYS2_gpout_hw(fd, block_2_pull[i], false);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed to turn OFF block 2: %d\n\r",
             ret);
        close(fd);
        SYS2_leave_critical_section();
        return ret;
      }
    }
  }

#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */

  /* Shutdown:    Put on reset:
   * 5,0V         (No change)
   */
  if (block & SYSTEM_2_BLOCK_3)
  {
    /* 0 (false) - turn OFF */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_5V0, false);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed to turn OFF block 3: %d\n\r",
            ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  /* Shutdown:    Put on reset:
   * 12V_0        (No change)
   */
  if (block & SYSTEM_2_PU_1)
  {
    /* 1 (true) - turn OFF */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_PWR1, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed to turn OFF PU 1: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }

  /* Shutdown:    Put on reset:
   * 12V_1        (No change)
   */
  if (block & SYSTEM_2_PU_2)
  {
    /* 1 (true) - turn OFF */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_PWR2, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed to turn OFF PU 2: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }

  /* Powerup:
   * 12.0V_BAT
   */
  if (block & SYSTEM_2_ACC)
  {
    /* 0 (false) - turn OFF */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_CONTROL, false);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed to turn OFF 12V BAT: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

  close(fd);
  SYS2_leave_critical_section();

  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
