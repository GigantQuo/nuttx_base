/****************************************************************************
 * drivers/sensors/max662.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/nuttx.h>

#include <stdlib.h>
#include <fixedmath.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/kmalloc.h>
#include <nuttx/fs/fs.h>
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/sensors/max662.h>
#include <nuttx/sensors/sensor.h>

#if defined(CONFIG_I2C) && defined(CONFIG_SENSORS_MAX662)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MAX662_ADDR						CONFIG_MAX662_I2C_ADDR

#define MAX662_FREQ						CONFIG_MAX662_I2C_FREQUENCY


#define MAX662_POINTER_REG				(NULL)
#define MAX662_TEMP_REG					(0x0)
#define MAX662_CONFIG_REG				(0x1)
#define MAX662_TLOW_REG					(0x2)
#define MAX662_THIGH_REG				(0x3)

/* Configuration register modes definition **********************************/

/* Power modes */
#define MAX662_PWR_MODE_MSK				(0x1 << 0)

#define MAX662_NORM_MODE				(0x0 << 0)
#define MAX662_SHDN_MODE				(0x1 << 0)

/* OT output configuration */
#define MAX662_OT_MODE_MSK				(0x1 << 1)

#define MAX662_COMPARATOR_MODE			(0x0 << 1)
#define MAX662_INTERRUPT_MODE			(0x1 << 1)

/* OT output polarity configuration */
#define MAX662_ACT_MODE_MSK				(0x1 << 2)

#define MAX662_ACTLOW_MODE				(0x0 << 2)
#define MAX662_ACTHIGH_MODE				(0x1 << 2)

/* Fault queue depth configuration */
#define MAX662_FAULT_MODE_MSK			(0x3 << 3)

#define MAX662_FAULT_QUEUE_1			(0x0 << 3)
#define MAX662_FAULT_QUEUE_2			(0x1 << 3)
#define MAX662_FAULT_QUEUE_4			(0x2 << 3)
#define MAX662_FAULT_QUEUE_6			(0x3 << 3)

/* TLOW register modes definition *******************************************/

#define MAX662_TLOW_DEFAULT				(0x4B00) /* +75°C */

/* THIGH register modes definition ******************************************/

#define MAX662_THIGH_DEFAULT			(0x5000) /* +80°C */

/* Data combined from bytes to uint16_t */

#define COMBINE(d)						(((uint16_t)(d)[0] << 8) | \
										((uint16_t)(d)[1]) << 4)

#define CLEARBITS(reg, msk)				((reg) &= ~msk)
#define SETBITS(reg, msk)				((reg) |= msk)

/****************************************************************************
 * Private Type
 ****************************************************************************/

struct max662_dev_s
{
	FAR struct i2c_master_s *i2c;
	uint8_t addr;
	uint32_t freq;

	bool mode;
	bool activated;
	bool irqenabled;
	bool overtemp;
	volatile bool int_pending;


	mutex_t devlock;
	sem_t waitsem;

