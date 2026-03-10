/****************************************************************************
 * arch/arm/src/samd2l2/sam_board_reset_cause.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <debug.h>

#include "arm_internal.h"
#include "sam_config.h"

#include <sys/boardctl.h>

#include "sam_pm.h"

#ifdef CONFIG_BOARDCTL_RESET_CAUSE
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_reset_cause
 *
 * Description:
 *   Get the cause of last board reset. This should call architecture
 *   specific logic to handle the register read.
 *
 * Input Parameters:
 *   cause - Pointer to boardioc_reset_cause_s structure to which the
 *      reason (and potentially subreason) is saved.
 *
 * Returned Value:
 *   This functions should always return successfully with 0. We save
 *   BOARDIOC_RESETCAUSE_UNKOWN in cause structure if we are
 *   not able to get last reset cause from HW (which is unlikely).
 *
 ****************************************************************************/

int board_reset_cause(struct boardioc_reset_cause_s *cause)
{
  uint8_t rst_cause;

  /* Get the reset cause from hardware */

  rst_cause = getreg8(SAM_PM_RCAUSE);

  switch (rst_cause)
  {
    case PM_RCAUSE_POR:

      /* Power up */

      cause->cause = BOARDIOC_RESETCAUSE_SYS_CHIPPOR;
      break;
    case PM_RCAUSE_WDT:

      /* Watchdog error */

      cause->cause = BOARDIOC_RESETCAUSE_CPU_RWDT;
      break;
    case PM_RCAUSE_SYST:

      /* SW reset */

      cause->cause = BOARDIOC_RESETCAUSE_CPU_SOFT;
      break;
    case PM_RCAUSE_EXT:

      /* Reset from user by pressing reset button */

      cause->cause = BOARDIOC_RESETCAUSE_PIN;
      break;
    case PM_RCAUSE_BOD12:
    case PM_RCAUSE_BOD33:

      /* Reset from user by brownout */

      cause->cause = BOARDIOC_RESETCAUSE_SYS_BOR;
      break;
    default:

      /* Unknown cause returned from HW */

      cause->cause = BOARDIOC_RESETCAUSE_UNKOWN;
      break;
  }

  return OK;
}
#endif /* CONFIG_BOARDCTL_RESET_CAUSE */

/****************************************************************************
 * Name: board_reset
 *
 * Description:
 *   Reset board.  Support for this function is required by board-level
 *   logic if CONFIG_BOARDCTL_RESET is selected.
 *
 * Input Parameters:
 *   status - Status information provided with the reset event.  This
 *            meaning of this status information is board-specific.  If not
 *            used by a board, the value zero may be provided in calls to
 *            board_reset().
 *
 * Returned Value:
 *   If this function returns, then it was not possible to power-off the
 *   board due to some constraints.  The return value int this case is a
 *   board-specific reason for the failure to shutdown.
 *
 ****************************************************************************/

#ifdef CONFIG_BOARDCTL_RESET
int board_reset(int status)
{
  up_systemreset();
  return 0;
}
#endif /* CONFIG_BOARDCTL_RESET */
