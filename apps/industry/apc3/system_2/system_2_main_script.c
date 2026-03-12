/****************************************************************************
 * apps/industry/apc3/system_2/system_2_main_script.c
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

static int SYS2_check_PU(char *bad_pu);

static int SYS2_handle_PU_check(char *bad_pu);

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
static inline int SYS2_switch_PU(char *bad_pu);
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

/****************************************************************************
 * Private Data
 ****************************************************************************/

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
static char g_pu_ok = 0x0; /* PUs is not ok */
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_check_PU
 *
 * Description:
 *   Check the PUs state
 *
 * Arguments:
 *   bad_pu - a pointer to the byte where bad_pu value is stored
 *
 * Returned values:
 *   0          - if it is OK, but no one PU is functional
 *   1          - if it is OK and at least one PU is functional
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static int SYS2_check_PU(char *bad_pu)
{
  char *devpath;
  char *PU;
  char raw[(BOARD_NGPINP0/8)+1];
  int ret;
  int fd;
  uint8_t i;
  static char prev_raw = 0xF0;

  ret = OK;
  devpath = "/dev/gpinp0";

  /* Enter the task critical section */
  ret = SYS2_enter_critical_section();
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to enter critical section: %d\n\r",
         ret);
    return ret;
  }

  fd = open(devpath, O_RDONLY);
  if (fd < 0)
  {
    ret = fd;
    gpioerr("ERROR: SYSTEM_2: Failed to open /dev/gpinp0: %d\n\r",
            ret);
    SYS2_leave_critical_section();
    return ret;
  }

  ret = read(fd, raw, sizeof(raw));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_2: Failed to read /dev/gpinp0: %d\n\r",
            ret);
    CLOSE(fd);
    return ret;
  }

  /* Update the global buffer */
  g_gpinp0_data_buffer[0] = MNPWRST_CNV(raw);
  g_gpinp0_data_buffer[1] = BSPWRST_CNV(raw);

  ret = 0;

  CLOSE(fd);

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  /* Cyclic checking of every PU */
  for (i = 0; i < 2; i++)
  {
    PU = (i == 0) ? "Left" : "Right";


    if (raw[0] & (0x1 << (i*4))) /* PWR_PRESENT_CLAMP = 1 */
    {
      if (prev_raw != raw[0])
      {
        _info("%s PU is gone!", PU);
      }
      *bad_pu &= ~(0x1 << i);
      continue;
    }
    if (!(raw[0] & (0x2 << (i*4)))) /* PWR_AC_OK_CLAMP = 0 */
    {
      if (prev_raw != raw[0])
      {
        _info("%s PU AC is gone!", PU);
      }
      *bad_pu &= ~(0x1 << i);
    }
    if (!(raw[0] & (0x4 << (i*4)))) /* PWR_ALERT_CLAMP = 0 */
    {
      if (prev_raw != raw[0])
      {
        _info("%s PU ALERT!", PU);
      }
      *bad_pu &= ~(0x1 << i);
    }

    /* Aftercheck only current PU */
    if ((*bad_pu & 0x30) == ((i + 1) << 0x4))
    {
      if (!(raw[0] & (0x8 << (i*4)))) /* PWR_PW_OK_CLAMP = 0 */
      {
        if (prev_raw != raw[0])
        {
          _info("%s PU PW_OK is gone!", PU);
        }
        *bad_pu &= ~(0x1 << i);
      }
    }
  }
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  /* Cyclic checking of every PU */
  for (i = 0; i < 2; i++)
  {
    PU = (i == 0) ? "Right" : "Left";

    if (raw[0] & (0x1 << (i*2))) /* PWR_PRE = 1 */
    {
      if (prev_raw != raw[0])
      {
        _info("%s PU is gone!", PU);
      }
      *bad_pu &= ~(0x1 << i);
      continue;
    }
    if (raw[0] & (0x2 << (i*2))) /* PWR_PG = 1 */
    {
      if (prev_raw != raw[0])
      {
        _info("%s PU AC is gone!", PU);
      }
      *bad_pu &= ~(0x1 << i);
    }
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

  if ((*bad_pu & 0x3) > 0x0)
  {
    ret = 1;
  }
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  else
  {
    g_pu_ok = 0x0;
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

  prev_raw = raw[0];

  return ret;
}

/****************************************************************************
 * Name: SYS2_handle_PU_check
 *
 * Description:
 *   Handle the result of SYS2_check_PU function
 *
 * Arguments:
 *   bad_pu - a pointer to the byte where bad_pu value is stored
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static int SYS2_handle_PU_check(char *bad_pu)
{
  int ret;

  ret = OK;

  ret = SYS2_check_PU(bad_pu);
  if (ret < 0)
  {
    /* Software or hardware error */
    _err("ERROR: SYSTEM_2: Failed to check PU: %d\n\r",
         ret);
    return ret;
  }
  else if ((ret == 0) ||
          ((*bad_pu & 0x3) == 0))
  {
    /* PU check is failed:
     * No one PU is operable
     */
    return ret;
  }
  else if ((ret == 1) &
          ((*bad_pu & 0x3) != 0x3))
  {
    /* PU check is warning, but it is still passed:
     * One of the PU (first:  *bad_pu == 0x1, or
     *                second: *bad_pu == 0x2)
     * is functional, but the one that is not
     */

    /* Fall through */
  }
  else if ((ret == 1) &
          ((*bad_pu & 0x3) == 0x3))
  {
    /* PU check is fully passed:
     * Both PU is functional.
     */

    /* Fall through */
  }
  else
  {
    /* Unknown code */
    ret = -EUNKNOWN;
    return ret;
  }

  return ret;
}

