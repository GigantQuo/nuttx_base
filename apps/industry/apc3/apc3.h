/****************************************************************************
 * apps/industry/apc3/apc3.h
 ****************************************************************************/

#ifndef __APPS_INDUSTRY_APC3_H
#define __APPS_INDUSTRY_APC3_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/compiler.h>
#include <nuttx/config.h>

#include "industry/apc3.h"

#ifdef CONFIG_INDUSTRY_APC3_SYSTEM_0
#include "system_0/system_0.h"
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_0 */

#ifdef CONFIG_INDUSTRY_APC3_SYSTEM_1
#include "system_1/system_1.h"
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_1 */

#ifdef CONFIG_INDUSTRY_APC3_SYSTEM_2
#include "system_2/system_2.h"
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */

#ifdef CONFIG_INDUSTRY_APC3_SYSTEM_3
#include "system_3/system_3.h"
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_3 */

#ifdef CONFIG_INDUSTRY_APC3_SYSTEM_4
#include "system_4/system_4.h"
#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_4 */

#if defined(CONFIG_INDUSTRY_APC3)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
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

int apc3_main(int argc, FAR char* argv[]);

#endif /* CONFIG_INDUSTRY_APC3 */
#endif /* __APPS_INDUSTRY_APC3_H */
