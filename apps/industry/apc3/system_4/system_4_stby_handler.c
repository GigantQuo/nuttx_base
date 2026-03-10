/****************************************************************************
 * apps/industry/apc3/system_4/system_4_stby_handler.c
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

#include "system_4.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_4)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static inline int SYS4_rst_logic(void);

static inline int SYS4_rst_sys_pin(void);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS4_rst_logic
 *
 * Description:
 *   Perform the main logic of reset event.
 *
 * Arguments:
 *   system_pids - a pointer to the task_data structure with the pids of
 *    all tasks in the apc3.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS4_rst_logic(void)
{
  int ret;

  ret = OK;

  /* Turn the RESET_SYSTEM pin */
  ret = SYS4_rst_sys_pin();
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_4: Failed to turn the RESET_SYSTEM pin: %d\n\r",
         ret);
    return ret;
  }

  /* Send the SIGTTOU signal to system_2 */
  ret = kill(g_system_pids->pids[2], SIGTTOU);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_4: Failed to send the SIGTTOU to system_2: %d\n\r",
         ret);
  }

  return ret;
}

/****************************************************************************
 * Name: SYS4_rst_sys_pin
 *
 * Description:
 *   Perform RESET_SYSTEM output operations.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS4_rst_sys_pin(void)
{
  char *devpath;
  char rst[1];
  char check[1];
  ssize_t ret;
  int fd;

  ret = OK;
  devpath = "/dev/gpout3";

  /* Buffer initialization */
  memset(rst, 0x0, sizeof(rst));
  memset(check, 0x0, sizeof(check));

  /* Open the driver that drives the power enable outputs */
  fd = open(devpath, O_RDWR);
  if (fd < 0)
  {
    ret = fd;
    gpioerr("ERROR: SYSTEM_4: Failed to open /dev/gpout3: %d\n\r",
            ret);
    return ret;
  }

  /* Turn ON reset system */
  rst[0] = 0x0;

  /* Write the changing */
  ret = write(fd, rst, sizeof(rst));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_4: Failed to write /dev/gpout3: %d\r\n",
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
    gpioerr("ERROR: SYSTEM_4: Failed to read /dev/gpout3: %d\r\n",
            ret);
    close(fd);
    return ret;
  }

  if (check[0] != rst[0])
  {
    ret = -ERSTOUT;
    close(fd);
    return ret;
  }

  /* Wait 100ms and turn OFF reset system */
  usleep(100000);

  rst[0] = 0x1;

  /* Write the changing */
  ret = write(fd, rst, sizeof(rst));
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_4: Failed to write /dev/gpout3: %d\r\n",
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
    gpioerr("ERROR: SYSTEM_4: Failed to read /dev/gpout3: %d\r\n",
            ret);
    close(fd);
    return ret;
  }

  if (check[0] != rst[0])
  {
    ret = -ERSTOUT;
  }

  close(fd);

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS4_stby_handler
 *
 * Description:
 *   The signal handler function that perform the system reset.
 *
 * Arguments:
 *   signo   - the number of received signal
 *   info    - a pointer to a signal info structure with a necessry data
 *   context - the context of this task
 *
 ****************************************************************************/

void SYS4_stby_handler(int signo,
                       siginfo_t *info,
                       void *context)
{
  sig_ctx_s4_t *sig_ctx;
  int ret;

  ret = OK;

  sig_ctx = (sig_ctx_s4_t *)info->si_user;

  /* Sanity check */
  if (sig_ctx == NULL) return;
  if (sig_ctx->fd <= 0) return;

  /* Disable the interrupts
   * during the signal handler
   * execution.
   */
  ret = ioctl(sig_ctx->fd,
              GPINT_BIT_DISABLE,
              (unsigned long)&sig_ctx->enable);
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_4: Failed to disable interrupts: %d\n",
            ret);
    close(sig_ctx->fd);
    return;
  }


  /* Execute the main logic */
  ret = SYS4_rst_logic();
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_4: Failed to execute the rst logic script: %d\r\n",
         ret);
  }


  /* Sleep 3 ms until the fluctuations is gone */
  usleep(3000);
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_4 */
