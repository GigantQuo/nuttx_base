/****************************************************************************
 * apps/industry/apc3/system_3/system_3.h
 ****************************************************************************/

#ifndef __APPS_INDUSTRY_APC3_SYSTEM_3_H
#define __APPS_INDUSTRY_APC3_SYSTEM_3_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include "industry/apc3.h"

#include <nuttx/ioexpander/gpint.h>

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_3)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ELEDSOUT               2001
#define ELEDSOUT_STR           "LEDs system is out of control"
#define EUNKNOWN               2002
#define EUNKNOWN_STR           "Unknown code"

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef struct
{
  int fd;
  char *devpath;
  char pinmask;
  struct gpint_enable_s enable;
} sig_ctx_s3_t;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
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

int system_3_start(int argc, char* argv[]);

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
                      void *context);


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_3 */
#endif /* __APPS_INDUSTRY_APC3_SYSTEM_3_H */
