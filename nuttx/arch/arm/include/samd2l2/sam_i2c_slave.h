/****************************************************************************
 * arch/arm/include/samd2l2/sam_i2c_slave.h
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

#ifndef __ARCH_ARM_SRC_SAMD2L2_SAM_I2C_SLAVE_GLOBAL_H
#define __ARCH_ARM_SRC_SAMD2L2_SAM_I2C_SLAVE_GLOBAL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Name: sam_intime_register
 *
 * Description:
 *    Register the user provided intime handler.
 *
 * Arguments:
 *    bus          - the number of a i2c slave interface
 *    user_handler - a pointer to the handler
 *
 * Returned values:
 *    OK         - if it is OK
 *    Error code - if sometheing wents wrong
 *
 ****************************************************************************/

#if defined(CONFIG_SAMD2L2_I2C_SLAVE_INTIME)
int sam_intime_register(uint8_t bus, void *user_handler);
#endif /* CONFIG_SAMD2L2_I2C_SLAVE_INTIME */


#endif /* __ARCH_ARM_SRC_SAMD2L2_SAM_I2C_SLAVE_GLOBAL_H */
