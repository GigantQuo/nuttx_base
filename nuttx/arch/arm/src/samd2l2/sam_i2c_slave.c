/****************************************************************************
 * arch/arm/src/samd2l2/sam_i2c_slave.c
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

/* References:
 *   SAMD/SAML Series Data Sheet
 */

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
#include <nuttx/irq.h>
#include <nuttx/wdog.h>
#include <nuttx/clock.h>
#include <nuttx/mutex.h>
#include <nuttx/semaphore.h>
#include <nuttx/i2c/i2c_slave.h>

#include <arch/board/board.h>
#include <arch/chip/sam_i2c_slave.h>

#include "arm_internal.h"
#include "chip.h"
#include "sam_pinmap.h"
#include "sam_gclk.h"
#include "sam_port.h"
#include "sam_sercom.h"
#include "sam_i2c_slave.h"

#if defined(CONFIG_SAMD2L2_HAVE_I2C_SLAVE)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* Invariant attributes of a I2C bus */

struct i2c_attr_slave_s
{
  uint8_t i2c;            /* I2C device number (for debug output) */
  uint8_t sercom;         /* Identifies the SERCOM peripheral */
  uint8_t irq;            /* SERCOM IRQ number */
  uint8_t gclkgen;        /* Source GCLK generator */
  uint8_t slowgen;        /* Slow GCLK generator */
  port_pinset_t pad0;     /* Pin configuration for PAD0 */
  port_pinset_t pad1;     /* Pin configuration for PAD1 */
  uint32_t muxconfig;     /* Pad multiplexing configuration */
  uint32_t srcfreq;       /* Source clock frequency */
  uintptr_t base;         /* Base address of I2C registers */
  bool runinstdby;        /* Run in Stand-by ? */
  uint32_t sdaholdtime;   /* Hold time after start bit */
  uint32_t speed;         /* I2C Speed: Standard; Fast; High */
  bool scllowtout;        /* SCL low timeout */
  uint32_t inactout;      /* Inactive Bus Timeout */
  bool sclstretch;        /* SCL stretch only after ACK */
  bool sclslvextout;      /* SCL Slave extend timeout */
  bool sclmstextout;      /* SCL Master extend timeout */
};

struct sam_i2c_dev_s
{
  struct i2c_slave_s dev;
  const struct i2c_attr_slave_s* attr;
  uint32_t frequency;
  mutex_t lock;

  int result;
  const uint8_t* tx_buffer;
  uint8_t* rx_buffer;
  int rx_buflen;
  int rx_curptr;
  int tx_buflen;
  int tx_curptr;
  int rx_received;
  i2c_slave_callback_t* callback;
  void* callback_arg;
  bool read;
  bool data_ready;
  sem_t data_sem;
  bool read_pending;
#if defined(CONFIG_SAMD2L2_I2C_SLAVE_INTIME)
  void (*intime_handler)(const uint8_t *rx,
                         void *tx,
                         int *buflen,
                         int *curptr);
#endif /* CONFIG_SAMD2L2_I2C_SLAVE_INTIME */
};


/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int sam_i2c_setup(struct i2c_slave_s* dev);

static int sam_i2c_shutdown(struct i2c_slave_s* dev);

static int sam_i2c_setownaddress(struct i2c_slave_s* dev,
                                 int addr,
                                 int nbits);

static int sam_i2c_write(struct i2c_slave_s* dev,
                         const uint8_t* buffer,
                         int buflen);

static int sam_i2c_read(struct i2c_slave_s* dev,
                        uint8_t* buffer,
                        int buflen);

static int sam_i2c_registercallback(struct i2c_slave_s* dev,
                                    i2c_slave_callback_t* callback,
                                    void* arg);

static int sam_i2c_isr(int irq,
                       void* context,
                       void* arg);

static int i2c_hw_initialize(struct sam_i2c_dev_s* priv,
                             uint32_t frequency);

static void i2c_hw_uninitialize(struct sam_i2c_dev_s* priv);


/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct i2c_slaveops_s g_i2cops =
{
  .setownaddress = sam_i2c_setownaddress,
  .write = sam_i2c_write,
  .read = sam_i2c_read,
  .registercallback = sam_i2c_registercallback,
  .setup = sam_i2c_setup,
  .shutdown = sam_i2c_shutdown
};

#if defined(CONFIG_SAMD2L2_SERCOM0_ISI2C_SLAVE)
static const struct i2c_attr_slave_s g_i2c0attr =
{
  .i2c    = 0,
  .sercom    = 0,
  .irq    = SAM_IRQ_SERCOM0,
  .gclkgen  = BOARD_SERCOM0_GCLKGEN,
  .slowgen  = BOARD_SERCOM0_SLOW_GCLKGEN,
  .pad0    = BOARD_SERCOM0_PINMAP_PAD0,
  .pad1    = BOARD_SERCOM0_PINMAP_PAD1,
  .muxconfig  = BOARD_SERCOM0_MUXCONFIG,
  .srcfreq  = BOARD_SERCOM0_FREQUENCY,
  .base    = SAM_SERCOM0_BASE
};

