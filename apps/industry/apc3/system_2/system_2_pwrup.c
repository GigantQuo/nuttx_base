/****************************************************************************
 * apps/industry/apc3/system_2/system_2_pwrup.c
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
 * Name: SYS2_pwrup
 *
 * Description:
 *   Turn on the specified block.
 *
 * Arguments:
 *   rvoltage - the mask of selected voltage and reset
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_pwrup(const uint32_t rvoltage)
{
  char *devpath;
  int ret;
  int fd;

  ret = OK;

  if (rvoltage == 0x0)
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

  /* Open the driver that drives the power enable outputs */
  fd = open(devpath, O_RDWR);
  if (fd < 0)
  {
    ret = fd;
    gpioerr("ERROR: SYSTEM_2: Failed to open /dev/gpout0: %d\n\r",
            ret);
    SYS2_leave_critical_section();
    return ret;
  }

  /* Perform the turning on the
   * Power DC-DC drivers
   */

  /* Block 1 */

  /* Powerup:
   * 3.3V
   */
  if (rvoltage & SYSTEM_2_3V3)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_3V3, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 3.3V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
  /* Powerup:
   * 1.8V
   */
  if (rvoltage & SYSTEM_2_1V8)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_1V8, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 1.8V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
  /* Powerup:
   * 1.5V
   */
  if (rvoltage & SYSTEM_2_1V5)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_1V5, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 1.5V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
  /* Powerup:
   * 1.02V_0
   */
  if (rvoltage & SYSTEM_2_1V02_0)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_1V02_0, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 1.02V_0: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
  /* Powerup:
   * 1.02V_1
   */
  if (rvoltage & SYSTEM_2_1V02_1)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_1V02_1, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 1.02V_1: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_-S */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  /* Powerup:
   * 0.9V
   */
  if (rvoltage & SYSTEM_2_0V9)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_0V9, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 0.9V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  /* Block 2 */

  /* Powerup:
   * 1.0V_0
   */
  if (rvoltage & SYSTEM_2_1V0_0)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_1V0_0, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 1.0_0V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
  /* Powerup:
   * 3.3V_0
   */
  if (rvoltage & SYSTEM_2_3V3_0)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_3V3_0, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 3.3_0V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
  /* Powerup:
   * 1.0V_1
   */
  if (rvoltage & SYSTEM_2_1V0_1)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_1V0_1, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 1.0_1V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
  /* Powerup:
   * 3.3V_0
   */
  if (rvoltage & SYSTEM_2_3V3_1)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_3V3_1, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 3.3_1V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS */
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */

  /* Block 3 */

  /* Powerup:
   * 5.0V
   */
  if (rvoltage & SYSTEM_2_5V0)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_5V0, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 5.0V: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  /* PU voltage */

  /* Powerup:
   * 12.0V_0
   */
  if (rvoltage & SYSTEM_2_PU_1)
  {
    /* 0 (false) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_PWR1, false);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP PU 1: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
  /* Powerup:
   * 12.0V_1
   */
  if (rvoltage & SYSTEM_2_PU_2)
  {
    /* 0 (false) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_PWR2, false);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP PU 2: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
  /* Powerup:
   * 12.0V_BAT
   */
  if (rvoltage & SYSTEM_2_ACC)
  {
    /* 1 (true) - turn ON */
    ret = SYS2_gpout_hw(fd, BOARD_GPOUT0_CONTROL, true);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed perform hardware UP 12V BAT: %d\n\r",
           ret);
      close(fd);
      SYS2_leave_critical_section();
      return ret;
    }
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

  close(fd);



  /* Reset */
  if (rvoltage & (  SYSTEM_2_SW0
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
                  | SYSTEM_2_SW1
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_-S */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                  | SYSTEM_2_TCA
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                 ))
  {
    devpath = "/dev/gpout1";

    /* Open the driver that drives the reset outputs */
    fd = open(devpath, O_RDWR);
    if (fd < 0)
    {
      ret = fd;
      gpioerr("ERROR: SYSTEM_2: Failed to open /dev/gpout1: %d\n\r",
              ret);
      SYS2_leave_critical_section();
      return ret;
    }

    /* Take from reset:
     * SW0
     */
    if (rvoltage & SYSTEM_2_SW0)
    {
      /* 1 (true) - take from reset */
      ret = SYS2_gpout_hw(fd, BOARD_GPOUT1_SW0, true);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed perform hardware unreset SW0: %d\n\r",
             ret);
        close(fd);
        SYS2_leave_critical_section();
        return ret;
      }
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
    /* Take from reset:
     * SW1
     */
    if (rvoltage & SYSTEM_2_SW1)
    {
      /* 1 (true) - take from reset */
      ret = SYS2_gpout_hw(fd, BOARD_GPOUT1_SW1, true);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed perform hardware unreset SW1: %d\n\r",
             ret);
        close(fd);
        SYS2_leave_critical_section();
        return ret;
      }
    }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_-S */

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
    /* Take from reset:
     * TCA
     */
    if (rvoltage & SYSTEM_2_TCA)
    {
      /* 1 (true) - take from reset */
      ret = SYS2_gpout_hw(fd, BOARD_GPOUT1_TCA, true);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed perform hardware unreset TCA: %d\n\r",
             ret);
        close(fd);
        SYS2_leave_critical_section();
        return ret;
      }
    }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

    close(fd);
  }

  SYS2_leave_critical_section();

  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
