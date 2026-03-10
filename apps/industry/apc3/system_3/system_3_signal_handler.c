/****************************************************************************
 * apps/industry/apc3/system_3/system_3_signal_handler.c
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

#include <nuttx/ioexpander/gpint.h>

#include "system_3.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_3)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static inline int SYS3_led_logic(void);

static inline int SYS3_request(siginfo_t *info);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS3_led_logic
 *
 * Description:
 *   Perform the main logic of LEDs management.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS3_led_logic(void)
{
  char *devpath;
  ssize_t ret;
  char leds[1];
  char check[1];
  int fd;

  ret = OK;
  devpath = "/dev/gpout2";

  /* Buffer initialization */
  memset(leds, 0x0, sizeof(leds));
  memset(check, 0x0, sizeof(check));

  /* Open the driver that drives the LEDs */
  fd = open(devpath, O_RDWR);
  if (fd < 0)
  {
    ret = fd;
    gpioerr("ERROR: SYSTEM_3: Failed to open gpout2: %d\r\n",
            ret);
    return ret;
  }

  /* We need to read current state of LEDs */
  ret = read(fd, leds, sizeof(leds));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_3: Failed to read gpout2: %d\r\n",
            ret);
    close(fd);
    return ret;
  }

  /* Apply the actual values mask */
  leds[0] &= (1 << BOARD_GPOUT2_LED1) |
             (1 << BOARD_GPOUT2_LED2);



  /* Check for illegal values:
   * Only one LED is enables.
   */
  if ((leds[0] == (1 << BOARD_GPOUT2_LED1)) ||
      (leds[0] == (1 << BOARD_GPOUT2_LED2)))
  {
    _warn("WARNING: SYSTEM_3: Illegal LEDs state\n\r");

    /* Turn them both OFF */
    leds[0] = 0x0;
  }
  /* Both LEDs is disabled - enable both */
  else if (leds[0] == 0x0)
  {
    leds[0] = (1 << BOARD_GPOUT2_LED1) |
              (1 << BOARD_GPOUT2_LED2);
  }
  /* Both LEDs is enabled - disable both */
  else if (leds[0] == ((1 << BOARD_GPOUT2_LED1) |
                       (1 << BOARD_GPOUT2_LED2)))
  {
    leds[0] = 0x0;
  }
  /* If the mask has not been applied */
  else
  {
    _warn("WARNING: SYSTEM_3: Wrong LEDs state\n\r");

    /* Turn them both OFF */
    leds[0] = 0x0;
  }

  /* Write the changing */
  ret = write(fd, leds, sizeof(leds));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_3: Failed to write gpout2: %d\r\n",
            ret);
    close(fd);
    return ret;
  }

  /* Wait until the value has been installed */
  usleep(5);

  /* Perform check after write */
  ret = read(fd, check, sizeof(check));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_3: Failed to read gpout2: %d\r\n",
            ret);
    close(fd);
    return ret;
  }

  if (check[0] != leds[0])
  {
    ret = -ELEDSOUT;
  }

  close(fd);

  return ret;
}

/****************************************************************************
 * Name: SYS3_request
 *
 * Description:
 *   Perform the request signal script.
 *
 * Arguments:
 *   sig_ctx - the signal context with all necessaru data.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS3_request(siginfo_t *info)
{
  char *devpath;
  ssize_t ret;
  char leds[1];
  char check[1];
  int fd;

  ret = OK;
  devpath = "/dev/gpout2";

  /* Buffer initialization */
  memset(leds, 0x0, sizeof(leds));
  memset(check, 0x0, sizeof(check));

  switch (info->si_value.sival_int)
  {
    /* Turn the LEDs off command */
    case (0x0U):
      leds[0] = 0x0;
      break;

    /* Turn the LEDs on command */
    case (0x1U):
      leds[0] = (1 << BOARD_GPOUT2_LED1) |
                (1 << BOARD_GPOUT2_LED2);
      break;

    /* Unknown command */
    default:
      ret = -EUNKNOWN;
      return ret;
  }

  /* Open the driver that drives the LEDs */
  fd = open(devpath, O_RDWR);
  if (fd < 0)
  {
    ret = fd;
    gpioerr("ERROR: SYSTEM_3: Failed to open gpout2: %d\r\n",
            ret);
    return ret;
  }

  /* Write the value */
  ret = write(fd, leds, sizeof(leds));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_3: Failed to write gpout2: %d\r\n",
            ret);
    close(fd);
    return ret;
  }

  /* Wait until the value has been installed */
  usleep(5);

  /* Perform check after write */
  ret = read(fd, check, sizeof(check));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_3: Failed to read gpout2: %d\r\n",
            ret);
    close(fd);
    return ret;
  }

  if (check[0] != leds[0])
  {
    ret = -ELEDSOUT;
  }

  close(fd);
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS3_led_handler
 *
 * Description:
 *   The signal handler function that drives the LEDs.
 *
 * Arguments:
 *   signo   - the number of received signal
 *   info    - a pointer to a signal info structure with a necessry data
 *   context - the context of this task
 *
 ****************************************************************************/

void SYS3_led_handler(int signo,
                      siginfo_t *info,
                      void *context)
{
  sig_ctx_s3_t *sig_ctx;
  int ret;

  ret = OK;
  sig_ctx = (sig_ctx_s3_t *)info->si_user;

  /* Disable the interrupts
   * due to protection against
   * a contact rattling.
   * Use the same mode and pinmask.
   */
  ret = ioctl(sig_ctx->fd,
              GPINT_BIT_DISABLE,
              (unsigned long)&sig_ctx->enable);
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_3: Failed to disable interrupts: %d\n",
            ret);
    return;
  }

  /* Sanity check */
  if (info == NULL) return;
  if (sig_ctx == NULL) return;
  if (sig_ctx->fd <= 0) return;

  if      (signo == SIGINT)   ret = SYS3_led_logic();
  else if (signo == SIGTTOU)  ret = SYS3_request(info);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to execute the signal logic: %d\n\r",
         ret);
  }

  /* Sleep 300 ms until the rattlings gone */
  usleep(300000);
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_3 */