static const struct i2c_slave_s i2c0_slave =
{
  .ops = &g_i2cops
};

static struct sam_i2c_dev_s g_i2c0 =
{
  .dev    = i2c0_slave,
  .attr    = &g_i2c0attr,
  .lock    = NXMUTEX_INITIALIZER,
  .result    = OK,
  .tx_buffer  = NULL,
  .rx_buffer  = NULL,
  .rx_buflen  = 0,
  .rx_curptr  = 0,
  .tx_curptr  = 0,
  .data_ready = false,
  .read_pending = false,
  .data_sem = SEM_INITIALIZER(0)
};
#endif /* CONFIG_SAMD2L2_SERCOM0_ISI2C_SLAVE */

#if defined(CONFIG_SAMD2L2_SERCOM1_ISI2C_SLAVE)
static const struct i2c_attr_slave_s g_i2c1attr =
{
  .i2c    = 1,
  .sercom    = 1,
  .irq    = SAM_IRQ_SERCOM1,
  .gclkgen  = BOARD_SERCOM1_GCLKGEN,
  .slowgen  = BOARD_SERCOM1_SLOW_GCLKGEN,
  .pad0    = BOARD_SERCOM1_PINMAP_PAD0,
  .pad1    = BOARD_SERCOM1_PINMAP_PAD1,
  .muxconfig  = BOARD_SERCOM1_MUXCONFIG,
  .srcfreq  = BOARD_SERCOM1_FREQUENCY,
  .base    = SAM_SERCOM1_BASE
};

static const struct i2c_slave_s i2c1_slave =
{
  .ops = &g_i2cops
};

static struct sam_i2c_dev_s g_i2c1 =
{
  .dev    = i2c1_slave,
  .attr    = &g_i2c1attr,
  .lock    = NXMUTEX_INITIALIZER,
  .result    = OK,
  .tx_buffer  = NULL,
  .rx_buffer  = NULL,
  .rx_buflen  = 0,
  .rx_curptr  = 0,
  .tx_curptr  = 0,
  .data_ready = false,
  .read_pending = false,
  .data_sem = SEM_INITIALIZER(0)
};
#endif /* CONFIG_SAMD2L2_SERCOM1_ISI2C_SLAVE */

#if defined(CONFIG_SAMD2L2_SERCOM2_ISI2C_SLAVE)
static const struct i2c_attr_slave_s g_i2c2attr =
{
  .i2c    = 2,
  .sercom    = 2,
  .irq    = SAM_IRQ_SERCOM2,
  .gclkgen  = BOARD_SERCOM2_GCLKGEN,
  .slowgen  = BOARD_SERCOM2_SLOW_GCLKGEN,
  .pad0    = BOARD_SERCOM2_PINMAP_PAD0,
  .pad1    = BOARD_SERCOM2_PINMAP_PAD1,
  .muxconfig  = BOARD_SERCOM2_MUXCONFIG,
  .srcfreq  = BOARD_SERCOM2_FREQUENCY,
  .base    = SAM_SERCOM2_BASE
};

static const struct i2c_slave_s i2c2_slave =
{
  .ops = &g_i2cops
};

static struct sam_i2c_dev_s g_i2c2 =
{
  .dev    = i2c2_slave,
  .attr    = &g_i2c2attr,
  .lock    = NXMUTEX_INITIALIZER,
  .result    = OK,
  .tx_buffer  = NULL,
  .rx_buffer  = NULL,
  .rx_buflen  = 0,
  .rx_curptr  = 0,
  .tx_curptr  = 0,
  .data_ready = false,
  .read_pending = false,
  .data_sem = SEM_INITIALIZER(0)
};
#endif /* CONFIG_SAMD2L2_SERCOM3_ISI2C_SLAVE */

#if defined(CONFIG_SAMD2L2_SERCOM3_ISI2C_SLAVE)
static const struct i2c_attr_slave_s g_i2c3attr =
{
  .i2c    = 3,
  .sercom    = 3,
  .irq    = SAM_IRQ_SERCOM3,
  .gclkgen  = BOARD_SERCOM3_GCLKGEN,
  .slowgen  = BOARD_SERCOM3_SLOW_GCLKGEN,
  .pad0    = BOARD_SERCOM3_PINMAP_PAD0,
  .pad1    = BOARD_SERCOM3_PINMAP_PAD1,
  .muxconfig  = BOARD_SERCOM3_MUXCONFIG,
  .srcfreq  = BOARD_SERCOM3_FREQUENCY,
  .base    = SAM_SERCOM3_BASE
};

