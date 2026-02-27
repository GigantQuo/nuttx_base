/****************************************************************************
 * apps/industry/apc3/system_2/system_2_blocks.c
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

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile char g_b1_pwout_cntr = 0x0;
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
static volatile char g_b2_pwout_cntr = 0x0;
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */
static volatile char g_b3_pwout_cntr = 0x0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_check_block_1
 *
 * Description:
 *   Perform the checking of block 1.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_check_block_1(void)
{
  struct adc_msg_s voltages;
  char *devpath;
  char pwout_cntr;
  int ret;
  int fd;
  uint8_t i;

  ret = 1;
  pwout_cntr = 0;
  devpath = "/dev/adc0";

  /* We need to sequential turn on
   * and check this voltages:
   * 1. 3.3V;
   * 2. 1.8V, 1.5V;
   * 3. 1.02V_0, 1.02V_1;
   * 4. 0.9V.
   * And provide the 100ms delay
   * between the steps.
   */
  ret = SYS2_pwrup(SYSTEM_2_3V3);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to turn ON 3.3V: %d\n\r",
         ret);
    return ret;
  }
  /* Delay */
  usleep(100000);

  ret = SYS2_pwrup(SYSTEM_2_1V8 |
                   SYSTEM_2_1V5);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to turn ON 1.8V or 1.5V: %d\n\r",
         ret);
    return ret;
  }
  /* Delay */
  usleep(100000);

  ret = SYS2_pwrup(SYSTEM_2_1V02_0
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
                   | SYSTEM_2_1V02_1
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_-S */
                   );
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to turn ON 1.02V_0 or 1.02V_1: %d\n\r",
         ret);
    return ret;
  }
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  /* Delay */
  usleep(100000);

  ret = SYS2_pwrup(SYSTEM_2_0V9);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to turn ON 0.9V: %d\n\r",
         ret);
    return ret;
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */



  /* Open the ADC driver */
  fd = open(devpath, O_RDONLY);
  if (fd < 0)
  {
    ret = fd;
    aerr("ERROR: SYSTEM_2: Failed to open /dev/adc0: %d\n\r",
         ret);
    return ret;
  }

  /* Begin the checking */
  for (i = 0; i < (BOARD_ADC_NUM_CHANNELS + 1); i++)
  {
    /* Check this voltages */
    ret = read(fd, &voltages, sizeof(voltages));
    if (ret < 0)
    {
      aerr("ERROR: SYSTEM_2: Failed to read /dev/adc0: %d\n\r",
           ret);
      close(fd);
      return ret;
    }
    else if (ret == 0)
    {
      pwout_cntr++;
    }

    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_3V3_CH) &&
        (i != 0))
    {
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
      /* Check +- 5% from 3V3:
       * There is a input external resistive divider /2
       * and internal input divider /2, but reference is
       * VDDANA/2 = 1.65V.
       * Then 3.3V/4 = 0.825V <=> 1.65V
       * 3.3V-5% : 31129 (3.14V)
       * 3.3V    : 32768
       * 3.3V+5% : 34407 (3.46V)
       */
      if ((voltages.am_data < 31129) ||
          (voltages.am_data > 34407))
      {
        printf("3.3V voltage out of range (3.14V-3.46V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data) * 2);

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_3V3_CH_OFFSET] =
        (uint8_t)((ADC2MVOLT(voltages.am_data) * 2) / 100);

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
      /* Check +- 5% from 3V3:
       * There is a internal input divider /2,
       * but reference is VDDANA/2 = 1.65V.
       * Then 3.3V/4 = 0.825V <=> 1.65V
       * 3.3V-5% : 62258 (3.14V)
       * 3.3V    : 65535 (ceiling)
       */
      if (voltages.am_data < 62258)
      {
        printf("3.3V voltage out of range (3.14V-3.46V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data));

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_3V3_CH_OFFSET] =
        (uint8_t)(ADC2MVOLT(voltages.am_data) / 100);

#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */
    }

    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_1V8_CH) &&
        (i != 0))
    {
      /* Check +- 5% from 1V8:
       * There is a internal input divider /2,
       * but reference is VDDANA/2 = 1.65V.
       * Then 1.8V/2 = 0.9V <=> 1.65V
       * 1.8V-5% : 33959 (1.71V)
       * 1.8V    : 35747
       * 1.8V+5% : 37535 (1.89V)
       */
      if ((voltages.am_data < 33959) ||
          (voltages.am_data > 37535))
      {
        printf("1.8V voltage out of range (1.71V-1.89V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data));

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_1V8_CH_OFFSET] =
        (uint8_t)(ADC2MVOLT(voltages.am_data) / 100);
    }

    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_1V5_CH) &&
        (i != 0))
    {
      /* Check +- 5% from 1V5:
       * There is a internal input divider /2,
       * but reference is VDDANA/2 = 1.65V.
       * Then 1.5V/2 = 0.75V <=> 1.65V
       * 1.5V-5% : 28299 (1.43V)
       * 1.5V    : 29789
       * 1.5V+5% : 31279 (1.57V)
       */
      if ((voltages.am_data < 28299) ||
          (voltages.am_data > 31279))
      {
        printf("1.5V voltage out of range (1.43V-1.57V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data));

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_1V5_CH_OFFSET] =
        (uint8_t)(ADC2MVOLT(voltages.am_data) / 100);
    }

    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_1V02_0_CH) &&
        (i != 0))
    {
      /* Check +- 5% from 1V0:
       * There is a internal input divider /2,
       * but reference is VDDANA/2 = 1.65V.
       * Then 1.0V/2 = 0.5V <=> 1.65V
       * 1.0V-5% : 18866 (0.96V)
       * 1.0V    : 19859
       * 1.0V+5% : 20852 (1.04V)
       */
      if ((voltages.am_data < 18866) ||
          (voltages.am_data > 20852))
      {
        printf("1.02V_0 voltage out of range (0.96V-1.04V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data));

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_1V02_0_CH_OFFSET] =
        (uint8_t)(ADC2MVOLT(voltages.am_data) / 100);
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_1V02_1_CH) &&
        (i != 0))
    {
      /* Check +- 5% from 1V0:
       * There is a internal input divider /2,
       * but reference is VDDANA/2 = 1.65V.
       * Then 1.0V/2 = 0.5V <=> 1.65V
       * 1.0V-5% : 18866 (0.96V)
       * 1.0V    : 19859
       * 1.0V+5% : 20852 (1.04V)
       */
      if ((voltages.am_data < 18866) ||
          (voltages.am_data > 20852))
      {
        printf("1.02V_1 voltage out of range (0.96V-1.04V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data));

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_1V02_1_CH_OFFSET] =
        (uint8_t)(ADC2MVOLT(voltages.am_data) / 100);
    }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_-S */

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_0V9_CH) &&
        (i != 0))
    {
      /* Check +- 5% from 0V9:
       * There is a internal input divider /2,
       * but reference is VDDANA/2 = 1.65V.
       * Then 0.9V/2 = 0.45V <=> 1.65V
       * 0V9V-5% : 16979 (0.85V)
       * 0V9V    : 17873
       * 0V9V+5% : 18767 (0.94V)
       */
      if ((voltages.am_data < 16979) ||
          (voltages.am_data > 18767))
      {
        printf("0.9V voltage out of range (0.94V-0.85V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data));

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_0V9_CH_OFFSET] =
        (uint8_t)(ADC2MVOLT(voltages.am_data) / 100);
    }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
  }

  close(fd);

  /* If global power out counter > 5 PWOUT iteration
   * then turn OFF Block,
   * Also if there more than 2 PWOUT state during 1 iteration
   * then turn OFF Block,
   * If there is no PWOUT states, then decrement global counter
   */
  if (pwout_cntr == 0)
  {
    g_b1_pwout_cntr = (g_b1_pwout_cntr > 0) ? (g_b1_pwout_cntr - 1) : 0;
  }
  else if (pwout_cntr > 2)
  {
    g_b1_pwout_cntr++;
    ret = 1;
    return ret;
  }
  else
  {
    g_b1_pwout_cntr++;
  }

  if (g_b1_pwout_cntr >= 5)
  {
    ret = 1;
    return ret;
  }

  /* Perform the taking from reset SW0 (and SW1) */
  ret = SYS2_pwrup(SYSTEM_2_SW0
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
                   | SYSTEM_2_SW1
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_-S */
                   );
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to take from reset SW0 or SW1: %d\n\r",
         ret);
    return ret;
  }
  /* Delay */
  usleep(100000);

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  /* Perform the taking from reset TCA */
  ret = SYS2_pwrup(SYSTEM_2_TCA);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to take from reset TCA: %d\n\r",
         ret);
    return ret;
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

  ret = 2;

  return ret;
}

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
/****************************************************************************
 * Name: SYS2_check_block_2
 *
 * Description:
 *   Perform the checking of block 2
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_check_block_2(void)
{
  struct adc_msg_s voltages;
  char *devpath;
  char pwout_cntr;
  int ret;
  int fd;
  uint8_t i;

  ret = 2;
  pwout_cntr = 0;
  devpath = "/dev/adc0";

  /* We need to sequential turn on
   * and check this voltages:
   * 1. 1.0V_0, 1.0V_1;
   * 2. 3.3V_0;
   * 3. 3.3V_1.
   * And provide the 100ms delay
   * between the steps.
   */
  ret = SYS2_pwrup(  SYSTEM_2_1V0_0
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
                   | SYSTEM_2_1V0_1
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS */
                  );
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to turn ON 1.0V_0 or 1.0V_1: %d\n\r",
         ret);
    return ret;
  }
  /* Delay */
  usleep(100000);
  ret = SYS2_pwrup(SYSTEM_2_3V3_0);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to turn ON 3.3_0V: %d\n\r",
         ret);
    return ret;
  }
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
  /* Delay */
  usleep(100000);

  ret = SYS2_pwrup(SYSTEM_2_3V3_1);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to turn ON 3.3_1V: %d\n\r",
         ret);
    return ret;
  }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS */



  /* Open the ADC driver */
  fd = open(devpath, O_RDONLY);
  if (fd < 0)
  {
    ret = fd;
    aerr("ERROR: SYSTEM_2: Failed to open /dev/adc0: %d\n\r",
         ret);
    return ret;
  }

  /* Begin the checking */
  for (i = 0; i < (BOARD_ADC_NUM_CHANNELS + 1); i++)
  {
    /* Check this voltages */
    ret = read(fd, &voltages, sizeof(voltages));
    if (ret < 0)
    {
      aerr("ERROR: SYSTEM_2: Failed to read /dev/adc0: %d\n\r",
           ret);
      close(fd);
      return ret;
    }
    else if (ret == 0)
    {
      pwout_cntr++;
    }

    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_1V0_0_CH) &&
        (i != 0))
    {
      /* Check +- 5% from 1V0:
       * There is a internal input divider /2,
       * but reference is VDDANA/2 = 1.65V.
       * Then 1.0V/2 = 0.5V <=> 1.65V
       * 1.0V-5% : 18866 (0.96V)
       * 1.0V    : 19859
       * 1.0V+5% : 20852 (1.04V)
       */
      if ((voltages.am_data < 18866) ||
          (voltages.am_data > 20852))
      {
        printf("1.0V_0 voltage out of range (0.96V-1.04V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data));

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_1V0_0_CH_OFFSET] =
        (uint8_t)(ADC2MVOLT(voltages.am_data) / 100);
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_1V0_1_CH) &&
        (i != 0))
    {
      /* Check +- 5% from 1V0:
       * There is a internal input divider /2,
       * but reference is VDDANA/2 = 1.65V.
       * Then 1.0V/2 = 0.5V <=> 1.65V
       * 1.0V-5% : 18866 (0.96V)
       * 1.0V    : 19859
       * 1.0V+5% : 20852 (1.04V)
       */
      if ((voltages.am_data < 18866) ||
        (voltages.am_data > 20852))
      {
        printf("1.0V_1 voltage out of range (0.96V-1.04V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data));

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_1V0_1_CH_OFFSET] =
        (uint8_t)(ADC2MVOLT(voltages.am_data) / 100);
    }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS */

    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_3V3_0_CH) &&
        (i != 0))
    {
      /* Check +- x% from 3V3:
       * There is a input external resistive divider /2
       * and internal input divider /2, but reference is
       * VDDANA/2 = 1.65V.
       * Then 3.3V/4 = 0.825V <=> 1.65V
       * 3.3V-x% : 26460 (2.66V)
       * 3.3V    : 32768
       * 3.3V+x% : 36480 (3.67V)
       */
      if ((voltages.am_data < 20000) ||
          (voltages.am_data > 36480))
      {
        printf("3.3V_0 voltage out of range (2.66V-3.67V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data) * 2);

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_3V3_0_CH_OFFSET] =
        (uint8_t)((ADC2MVOLT(voltages.am_data) * 2) / 100);
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_3V3_1_CH) &&
        (i != 0))
    {
      /* Check +- x% from 3V3:
       * There is a input external resistive divider /2
       * and internal input divider /2, but reference is
       * VDDANA/2 = 1.65V.
       * Then 3.3V/4 = 0.825V <=> 1.65V
       * 3.3V-x% : 26460 (2.66V)
       * 3.3V    : 32768
       * 3.3V+x% : 36480 (3.67V)
       */
      if ((voltages.am_data < 20000) ||
          (voltages.am_data > 36480))
      {
        printf("3.3V_1 voltage out of range (2.66V-3.67V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data) * 2);

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_3V3_1_CH_OFFSET] =
        (uint8_t)((ADC2MVOLT(voltages.am_data) * 2) / 100);
    }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS */
  }

  close(fd);

  /* If global power out counter > 5 PWOUT iteration
   * then turn OFF Block,
   * Also if there more than 2 PWOUT state during 1 iteration
   * then turn OFF Block,
   * If there is no PWOUT states, then decrement global counter
   */
  if (pwout_cntr == 0)
  {
    g_b2_pwout_cntr = (g_b2_pwout_cntr > 0) ? (g_b2_pwout_cntr - 1) : 0;
  }
  else if (pwout_cntr > 2)
  {
    g_b2_pwout_cntr++;
    ret = 2;
    return ret;
  }
  else
  {
    g_b2_pwout_cntr++;
  }

  if (g_b2_pwout_cntr >= 5)
  {
    ret = 2;
    return ret;
  }

  ret = 3;

  return ret;
}
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */

