/****************************************************************************
* arch/arm/src/samd2l2/sam_gpinp.c
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
#include <nuttx/ioexpander/gpinp.h>
#include <nuttx/mm/mm.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "sam_pinmap.h"
#include "sam_port.h"
#include "chip.h"

#if defined(CONFIG_SAMD2L2_GPINP)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct sam_gpinp_dev_s
{
  struct gpinp_dev_s dev;                /* Upper half driver structure */
  mutex_t lock;                          /* Mutex to atomic access */
  const volatile uint32_t *gpinp_pull;   /* Pull of inputs */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int sam_gpinp_read(FAR struct gpinp_dev_s *dev,
                          uint32_t index,
                          FAR bool *value);

static inline int sam_gpinp_hw_init(struct sam_gpinp_dev_s *priv);
static inline void sam_gpinp_hw_uninit(struct sam_gpinp_dev_s *priv);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct gpinp_operations_s g_gpinp_ops =
{
  .go_read = sam_gpinp_read
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpinp_hw_init
 *
 * Description:
 *    Perform harware initializaion for the pins in the pull as inputs.
 *
 * Arguments:
 *    priv - main gpinp structure
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static inline int sam_gpinp_hw_init(struct sam_gpinp_dev_s *priv)
{
  int ret = OK;
  int i;

  /* Perform sequential initialization of the every pin in the pull */
  for (i = 0; i < priv->dev.inp_number; i++)
  {
    ret = sam_configport((port_pinset_t)priv->gpinp_pull[i]);
    if (ret < 0)
    {
      return ret;
    }
  }

  return ret;
}

/****************************************************************************
 * Name: sam_gpinp_hw_uninit
 *
 * Description:
 *    Perform harware initializaion for the pins in the pull.
 *
 * Arguments:
 *    priv - main gpinp structure
 *
 ****************************************************************************/

static inline void sam_gpinp_hw_uninit(struct sam_gpinp_dev_s *priv)
{
  int i;

  /* Perform sequential uninitialization of the every pin in the pull */
  for (i = 0; i < priv->dev.inp_number; i++)
  {
    sam_configreset((port_pinset_t)priv->gpinp_pull[i]);
  }
}

/****************************************************************************
 * Name: sam_gpinp_read
 *
 * Description:
 *    Perform reading of indexed pin value from hardware.
 *
 * Arguments:
 *    dev - upperhalf driver structure
 *    index - index of gpinput in the pull
 *    value - boolean data buffer
 *
 * Returned values:
 *    OK - if it is OK,
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static int sam_gpinp_read(FAR struct gpinp_dev_s *dev,
                          uint32_t index,
                          FAR bool *value)
{
  struct sam_gpinp_dev_s *priv;
  irqstate_t flags;
  int ret = OK;

  DEBUGASSERT(dev != NULL);

  /* Get the private structure from the upper half driver */
  priv = (struct sam_gpinp_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return ret;
  }
  flags = enter_critical_section();

  *value = sam_portread((port_pinset_t)priv->gpinp_pull[index]) & 0x1;

  leave_critical_section(flags);
  nxmutex_unlock(&priv->lock);
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpinp_initialize
 *
 * Description:
 *    Perform sam_gpinp_dev_s structure and hardware initializaion.
 *
 * Arguments:
 *    pull   - The array contains the values of the gpinputs
 *             initialization data
 *    ngpinp - The number of inputs in this driver to be initialized
 *
 * Returned values:
 *    struct gpinp_dev_s - if it is OK
 *    NULL - if something went wrong
 *
 ****************************************************************************/

struct gpinp_dev_s *sam_gpinp_initialize(const volatile uint32_t *pull,
                                         const uint32_t ngpinp)
{
  struct sam_gpinp_dev_s *priv;
  int ret = OK;

  /* Attach the pull of gpinputs */
  DEBUGASSERT(pull != NULL);
  if (ngpinp == 0)
  {
    return NULL;
  }

  priv = kmm_zalloc(sizeof(struct sam_gpinp_dev_s));

  priv->gpinp_pull = pull;

  priv->dev.inp_number = ngpinp;
  priv->dev.ops = &g_gpinp_ops;

  nxmutex_init(&priv->lock);

  /* Perform harware initialization */
  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    kmm_free(priv);
    return NULL;
  }

  ret = sam_gpinp_hw_init(priv);
  if (ret < 0)
  {
    gpioinfo("GPINP hardware initialization error\n\r");
    sam_gpinp_hw_uninit(priv);

    nxmutex_unlock(&priv->lock);
    kmm_free(priv);
    return NULL;
  }

  gpioinfo("GPINP hardware initialization success!\n\r");

  nxmutex_unlock(&priv->lock);
  return &priv->dev;
}

/****************************************************************************
 * Name: sam_gpinp_uninitialize
 *
 * Description:
 *    Perform sam_gpinp structure and hardware uninitialization.
 *
 * Arguments:
 *    dev - An upperhalf driver structure
 *
 ****************************************************************************/

void sam_gpinp_uninitialize(FAR struct gpinp_dev_s *dev)
{
  struct sam_gpinp_dev_s *priv;
  int ret = OK;

  DEBUGASSERT(dev != NULL);

  /* Get the private structure from the upper half driver */
  priv = (struct sam_gpinp_dev_s *)dev;

  ret = nxmutex_lock(&priv->lock);
  if (ret < 0)
  {
    return;
  }

  sam_gpinp_hw_uninit(priv);

  /* Clear the pull pointer to prevent use-after-free */
  priv->gpinp_pull = NULL;
  priv->dev.inp_number = 0;

  gpioinfo("GPINP hardware uninitialization success!\n\r");

  nxmutex_unlock(&priv->lock);
  kmm_free(priv);
}


#endif /* CONFIG_SAMD2L2_GPINP */
