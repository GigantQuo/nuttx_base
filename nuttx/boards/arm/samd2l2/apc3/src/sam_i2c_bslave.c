/****************************************************************************
 * boards/arm/samd2l2/apc3/src/sam_i2c_bslave.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>

#include <nuttx/config.h>
#include <arch/board/board.h>
#include <nuttx/board.h>

#include <nuttx/i2c/i2c_slave.h>

#include "sam_i2c_slave.h"

#include "apc3.h"

#ifdef CONFIG_I2C_SLAVE_DRIVER

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
 * Name: sam_i2c_slave_register
 *
 * Description:
 *   Register one I2C drivers for the I2C tool.
 *
 ****************************************************************************/

struct i2c_slave_s* sam_i2c_slave_register(	int bus,
                                            int addr)
{
  struct i2c_slave_s* i2c;
  int ret;

  i2cinfo("\nI2C%d-slave Initializing!\n", bus);

  i2c = sam_i2cbus_slave_initialize(bus);
  if (i2c == NULL)
  {
    _err("ERROR: Failed to get I2C%d interface\n", bus);
    return i2c;
  }
  else
  {
    ret = i2c_slave_register(	i2c,
                              bus,
                              addr,
                              SAM_NBITS);
    if (ret < 0)
    {
      _err("ERROR: Failed to register I2C%d driver: %d\n", bus, ret);
      sam_i2cbus_slave_uninitialize(i2c);
      return NULL;
    }

    i2cinfo("I2C%d-slave Initializing is done!\n", bus);
  }
  return i2c;
}

#endif
