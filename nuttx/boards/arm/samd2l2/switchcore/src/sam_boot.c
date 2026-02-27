/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_boot.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
*****************************************************************************/

/****************************************************************************
 * Name: sam_boardinitialize
 *
 * Description:
 *   All SAM3U architectures must provide the following entry point.
 *   This entry point is called early in the initialization -- after all
 *   memory has been configured and mapped but before any devices have been
 *   initialized.
 *
 ****************************************************************************/

void sam_boardinitialize(void)
{}

#ifdef CONFIG_BOARD_LATE_INITIALIZE
void board_late_initialize(void)
{
	/* Perform board initialization */

	sam_bringup();
}
#endif /* CONFIG_BOARD_LATE_INITIALIZE */