	FAR max662_config_t *config;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* Sensor methods */

static int max662_open(			FAR struct file *filep);

static int max662_close(		FAR struct file *filep);

static ssize_t max662_read(		FAR struct file *filep,
								FAR char *buffer,
								size_t buflen);

static ssize_t max662_write(	FAR struct file *filep,
								FAR const char *buffer,
								size_t buflen);

static int max662_ioctl(		FAR struct file *filep,
								int cmd,
								unsigned long arg);


/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_max662_fops =
{
	.open = max662_open,
	.close = max662_close,
	.read = max662_read,
	.write = max662_write,
	.seek = NULL,
	.ioctl = max662_ioctl
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: max662_getreg8
 *
 * Description:
 *   Read from an 8-bit max662 register
 *
 ****************************************************************************/

static uint8_t max662_getreg8(	FAR struct max662_dev_s *priv,
								uint8_t regaddr)
{
	struct i2c_msg_s msg[2];
	uint8_t regval = 0;
	int ret = OK;

	msg[0].frequency = priv->freq;
	msg[0].addr      = priv->addr;
	msg[0].flags     = 0;
	msg[0].buffer    = &regaddr;
	msg[0].length    = 1;

	msg[1].frequency = priv->freq;
	msg[1].addr      = priv->addr;
	msg[1].flags     = I2C_M_READ;
	msg[1].buffer    = &regval;
	msg[1].length    = 1;

	ret = I2C_TRANSFER(priv->i2c, msg, 2);
	if (ret < 0)
	{
		snerr("I2C_TRANSFER failed: %d\n", ret);
		return 0;
 	}

	return regval;
}


/****************************************************************************
 * Name: max662_getregs
 *
 * Description:
 *   Read two 8-bit from a MAX662 register
 *
 ****************************************************************************/

static int max662_getregs(	FAR struct max662_dev_s *priv,
							uint8_t regaddr,
							uint8_t *rxbuffer,
							uint8_t length)
{
	struct i2c_msg_s msg[2];
	int ret = OK;

	msg[0].frequency = priv->freq;
	msg[0].addr      = priv->addr;
	msg[0].flags     = 0;
	msg[0].buffer    = &regaddr;
	msg[0].length    = 1;

	msg[1].frequency = priv->freq;
	msg[1].addr      = priv->addr;
	msg[1].flags     = I2C_M_READ;
	msg[1].buffer    = rxbuffer;
	msg[1].length    = length;

	ret = I2C_TRANSFER(priv->i2c, msg, 2);
	if (ret < 0)
	{
	snerr("I2C_TRANSFER failed: %d\n", ret);
		return -1;
	}

	return ret;
}


/****************************************************************************
 * Name: max662_putreg8
 *
 * Description:
 *   Write to an 8-bit max662 register
 *
 ****************************************************************************/

static int max662_putreg8(	FAR struct max662_dev_s *priv,
							uint8_t regaddr,
							uint8_t regval)
{
	struct i2c_msg_s msg[2];
	uint8_t txbuffer[2];
	int ret = OK;

	txbuffer[0] = regaddr;
	txbuffer[1] = regval;

	msg[0].frequency = priv->freq;
	msg[0].addr      = priv->addr;
	msg[0].flags     = 0;
	msg[0].buffer    = txbuffer;
	msg[0].length    = 2;

	ret = I2C_TRANSFER(priv->i2c, msg, 1);
	if (ret < 0)
	{
		snerr("I2C_TRANSFER failed: %d\n", ret);
	}

	return ret;
}


/****************************************************************************
 * Name: max662_putregs
 *
 * Description:
 *   Write to max662 register
 *
 ****************************************************************************/

static int max662_putregs(	FAR struct max662_dev_s *priv,
							uint8_t regaddr,
							uint8_t* valbuf,
							uint8_t number)
{
	struct i2c_msg_s msg[1];
	uint8_t txbuffer[number + 1];
	int ret = OK;

	txbuffer[0] = regaddr;
	memcpy(txbuffer + 1, valbuf, number);

	msg[0].frequency = priv->freq;
	msg[0].addr      = priv->addr;
	msg[0].flags     = 0;
	msg[0].buffer    = txbuffer;
	msg[0].length    = number + 1;

	ret = I2C_TRANSFER(priv->i2c, msg, 1);
	if (ret < 0)
	{
		snerr("I2C_TRANSFER failed: %d\n", ret);
	}

	return ret;
}


/****************************************************************************
 * Name: max662_initialize
 *
 * Description:
 *	Initialize max662 device:
 *		Shutdown mode
 *		Comparator mode
 *		Active high interrup etxint
 *		Fault queue depth = 6
 *
 ****************************************************************************/

static int max662_initialize(FAR struct max662_dev_s *priv)
{
	int ret = OK;
	uint16_t regval;

	/* Clear pending interrupt (just in case) */
	priv->config->irq_clear(priv->config);

	/* Set device mode to default */

	/* Set confuguration register */
	sninfo("Initialization in CONFIG\n");

	regval =	MAX662_SHDN_MODE |
				MAX662_COMPARATOR_MODE |
				MAX662_ACTHIGH_MODE |
				MAX662_FAULT_QUEUE_6;

	ret = max662_putreg8(priv, MAX662_CONFIG_REG, (uint8_t)regval);
	if (ret != OK)
	{
		snerr("Failed to set default config.\n");
		return ret;
	}
	priv->activated = INACTIVE;
	priv->mode = COMPARATOR;
	priv->overtemp = GOOD;

	/* Set Tlow register with default values */
	sninfo("Initialization in Tlow\n");

	regval = MAX662_TLOW_DEFAULT;

	ret = max662_putregs(priv, MAX662_TLOW_REG, (uint8_t*)&regval, 2);
	if (ret != OK)
	{
		snerr("Failed to set default tlow.\n");
		return ret;
	}

	/* Set Thigh register with default values */
	sninfo("Initialization in Thigh\n");

	regval = MAX662_THIGH_DEFAULT;

	ret = max662_putregs(priv, MAX662_THIGH_REG, (uint8_t*)&regval, 2);
	if (ret != OK)
	{
		snerr("Failed to set default thigh.\n");
		return ret;
	}

	return ret;
}


/****************************************************************************
 * Name: max662_set_polarity
 *
 * Description:
 * 	Set polarity of OT interrupt output
 * 	Available only in INTERRUPT mode
 *
 ****************************************************************************/

static inline int max662_set_polarity(	FAR struct max662_dev_s *priv,
										MAX662_POL polarity)
{
	int ret = OK;

	uint8_t regval = max662_getreg8(priv, MAX662_CONFIG_REG);

	polarity ?	SETBITS(regval, MAX662_ACT_MODE_MSK) :
				CLEARBITS(regval, MAX662_ACT_MODE_MSK);

	ret = max662_putreg8(priv, MAX662_CONFIG_REG, regval);

	return ret;
}


/****************************************************************************
 * Name: max662_set_mode
 *
 * Description:
 * 	Set the functionality of OT output as an interrupt or comparator output
 *
 ****************************************************************************/

static inline int max662_set_mode(	FAR struct max662_dev_s *priv,
									MAX662_MODE mode)
{
	int ret = OK;

	uint8_t regval = max662_getreg8(priv, MAX662_CONFIG_REG);

	mode ?	SETBITS(regval, MAX662_OT_MODE_MSK) :
			CLEARBITS(regval, MAX662_OT_MODE_MSK);

	ret = max662_putreg8(priv, MAX662_CONFIG_REG, regval);

	priv->mode = mode;

	return ret;
}


/****************************************************************************
 * Name: max662_set_fault_depth
 *
 * Description:
 * 	Set the fault queue depth to toggle the logic level ob OT output
 * 	Available only in COMPARATOR mode
 *
 ****************************************************************************/

static inline int max662_set_fault_depth(	FAR struct max662_dev_s *priv,
											MAX662_FAULT_DEPTH depth)
{
	int ret = OK;

	uint8_t regval = max662_getreg8(priv, MAX662_CONFIG_REG);
	CLEARBITS(regval, MAX662_FAULT_MODE_MSK);

	switch (depth)
	{
		case (DEPTH_1):
			regval |= MAX662_FAULT_QUEUE_1;
			break;

		case (DEPTH_2):
			regval |= MAX662_FAULT_QUEUE_2;
			break;

		case (DEPTH_4):
			regval |= MAX662_FAULT_QUEUE_4;
			break;

		case (DEPTH_6):
			regval |= MAX662_FAULT_QUEUE_6;
			break;
	}

	ret = max662_putreg8(priv, MAX662_CONFIG_REG, regval);

	return ret;
}


/****************************************************************************
 * Name: max662_int_handler
 *
 * Description:
 * 	MAX662 interrupt hander, which action depends on the current OT output
 * 	mode
 *
 ****************************************************************************/

static int max662_int_handler(	int irq,
								FAR void* context,
								FAR void* arg)
{
	FAR struct max662_dev_s* priv = (FAR struct max662_dev_s*)arg;

	int ret = OK;

	if (priv->mode == INTERRUPT)
	{
		priv->overtemp = !priv->overtemp;
	}
	else
	{
		priv->overtemp = priv->config->get_state(priv->config);
	}

	priv->int_pending = true;
	nxsem_post(&priv->waitsem);

	/* Clear pending interrupt */
	priv->config->irq_clear(priv->config);

	return ret;
}

/****************************************************************************
 * Name: max662_open
 *
 * Description:
 * 	Open the MAX662 driver, what means:
 * 		Changing SHUTDOWN mode to NORMAL;
 * 		Enabling irq on OT output
 *
 ****************************************************************************/

static int max662_open(FAR struct file *filep)
{
	FAR struct inode* inode = filep->f_inode;
	FAR struct max662_dev_s* priv = inode->i_private;

	int ret = OK;
	uint8_t regval;

	ret = nxmutex_lock(&priv->devlock);
	if (ret < 0)
	{
		return ret;
	}

	/* Set power mode to normal */

	regval = max662_getreg8(priv, MAX662_CONFIG_REG);
	CLEARBITS(regval, MAX662_PWR_MODE_MSK);
	ret = max662_putreg8(priv, MAX662_CONFIG_REG, regval);

	if (ret >= 0)
	{
		priv->activated = ACTIVE;
	}

	priv->config->irq_enable(priv->config, true);
	priv->irqenabled = true;

	nxmutex_unlock(&priv->devlock);

	sninfo("MAX662 opened successfully!\n");

	return ret;
}


/****************************************************************************
 * Name: max662_close
 *
 * Description:
 * 	Close the MAX662 driver, what means:
 * 		Changing NORMAL mode to SHUTDOWN;
 * 		Disabling irq on OT output
 *
 ****************************************************************************/

static int max662_close(FAR struct file *filep)
{
	FAR struct inode* inode = filep->f_inode;
	FAR struct max662_dev_s* priv = inode->i_private;

	int ret = OK;
	uint8_t regval;

	ret = nxmutex_lock(&priv->devlock);
	if (ret < 0)
	{
		return ret;
	}

	priv->config->irq_enable(priv->config, false);
	priv->irqenabled = false;

	/* Set to sleep mode */

	regval = max662_getreg8(priv, MAX662_CONFIG_REG);
	SETBITS(regval, MAX662_PWR_MODE_MSK);
	ret = max662_putreg8(priv, MAX662_CONFIG_REG, regval);

	if (ret >= 0)
	{
		priv->activated = INACTIVE;
	}

	sninfo("MAX662 closed successfully!\n");

	nxmutex_unlock(&priv->devlock);
	return ret;
}

/****************************************************************************
 * Name: max662_fetch
 *
 * Description:
 * 	Reading from MAX662 driver:
 * 		Get current temperature value from TEMP register
 *
 ****************************************************************************/

static ssize_t max662_read(	FAR struct file *filep,
							FAR char *buffer,
							size_t buflen)
{
	FAR struct inode* inode = filep->f_inode;
	FAR struct max662_dev_s* priv = inode->i_private;

	uint8_t buf[2];
	int32_t temp;

	int ret = OK;
	char regval;

	ret = nxmutex_lock(&priv->devlock);
	if (ret < 0)
	{
		return (ssize_t)ret;
	}


	if (buflen != sizeof(temp))
	{
		return -EINVAL;
	}

	if (!priv->activated)
	{
		/* Sensor is asleep, go to force mode to read once */

		regval = max662_getreg8(priv, MAX662_CONFIG_REG);
		CLEARBITS(regval, MAX662_PWR_MODE_MSK);
		ret = max662_putreg8(priv, MAX662_CONFIG_REG, regval);

		if (ret < 0)
		{
			return (ssize_t)ret;
		}

		priv->activated = ACTIVE;
	}

	/* Read temperature data */

	ret = max662_getregs(priv, MAX662_TEMP_REG, buf, 2);

	if (ret < 0)
	{
		return (ssize_t)ret;
	}

	temp = COMBINE(buf);

	sninfo("MAX662 readed successfully!\n");

	memcpy(buffer, &temp, sizeof(temp));

	nxmutex_unlock(&priv->devlock);
	return buflen;
}


/****************************************************************************
 * Name: max662_write
 *
 * Descriotion:
 * 	Write into MAX662 driver:
 * 		Dummy method
 ****************************************************************************/

static ssize_t max662_write(	FAR struct file *filep,
								FAR const char *buffer,
								size_t buflen)
{
  ssize_t length = 0;

  return length;
}


/****************************************************************************
 * Name: max662_ioctl
 *
 * Description:
 * 	MAX662 commands:
 * 		SNIOC_OT_POLARITY:
 * 			Change polarity of OT interrupt output
 * 			Availabe only in INTERRUPT mode
 * 			arg ---> enum MAX662_POL
 *
 * 		SNIOC_OT_MODE:
 * 			Change mode of OT output
 * 			arg ---> enum MAX662_MODE
 *
 * 		SNIOC_FAULT_DEPTH:
 * 			Change fault queue depth to toggle logic level on OT output
 * 			Available only in COMPARATOR mode
 * 			arg ---> enum MAX662_FAULT_DEPTH
 *
 ****************************************************************************/

static int max662_ioctl(	FAR struct file *filep,
							int cmd,
							unsigned long arg)
{
	FAR struct inode* inode = filep->f_inode;
	FAR struct max662_dev_s* priv = inode->i_private;

	int ret = OK;

	ret = nxmutex_lock(&priv->devlock);
	if (ret < 0)
	{
		return ret;
	}

	switch (cmd)
	{
		case (SNIOC_OT_POLARITY):
			ret = max662_set_polarity(priv, (MAX662_POL)arg);
			break;

		case (SNIOC_OT_MODE):
			ret = max662_set_mode(priv, (MAX662_MODE)arg);
			break;

		case (SNIOC_FAULT_DEPTH):
			ret = max662_set_fault_depth(priv, (MAX662_FAULT_DEPTH)arg);
			break;

		default:
			ret = -ENOTTY;
			break;
	}

	if (ret >= 0)
	{
		sninfo("Success!\n");
	}


	nxmutex_unlock(&priv->devlock);
	return ret;
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: max662_register
 *
 * Description:
 *   Register the MAX662 character device
 *
 * Input Parameters:
 *   devno   - Instance number for driver
 *   i2c     - An instance of the I2C interface to use to communicate with
 *             max662
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int max662_register(	FAR const char *devpath,
						struct i2c_master_s* i2c,
						FAR max662_config_t *config)
{
	FAR struct max662_dev_s *priv;
	int ret = OK;

	/* Initialize the max662 device structure */

	priv = kmm_zalloc(sizeof(struct max662_dev_s));
	if (!priv)
	{
		snerr("Failed to allocate instance\n");
		return -ENOMEM;
	}

	nxmutex_init(&priv->devlock);
	nxsem_init(&priv->waitsem, 0, 0);

	priv->i2c = i2c;
	priv->addr = MAX662_ADDR;
	priv->freq = MAX662_FREQ;
	priv->config = config;

	ret = max662_initialize(priv);

	if (ret < 0)
	{
		snerr("Failed to initialize physical device max662:%d\n", ret);

		nxmutex_destroy(&priv->devlock);
		nxsem_destroy(&priv->waitsem);

		kmm_free(priv);
		return ret;
	}

	/* Register the character driver */

	ret = register_driver(devpath, &g_max662_fops, 0666, priv);

	if (ret < 0)
	{
		snerr("Failed to register driver: %d\n", ret);

		nxmutex_destroy(&priv->devlock);
		nxsem_destroy(&priv->waitsem);

		kmm_free(priv);
		return ret;
	}

	priv->config->irq_attach(config, max662_int_handler, priv);
	priv->config->irq_enable(config, false);
	priv->irqenabled = false;

	sninfo("MAX662 driver loaded successfully!\n");
	return ret;
}


#endif /* CONFIG_I2C && CONFIG_SENSORS_MAX662 */