static const struct i2c_slave_s i2c3_slave =
{
  .ops = &g_i2cops
};

static struct sam_i2c_dev_s g_i2c3 =
{
  .dev    = i2c3_slave,
  .attr    = &g_i2c3attr,
  .lock    = NXMUTEX_INITIALIZER,
  .result    = OK,
  .tx_buffer  = NULL,
  .rx_buffer  = NULL,
  .rx_buflen  = 0,
  .rx_curptr  = 0,
  .tx_curptr  = 0,
  .data_ready = false,
  .read_pending = false,
  .data_sem = SEM_INITIALIZER(0)
};
#endif /* CONFIG_SAMD2L2_SERCOM3_ISI2C_SLAVE */

#if defined(CONFIG_SAMD2L2_SERCOM4_ISI2C_SLAVE)
static const struct i2c_attr_slave_s g_i2c4attr =
{
  .i2c    = 4,
  .sercom    = 4,
  .irq    = SAM_IRQ_SERCOM4,
  .gclkgen  = BOARD_SERCOM4_GCLKGEN,
  .slowgen  = BOARD_SERCOM4_SLOW_GCLKGEN,
  .pad0    = BOARD_SERCOM4_PINMAP_PAD0,
  .pad1    = BOARD_SERCOM4_PINMAP_PAD1,
  .muxconfig  = BOARD_SERCOM4_MUXCONFIG,
  .srcfreq  = BOARD_SERCOM4_FREQUENCY,
  .base    = SAM_SERCOM4_BASE
};

static const struct i2c_slave_s i2c4_slave =
{
  .ops = &g_i2cops
};

static struct sam_i2c_dev_s g_i2c4 =
{
  .dev    = i2c4_slave,
  .attr    = &g_i2c4attr,
  .lock    = NXMUTEX_INITIALIZER,
  .result    = OK,
  .tx_buffer  = NULL,
  .rx_buffer  = NULL,
  .rx_buflen  = 0,
  .rx_curptr  = 0,
  .tx_curptr  = 0,
  .data_ready = false,
  .read_pending = false,
  .data_sem = SEM_INITIALIZER(0)
};
#endif /* CONFIG_SAMD2L2_SERCOM4_ISI2C_SLAVE */

#if defined(CONFIG_SAMD2L2_SERCOM5_ISI2C_SLAVE)
static const struct i2c_attr_slave_s g_i2c5attr =
{
  .i2c    = 5,
  .sercom    = 5,
  .irq    = SAM_IRQ_SERCOM5,
  .gclkgen  = BOARD_SERCOM5_GCLKGEN,
  .slowgen  = BOARD_SERCOM5_SLOW_GCLKGEN,
  .pad0    = BOARD_SERCOM5_PINMAP_PAD0,
  .pad1    = BOARD_SERCOM5_PINMAP_PAD1,
  .muxconfig  = BOARD_SERCOM5_MUXCONFIG,
  .srcfreq  = BOARD_SERCOM5_FREQUENCY,
  .base    = SAM_SERCOM5_BASE
};

static const struct i2c_slave_s i2c5_slave =
{
  .ops = &g_i2cops
};

static struct sam_i2c_dev_s g_i2c5 =
{
  .dev    = i2c5_slave,
  .attr    = &g_i2c5attr,
  .lock    = NXMUTEX_INITIALIZER,
  .result    = OK,
  .tx_buffer  = NULL,
  .rx_buffer  = NULL,
  .rx_buflen  = 0,
  .rx_curptr  = 0,
  .tx_curptr  = 0,
  .data_ready = false,
  .read_pending = false,
  .data_sem = SEM_INITIALIZER(0)
};
#endif /* CONFIG_SAMD2L2_SERCOM5_ISI2C_SLAVE */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: i2c_getreg8
 *
 * Description:
 *   Get a 8-bit register value by offset
 *
 ****************************************************************************/

static inline uint8_t i2c_getreg8(struct sam_i2c_dev_s* priv,
                                  unsigned int offset)
{
  return getreg8(priv->attr->base + offset);
}

/****************************************************************************
 * Name: i2c_putreg8
 *
 * Description:
 *  Put a 8-bit register value by offset
 *
 ****************************************************************************/

static inline void i2c_putreg8(struct sam_i2c_dev_s* priv,
                               uint8_t regval,
                               unsigned int offset)
{
  putreg8(regval, priv->attr->base + offset);
}

/****************************************************************************
 * Name: i2c_getreg16
 *
 * Description:
 *   Get a 16-bit register value by offset
 *
 ****************************************************************************/

