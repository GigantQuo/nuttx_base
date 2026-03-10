/****************************************************************************
 * arch/arm/src/samd2l2/sam_gpint.c
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

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/mutex.h>
#include <nuttx/ioexpander/gpint.h>
#include <nuttx/mm/mm.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "sam_pinmap.h"
#include "sam_port.h"
#include "sam_eic.h"
#include "chip.h"

#if defined(CONFIG_SAMD2L2_GPINT)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_gpint_dev_s
{
  struct gpint_dev_s dev;              /* Upper half driver structure */
  mutex_t lock;                        /* Mutex to atomic access */
  gpint_callback_t callback;           /* Attached callback function */
  const volatile uint32_t *gpint_pull; /* Pull of interrupt inputs */
  const volatile uint32_t *irq_pull;   /* Pull of interrupt numbers*/
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int sam_gpint_read(FAR struct gpint_dev_s *dev,
                          uint32_t index,
                          FAR bool *value);
static int sam_gpint_attach(FAR struct gpint_dev_s *dev,
                            gpint_callback_t callback);
static int sam_gpint_enable(FAR struct gpint_dev_s *dev,
                            uint32_t index,
                            bool enable);

static int sam_gpint_handler(int irq,
                            void *context,
                            void *arg);

static inline int sam_gpint_hw_init(struct sam_gpint_dev_s *priv);
static inline void sam_gpint_hw_uninit(struct sam_gpint_dev_s *priv);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct gpint_operations_s g_gpint_ops =
{
  .go_read    = sam_gpint_read,
  .go_attach  = sam_gpint_attach,
  .go_enable  = sam_gpint_enable
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpint_hw_init
 *
 * Description:
 *    Perform harware initializaion for the pins in the pull as interrupt
 *    inputs.
 *
 * Arguments:
 *    priv - main gpint structure
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static inline int sam_gpint_hw_init(struct sam_gpint_dev_s *priv)
{
  int ret = OK;
  int i;

  /* Perform sequential initialization of the every pin in the pull */
  for (i = 0; i < priv->dev.int_number; i++)
  {
    ret = sam_configport((port_pinset_t)priv->gpint_pull[i]);
    if (ret < 0)
    {
      return ret;
    }
  }

  return ret;
}

/****************************************************************************
 * Name: sam_gpint_hw_uninit
 *
 * Description:
 *    Perform harware initializaion for the pins in the pull.
 *
 * Arguments:
 *    priv - main gpinp structure
 *
 ****************************************************************************/

static inline void sam_gpint_hw_uninit(struct sam_gpint_dev_s *priv)
{
  int i;

  /* Perform sequential uninitialization of the every pin in the pull */
  for (i = 0; i < priv->dev.int_number; i++)
  {
    sam_configreset((port_pinset_t)priv->gpint_pull[i]);
  }
}

/****************************************************************************
 * Name: sam_gpint_read
 *
 * Description:
 *    Perform reading of the indexed pin value from a hardware.
 *
 * Arguments:
 *    dev   - upperhalf driver structure
 *    index - index of gpinput in the pull
 *    value - boolean data buffer
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static int sam_gpint_read(FAR struct gpint_dev_s *dev,
                          uint32_t index,
                          FAR bool *value)
{
  struct sam_gpint_dev_s *priv;
  irqstate_t flags;
  int ret = OK;

  /* Get the private structure from the upper half driver */
  DEBUGASSERT(dev != NULL);
  priv = (struct sam_gpint_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return ret;
  }
  flags = enter_critical_section();

  *value = sam_portread((port_pinset_t)priv->gpint_pull[index]) & 0x1;

  leave_critical_section(flags);
  nxmutex_unlock(&priv->lock);
  return ret;
}

/****************************************************************************
 * Name: sam_gpint_attach
 *
 * Description:
 *    Attaching callback interrupt service routing function.
 *
 * Arguments:
 *    dev       - A pointer to upperhalf driver structure
 *    callback  - A pointer to a callback function provided by
 *                  upperhalf driver
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static int sam_gpint_attach(FAR struct gpint_dev_s *dev,
                            gpint_callback_t callback)
{
  struct sam_gpint_dev_s *priv;
  irqstate_t flags;
  int ret = OK;
  int i;

  /* Get the private structure from the upper half driver */
  DEBUGASSERT(dev != NULL);
  priv = (struct sam_gpint_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return ret;
  }
  flags = enter_critical_section();

  /* If the provided callback is a pointer on an existing
   *  upper half callback function then attach it to
   *  all an interrupt numbers in the irq_pull array.
   * Else if the provided callback is NULL then perform the
   *  detaching of the current callback function.
   */

  priv->callback = callback;
  if (callback != NULL)
  {
    /* Provided callback function is valid */

    gpioinfo("Attaching the callback...\n\r");

    /* Make sure the interrupt is detached */
    for (i = 0; i < priv->dev.int_number; i++)
    {
      irq_detach(priv->irq_pull[i]);
    }

    /* Attach archeture specific callback sam_gpint_handler */
    for (i = 0; i < priv->dev.int_number; i++)
    {
      ret = irq_attach(priv->irq_pull[i],
                      sam_gpint_handler,
                      (void *)priv);
      if (ret < 0)
      {
        gpioerr("Error while attaching GPINT callback!\n\r");
        leave_critical_section(flags);
        nxmutex_unlock(&priv->lock);
        return ret;
      }
    }

    gpioinfo("GPINT callback is successfuly attached!\n\r");
  }
  else
  {
    /* Provided callback function is NULL.
     * Perform the detaching of the current
     *  callback function.
     */
    gpioinfo("Detaching the callback\n\r");

    for (i = 0; i < priv->dev.int_number; i++)
    {
      irq_detach(priv->irq_pull[i]);
    }
  }

  leave_critical_section(flags);
  nxmutex_unlock(&priv->lock);
  return OK;
}

