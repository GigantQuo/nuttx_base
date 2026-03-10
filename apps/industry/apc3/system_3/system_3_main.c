/****************************************************************************
 * apps/industry/apc3/system_3/system_3_main.c
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

static inline int SYS3_siginit(struct sigaction *act,
                               sigset_t *wait_mask,
                               struct sigevent *notify,
                               sig_ctx_s3_t *sig_ctx);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS3_siginit
 *
 * Description:
 *   Initalize the task signal.
 *
 * Arguments:
 *   act       - a pointer to a sigaction struct to be initialized
 *   wait_mask - a pointer to a sigset with mask of blocked signals
 *   notify    - a pointer to a notify structure to get it to interrupt
 *                handler in the gpint0 driver.
 *   sig_ctx   - a pointer to a signal context structure with a data for
 *                the signal handler.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS3_siginit(struct sigaction *act,
                               sigset_t *wait_mask,
                               struct sigevent *notify,
                               sig_ctx_s3_t *sig_ctx)
{
  int ret;

  ret = OK;

  /* Notify structure initialization */
  notify->sigev_notify = SIGEV_SIGNAL;
  notify->sigev_signo = SIGINT;
  notify->sigev_value.sival_ptr = NULL;
  notify->sigev_value.sival_int = 0x0;


  /* sigaction struct initialization */
  memset(act, 0x0, sizeof(struct sigaction));

  /* Initialize the signal handler */
  act->sa_sigaction = SYS3_led_handler;
  act->sa_flags = SA_SIGINFO;
  act->sa_user = (void *)sig_ctx;

  /* Initalize the masks */
  ret = sigemptyset(&act->sa_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to empty sigset (act): %d\n\r",
         ret);
    return ret;
  }
  ret = sigemptyset(wait_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to empty sigset (wait_mask): %d\n\r",
         ret);
    return ret;
  }

  /* Block all signals */
  ret = sigfillset(&act->sa_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to fill sigset (act): %d\n\r",
         ret);
    return ret;
  }
  ret = sigfillset(wait_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to fill sigset (wait_mask): %d\n\r",
         ret);
    return ret;
  }

  /* Unblock SIGINT signal */
  ret = sigdelset(wait_mask,
                  SIGINT);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to delete signal (wait_mask): %d\n\r",
         ret);
    return ret;
  }
  /* Unblock SIGTTOU signal */
  ret = sigdelset(wait_mask,
                  SIGTTOU);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to delete signal (wait_mask): %d\n\r",
         ret);
    return ret;
  }

  /* Attach the signal handler */
  ret = sigaction(SIGINT,
                  act,
                  NULL);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to set up sigaction (SIGINT): %d\n\r",
         ret);
    return ret;
  }
  /* Attach the signal handler */
  ret = sigaction(SIGTTOU,
                  act,
                  NULL);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to set up sigaction (SIGTTOU): %d\n\r",
         ret);
    return ret;
  }

  /* Apply the mask on this task */
  ret = sigprocmask(SIG_SETMASK,
                    &act->sa_mask,
                    NULL);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to apply sigset (act): %d\n\r",
         ret);
    return ret;
  }

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: system_3_start
 *
 * Description:
 *   The Arlan PonCat3 LED Pointer start function.
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

int system_3_start(int argc, char* argv[])
{
  struct sigaction act;
  sigset_t wait_mask;
  struct sigevent notify;
  static sig_ctx_s3_t sig_ctx;
  int ret;

  _info("SYSTEM_3 Started successfully!");

  ret = OK;

  /* Signal context structure initialization */
  sig_ctx.devpath = "/dev/gpint0";
  sig_ctx.pinmask = (1 << BOARD_GPINT0_BTN1) |
                    (1 << BOARD_GPINT0_BTN2);

  /* Enable all interrupts in the /dev/gpint0 driver */
  sig_ctx.enable.mode = GPINT_MODE_MASKED;
  sig_ctx.enable.val.mask = &sig_ctx.pinmask;

  /* Initialize the signal */
  ret = SYS3_siginit(&act,
                     &wait_mask,
                     &notify,
                     &sig_ctx);

  if (ret < 0)
  {
    _err("ERROR: SYSTEM_3: Failed to initialize signal: %d\n",
         ret);
    return ret;
  }

  /* Open the interrupt driver */
  sig_ctx.fd = open(sig_ctx.devpath,
                    O_RDONLY);
  if (sig_ctx.fd < 0)
  {
    ret = sig_ctx.fd;
    gpioerr("ERROR: SYSTEM_3: Failed to open gpint0: %d\n\r",
            ret);
    return ret;
  }

  /* Register the signal */
  ret = ioctl(sig_ctx.fd,
              GPINT_REGISTER,
              (unsigned long)&notify);
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_3: Failed to setup for signal: %d\n",
            ret);
    close(sig_ctx.fd);
    return ret;
  }

  /* Enable interrupts */
  ret = ioctl(sig_ctx.fd,
              GPINT_BIT_ENABLE,
              (unsigned long)&sig_ctx.enable);
  if (ret < 0)
  {
    gpioerr("ERROR: SYSTEM_3: Failed to enable interrupts: %d\n",
            ret);
    close(sig_ctx.fd);
    return ret;
  }


  /* Ever wait for SIGINT or SIGTTOU signals */
  while (1)
  {
    ret = sigsuspend(&wait_mask);

    /* Enable the interrupts again
     * Use the same mode and pinmask.
     */
    ret = ioctl(sig_ctx.fd,
                GPINT_BIT_ENABLE,
                (unsigned long)&sig_ctx.enable);
    if (ret < 0)
    {
      gpioerr("ERROR: SYSTEM_3: Failed to enable interrupts: %d\n",
              ret);
      close(sig_ctx.fd);
      return ret;
    }
  }

  /* Will never be executed */
  close(sig_ctx.fd);

  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_3 */
