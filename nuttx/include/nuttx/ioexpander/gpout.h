/****************************************************************************
 * include/nuttx/ioexpander/gpout.h
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

#ifndef __INCLUDE_NUTTX_IOEXPANDER_GPOUT_H
#define __INCLUDE_NUTTX_IOEXPANDER_GPOUT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include <nuttx/fs/ioctl.h>

#ifdef CONFIG_DEV_GPOUT

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Command:     GPOUT_BIT_READ
 * Description: Read the value of a dedicated output GPOUT
 * Argument:    A pointer to a struct bitval_s to receive the result
 *              and point on desired bit position.
 *
 * Command:     GPOUT_BIT_WRITE
 * Description: Write the value to a dedicated output GPOUT
 * Argument:    A pointer to a struct bitval_s to write the value
 *              and point on desired bit position.
 */

#define GPOUT_BIT_READ       _GPIOC(8)
#define GPOUT_BIT_WRITE      _GPIOC(9)


/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Pin interface vtable definition.
 *
 *   - go_read.   Perform the one-bit reading of pin state.
 *   - go_write.  Perform the one-bit writing the pin state.
 */

struct gpout_dev_s;
struct gpout_operations_s
{
  /* Interface methods */
  CODE int (*go_read)(FAR struct gpout_dev_s *dev,
                      uint32_t index,
                      FAR bool *value);
  CODE int (*go_write)(FAR struct gpout_dev_s *dev,
                       uint32_t index,
                       FAR bool *value);
};

/* Pin interface definition.  Must lie in writable memory. */

struct gpout_dev_s
{
  /* Number of times the device has been registered */
  uint8_t register_count;

  /* Number of registered outputs */
  uint32_t out_number;

  /* Driver methods structure */
  FAR const struct gpout_operations_s *ops;
};

/* Struct, needed to ioctl
 * GPOUT_BIT_READ command,
 * GPOUT_BIT_WRITE command,
 * contains desired bit position to read
 * and the pointer to bool value to save value
 */

#ifndef GPIO_BITVAL_S
struct bitval_s
{
  uint8_t bit;     /* bit position */
  bool* val;       /* bool buffer */
};
  #define GPIO_BITVAL_S
#else
extern struct bitval_s;
#endif /* GPIO_BITVAL_S */

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
  #define EXTERN extern "C"
extern "C"
{
#else
  #define EXTERN extern
#endif /* __cplusplus */

/****************************************************************************
 * Name: gpout_register
 *
 * Description:
 *   Register GPOUT pin device driver at /dev/gpoutN, where N is the
 *   provided minor number.
 *
 * Input Parameters:
 *   dev    - A pointer to a gpout_dev_s
 *   minor  - An integer value to be concatenated with '/dev/gpout'
 *            to form the device name.
 *
 * Returned values:
 *  OK - if it is OK,
 *  Error code - if something went wrong
 *
 ****************************************************************************/

int gpout_register(FAR struct gpout_dev_s *dev, int minor);

/****************************************************************************
 * Name: gpout_unregister
 *
 * Description:
 *   Unregister GPOUT pin device driver at /dev/gpoutN, where N is the
 *   provided minor number.
 *
 * Input Parameters:
 *   dev    - A pointer to a gpinp_dev_s
 *   minor  - An integer value to be concatenated with '/dev/gpout'
 *            to form the device name.
 *
 * Returned values:
 *  OK - if it is OK,
 *  Error code - if something went wrong
 *
 ****************************************************************************/

int gpout_unregister(FAR struct gpout_dev_s *dev, int minor);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONFIG_DEV_GPOUT */
#endif /* __INCLUDE_NUTTX_IOEXPANDER_GPOUT_H */
