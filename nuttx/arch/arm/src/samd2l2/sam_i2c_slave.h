/****************************************************************************
 * arch/arm/src/samd2l2/sam_i2c_slave.h
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

#ifndef __ARCH_ARM_SRC_SAMD2L2_SAM_I2C_SLAVE_H
#define __ARCH_ARM_SRC_SAMD2L2_SAM_I2C_SLAVE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "sam_config.h"

#if defined(CONFIG_ARCH_FAMILY_SAMD20) || defined(CONFIG_ARCH_FAMILY_SAMD21)
#  include "hardware/samd_i2c_slave.h"
#elif defined(CONFIG_ARCH_FAMILY_SAML21)
#  include "hardware/saml_i2c_slave.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define EBUSERR         141
#define EBUSERR_STR     "Bus error"
#define ECOLL           142
#define ECOLL_STR       "Bus collision"
#define ELOWTOUT        143
#define ELOWTOUT_STR    "Time out for low SCL"
#define ESEXTTOUT       144
#define ESEXTTOUT_STR   "Time out for low extended SCL"


/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: sam_i2cbus_slave_initialize
 *
 * Description:
 *    Initialize on I2C bus as a slave and get pointer to i2c_slave_s struct
 *
 * Arguments:
 *    bus - the i2c slave bus number
 *
 * Returned values:
 *   i2c_slave_s*  - if it is OK, the structrue pointer
 *   NULL          - if sometheing went wrong
 *
 ****************************************************************************/

struct i2c_slave_s* sam_i2cbus_slave_initialize(int bus);

/****************************************************************************
 * Name: sam_i2cbus_slave_uninitialize
 *
 * Description:
 *    Unintialize the given i2c slave device.
 *
 * Arguments:
 *    dev          - a pointer to i2c_slave_s stucture
 *
 * Returned values:
 *   OK         - if it is OK, always
 *
 ****************************************************************************/

int sam_i2cbus_slave_uninitialize(struct i2c_slave_s *dev);


#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_SAMD2L2_SAM_I2C_SLAVE_H */
