/****************************************************************************
 * apps/industry/apc3/apc3_main.c
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

#include <sys/ioctl.h>

#include <arch/board/board.h>

#include <nuttx/sched.h>

#include <nuttx/timers/watchdog.h>

#include "apc3.h"

#if defined(CONFIG_INDUSTRY_APC3)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

#if defined(CONFIG_WATCHDOG)
static inline void watchdog(void);
#endif /* CONFIG_WATCHDOG */

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* The Process IDs of the systems */
task_data_t *g_system_pids;

/* The global buffers for drivers data */
uint8_t g_adc0_data_buffer[BOARD_ADC_NUM_CHANNELS];
uint8_t g_gpinp0_data_buffer[2];


/****************************************************************************
 * Private Functions
 ****************************************************************************/

#if defined(CONFIG_WATCHDOG)
static inline void watchdog(void)
{
  char* devpath;
  int ret;
  int fd;

  ret = OK;
  devpath = "/dev/watchdog0";

  /* Open the watchdog driver */
  fd = open(devpath, O_RDWR);
  if (fd < 0)
  {
    ret = fd;
    wderr("ERROR: MAIN: Failed to open /dev/watchdog0: %d\n\r",
          ret);
    return;
  }

  /* Reset Watchdog */
  ret = ioctl(fd, WDIOC_KEEPALIVE, NULL);
  if (ret < 0)
  {
    wderr("ERROR: MAIN: Failed to kick /dev/watchdog0: %d\n\r",
          ret);
    close(fd);
    return;
  }

  /* Set 3 second timeout */
  ret = ioctl(fd, WDIOC_SETTIMEOUT, 3000UL);
  if (ret < 0)
  {
    wderr("ERROR: MAIN: Failed to set timeout /dev/watchdog0: %d\n\r",
          ret);
    close(fd);
    return;
  }

  /* Start watchdog */
  ret = ioctl(fd, WDIOC_START, NULL);
  if (ret < 0)
  {
    wderr("ERROR: MAIN: Failed to start /dev/watchdog0: %d\n\r",
          ret);
    close(fd);
    return;
  }

  while(1)
  {
    /* Update once in second */
    sleep(1);

    /* Kick watchdog */
    ret = ioctl(fd, WDIOC_KEEPALIVE, NULL);
    if (ret < 0)
    {
      wderr("ERROR: MAIN: Failed to kick /dev/watchdog0: %d\n\r",
            ret);
    }
  }

  /* Never be executed */
  close(fd);
}
#endif /* CONFIG_WATCHDOG */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: apc3_main
 *
 * Description:
 *   Arlan PonCat3 firmware main function.
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

int apc3_main(int argc, FAR char *argv[])
{
  int ret;

  ret = OK;

  /* Initialize the global data buffers */
  memset(g_gpinp0_data_buffer, 0x0, sizeof(g_gpinp0_data_buffer));
  memset(g_adc0_data_buffer, 0x0, sizeof(g_adc0_data_buffer));

  /* Allocate a memory for the systems pids structure */
  g_system_pids = (task_data_t *)zalloc(sizeof(task_data_t));

  printf("Arlan PonCat3 firmware\n\r");

  /* Get the pid of main task (apc3_main) */
  g_system_pids->selfpid = getpid();

  /************** Starting SYSTEM_0 - Debug Console */
#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_0)
  {
    _info("SYSTEM_0 Starting...\n\r");

    g_system_pids->pids[0] = task_create("SYSTEM_0",
                             CONFIG_INDUSTRY_APC3_SYSTEM_0_PRIORITY,
                             CONFIG_INDUSTRY_APC3_SYSTEM_0_STACKSIZE,
                             system_0_start,
                             argv);
    if (g_system_pids->pids[0] < 0)
    {
      ret = g_system_pids->pids[0];
      _err("ERROR: Failed to start SYSTEM_0 (Debug Console): %d\n\r",
           ret);
     return ret;
    }
  }
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_0 */

  /************** Starting SYSTEM_1 - I2C Slave Interface */
#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_1)
  {
    _info("SYSTEM_1 Starting...\n\r");

    g_system_pids->pids[1] = task_create("SYSTEM_1",
                             CONFIG_INDUSTRY_APC3_SYSTEM_1_PRIORITY,
                             CONFIG_INDUSTRY_APC3_SYSTEM_1_STACKSIZE,
                             system_1_start,
                             argv);
    if (g_system_pids->pids[1] < 0)
    {
      ret = g_system_pids->pids[1];
      _err("ERROR: Failed to start SYSTEM_1 (I2C Slave Interface): %d\n\r",
           ret);
    }
  }
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_1 */

  /************** Starting SYSTEM_2 - Power Manager */
#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2)
  {
    _info("SYSTEM_2 Starting...\n\r");

    g_system_pids->pids[2] = task_create("SYSTEM_2",
                             CONFIG_INDUSTRY_APC3_SYSTEM_2_PRIORITY,
                             CONFIG_INDUSTRY_APC3_SYSTEM_2_STACKSIZE,
                             system_2_start,
                             argv);
    if (g_system_pids->pids[2] < 0)
    {
      ret = g_system_pids->pids[2];
      _err("ERROR: Failed to start SYSTEM_2 (Power Manager): %d\n\r",
           ret);
    }
  }
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */

  /************** Starting SYSTEM_3 - LED Pointer */
#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_3)
  {
    _info("SYSTEM_3 Starting...\n\r");

    g_system_pids->pids[3] = task_create("SYSTEM_3",
                             CONFIG_INDUSTRY_APC3_SYSTEM_3_PRIORITY,
                             CONFIG_INDUSTRY_APC3_SYSTEM_3_STACKSIZE,
                             system_3_start,
                             argv);
    if (g_system_pids->pids[3] < 0)
    {
      ret = g_system_pids->pids[3];
      _err("ERROR: Failed to start SYSTEM_3 (LED Pointer): %d\n\r",
           ret);
    }
  }
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_3 */

  /************** Starting SYSTEM_4 - Reset Manager */
#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_4)
  {
    _info("SYSTEM_4 Starting...\n\r");

    g_system_pids->pids[4] = task_create("SYSTEM_4",
                             CONFIG_INDUSTRY_APC3_SYSTEM_4_PRIORITY,
                             CONFIG_INDUSTRY_APC3_SYSTEM_4_STACKSIZE,
                             system_4_start,
                             argv);
    if (g_system_pids->pids[4] < 0)
    {
      ret = g_system_pids->pids[4];
      _err("ERROR: Failed to start SYSTEM_4 (Reset Manager): %d\n\r",
           ret);
    }
  }
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_4 */


#if defined(CONFIG_WATCHDOG)
  watchdog();
#endif /* CONFIG_WATCHDOG */

  /* Never be executed */
  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3 */
