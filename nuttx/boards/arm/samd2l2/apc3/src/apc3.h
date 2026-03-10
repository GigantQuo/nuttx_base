/****************************************************************************
 * boards/arm/samd2l2/apc3/src/apc3.h
 ****************************************************************************/

#ifndef __BOARDS_ARM_SAMD2L2_SAMD20_APC3_H
#define __BOARDS_ARM_SAMD2L2_SAMD20_APC3_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <stdint.h>

#include <arch/irq.h>
#include <nuttx/irq.h>

#include "sam_config.h"
#include "sam_pinmap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_SAM_I2C_NBITS
#	define SAM_NBITS        10
#else
#	define SAM_NBITS        7
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
  #define EXTERN extern "C"
extern "C"
{
#else
  #define EXTERN extern
#endif /* __cplusplus */

/****************************************************************************
 * Public Functions Definitions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_boardinitialize
 *
 * Description:
 *   All architectures must provide the following entry point.
 *   This entry point is called early in the initialization -- after all
 *   memory has been configured and mapped but before any devices have been
 *   initialized.
 *
 ****************************************************************************/

void sam_boardinitialize(void);

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description:
 *   Perform application specific initialization.  This function is never
 *   called directly from application code, but only indirectly via the
 *   (non-standard) boardctl() interface using the command BOARDIOC_INIT.
 *
 * Input Parameters:
 *   arg - The boardctl() argument is passed to the board_app_initialize()
 *         implementation without modification.  The argument has no
 *         meaning to NuttX; the meaning of the argument is a contract
 *         between the board-specific initialization logic and the
 *         matching application logic.  The value could be such things as a
 *         mode enumeration value, a set of DIP switch switch settings, a
 *         pointer to configuration data read from a file or serial FLASH,
 *         or whatever you would like to do with it.  Every implementation
 *         should accept zero/NULL as a default configuration.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   any failure to indicate the nature of the failure.
 *
 ****************************************************************************/

#if defined(CONFIG_BOARDCTL)
int board_app_initialize(uintptr_t arg);
#endif /* CONFIG_BOARDCTL */

/****************************************************************************
 * Name: sam_bringup
 *
 * Description:
 *   Bring up board features
 *
 ****************************************************************************/

int sam_bringup(void);

/****************************************************************************
 * Name: sam_i2c_slave_register
 *
 * Description:
 *   Register one I2C drivers for the I2C tool.
 *
 ****************************************************************************/

#ifdef CONFIG_I2C_SLAVE_DRIVER
struct i2c_slave_s* sam_i2c_slave_register(	int bus,
                                            int addr);
#endif /* CONFIG_I2C_SLAVE_DRIVER */

/****************************************************************************
 * Name: sam_adc_register
 *
 * Description:
 *   Initialize ADC and register the ADC driver.
 *
 ****************************************************************************/

#ifdef CONFIG_ADC
void sam_adc_register(void);
#endif /* CONFIG_ADC */

/****************************************************************************
 * Name: sam_gpout_register
 *
 * Arguments:
 *   minor - number of registering driver
 *
 * Description:
 *   Register GPOUT driver
 *
 ****************************************************************************/

#ifdef CONFIG_DEV_GPOUT
void sam_gpout_register(unsigned int minor);
#endif /* CONFIG_DEV_GPOUT */

/****************************************************************************
 * Name: sam_gpinp_register
 *
 * Arguments:
 *   minor - number of registering driver
 *
 * Description:
 *   Register GPINP driver
 *
 ****************************************************************************/

#ifdef CONFIG_DEV_GPINP
void sam_gpinp_register(unsigned int minor);
#endif /* CONFIG_DEV_GPINP */

/****************************************************************************
 * Name: sam_gpint_register
 *
 * Arguments:
 *  minor - number of registering driver
 *
 * Description:
 *   Register GPINT driver
 *
 ****************************************************************************/

#ifdef CONFIG_DEV_GPINT
void sam_gpint_register(unsigned int minor);
#endif /* CONFIG_DEV_GPINT */

/****************************************************************************
 * Name: sam_wdt_register
 *
 * Description:
 *   Initialize and register WatchDog driver.
 *
 ****************************************************************************/

#ifdef CONFIG_WATCHDOG
void sam_wdt_register(void);
#endif /* CONFIG_WATCHDOG */

/****************************************************************************
 * Name: sam_nvm
 *
 * Description:
 *   Initialize the NVM controller.
 *
 ****************************************************************************/

#if defined(CONFIG_SAMD2L2_NVMCTRL)
void sam_nvm(void);
#endif /* CONFIG_SAMD2L2_NVMCTRL */


#undef EXTERN
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __BOARDS_ARM_SAMD2L2_SAMD20_APC3_H */
