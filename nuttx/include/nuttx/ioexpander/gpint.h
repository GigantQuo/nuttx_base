/****************************************************************************
 * include/nuttx/ioexpander/gpint.h
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

#ifndef __INCLUDE_NUTTX_IOEXPANDER_GPINT_H
#define __INCLUDE_NUTTX_IOEXPANDER_GPINT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/types.h>

#include <stdint.h>
#include <stdbool.h>

#include <nuttx/signal.h>
#include <nuttx/fs/ioctl.h>

#ifdef CONFIG_DEV_GPINT

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Command:     GPINT_BIT_READ
 * Description: Read the value to a dedicated input GPINT.
 * Argument:    A pointer to a struct bitval_s to receive the value
 *              and point on desired bit position.
 *
 * Command:     GPINT_REGISTER
 * Description: Register to receive a signal whenever there an
 *              interrupt is received on an input interrupt pin.
 * Argument:    The sigevent structure.
 *
 * Command:     GPINT_UNREGISTER
 * Description: Stop receiving signals for pin interrupts.
 * Argument:    NULL
 *
 * Command:     GPINT_BIT_ENABLE
 * Description: Enable the interrupt generating on dedicated pin.
 * Argument:    The index of desired interrupt pin.
 *
 * Command:     GPINT_BIT_DISABLE
 * Description: Disable the interrupt generating on dedicated pin.
 * Argument:    The index of desired interrupt pin.
 */

#define GPINT_BIT_READ          _GPIOC(10)
#define GPINT_REGISTER          _GPIOC(11)
#define GPINT_UNREGISTER        _GPIOC(12)
#define GPINT_BIT_ENABLE        _GPIOC(13)
#define GPINT_BIT_DISABLE       _GPIOC(14)


/* Modes of ioctl GPINT_BIT_ENABLE, GPINT_BIT_DISABLE commands */

/* Mode:        DIRECT
 * Description: mask value contains the discrete pin position in the
 *              interrupt pins pull. Used when need to enable/disable
 *              interrupt on the one dedicated pin.
 *
 * Mode:        MASKED
 * Description: mask value conains the mask to the interrupt pins pull.
 *              Used when need to enable/disable interrupt on a group of
 *              pins.
 */

#define GPINT_MODE_DIRECT       0
#define GPINT_MODE_MASKED       1


/****************************************************************************
 * Public Types
 ****************************************************************************/

struct gpint_dev_s;
typedef CODE int (*gpint_callback_t)(struct gpint_dev_s *dev);

/* Pin interface vtable definition.
 *
 *   - go_read.     Perform the one-bit reading of pin state.
 *   - go_attach.   Perform the attaching (or detaching, if callback = NULL)
 *                    the callback to correcponding pin interrupt.
 *   - go_enable.   Perform the enabling/disabling of desired pin interrupt.
 */

struct gpint_operations_s
{
  /* Interface methods */
  CODE int (*go_read)(FAR struct gpint_dev_s *dev,
                      uint32_t index,
                      FAR bool *value);
  CODE int (*go_attach)(FAR struct gpint_dev_s *dev,
                        gpint_callback_t callback);
  CODE int (*go_enable)(FAR struct gpint_dev_s *dev,
                        uint32_t index,
                        bool enable);
};

/* Signal information */
struct gpint_signal_s
{
  struct sigevent gp_event;   /* Signal event structure */

  /* If the workqueue defined if system
   * and would work only in FLAT build
   */
#ifdef CONFIG_SIG_EVTHREAD
  struct sigwork_s gp_work;   /* Work queue structure */
#endif
  pid_t gp_pid;               /* The task to be signaled */
};

/* Pin interface definition.  Must lie in writable memory. */
struct gpint_dev_s
{
  /* Number of times the device has been registered */
  uint8_t register_count;

  /* Field to check was a callback registered */
  bool attached;

  /* Number of registered interrupt inputs */
  uint32_t int_number;

  /* The interrupt counter */
  uint32_t int_count;

  /* The current interrupt number */
  uint32_t int_current;

  /* Driver methods structure */
  FAR const struct gpint_operations_s *ops;

#if CONFIG_DEV_GPINT_NSIGNALS > 0
  struct gpint_signal_s gp_signals[CONFIG_DEV_GPINT_NSIGNALS];
#endif

#if CONFIG_DEV_GPINT_NPOLLWAITERS > 0
  FAR struct pollfd *fds[CONFIG_DEV_GPINT_NPOLLWAITERS];
#endif

  /* Device specific, lower-half information may follow. */
};


/* Struct, needed by ioctl
 * GPINT_BIT_READ command,
 * contains the desired bit position to read
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

/* Struct, needed by ioctl
 * GPINT_BIT_ENABLE command,
 * GPINT_BIT_DISABLE command,
 * contains the mode field and mask field
 */
struct gpint_enable_s
{
  uint8_t mode;      /* Can be in the DIRECT or MASKED mode */

  /* Contains the pin position in DIRECT mode
   * and contains pinmask in MASK mode
   */
  union
  {
    uint32_t pos;
    char *mask;
  } val;
};


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
 * Name: gpint_register
 *
 * Description:
 *   Register gpint device driver.
 *
 * Arguments:
 *   dev    - A pointer to a gpint_dev_s
 *   minor  - A number of the registering driver
 *
 * Returned values:
 *  OK - if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

int gpint_register(FAR struct gpint_dev_s *dev,
                    int minor);

/****************************************************************************
 * Name: gpint_unregister
 *
 * Description:
 *   Unregister gpinp device driver at /dev/gpint(minor).
 *
 * Arguments:
 *   dev      - A pointer to a gpint_dev_s
 *   minor  - A number of the unregistering driver
 *
 * Returned values:
 *  OK - if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

int gpint_unregister(FAR struct gpint_dev_s *dev,
                      int minor);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONFIG_DEV_GPINT */
#endif /* __INCLUDE_NUTTX_IOEXPANDER_GPINT_H */
