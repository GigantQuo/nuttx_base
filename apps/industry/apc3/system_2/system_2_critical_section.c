/****************************************************************************
 * apps/industry/apc3/system_2/system_2_critical_section.c
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
 * Name: SYS2_enter_critical_section
 *
 * Description:
 *   Protect a critical sections by blocking all signals receiving
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_enter_critical_section(void)
{
  sigset_t full_mask;
  int ret;

  ret = OK;

  /* Initalize the masks */
  ret = sigemptyset(&full_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to empty sigset (full_mask): %d\n\r",
         ret);
    return ret;
  }

  /* Block all signals */
  ret = sigfillset(&full_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to fill sigset (full_mask): %d\n\r",
         ret);
    return ret;
  }

  /* Apply the mask on this task */
  ret = sigprocmask(SIG_SETMASK,
                    &full_mask,
                    NULL);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to apply sigset (full_mask): %d\n\r",
         ret);
    return ret;
  }

  return ret;
}

/****************************************************************************
 * Name: SYS2_leave_critical_section
 *
 * Description:
 *   Leave from critical section by enabling the receiving signals.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

void SYS2_leave_critical_section(void)
{
  /* Apply the mask on this task */
  sigprocmask(SIG_SETMASK,
              &g_wait_mask,
              NULL);
}



#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