/****************************************************************************
 * Name: SYS2_check_block_3
 *
 * Description:
 *   Perform the checking of block 3
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_check_block_3(void)
{
  struct adc_msg_s voltages;
  char *devpath;
  char pwout_cntr;
  int ret;
  int fd;
  uint8_t i;

  ret = 1;
  pwout_cntr = 0;
  devpath = "/dev/adc0";

  /* We need to turn on
   * and check this voltages:
   * 1. 5.0V
   */
  ret = SYS2_pwrup(SYSTEM_2_5V0);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to turn on 5.0V: %d\n\r",
         ret);
    return ret;
  }



  /* Open the ADC driver */
  fd = open(devpath, O_RDONLY);
  if (fd < 0)
  {
    ret = fd;
    aerr("ERROR: SYSTEM_2: Failed to open /dev/adc0: %d\n\r",
         ret);
    return ret;
  }

  for (i = 0; i < (BOARD_ADC_NUM_CHANNELS + 1); i++)
  {
    /* Check this voltages */
    ret = read(fd, &voltages, sizeof(voltages));
    if (ret < 0)
    {
      aerr("ERROR: SYSTEM_2: Failed to read /dev/adc0: %d\n\r",
           ret);
      close(fd);
      return ret;
    }
    else if (ret == 0)
    {
      pwout_cntr++;
    }

    /* First conversion in the group is false - ignore it */
    if ((voltages.am_channel == BOARD_ADC_5V0_CH) &&
        (i != 0))
    {
      /* Check +- 5% from 5V0:
       * There is a input external resistive divider /2
       * and internal input divider /2, but reference is
       * VDDANA/2 = 1.65V.
       * Then 5.0V/4 = 1.25V <=> 1.65V
       * 5.0V-5% : 47165 (4.75V)
       * 5.0V    : 49648
       * 5.0V+5% : 52131 (5.25V)
       */
      if ((voltages.am_data < 47165) ||
          (voltages.am_data > 52131))
      {
        printf("5.0V voltage out of range (4.75V-5.25V): %d mV\n\r",
               (int)ADC2MVOLT(voltages.am_data) * 2);

        pwout_cntr++;
      }

      /* Update the global buffer */
      g_adc0_data_buffer[BOARD_ADC_5V0_CH_OFFSET] =
        (uint8_t)((ADC2MVOLT(voltages.am_data) * 2) / 100);
    }
  }

  close(fd);


  if (pwout_cntr == 0)
  {
    g_b3_pwout_cntr = (g_b3_pwout_cntr > 0) ? (g_b3_pwout_cntr - 1) : 0;
  }
  else
  {
    g_b3_pwout_cntr++;
  }

  if (g_b3_pwout_cntr >= 2)
  {
    ret = 3;
    return ret;
  }

  ret = 4;

  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