/****************************************************************************
 * Name: sam_gpint_enable
 *
 * Description:
 *    Enable corresponding interrupt
 *
 * Arguments:
 *    dev    - A pointer to upperhalf driver structure
 *    index  - An index of irq in the irq_pull
 *    enable - A enable/disable value
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static int sam_gpint_enable(FAR struct gpint_dev_s *dev,
                            uint32_t index,
                            bool enable)
{
  struct sam_gpint_dev_s *priv;
  irqstate_t flags;
  int ret = OK;

  /* Get the private structure from the upper half driver */
  DEBUGASSERT(dev != NULL);
  priv = (struct sam_gpint_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return ret;
  }
  flags = enter_critical_section();

  if (enable == true)
  {
    if (priv->callback != NULL)
    {
      gpioinfo("Enabling the GPINT interrupt\n\r");
      sam_eic_irq_enable(priv->irq_pull[index]);
    }
    else
    {
      gpioerr("Cannot enable - no callback registered\n\r");
      leave_critical_section(flags);
      nxmutex_unlock(&priv->lock);
      return -EINVAL;
    }
  }
  else
  {
    gpioinfo("Disable the interrupt\n\r");
    sam_eic_irq_disable(priv->irq_pull[index]);
  }

  leave_critical_section(flags);
  nxmutex_unlock(&priv->lock);
  return OK;
}

/****************************************************************************
 * Name: sam_gpint_handler
 *
 * Description:
 *    Hardware specific interrupt service routing function.
 *
 * Arguments:
 *    irq     - An interrupt number
 *    context - A pointer to a context
 *    arg     - An argument to irq (struct sam_gpint_dev_s *)
 *
 * Returned values:
 *    OK - Always
 *
 ****************************************************************************/

static int sam_gpint_handler(int irq,
                             void *context,
                             void *arg)
{
  struct sam_gpint_dev_s *priv;
  uint32_t int_current;
  int ret = OK;

  /* Get the private structure from the arg */
  DEBUGASSERT(arg != NULL);
  priv = (struct sam_gpint_dev_s *)arg;

  /* Get the interrupt number from context */
  DEBUGASSERT(context != NULL);

  int_current = (uint32_t)context;

  if ((int_current <= SAM_IRQ_EXTINT15) ||
       int_current >= SAM_IRQ_EXTINT0)
  {
    gpioinfo("GPINT handler present!\n\r");

    priv->dev.int_current = int_current;
    priv->callback(&priv->dev);
  }
  else
  {
    gpioinfo("Wrong EXTINT interrupt number!\n\r");
    ret = -ERROR;
  }

  priv->dev.int_current = 0;

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpint_initialize
 *
 * Description:
 *    Perform sam_gpint_dev_s structure and hardware initializaion.
 *
 * Arguments:
 *    pull      - The array contains the values of the gpinputs
 *                initialization data
 *    ngpinp    - The number of interrupt inputs in this driver to be
 *                initialized
 *    irq_pull  - The array contains the number of interrupt in the vector
 *                table
 *
 * Returned values:
 *    struct gpint_dev_s - if it is OK
 *    NULL - if something went wrong
 *
 ****************************************************************************/

struct gpint_dev_s *sam_gpint_initialize(const uint32_t *pull,
                                         const uint32_t ngpint,
                                         const uint32_t *irq_pull)
{
  struct sam_gpint_dev_s *priv;
  int ret = OK;

  /* Attach the pull of gpinputs */
  DEBUGASSERT(pull != NULL);
  if (ngpint == 0)
  {
    return NULL;
  }

  priv = kmm_zalloc(sizeof(struct sam_gpint_dev_s));

  priv->gpint_pull = pull;

  priv->dev.int_number = ngpint;
  priv->dev.ops = &g_gpint_ops;

  nxmutex_init(&priv->lock);

  /* Attach the pull of irqs */
  DEBUGASSERT(irq_pull != NULL);
  priv->irq_pull = irq_pull;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    kmm_free(priv);
    return NULL;
  }

  /* Perform harware initialization */
  ret = sam_gpint_hw_init(priv);
  if (ret < 0)
  {
    gpioinfo("GPINP hardware initialization error\n\r");
    sam_gpint_hw_uninit(priv);

    nxmutex_unlock(&priv->lock);
    kmm_free(priv);
    return NULL;
  }

  gpioinfo("GPINP hardware initialization success!\n\r");

  nxmutex_unlock(&priv->lock);
  return &priv->dev;
}

/****************************************************************************
 * Name: sam_gpint_uninitialize
 *
 * Description:
 *    Perform sam_gpint structure and harware uninitialization.
 *
 * Arguments:
 *    dev - An upperhalf driver structure
 *
 ****************************************************************************/

void sam_gpint_uninitialize(FAR struct gpint_dev_s *dev)
{
  struct sam_gpint_dev_s *priv;
  int ret = OK;

  DEBUGASSERT(dev != NULL);

  /* Get the private structure from the upper half driver */
  priv = (struct sam_gpint_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return;
  }

  /* Perform harware uninitialization */
  sam_gpint_hw_uninit(priv);

  /* Clear the pull pointer to prevent use-after-free */
  priv->gpint_pull  = NULL;
  priv->irq_pull    = NULL;

  gpioinfo("GPINP hardware uninitialization success!\n\r");

  nxmutex_unlock(&priv->lock);

  kmm_free(priv);
}


#endif /* CONFIG_SAMD2L2_GPINT */
