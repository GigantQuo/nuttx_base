/****************************************************************************
 * apps/industry/apc3/system_2/system_2_rstrq_handler.c
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

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_rstrq_handler
 *
 * Description:
 *   The signal handler function that performs the requested reset.
 *
 * Arguments:
 *   signo   - the number of received signal
 *   info    - a pointer to a signal info structure with a necessry data
 *   context - the context of this task
 *
 ****************************************************************************/

void SYS2_rstrq_handler(int signo,
                        siginfo_t *info,
                        void *context)
{}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
