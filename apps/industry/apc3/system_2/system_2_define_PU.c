/****************************************************************************
 * apps/industry/apc3/system_2/system_2_define_PU.c
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

#include <arch/board/board.h>

#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>

#include "system_2.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  #define PWR1_PRESENT(raw)   ((raw[0] & (1 << BOARD_GPINP0_PRESENT_1)) != 0)
  #define PWR2_PRESENT(raw)   ((raw[0] & (1 << BOARD_GPINP0_PRESENT_2)) != 0)
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  #define PWR1_PRESENT(raw)   ((raw[0] & (1 << BOARD_GPINP0_PWR_PRE0)) != 0)
  #define PWR2_PRESENT(raw)   ((raw[0] & (1 << BOARD_GPINP0_PWR_PRE1)) != 0)

#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static int SYS2_check_ACC_PRSNT(void);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_define_PU
 *
 * Description:
 *   The function to define what the current Power Unit in the device
 *
 * Returned values:
 *   1          - if it is OK, and there is ACC power
 *   0          - if it is OK, and there is no ACC power
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS2_check_ACC_PRSNT(void)
{
  struct adc_msg_s voltage;
  char *devpath;
  uint8_t i;
  int ret;
  int fd;

  ret = 0;
  devpath = "/dev/adc0";

  /* Enter the task critical section */
  ret = SYS2_enter_critical_section();
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to enter critical section: %d\n\r",
         ret);
    return ret;
  }

  /* We need to check the 12.0V
   * VBAT+, if it is more than 5V
   * then ACC BAT present.
   */

  fd = open(devpath, O_RDONLY);
  if (fd < 0)
  {
    ret = fd;
    _err("ERROR: SYSTEM_2: Failed to open /dev/adc0: %d\n\r",
         ret);
    SYS2_leave_critical_section();
    return ret;
  }

  for (i = 0; i < (BOARD_ADC_NUM_CHANNELS + 1); i++)
  {
    /* Check this voltage */
    ret = read(fd, &voltage, sizeof(voltage));
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed to read /dev/adc0: %d\n\r",
           ret);
      CLOSE(fd);
      return ret;
    }
    else if (ret == 0)
    {
      CLOSE(fd);
      return ret;
    }

    /* First conversion in the group is false - ignore it */
    if ((voltage.am_channel == BOARD_ADC_VBAT_CH) &&
        (i != 0))
    {
      /* Check if the BAT voltage more than 5.0V:
        * There is a input external resistive divider /11
        * and internal input divider /2, but reference is
        * VDDANA/2 = 1.65V.
        * Then 12.0V/22 = 0.55V <=> 1.65V
        * 5.0V      : 9027
        * 12.0V     : 21665
        */
      ret = (voltage.am_data > 9027) ? 1 : 0;

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_VBAT_CH_OFFSET] =
        (uint8_t)((ADC2MVOLT(voltage.am_data + 600) * 11) / 100);

      CLOSE(fd);
      return ret;
    }
  }

  CLOSE(fd);

  ret = 0;

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_define_PU
 *
 * Description:
 *   The function to define what the current Power Unit in the device
 *
 * Arguments:
 *   script - a pointer to a buffer to write a results
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_define_PU(char* script)
{
  char *devpath;
  char raw[(BOARD_NGPINP0/8)+1];
  char pus;
  int ret;
  int fd;

  ret = OK;
  devpath = "/dev/gpinp0";
  pus = 0;

  /* Enter the task critical section */
  ret = SYS2_enter_critical_section();
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to enter critical section: %d\n\r",
         ret);
    return ret;
  }

  /* Open the driver that drives the inputs */
  fd = open(devpath, O_RDONLY);
  if (fd < 0)
  {
    ret = fd;
    _err("ERROR: SYSTEM_2: Failed to open /dev/gpinp0: %d\n\r",
            ret);
    SYS2_leave_critical_section();
    return ret;
  }

  /* Read all data from gpinp0 */
  ret = read(fd, raw, sizeof(raw));
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to read /dev/gpinp0: %d\n\r",
            ret);
    CLOSE(fd);
    return ret;
  }

  /* Update the global buffer */
  g_gpinp0_data_buffer[0] = MNPWRST_CNV(raw);
  g_gpinp0_data_buffer[1] = BSPWRST_CNV(raw);

  CLOSE(fd);

  /* Check the ACC by scaning its voltage */
  ret = SYS2_check_ACC_PRSNT();
  if (ret < 0) {
      _err("ERROR: SYSTEM_2: Failed to check ACC present: %d\n\r",
          ret);
      return ret;
  }
  /* Aply mask and perform the comparing */
  if (ret == 1)
  {
    /* Condition when the acc power is present */
    pus |= (1 << 0);
  }

  /* Check the MAINS by scaning its present outputs */
  if ((PWR1_PRESENT(raw) == 0) ||
      (PWR2_PRESENT(raw) == 0))
  {
    /* Condition when the mains power is present */
    pus |= (1 << 1);
  }

  switch (pus)
  {
    case (0x0):
      *script = SYSTEM_2_NO_PU;
      break;

    case (0x1):
      *script = SYSTEM_2_ACC_PU;
      break;

    case (0x2):
      *script = SYSTEM_2_MAIN_PU;
      break;

    case (0x3):
      *script = SYSTEM_2_BOTH_PU;
      break;
  }

  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