static inline uint16_t i2c_getreg16(struct sam_i2c_dev_s* priv,
                                    unsigned int offset)
{
  return getreg16(priv->attr->base + offset);
}

/****************************************************************************
 * Name: i2c_putreg16
 *
 * Description:
 *  Put a 16-bit register value by offset
 *
 ****************************************************************************/

static inline void i2c_putreg16(struct sam_i2c_dev_s* priv,
                                uint16_t regval,
                                unsigned int offset)
{
  putreg16(regval, priv->attr->base + offset);
}

/****************************************************************************
 * Name: i2c_getreg32
 *
 * Description:
 *   Get a 32-bit register value by offset
 *
 ****************************************************************************/

static inline uint32_t i2c_getreg32(struct sam_i2c_dev_s* priv,
                                    unsigned int offset)
{
  return getreg32(priv->attr->base + offset);
}

/****************************************************************************
 * Name: i2c_putreg32
 *
 * Description:
 *  Put a 32-bit register value by offset
 *
 ****************************************************************************/

static inline void i2c_putreg32(struct sam_i2c_dev_s* priv,
                                uint32_t regval,
                                unsigned int offset)
{
  putreg32(regval, priv->attr->base + offset);
}

/****************************************************************************
 * Name: i2c_wait_synchronization
 *
 * Description:
 *   Wait until the SERCOM I2C reports that it is synchronized.
 *
 ****************************************************************************/

static inline void i2c_wait_synchronization(struct sam_i2c_dev_s* priv)
{
#ifdef CONFIG_ARCH_FAMILY_SAMD21
  while((i2c_getreg32(priv, SAM_I2C_SYNCBUSY_OFFSET) & 0x3) != 0);
#endif

#ifdef CONFIG_ARCH_FAMILY_SAMD20
  while((i2c_getreg16(priv, SAM_I2C_STATUS_OFFSET) & \
        I2C_STATUS_SYNCBUSY) != 0);
#endif
}

/****************************************************************************
 * Name: i2c_terminate
 *
 * Description:
 *  Execute terminating sequence by sending ACK/NACK
 *
 ****************************************************************************/

static inline void i2c_terminate(struct sam_i2c_dev_s* priv)
{
  uint32_t regval;

  /* Prepare CMD bits */
  regval = i2c_getreg32(priv, SAM_I2C_CTRLB_OFFSET);
  regval &= ~I2C_CTRLB_CMD_MASK;

  /* Prepare acknowledge value */
  regval &= ~I2C_CTRLB_ACKACT; /* Send ACK */

  /* Execute acknowledge action */
  regval |= I2C_CTRLB_CMD_ACKREAD;
  i2c_putreg32(priv, regval, SAM_I2C_CTRLB_OFFSET);
  i2c_wait_synchronization(priv);
}

/****************************************************************************
 * Name: i2c_stop
 *
 * Description:
 *  Process stop-condition on bus
 *
 ****************************************************************************/

static inline void i2c_stop(struct sam_i2c_dev_s* priv)
{
  uint32_t regval;

  /* Prepare CMD bits */
  regval = i2c_getreg32(priv, SAM_I2C_CTRLB_OFFSET);
  regval &= ~I2C_CTRLB_CMD_MASK;

  /* Prepare acknowledge value */
  regval &= ~I2C_CTRLB_ACKACT; /* Send ACK */

  /* Execute acknowledge action */
  regval |= I2C_CTRLB_CMD_WAITSTART;
  i2c_putreg32(priv, regval, SAM_I2C_CTRLB_OFFSET);
  i2c_wait_synchronization(priv);
}

/****************************************************************************
 * Name: sam_i2c_setup
 *
 * Description:
 *   Sets up the sam I2C peripheral
 *
 ****************************************************************************/

static int sam_i2c_setup(struct i2c_slave_s* dev)
{
  struct sam_i2c_dev_s* priv = (struct sam_i2c_dev_s*)dev;
  uint32_t regval;

  DEBUGASSERT(dev);

  i2cinfo("SETUP I2C-slave bus %d\n", priv->attr->sercom);

  /* Enable the interrupts here. This function will be called when the device
   * is opened for the first time.
   */

  irq_attach(  priv->attr->irq,
        sam_i2c_isr,
        priv);

  regval = I2C_INT_ALL;

  i2c_putreg8(priv, regval, SAM_I2C_INTENSET_OFFSET);
  i2c_wait_synchronization(priv);

  return OK;
}

/****************************************************************************
 * Name: sam_i2c_shutdown
 *
 * Description:
 *   Shutdown the sam I2C peripheral
 *
 ****************************************************************************/

