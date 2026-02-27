/****************************************************************************
 * boards/arm/samd2l2/switchcore/src/sam_usb.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

#include <nuttx/usb/usbdev.h>

#if defined(CONFIG_SAMD2L2_USB)
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name:  sam_usbsuspend
 *
 * Description:
 *   Board logic must provide the sam_usbsuspend logic if the USBDEV driver
 *   is used.
 *   This function is called whenever the USB enters or leaves suspend mode.
 *   This is an opportunity for the board logic to shutdown clocks, power,
 *   etc. while the USB is suspended.
 *
 ****************************************************************************/

#ifdef CONFIG_USBDEV
void sam_usb_suspend(struct usbdev_s *dev, bool resume)
{
	uinfo("board: resume: %d\n", resume);
}
#endif /* defined(CONFIG_SAMD2L2_USB) */

#endif /* CONFIG_SAMDL_USB */
