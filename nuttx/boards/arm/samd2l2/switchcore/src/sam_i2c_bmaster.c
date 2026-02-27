/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_i2c_bmaster.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

#include <nuttx/i2c/i2c_master.h>

#include "sam_i2c_master.h"

#ifdef CONFIG_I2C_DRIVER
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_i2c_register
 *
 * Description:
 *   Register one I2C drivers for the I2C tool.
 *
 ****************************************************************************/

struct i2c_master_s* sam_i2c_register(int bus)
{
	int ret;
	struct i2c_master_s* i2c;

	i2cinfo("\nI2C%d-master Initializing!\n", bus);

	i2c = sam_i2c_master_initialize(bus);
	if (i2c == NULL)
	{
		_err("ERROR: Failed to get I2C%d interface\n", bus);
	}
	else
	{
		ret = i2c_register(i2c, bus);
		if (ret < 0)
		{
			_err("ERROR: Failed to register I2C%d driver: %d\n", bus, ret);
			sam_i2c_uninitialize(i2c);
		}
	}

	i2cinfo("I2C%d-master Initializing is done!\n", bus);
	return i2c;
}

#endif /* CONFIG_I2C_DRIVER */