/****************************************************************************
 * Name: SYS2_switch_PU
 *
 * Description:
 *   Switch current PU to functional. If it both is functional then current
 *    is PU1
 *
 * Arguments:
 *   bad_pu - a pointer to the byte where bad_pu value is stored
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
static inline int SYS2_switch_PU(char *bad_pu)
{
  int ret;

  ret = OK;

  switch ((*bad_pu & 0x3))
  {
    case (0x0):
      /* Left PU 1 is broken,
       * Right PU 2 is broken
       */

      /* Update current PU */
      *bad_pu &= 0xCF; /* No one */
      break;


    case (0x1):
      /* Left PU 1 is normal,
       * Right PU 2 is broken
       */

      /* Fall through */
/* Not used now: reserved algorithm */
#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2_RESERVED)
    case (0x3):
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2_RESERVED */
      /* Left PU 1 is normal,
       * Right PU 2 is normal
       */

      /* Update current PU */
      *bad_pu &= 0xCF;
      *bad_pu |= 0x10; /* PU 1 */

      /* Power:
       * PU 1     - ON,
       * PU 2     - no change,
       * Block 1  - no change,
       * Block 2  - no change,
       * Block 3  - no change
       */
      ret = SYS2_pwrup(SYSTEM_2_PU_1);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed to turn on PU 1: %d\n\r",
             ret);
        return ret;
      }
      break;

    case (0x2):
      /* Left PU 1 is broken,
       * Right PU 2 is normal
       */

      /* Update current PU */
      *bad_pu &= 0xCF;
      *bad_pu |= 0x20; /* PU 2 */

      /* Power:
       * PU 1     - no change,
       * PU 2     - ON,
       * Block 1  - no change,
       * Block 2  - no change,
       * Block 3  - no change
       */
      ret = SYS2_pwrup(SYSTEM_2_PU_2);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed to turn on PU 2: %d\n\r",
             ret);
        return ret;
      }
      break;

#if !defined(CONFIG_INDUSTRY_APC3_SYSTEM_2_RESERVED)
    case (0x3):
      /* Left PU 1 is normal,
       * Right PU 2 is normal
       */

      /* Update current PU */
      *bad_pu &= 0xCF;
      *bad_pu |= 0x30; /* Both */

      /* Power:
       * PU 1     - ON,
       * PU 2     - ON,
       * Block 1  - no change,
       * Block 2  - no change,
       * Block 3  - no change
       */
      ret = SYS2_pwrup(SYSTEM_2_PU_1 |
                       SYSTEM_2_PU_2);
      if (ret < 0)
      {
        _err("ERROR: SYSTEM_2: Failed to turn on PU 2: %d\n\r",
             ret);
        return ret;
      }
      break;
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2_RESERVED */

    default:
      /* Unknown code */

      /* Update current PU */
      *bad_pu &= 0xCF; /* No one */
      ret = -EUNKNOWN;
      break;
  }

  /* The shutdown is performed
   * in the main switch-case
   */

  return ret;
}
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_main_script
 *
 * Description:
 *   The function describes the algorithm that operates when the UPxxxxR PU:
 *   mains power present in the device.
 *
 * Arguments:
 *   bad_pu - a pointer to char where bad_pu value will be saved
 *
 * Returned values:
 *   Described in system_2.h
 *
 ****************************************************************************/

