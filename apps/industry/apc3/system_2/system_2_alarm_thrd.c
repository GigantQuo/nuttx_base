/****************************************************************************
 * apps/industry/apc3/system_2/system_2_alarm_thrd.c
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
#include <pthread.h>

#include <sys/ioctl.h>

#include <arch/board/board.h>

#include <nuttx/ioexpander/gpout.h>

#include "system_2.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static void SYS2_alrm_cleanup_handler(void *arg);

static void *SYS2_alrm_thr(void *arg);

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_alrm_cleanup_handler
 *
 * Description:
 *   Cleanup handler for alarm thread. Closes the file descriptor.
 *
 * Arguments:
 *   arg - the standart universal argument
 *
 ****************************************************************************/

static void SYS2_alrm_cleanup_handler(void *arg)
{
  int fd = (int)(long)arg;

  if (fd >= 0)
  {
    char reset[1];

    /* Turn off the alarm output */
    memset(reset, 0x00, sizeof(reset));

    /* We have no any error check
     * becouse it is cleanup handler
     */
    write(fd, reset, sizeof(reset));

    /* Close the fd that was opened in the thread function */
    close(fd);
  }
}

/****************************************************************************
 * Name: SYS2_alrm_thr
 *
 * Description:
 *   Alarm blinking function.
 *
 * Arguments:
 *   arg - a standart argument of the pthread function
 *
 * Returned values:
 *   NULL - always (a standart returned value of the pthread function),
 *            but it will never return, only external cancel method
 *
 ****************************************************************************/

static void *SYS2_alrm_thr(void *arg)
{
  struct bitval_s alrm_out;
  bool blinking;
  char *devpath;
  int fd;

  /* Set the cancel type - DEFERRED */
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  devpath = "/dev/gpout1";

  /* We have no any error check
   * becouse it is already
   * bad script
   */
  fd = open(devpath, O_WRONLY);

  /* Installing the cleanup handler
   * for close fd and
   * turn off the alarm output
   */
  pthread_cleanup_push(SYS2_alrm_cleanup_handler, (void *)(long)fd);

  alrm_out.bit = 1;
  alrm_out.val = &blinking;

  blinking = 0;

  /* Infinite loop -
   * the pthread will run
   * until an external cancellaion
   * is received
   */
  while (1)
  {
    blinking = (blinking == 1) ? 0 : 1;

    ioctl(fd, GPOUT_BIT_WRITE, &alrm_out);

    usleep(250000); /* 250ms */

    /* The point of the cancellation */
    pthread_testcancel();
  }

  pthread_cleanup_pop(0);

  pthread_exit((void *)0);
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_out_of_control
 *
 * Description:
 *   Execute the PANIC algorithm.
 *
 ****************************************************************************/

void SYS2_out_of_control(void)
{
  int ret;

  ret = OK;

  printf("\n\rPANIC! PANIC! PANIC!\n\r");
  printf("!THE POWER IS OUT OF CONTROL!\n\r");

  if (alarm_st == SYSTEM_2_NORMAL_STATE)
  {
    ret = pthread_create(&alrm_thr_id,       /* Thread ID */
                         &alrm_thr_attr,     /* Thread attributes */
                         SYS2_alrm_thr,      /* Thread function */
                         NULL);              /* Thread argument */
    if (ret < 0)
    {
      serr("ERROR: SYSTEM_2: Failed to create pthread: %d\n\r",
           ret);
      return;
    }

    alarm_st = SYSTEM_2_ALARM_STATE;
  }
}

/****************************************************************************
 * Name: SYS2_under_control
 *
 * Description:
 *   Stop the PANIC algorithm.
 *
 ****************************************************************************/

void SYS2_under_control(void)
{
  int ret;
  void *alrm_thread_retval;

  ret = OK;

  if (alarm_st == SYSTEM_2_ALARM_STATE)
  {
    /* Stop the alarm blinking thread */
    ret = pthread_cancel(alrm_thr_id);
    if (ret < 0)
    {
      serr("ERROR: SYSTEM_2: Failed to cancel pthread: %d\n\r",
           ret);
      return;
    }

    /* Wait until the thread closes */
    ret = pthread_join(alrm_thr_id, &alrm_thread_retval);
    if (ret < 0)
    {
      serr("ERROR: SYSTEM_2: Failed to join pthread: %d\n\r",
           ret);
      return;
    }

    if (alrm_thread_retval == PTHREAD_CANCELED)
    {
      /* If if is successfully closed
       * then perform reset the pthread variables
       */
      alrm_thr_id = 0;
      alarm_st = SYSTEM_2_NORMAL_STATE;
    }
    else _err("ERROR: SYSTEM_2: Failed to terminate alarm thread\n\r");
  }
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
