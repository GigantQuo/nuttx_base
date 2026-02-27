/****************************************************************************
 * boards/arm/samd2l2/switchore/src/switchore.h
 ****************************************************************************/

#ifndef __BOARDS_ARM_SAMD2L2_SWITCHCORE_PRIVATE_SRC_SWITCHCORE_PRIVATE_H
#define __BOARDS_ARM_SAMD2L2_SWITCHCORE_PRIVATE_SRC_SWITCHCORE_PRIVATE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_SAM_I2C_NBITS
#	define SAM_NBITS				10
#else
#	define SAM_NBITS				7
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
#endif

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
#endif

/****************************************************************************
 * Name: sam_gpio_initialize
 *
 * Description:
 *   Initialize GPIO drivers
 *
 ****************************************************************************/

#if defined(CONFIG_DEV_GPIO)
int sam_gpio_initialize(void);
#endif

/****************************************************************************
 * Name: sam_i2c_cdrvr
 *
 * Description:
 *   Register I2C drivers for the I2C tool.
 *
 ****************************************************************************/

#ifdef CONFIG_I2C_DRIVER
struct i2c_master_s* sam_i2c_register(int bus);
#endif

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
#endif

/****************************************************************************
 * Name: board_sht4x_initialize
 *
 * Description:
 *   Initialize and register the SHT4x Temperature Sensor driver.
 *
 * Input Parameters:
 *   devno - The device number, used to build the device
 *   busno - The I2C bus number
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

#ifdef CONFIG_SENSORS_SHT4X
int board_sht4x_initialize(	struct i2c_master_s* i2c,
							int devno,
							int busno);
#endif

/****************************************************************************
 * Name: board_sht4x_initialize
 *
 * Description:
 *   Initialize and register the SHT4x Temperature Sensor driver.
 *
 * Input Parameters:
 *   devno - The device number, used to build the device
 *   busno - The I2C bus number
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

#ifdef CONFIG_SENSORS_LPS25H
int board_lps22h_initialize (	struct i2c_master_s* i2c,
								int devno,
								int busno);
#endif

/****************************************************************************
 * Name: board_max662_initialize
 *
 * Description:
 *   Initialize and register the SHT4x Temperature Sensor driver.
 *
 * Input Parameters:
 *   devno - The device number, used to build the device
 *   busno - The I2C bus number
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

#ifdef CONFIG_SENSORS_MAX662
int board_max662_initialize (	struct i2c_master_s* i2c,
								int devno,
								int busno);
#endif

/****************************************************************************
 * Name: sam_wdt_register
 *
 * Description:
 *   Initialize and register WatchDog driver.
 *
 * Input Parameters:
 *   devno - The device path to driver
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

#if defined(CONFIG_SAMD2L2_WDT)
void sam_wdt_register(const char* devpath);
#endif

/****************************************************************************
 * Name: sam_bringup
 *
 * Description:
 *   Bring up board features
 *
 ****************************************************************************/

int sam_bringup(void);

#undef EXTERN
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __BOARDS_ARM_SAMD2L2_SWITCHCORE_PRIVATE_SRC_SWITCHCORE_PRIVATE_H */