int SYS2_main_script(char *bad_pu)
{
  int ret;

  ret = OK;

  /* bad_pu value decode:
   * Bits:
   * 7  6  5  4  3  2  1  0
   *       C2 C1       F2 F1
   * F1 - PU 1 is functional, 1: func; 0: not func
   * F2 - PU 2 is functional, 1: func; 0: not func
   * C1 - PU 1 is current,    1: curr; 0: not curr
   * C2 - PU 2 is current     1: curr; 0: not curr
   *
   * Reserved algorithm:
   * There is only one current PU, but both can be functional.
   * If PU 2 is current, then PU 1 definitely not functional
   *
   * No reserved algorithm:
   * If there is both PU is functional then turn ON both PU.
   */

  *bad_pu = 0x03;    /* Initial all PU is OK, but no current */

  /* Perform the PU checking */
  ret = SYS2_handle_PU_check(bad_pu);
  if (ret < 0)
  {
    /* Software or hardware error */
    _err("ERROR: SYSTEM_2: Failed to handle first PU check: %d\n\r",
         ret);
    return ret;
  }
  else if (ret == 1)
  {
    /* PUs check is passed */

    /* Fall through */
  }
  else if (ret == 0)
  {
    /* PUs check is failed */
    return ret;
  }
  else
  {
    /* Unknown code */
    ret = -EUNKNOWN;
    return ret;
  }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  /* Switch the PU to the functional one */
  ret = SYS2_switch_PU(bad_pu);
  if (ret < 0)
  {
    /* Software or hardware error */
    _err("ERROR: SYSTEM_2: Failed to switch PU: %d\n\r",
         ret);
    return ret;
  }

  if ((g_pu_ok == 0)
/* Not used now: reserved algorithm */
#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2_RESERVED)
       || (g_pu_ok != (*bad_pu & 0x30))
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2_RESERVED */
     )
  {
    /* Perform aftercheck with PWR_PW_OK_CLAMP signal */

    if ((*bad_pu & 0x30) != 0x00)
    {
      /* Wait until the power become stable */
      sleep(1);

      ret = SYS2_handle_PU_check(bad_pu);
      if (ret < 0)
      {
        /* Software or hardware error */
        _err("ERROR: SYSTEM_2: Failed to handle second PU check: %d\n\r",
            ret);
        return ret;
      }
      else if (ret == 1)
      {
        /* PUs check is passed */

        /* Fall through */
      }
      else if (ret == 0)
      {
        /* PUs check is failed */
        return ret;
      }
      else
      {
        /* Unknown code */
        ret = -EUNKNOWN;
        return ret;
      }
    }
    g_pu_ok = *bad_pu & 0x30;
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

  /* Blocks checking */

  /* Check Block 1 */
  ret = SYS2_check_block_1();
  if (ret < 0)
  {
    /* Software or hardware error */
    _err("ERROR: SYSTEM_2: Failed to check Block 1: %d\n\r",
         ret);
    return ret;
  }
  else if (ret == 1)
  {
    /* Block 1 check is failed */
    return ret;
  }
  else if (ret == 2)
  {
    /* Block 1 check is passed */

    /* Fall through */
  }
  else
  {
    /* Unknown code */
    ret = -EUNKNOWN;
    return ret;
  }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  /* Check Block 2 */
  ret = SYS2_check_block_2();
  if (ret < 0)
  {
    /* Software or hardware error */
    _err("ERROR: SYSTEM_2: Failed to check Block 2: %d\n\r",
         ret);
    return ret;
  }
  else if (ret == 2)
  {
    /* Block 2 check is failed */
    return ret;
  }
  else if (ret == 3)
  {
    /* Block 2 check is passed */

    /* Fall through */
  }
  else
  {
    /* Unknown code */
    ret = -EUNKNOWN;
    return ret;
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */

  /* Check Block 3 */
  ret = SYS2_check_block_3();
  if (ret < 0)
  {
    /* Software or hardware error */
    _err("ERROR: SYSTEM_2: Failed to check Block 3: %d\n\r",
         ret);
    return ret;
  }
  else if (ret == 3)
  {
    /* Block 3 check is failed */
    return ret;
  }
  else if (ret == 4)
  {
    /* Block 2 check is passed */

    /* Fall through */
  }
  else
  {
    /* Unknown code */
    ret = -EUNKNOWN;
    return ret;
  }

  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
