/****************************************************************************
 * apps/industry/apc3/system_4/system_4.h
 ****************************************************************************/

#ifndef __APPS_INDUSTRY_APC3_SYSTEM_4_H
#define __APPS_INDUSTRY_APC3_SYSTEM_4_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include "industry/apc3.h"

#include <nuttx/ioexpander/gpint.h>

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_4)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ERSTOUT               2001
#define ERSTOUT_STR           "Reset system is out of control"

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef struct
{
  int fd;
  char *devpath;
  struct gpint_enable_s enable;
} sig_ctx_s4_t;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: system_4_start
 *
 * Description:
 *   The Arlan PonCat3 Reset Manager start function.
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

int system_4_start(int argc, char* argv[]);

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
                       void *context);


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_4 */
#endif /* __APPS_INDUSTRY_APC3_SYSTEM_4_H */
