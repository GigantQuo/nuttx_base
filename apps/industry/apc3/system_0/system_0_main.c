/****************************************************************************
 * apps/industry/apc3/system_0/system_0_main.c
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

#include <arch/board/board.h>

#include "nshlib/nshlib.h"

#include "system_0.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_0)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: system_0_start
 *
 * Description:
 *   The Arlan PonCat3 Debug Console start function.
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

int system_0_start(int argc, char* argv[])
{
  int ret;

  _info("SYSTEM_0 Started successfully!");

  ret = OK;

  /* Initialize the NSH library */
  nsh_initialize();

#ifdef CONFIG_NSH_CONSOLE
  /* If the serial console front end is selected, run it on this thread */
  ret = nsh_consolemain(0x0, NULL);

  /* nsh_consolemain() should not return.  So if we get here, something
   * is wrong.
   */
  _err("ERROR: SYSTEM_0: nsh_consolemain() returned: %d\n\r",
       ret);
  ret = 1;
#endif /* CONFIG_NSH_CONSOLE */

  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_0 */
