/****************************************************************************
 * apps/industry/apc3/system_1/system_1_main.c
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
#include <signal.h>

#include <sys/ioctl.h>

#include <arch/board/board.h>

#include <arch/chip/sam_i2c_slave.h>

#include "system_1.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_1)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#if defined(CONFIG_SAMD2L2_I2C_SLAVE_INTIME)
static void SYS1_intime_handler(const uint8_t *rx,
                                void *tx,
                                int *buflen,
                                int *curptr);
#endif /* CONFIG_SAMD2L2_I2C_SLAVE_INTIME */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#if defined(CONFIG_SAMD2L2_I2C_SLAVE_INTIME)
static void SYS1_intime_handler(const uint8_t *rx,
                                void *tx,
                                int *buflen,
                                int *curptr)
{
  uint8_t **tx_p = (uint8_t **)tx;

  /* All commands described in the system_1.h */
  switch (rx[0])
  {
    /* Main Power Status command */
    case (SYSTEM_1_MNPWRST_CMD):
      /* Initialize the TX buffer */
      *curptr = 0;
      *buflen = sizeof(uint8_t);
      *tx_p = &g_gpinp0_data_buffer[0];
      return;

    /* Battery Power Voltage Status command */
    case (SYSTEM_1_VBPWRST_CMD):
      /* Initialize the TX buffer */
      *curptr = 0;
      *buflen = sizeof(uint8_t);
      *tx_p = &g_adc0_data_buffer[BOARD_ADC_VBAT_CH_OFFSET];
      return;

    /* Battery Power Status command */
    case (SYSTEM_1_BSPWRST_CMD):
      /* Initialize the TX buffer */
      *curptr = 0;
      *buflen = sizeof(uint8_t);
      *tx_p = &g_gpinp0_data_buffer[1];
      return;
  }
}
#endif /* CONFIG_SAMD2L2_I2C_SLAVE_INTIME */

/****************************************************************************
 * Public Functions
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

int system_1_start(int argc, char *argv[])
{
  union sigval code;
  char *devpath;
  uint8_t byte;
  ssize_t ret;
  int fd;

  _info("SYSTEM_1 Started successfully!\n\r");

  ret = OK;
  devpath = "/dev/i2cslv1";

#if defined(CONFIG_SAMD2L2_I2C_SLAVE_INTIME)
  /* Register the intime i2c slave handler */
  sam_intime_register(1, SYS1_intime_handler);
#endif /* CONFIG_SAMD2L2_I2C_SLAVE_INTIME */

  /* Open I2C slave driver.
   * The drive should be opened
   * during all working time of the system_1
   * due to minization of latency between the
   * READ and WRITE I2C transactions.
   */
  fd = open(devpath, O_RDWR);
  if (fd < 0)
  {
    ret = fd;
    i2cerr("ERROR: Failed to open /dev/i2cslv1: %d\n\r",
           ret);
    return ret;
  }

  /* I2C communication logic */
  while (1)
  {
    /* Blocking reading of a data */
    ret = read(fd, &byte, sizeof(byte));
    if (ret < 0)
    {
      i2cerr("ERROR: SYSTEM_1: Failed to read /dev/i2cslv1: %d\n\r",
             ret);
      continue;
    }

    /* All commands described in the system_1.h */
    switch (byte)
    {
      /* Turn the LEDs on command */
      case (SYSTEM_1_LEDSONN_CMD):

        code.sival_int = 0x1U;

        /* Send the signal to system_3 */
        ret = sigqueue(g_system_pids->pids[3], SIGTTOU, code);
        if (ret < 0)
        {
          _err("ERROR: SYSTEM_1: Failed to send the SIGTTOU: %d\n\r",
               ret);
        }
        continue;

      /* Turn the LEDs off command */
      case (SYSTEM_1_LEDSOFF_CMD):

        code.sival_int = 0x0U;

        /* Send the signal to system_3 */
        ret = sigqueue(g_system_pids->pids[3], SIGTTOU, code);
        if (ret < 0)
        {
          _err("ERROR: SYSTEM_1: Failed to send the SIGTTOU: %d\n\r",
               ret);
        }
        continue;

#if !defined(CONFIG_SAMD2L2_I2C_SLAVE_INTIME)
      /* Main Power Status command */
      case (SYSTEM_1_MNPWRST_CMD):

        /* Write the tx buffer with proper data */
        ret = write(fd,
                    &g_gpinp0_data_buffer[0],
                    sizeof(uint8_t));
        if (ret < 0)
        {
          i2cerr("ERROR: SYSTEM_1: Failed to write /dev/i2cslv1: %d\n\r",
                 ret);
        }
        continue;

      /* Battery Power Voltage Status command */
      case (SYSTEM_1_VBPWRST_CMD):

        /* Write the tx buffer with proper data */
        ret = write(fd,
                    &g_adc0_data_buffer[BOARD_ADC_VBAT_CH_OFFSET],
                    sizeof(uint8_t));
        if (ret < 0)
        {
          i2cerr("ERROR: SYSTEM_1: Failed to write /dev/i2cslv1: %d\n\r",
                 ret);
        }
        continue;

        /* Battery Power Status command */
      case (SYSTEM_1_BSPWRST_CMD):

        /* Write the tx buffer with proper data */
        ret = write(fd,
                    &g_gpinp0_data_buffer[1],
                    sizeof(uint8_t));
        if (ret < 0)
        {
          i2cerr("ERROR: SYSTEM_1: Failed to write /dev/i2cslv1: %d\n\r",
                 ret);
        }
        continue;
#endif /* !CONFIG_SAMD2L2_I2C_SLAVE_INTIME */

      /* Unknown command */
      default:
        _err("ERROR: SYSTEM_1: Unknown command: %x\n\r",
             byte);
        continue;
    }
  }

  /* Will never be executed */

  /* Close I2C slave driver */
  close(fd);
  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_1 */
