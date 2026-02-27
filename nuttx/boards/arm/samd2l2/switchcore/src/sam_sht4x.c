/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_sht4x.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

#include <nuttx/i2c/i2c_master.h>
#include <nuttx/sensors/sht4x.h>

#include "sam_i2c_master.h"

#ifdef CONFIG_SENSORS_SHT4X
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
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

int board_sht4x_initialize(	struct i2c_master_s* i2c,
							int devno,
							int busno)
{
	int ret;

	sninfo("\nInitializing SHT4x!\n");

	/* Initialize SHT4X */
	if (i2c)
	{
		ret = sht4x_register(i2c, devno, CONFIG_SHT4X_I2C_ADDR);
		if (ret < 0)
		{
			snerr("ERROR: Error registering SHT4x in I2C%d\n", busno);
		}
	}
	else
	{
		ret = -ENODEV;
	}

	return ret;
}

#endif /* CONFIG_SENSORS_SHT4X */