static int sam_i2c_shutdown(struct i2c_slave_s* dev)
{
  struct sam_i2c_dev_s* priv = (struct sam_i2c_dev_s*)dev;
  uint32_t regval;

  DEBUGASSERT(dev);

  i2cinfo("SHUTDOWN I2C-slave bus %d\n", priv->attr->sercom);

  /* Disable I2C */
  regval = i2c_getreg32(priv, SAM_I2C_CTRLA_OFFSET);
  regval &= ~I2C_CTRLA_ENABLE;
  i2c_putreg32(priv, regval, SAM_I2C_CTRLA_OFFSET);
  i2c_wait_synchronization(priv);

  /* Disable TX and TX interrupts. */
  up_disable_irq(priv->attr->irq);
  irq_detach(priv->attr->irq);

  return OK;
}

/****************************************************************************
 * Name: sam_i2c_setownaddress
 *
 * Description:
 *   Sets up the address of the I2C Slave
 *
 ****************************************************************************/

static int sam_i2c_setownaddress(struct i2c_slave_s* dev,
                                 int addr,
                                 int nbits)
{
  struct sam_i2c_dev_s* priv = (struct sam_i2c_dev_s*)dev;
  uint32_t regval;

  DEBUGASSERT(dev);

  i2cinfo("SETOWNADDR I2C %d on bus %d\n", addr, priv->attr->sercom);

  if (nbits == 10)
  {
    /* Ten bit address */
    regval =  (addr << I2C_ADDR_SHIFT);

#ifdef CONFIG_ARCH_FAMILY_SAMD21
    regval = regval | I2C_ADDR_TENBITEN;
#endif

    i2c_putreg32(priv, regval, SAM_I2C_ADDR_OFFSET);
  }
  else if (nbits == 7)
  {
    /* Seven bit address */
    regval =  (addr << I2C_ADDR_SHIFT);

    i2c_putreg32(priv, regval, SAM_I2C_ADDR_OFFSET);
  }
  else
  {
    /* Wrong nbits */
    return ERROR;
  }

  /* Enable I2C */

  regval = i2c_getreg32(priv, SAM_I2C_CTRLA_OFFSET);
  regval |= I2C_CTRLA_ENABLE;
  i2c_putreg32(priv, regval, SAM_I2C_CTRLA_OFFSET);
  i2c_wait_synchronization(priv);

  priv->read = false;

  up_enable_irq(priv->attr->irq);

  return OK;
}

/****************************************************************************
 * Name: sam_i2c_write
 *
 * Description:
 *   Receive a pointer to a buffer where to write data to
 *
 ****************************************************************************/

static int sam_i2c_write(struct i2c_slave_s* dev,
                         const uint8_t* buffer,
                         int buflen)
{
  struct sam_i2c_dev_s* priv = (struct sam_i2c_dev_s*)dev;
  int flags;

  DEBUGASSERT(dev);
  flags = enter_critical_section();

  /* Initialize the TX buffer. */
  priv->tx_buffer = buffer;
  priv->tx_buflen = buflen;
  priv->tx_curptr = 0;

  leave_critical_section(flags);
  return OK;
}

/****************************************************************************
 * Name: sam_i2c_read
 *
 * Description:
 *   Receive a pointer to a buffer where to read data to
 *
 ****************************************************************************/

static int sam_i2c_read(struct i2c_slave_s* dev,
                        uint8_t* buffer,
                        int buflen)
{
  struct sam_i2c_dev_s* priv = (struct sam_i2c_dev_s*)dev;
  int flags;

  DEBUGASSERT(dev);
  flags = enter_critical_section();

  /* Initialize the RX buffer. */
  priv->rx_buffer = buffer;
  priv->rx_buflen = buflen;

  leave_critical_section(flags);
  return OK;
}

/****************************************************************************
 * Name: sam_i2c_registercallback
 *
 * Description:
 *   Register a function which notifies the upperhalf driver
 *
 ****************************************************************************/

static int sam_i2c_registercallback(struct i2c_slave_s* dev,
                                    i2c_slave_callback_t* callback,
                                    void *arg)
{
  struct sam_i2c_dev_s* priv = (struct sam_i2c_dev_s*)dev;
  int flags;

  DEBUGASSERT(dev);
  flags = enter_critical_section();

  /* Initialize the pointer to a callback */
  priv->callback = callback;
  priv->callback_arg = arg;

  leave_critical_section(flags);
  return OK;
}

/****************************************************************************
 * Name: sam_i2c_isr
 *
 * Description:
 *   Common I2C interrupt service routine
 *
 ****************************************************************************/

