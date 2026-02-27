/****************************************************************************
 * arch/arm/src/samd2l2/sam_gpout.c
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
#include <nuttx/ioexpander/gpout.h>
#include <nuttx/mm/mm.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "sam_pinmap.h"
#include "sam_port.h"
#include "chip.h"

#if defined(CONFIG_SAMD2L2_GPOUT)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_gpout_dev_s
{
  struct gpout_dev_s dev;                /* Upper half driver structure */
  mutex_t lock;                          /* Mutex to atomic access */
  const volatile uint32_t *gpout_pull;   /* Pull of outputs */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline int sam_gpout_hw_init(struct sam_gpout_dev_s *priv);
static inline void sam_gpout_hw_uninit(struct sam_gpout_dev_s *priv);

static int sam_gpout_read(FAR struct gpout_dev_s *dev,
                          uint32_t index,
                          FAR bool *value);
static int sam_gpout_write(FAR struct gpout_dev_s *dev,
                           uint32_t index,
                           FAR bool *value);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct gpout_operations_s g_gpout_ops =
{
  .go_read = sam_gpout_read,
  .go_write = sam_gpout_write
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpout_hw_init
 *
 * Description:
 *    Perform harware initializaion for use dedicated pins as outputs
 *
 * Arguments:
 *    priv - main gpout structure
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static inline int sam_gpout_hw_init(struct sam_gpout_dev_s *priv)
{
  int ret = OK;
  int i;

  /* Perform sequential initialization of the every pin in the pull */
  for (i = 0; i < priv->dev.out_number; i++)
  {
    ret = sam_configport((port_pinset_t)priv->gpout_pull[i]);
    if (ret < 0)
    {
      return ret;
    }
  }

  return ret;
}

/****************************************************************************
 * Name: sam_gpout_hw_uninit
 *
 * Description:
 *    Perform harware uninitializaion for use dedicated pins as outputs.
 *
 * Arguments:
 *    priv - main gpout structure
 *
 ****************************************************************************/

static inline void sam_gpout_hw_uninit(struct sam_gpout_dev_s *priv)
{
  int i;

  /* Perform sequential uninitialization of the every pin in the pull */
  for (i = 0; i < priv->dev.out_number; i++)
  {
    sam_configreset((port_pinset_t)priv->gpout_pull[i]);
  }
}

/****************************************************************************
 * Name: sam_gpout_read
 *
 * Description:
 *    Perform reading of indexed pin value from hardware.
 *
 * Arguments:
 *    dev   - An upperhalf driver structure
 *    index - An index of gpoutput in the pull
 *    value - A pointer to boolean data buffer
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static int sam_gpout_read(FAR struct gpout_dev_s *dev,
                          uint32_t index,
                          FAR bool *value)
{
  struct sam_gpout_dev_s *priv;
  irqstate_t flags;
  int ret = OK;

  DEBUGASSERT(dev != NULL);

  /* Get the private structure from the upper half driver */
  priv = (struct sam_gpout_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return ret;
  }
  flags = enter_critical_section();

  *value = sam_portread((port_pinset_t)priv->gpout_pull[index]) & 0x1;

  leave_critical_section(flags);
  nxmutex_unlock(&priv->lock);
  return ret;
}

/****************************************************************************
 * Name: sam_gpout_write
 *
 * Description:
 *    Perform writing of indexed pin value to hardware.
 *
 * Arguments:
 *    dev   - An upperhalf driver structure
 *    index - An index of gpoutput in the pull
 *    value - A pointer to boolean data buffer
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static int sam_gpout_write(FAR struct gpout_dev_s *dev,
                           uint32_t index,
                           FAR bool *value)
{
  struct sam_gpout_dev_s *priv;
  irqstate_t flags;
  int ret = OK;

  DEBUGASSERT(dev != NULL);

  /* Get the private structure from the upper half driver */
  priv = (struct sam_gpout_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return ret;
  }
  flags = enter_critical_section();

  sam_portwrite((port_pinset_t)priv->gpout_pull[index],
                (*value & 0x1));

  leave_critical_section(flags);
  nxmutex_unlock(&priv->lock);
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpout_initialize
 *
 * Description:
 *    Perform sam_gpout_dev_s structure and hardware initializaion.
 *
 * Arguments:
 *    pull   - A pointer to an array contains the values of the gpoutputs
 *             initialization data
 *    ngpout - The number of outputs in this driver to be initialized
 *
 * Returned values:
 *    struct gpout_dev_s - if it is OK
 *    NULL - if something went wrong
 *
 ****************************************************************************/

struct gpout_dev_s *sam_gpout_initialize(const volatile uint32_t *pull,
                                         const uint32_t ngpout)
{
  struct sam_gpout_dev_s *priv;
  int ret = OK;

  /* Attach the pull of gpoutputs */
  DEBUGASSERT(pull != NULL);

  if (ngpout == 0)
  {
    return NULL;
  }

  priv = kmm_zalloc(sizeof(struct sam_gpout_dev_s));

  priv->gpout_pull = pull;

  priv->dev.out_number = ngpout;
  priv->dev.ops = &g_gpout_ops;

  nxmutex_init(&priv->lock);

  /* Perform harware initialization */
  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    kmm_free(priv);
    return NULL;
  }

  ret = sam_gpout_hw_init(priv);
  if (ret < 0)
  {
    gpioinfo("GPOUT hardware initialization error\n\r");
    sam_gpout_hw_uninit(priv);

    nxmutex_unlock(&priv->lock);
    kmm_free(priv);
    return NULL;
  }

  gpioinfo("GPOUT hardware initialization success!\n\r");

  nxmutex_unlock(&priv->lock);
  return &priv->dev;
}

/****************************************************************************
 * Name: sam_gpout_uninitialize
 *
 * Description:
 *    Perform sam_gpout_dev_s structure and hardware uninitialization.
 *
 * Arguments:
 *    dev - An upperhalf driver structure
 *
 ****************************************************************************/

void sam_gpout_uninitialize(FAR struct gpout_dev_s *dev)
{
  struct sam_gpout_dev_s *priv;
  int ret = OK;

  DEBUGASSERT(dev != NULL);

  /* Get the private structure from the upper half driver */
  priv = (struct sam_gpout_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return;
  }

  sam_gpout_hw_uninit(priv);

  /* Clear the pull pointer to prevent use-after-free */
  priv->gpout_pull = NULL;

  gpioinfo("GPOUT hardware uninitialization success!\n\r");

  nxmutex_unlock(&priv->lock);
  kmm_free(priv);
}

#endif
