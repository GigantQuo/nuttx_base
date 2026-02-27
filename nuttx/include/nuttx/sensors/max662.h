/****************************************************************************
 * include/nuttx/sensors/max662.h
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

#ifndef __INCLUDE_NUTTX_SENSORS_MAX662_H
#define __INCLUDE_NUTTX_SENSORS_MAX662_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/sensors/ioctl.h>

// #if defined(CONFIG_I2C) && defined(CONFIG_SENSORS_MAX662)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef struct max662_config_s
{
	/* Device characterization */

	int irq;				/* IRQ number received by interrupt handler. */

	/* IRQ/GPIO access callbacks.  These operations all hidden behind callbacks
	* to isolate the driver from differences in GPIO interrupt handling
	* by varying boards and MCUs.
	* irq_attach - Attach the interrupt handler to the GPIO interrupt
	* irq_enable - Enable or disable the GPIO
	* irq_clear - Acknowledge/clear any pending GPIO interrupt
	* get_state - Get the current logic level on interrupt pin
	*/

	CODE int (*irq_attach)(		FAR struct max662_config_s *state,
								xcpt_t isr,
						 		FAR void *arg);

	CODE void (*irq_enable)(	FAR const struct max662_config_s *state,
								bool enable);

	CODE void (*irq_clear)(		FAR const struct max662_config_s *state);

	CODE bool (*get_state)(		FAR const struct max662_config_s *state);

} max662_config_t;

typedef enum
{
	COMPARATOR = 0,
	INTERRUPT
} MAX662_MODE;

typedef enum
{
	GOOD = 0,
	ALARM
} MAX662_OVERTEMP;

typedef enum
{
	INACTIVE = 0,
	ACTIVE
} MAX662_STATE;

typedef enum
{
	LOW = 0,
	HIGH
} MAX662_POL;

typedef enum
{
	DEPTH_1 = 0,
	DEPTH_2,
	DEPTH_4,
	DEPTH_6
} MAX662_FAULT_DEPTH;


/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* IOCTL Commands ***********************************************************/

/* Command:      SNIOC_OT_POLARITY
 * Description:  Set interupt output polarity of OT output
 */

#define SNIOC_OT_POLARITY             _SNIOC(0x0001)

/* Command:      SNIOC_OT_MODE
 * Description:  Set OT output mode: interrupt or comparator
 */

#define SNIOC_OT_MODE                 _SNIOC(0x0002)

/* Command:      SNIOC_FAULT_DEPTH
 * Description:  Set fault queue depth for comparator mode
 */

#define SNIOC_FAULT_DEPTH             _SNIOC(0x0003)

/****************************************************************************
 * Name: max662_register
 *
 * Description:
 *   Register the MAX662 character device
 *
 * Input Parameters:
 *   devno   - Instance number for driver
 *   i2c     - An instance of the I2C interface to use to communicate with
 *             BMP280
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int max662_register(	FAR const char* devpath,
						struct i2c_master_s* i2c,
						FAR max662_config_t* config);


// #endif /* CONFIG_I2C && CONFIG_SENSORS_MAX662 */
#endif /* __INCLUDE_NUTTX_MAX662_H */