static int sam_i2c_isr(int irq,
                       void* context,
                       void* arg)
{
  struct sam_i2c_dev_s* priv = (struct sam_i2c_dev_s*)arg;
  volatile uint8_t rx;
  volatile uint8_t tx;
  uint8_t intflag;

  intflag = i2c_getreg8(priv, SAM_I2C_INTFLAG_OFFSET);


  /* An address match interrupt has occured */
  if (intflag & I2C_INT_AMATCH)
  {
     /* Clear amatch flag */
     i2c_putreg8(priv, I2C_INT_AMATCH, SAM_I2C_INTFLAG_OFFSET);
     i2c_wait_synchronization(priv);

     /* Repeated Start */
     if (priv->rx_curptr > 0 && priv->callback)
     {
      priv->callback(  priv->callback_arg,
                I2CS_RX_COMPLETE,
                priv->rx_curptr);
     }


     /* Read operation is in process */
     if (i2c_getreg8(priv, SAM_I2C_STATUS_OFFSET) & I2C_STATUS_DIR)
     {
       /* Prepare the remaining data to send */
       if (priv->tx_curptr < priv->tx_buflen)
       {
         tx = priv->tx_buffer[priv->tx_curptr];
       }
       else
       {
         /* Nothing to be sent. */
         tx = CONFIG_I2C_SLAVE_DEFAULT_TX;
       }

       i2c_putreg8(priv, tx, SAM_I2C_DATA_OFFSET);
       i2c_wait_synchronization(priv);

       priv->read = false;
     }
     /* Write operation is in process */
     else
     {
       /* Just prepare to write operation */
       priv->read = true;
       priv->rx_curptr = 0;
     }

     priv->result = OK;
     i2c_terminate(priv);
  }

  /* A data ready interrupt has occured */
  if (intflag & I2C_INT_DRDY)
  {
    /* Clear DRDY flag */
    i2c_putreg8(priv, I2C_INT_DRDY, SAM_I2C_INTFLAG_OFFSET);
    i2c_wait_synchronization(priv);

    /* Read operation is in process */
    if (i2c_getreg8(priv, SAM_I2C_STATUS_OFFSET) & I2C_STATUS_DIR)
    {
      /* Check, if anything must be sent */
      if (priv->tx_curptr < priv->tx_buflen)
      {
        /* Yes. Send without notification */
        priv->tx_curptr++;
        tx = priv->tx_buffer[priv->tx_curptr];
      }
      else if (priv->tx_curptr == priv->tx_buflen)
      {
        /* Last byte was transmitted. Notify upper driver */
        tx = CONFIG_I2C_SLAVE_DEFAULT_TX;
        if (priv->callback)
        {
          priv->callback(  priv->callback_arg,
                  I2CS_TX_COMPLETE,
                  priv->tx_curptr);
        }
      }
      else
      {
        /* No. Send the default value. */
        tx = CONFIG_I2C_SLAVE_DEFAULT_TX;
      }
      /* Directly sending */
      i2c_putreg8(priv, tx, SAM_I2C_DATA_OFFSET);
      i2c_wait_synchronization(priv);
    }
    /* Write operation is in process */
    else
    {
      /* Directly receiving */
      rx = i2c_getreg8(priv, SAM_I2C_DATA_OFFSET);

      /* We will fill it up if it is not overflowing */
      if (priv->rx_curptr < priv->rx_buflen)
      {
        priv->rx_buffer[priv->rx_curptr++] = rx;
      }

#if defined(CONFIG_SAMD2L2_I2C_SLAVE_INTIME)

       /* Execute the intime handler */
       priv->intime_handler(priv->rx_buffer,
                            (void *)&priv->tx_buffer,
                            &priv->tx_buflen,
                            &priv->tx_curptr);
#endif /* CONFIG_SAMD2L2_I2C_SLAVE_INTIME */

      i2c_terminate(priv);
    }

    priv->result = OK;
  }

  /* A stop condition interrupt has occured */
  if (intflag & I2C_INT_PREC)
  {
    /* Clear prec flag */
    i2c_putreg8(priv, I2C_INT_PREC, SAM_I2C_INTFLAG_OFFSET);
    i2c_wait_synchronization(priv);

    /* If RX was present, notify the upper driver. */
    if (priv->read)
    {
      if (priv->callback)
      {
        priv->callback(  priv->callback_arg,
                I2CS_RX_COMPLETE,
                priv->rx_curptr);

        priv->rx_curptr = 0;
      }
    }
    else
    {
      priv->tx_curptr = 0;
    }

    priv->result = OK;
    i2c_stop(priv);
  }

#ifdef CONFIG_ARCH_FAMILY_SAMD21
  /* An error interrupt has occured */
  if (i2c_getreg8(priv, SAM_I2C_INTFLAG_OFFSET) & I2C_INT_ERROR)
  {
    /* Bus error interrupt */
    if (i2c_getreg8(priv, SAM_I2C_STATUS_OFFSET) & I2C_STATUS_BUSERR)
    {
      priv->result = EBUSERR;
    }

    /* Collision interrupt */
    if (i2c_getreg8(priv, SAM_I2C_STATUS_OFFSET) & I2C_STATUS_COLL)
    {
      priv->result = ECOLL;
    }

    /* Low timeout interrupt */
    if (i2c_getreg8(priv, SAM_I2C_STATUS_OFFSET) & I2C_STATUS_LOWTOUT)
    {
      priv->result = ELOWTOUT;
    }

    /* Low extended timeout interrupt */
    if (i2c_getreg8(priv, SAM_I2C_STATUS_OFFSET) & I2C_STATUS_SEXTTOUT)
    {
      priv->result = ESEXTTOUT;
    }
  }
#endif

  /* Clear all i2c pending interrupt. Just in case */
  i2c_putreg8(priv, I2C_INT_ALL, SAM_I2C_INTFLAG_OFFSET);
  i2c_wait_synchronization(priv);

  return OK;
}


/****************************************************************************
 * Name: i2c_hw_initialize
 *
 * Description:
 *   Initialize sam I2C peripheral - clocks and pins
 *
 ****************************************************************************/

static int i2c_hw_initialize(struct sam_i2c_dev_s* priv,
                             uint32_t frequency)
{
  uint32_t regval;

  i2cinfo("I2C%d Initializing\n", priv->attr->i2c);

  /* Enable clocking to the SERCOM module in PM */
  sercom_enable(priv->attr->sercom);

  /* Configure the GCLKs for the SERCOM module */
  sercom_coreclk_configure(priv->attr->sercom, priv->attr->gclkgen, false);
  sercom_slowclk_configure(priv->attr->sercom, priv->attr->slowgen);

  /* Check if module is enabled */
  regval = i2c_getreg32(priv, SAM_I2C_CTRLA_OFFSET);
  if (regval & I2C_CTRLA_ENABLE)
  {
    i2cerr(
    "ERROR: Cannot initialize I2C because it is already initialized!\n"
    );
    return -EADDRINUSE;
  }

  /* Check if reset is in progress */
  regval = i2c_getreg32(priv, SAM_I2C_CTRLA_OFFSET);
  if (regval & I2C_CTRLA_SWRST)
  {
    i2cerr("ERROR: Module is in RESET process!\n");
    return -EBUSY;
  }

  /* Configure pads */
  if (priv->attr->pad0 != 0)
  {
    sam_configport(priv->attr->pad0);
  }
  if (priv->attr->pad1 != 0)
  {
    sam_configport(priv->attr->pad1);
  }

  regval =  I2C_CTRLA_MODE_SLAVE    |
            I2C_CTRLA_RUNSTDBY      |
#ifdef CONFIG_ARCH_FAMILY_SAMD21
            I2C_CTRLA_SPEED_FAST    |
#endif
            I2C_CTRLA_SDAHOLD_450NS |
            I2C_CTRLA_1WIRE         |
            priv->attr->muxconfig;

  i2c_putreg32(priv, regval, SAM_I2C_CTRLA_OFFSET);
  i2c_wait_synchronization(priv);

  /* Enable Smart Mode */
  regval =  I2C_CTRLB_SMEN;

  i2c_putreg32(priv, regval, SAM_I2C_CTRLB_OFFSET);
  i2c_wait_synchronization(priv);

  return OK;
}

/****************************************************************************
 * Name: i2c_hw_uninitialize
 *
 * Description:
 *   Uninitialize sam I2C peripheral
 *
 ****************************************************************************/

static void i2c_hw_uninitialize(struct sam_i2c_dev_s* priv)
{
  i2cinfo("I2C%d Uninitializing\n", priv->attr->i2c);

  /* Disable I2C interrupts */
  i2c_putreg8(priv, I2C_INT_ALL, SAM_I2C_INTENCLR_OFFSET);
  i2c_putreg8(priv, I2C_INT_ALL, SAM_I2C_INTFLAG_OFFSET);

  up_disable_irq(priv->attr->irq);

  /* Detach Interrupt Handler */
  irq_detach(priv->attr->irq);
}


/****************************************************************************
 * Public Functions
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

struct i2c_slave_s* sam_i2cbus_slave_initialize(int bus)
{
  struct sam_i2c_dev_s* priv;
  uint32_t frequency;
  irqstate_t flags;
  int ret = 0;

#if defined(CONFIG_SAMD2L2_SERCOM0_ISI2C_SLAVE)
  if (bus == 0)
  {
    /* Select up I2C0 and the (initial) I2C frequency */
    priv = &g_i2c0;
    frequency = CONFIG_SAM_I2C0_FREQUENCY;
  }
  else
#endif

#if defined(CONFIG_SAMD2L2_SERCOM1_ISI2C_SLAVE)
  if (bus == 1)
  {
    /* Select up I2C1 and the (initial) I2C frequency */
    priv = &g_i2c1;
    frequency = CONFIG_SAM_I2C1_FREQUENCY;
  }
  else
#endif

#if defined(CONFIG_SAMD2L2_SERCOM2_ISI2C_SLAVE)
  if (bus == 2)
  {
    /* Select up I2C2 and the (initial) I2C frequency */
    priv = &g_i2c2;
    frequency = CONFIG_SAM_I2C2_FREQUENCY;
  }
  else
#endif

#if defined(CONFIG_SAMD2L2_SERCOM3_ISI2C_SLAVE)
  if (bus == 3)
  {
    /* Select up I2C3 and the (initial) I2C frequency */
    priv = &g_i2c3;
    frequency = CONFIG_SAM_I2C3_FREQUENCY;
  }
  else
#endif

#if defined(CONFIG_SAMD2L2_SERCOM4_ISI2C_SLAVE)
  if (bus == 4)
  {
    /* Select up I2C4 and the (initial) I2C frequency */
    priv = &g_i2c4;
    frequency = CONFIG_SAM_I2C4_FREQUENCY;
  }
  else
#endif

#if defined(CONFIG_SAMD2L2_SERCOM5_ISI2C_SLAVE)
  if (bus == 5)
  {
    /* Select up I2C5 and the (initial) I2C frequency */
    priv = &g_i2c5;
    frequency = CONFIG_SAM_I2C5_FREQUENCY;
  }
  else
#endif
  {
    i2cerr("ERROR: Unsupported bus: I2C%d\n", bus);
    return NULL;
  }

  /* Perform one-time I2C initialization */
  flags = enter_critical_section();

  /* Perform repeatable I2C hardware initialization */
  ret = i2c_hw_initialize(priv, frequency);
  if (ret < 0)
  {
    i2cerr(
    "ERROR: Unable to perform hardware initialization on bus: %d\n", bus
    );

    leave_critical_section(flags);

    i2c_hw_uninitialize(priv);
    return NULL;
  }

  leave_critical_section(flags);
  return &priv->dev;
}

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

int sam_i2cbus_slave_uninitialize(struct i2c_slave_s* dev)
{
  struct sam_i2c_dev_s* priv = (struct sam_i2c_dev_s*)dev;
  irqstate_t flags;

  flags = enter_critical_section();

  i2c_hw_uninitialize(priv);

  leave_critical_section(flags);
  return OK;
}

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
int sam_intime_register(uint8_t bus, void *user_handler)
{
  int ret;

  ret = OK;

  switch(bus)
  {
  #if defined(CONFIG_SAMD2L2_SERCOM0_ISI2C_SLAVE)
    case (0):
      g_i2c0.intime_handler = user_handler;
      return ret;
  #endif /* CONFIG_SAMD2L2_SERCOM0_ISI2C_SLAVE */

  #if defined(CONFIG_SAMD2L2_SERCOM1_ISI2C_SLAVE)
    case (1):
      g_i2c1.intime_handler = user_handler;
      return ret;
  #endif /* CONFIG_SAMD2L2_SERCOM1_ISI2C_SLAVE */

  #if defined(CONFIG_SAMD2L2_SERCOM2_ISI2C_SLAVE)
    case (2):
      g_i2c2.intime_handler = user_handler;
      return ret;
  #endif /* CONFIG_SAMD2L2_SERCOM2_ISI2C_SLAVE */

  #if defined(CONFIG_SAMD2L2_SERCOM3_ISI2C_SLAVE)
    case (3):
      g_i2c3.intime_handler = user_handler;
      return ret;
  #endif /* CONFIG_SAMD2L2_SERCOM3_ISI2C_SLAVE */

  #if defined(CONFIG_SAMD2L2_SERCOM4_ISI2C_SLAVE)
    case (4):
      g_i2c4.intime_handler = user_handler;
      return ret;
  #endif /* CONFIG_SAMD2L2_SERCOM4_ISI2C_SLAVE */

  #if defined(CONFIG_SAMD2L2_SERCOM5_ISI2C_SLAVE)
    case (5):
      g_i2c5.intime_handler = user_handler;
      return ret;
  #endif /* CONFIG_SAMD2L2_SERCOM5_ISI2C_SLAVE */
  }

  i2cerr("No such bus: %d\n\r", bus);
  ret = -ENXIO;
  return ret;
}

#endif /* CONFIG_SAMD2L2_I2C_SLAVE_INTIME */


#endif /* SAMD2L2_HAVE_I2C_SLAVE */
