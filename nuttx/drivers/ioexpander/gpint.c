/****************************************************************************
 * drivers/ioexpander/gpint.c
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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>

#include <poll.h>
#include <signal.h>

#include <nuttx/fs/fs.h>
#include <nuttx/ioexpander/gpint.h>

#ifdef CONFIG_DEV_GPINT

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int      gpint_handler(FAR struct gpint_dev_s *dev);
static int      gpint_open(FAR struct file *filep);
static int      gpint_close(FAR struct file *filep);
static ssize_t  gpint_read(FAR struct file *filep,
                           FAR char *buffer,
                           size_t buflen);
static ssize_t  gpint_write(FAR struct file *filep,
                            FAR const char *buffer,
                            size_t buflen);
static int      gpint_ioctl(FAR struct file *filep,
                            int cmd,
                            unsigned long arg);
static int      gpint_poll(FAR struct file *filep,
                           FAR struct pollfd *fds,
                           bool setup);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_gpint_drvrops =
{
  gpint_open,   /* open */
  gpint_close,  /* close */
  gpint_read,   /* read */
  gpint_write,  /* write */
  NULL,         /* seek */
  gpint_ioctl,  /* ioctl */
  NULL,         /* mmap */
  NULL,         /* truncate */
  gpint_poll,   /* poll */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gpint_handler
 *
 * Description:
 *    GPINT interrupt callback function.
 *
 * Arguments:
 *    dev - The pointer to main driver gpint_dev_s structure
 *
 * Returned values:
 *    OK - Always
 *
 ****************************************************************************/

static int gpint_handler(FAR struct gpint_dev_s *dev)
{
#if CONFIG_DEV_GPINT_NSIGNALS > 0
  int i;
#endif

  DEBUGASSERT(dev != NULL);

  dev->int_count++;

#if CONFIG_DEV_GPINT_NPOLLWAITERS > 0
  poll_notify(dev->fds, CONFIG_DEV_GPINT_NPOLLWAITERS, POLLIN);
#endif

#if CONFIG_DEV_GPINT_NSIGNALS > 0
  for (i = 0; i < CONFIG_DEV_GPINT_NSIGNALS; i++)
  {
    FAR struct gpint_signal_s *signal = &dev->gp_signals[i];

    if (signal->gp_pid == 0)
    {
      break;
    }

    /* Fill the sigev_value field with the nubmer of current interrupt.
     * The current interrupt number must be provided by a lowerhalf
     * architecture specific handler.
     */
    signal->gp_event.sigev_value.sival_int = (int)dev->int_current;

  #ifdef CONFIG_SIG_EVTHREAD
    /* Perform the notification with work queue mechanism if defined */
    nxsig_notification(signal->gp_pid, &signal->gp_event,
                       SI_QUEUE, &signal->gp_work);
  #else
    /* Perform the signal notification */
    nxsig_notification(signal->gp_pid, &signal->gp_event,
                       SI_ASYNCIO, NULL);
  #endif

  }
#endif

  return OK;
}

/****************************************************************************
 * Name: gpint_open
 *
 * Description:
 *   Standard character driver open method.
 *
 * Arguments:
 *   filep  - A pointer to file identifier
 *
 * Returned values:
 *   OK - Always
 *
 ****************************************************************************/

static int gpint_open(FAR struct file *filep)
{
  FAR struct inode *inode;
  FAR struct gpint_dev_s *dev;

  inode = filep->f_inode;
  DEBUGASSERT(inode->i_private != NULL);
  dev = inode->i_private;

  filep->f_priv = (FAR void *)dev->int_count;
  return OK;
}

/****************************************************************************
 * Name: gpint_close
 *
 * Description:
 *   Standard character driver close method.
 *
 * Arguments:
 *   filep  - A pointer to file identifier
 *
 * Returned values:
 *   OK - Always
 *
 ****************************************************************************/

static int gpint_close(FAR struct file *filep)
{
  filep->f_priv = (FAR void*)0;
  return OK;
}

/****************************************************************************
 * Name: gpint_read
 *
 * Description:
 *   Standard character driver read method.
 *
 * Arguments:
 *   filep  - A pointer to file identifier
 *   buffer - A pointer to read buffer
 *   buflen - A buffer length
 *
 * Returned values:
 *  ssize_t - size of readed bytes, if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

static ssize_t gpint_read(FAR struct file *filep,
                          FAR char *buffer,
                          size_t buflen)
{
  FAR struct inode *inode;
  FAR struct gpint_dev_s *dev;
  int ret;
  uint32_t i;
  bool bit;

  inode = filep->f_inode;
  DEBUGASSERT(inode->i_private != NULL);
  dev = inode->i_private;

  DEBUGASSERT(buffer != NULL);
  if (buflen == 0)
  {
    return 0;  /* Zero will be interpreted as the End-of-File. */
  }

  /* Update interrupt count and read the GPINT value */
  filep->f_priv = (FAR void *)dev->int_count;

  memset((void *)buffer, 0x0, buflen);

  for (i = 0; (i < dev->int_number) && (i < (buflen * 8)); i++)
  {
    /* Perform sequiential reading of the bit positions from the
     * corresponding data register bit position
     */

    ret = dev->ops->go_read(dev, i, &bit);
    if (ret < 0)
    {
      return ret;
    }

    buffer[i/8] |= bit << (i%8);
  }

  return (ssize_t)((i/8) + 1);
}

/****************************************************************************
 * Name: gpint_write
 *
 * Description:
 *   Standard character driver write method.
 *
 * Arguments:
 *   filep  - A pointer to file identifier
 *   buffer - A pointer to write buffer
 *   buflen - A buffer length
 *
 * Returned values:
 *  ssize_t - size of writed bytes. Always OK, becouse of dummy method.
 *
 ****************************************************************************/

static ssize_t gpint_write(FAR struct file *filep,
                           FAR const char *buffer,
                           size_t buflen)
{
  return (ssize_t)buflen;
}

/****************************************************************************
 * Name: gpint_ioctl
 *
 * Description:
 *   Standard character driver ioctl method.
 *
 * Arguments:
 *   filep  - A pointer to file identifier
 *   cmd    - A command uniq number
 *   arg    - An argument, needed by command
 *
 * Returned value:
 *  OK - if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

static int gpint_ioctl(FAR struct file *filep,
                       int cmd,
                       unsigned long arg)
{
  FAR struct inode *inode;
  FAR struct gpint_dev_s *dev;
  irqstate_t flags;
  int ret = OK;
  bool cmdflag = true;
#if CONFIG_DEV_GPINT_NSIGNALS > 0
  pid_t pid;
  int i;
  int j;
#endif

  inode = filep->f_inode;
  DEBUGASSERT(inode->i_private != NULL);
  dev = inode->i_private;
  DEBUGASSERT(dev->ops != NULL);

  switch (cmd)
  {
    /* Command:     GPINT_BIT_READ
     * Description: Read the value of an interrupt input
     * Argument:    A pointer to a struct bitval_s to receive the value
     *              and point on desired bit position.
     */
    case GPINT_BIT_READ:

      struct bitval_s *pull = (struct bitval_s *)arg;

      /* Update interrupt count and read the GPINT value */
      filep->f_priv = (FAR void *)dev->int_count;

      ret = dev->ops->go_read(dev, pull->bit, pull->val);
      if (ret < 0)
      {
        return ret;
      }

      break;

    /* Command:     GPINT_REGISTER
     * Description: Register to receive a signal whenever there an
     *              interrupt is received on an input interrupt pin.
     * Argument:    A pointer to the sigevent structure.
     */
    case GPINT_REGISTER:
      flags = enter_critical_section();
#if CONFIG_DEV_GPINT_NSIGNALS > 0
      if (arg)
      {
        pid = nxsched_getpid();

        /* Perform searching for empty cell or current pid cell */
        for (i = 0; i < CONFIG_DEV_GPINT_NSIGNALS; i++)
        {
          FAR struct gpint_signal_s *signal = &dev->gp_signals[i];

          /* If it was found then perform copying from arg strcucture
           * to in-driver structure and assign the pid of current process
           * to this cell.
           */
          if (signal->gp_pid == 0 || signal->gp_pid == pid)
          {
            memcpy(&signal->gp_event, (FAR void *)arg,
                   sizeof(signal->gp_event));
            signal->gp_pid = pid;
            break;
          }
        }

        /* If there are no free cells then return EBUSY */
        if (i == CONFIG_DEV_GPINT_NSIGNALS)
        {
          leave_critical_section(flags);
          ret = -EBUSY;
          break;
        }
      }
#endif
      leave_critical_section(flags);

      /* Check if this cell already have the attached handler */
      if (dev->attached == true)
      {
        break;
      }

      /* Attach the gpint_handler */
      DEBUGASSERT(dev->ops->go_attach != NULL);
      ret = dev->ops->go_attach(dev,
                        (void *)gpint_handler);
      if (ret >= 0)
      {
        dev->attached = true;
      }
      break;

      /* Command:     GPIOC_UNREGISTER
       * Description: Stop receiving signals for pin interrupts.
       * Argument:    NULL
       */
      case GPINT_UNREGISTER:
        flags = enter_critical_section();

#if CONFIG_DEV_GPINT_NSIGNALS > 0
        pid = nxsched_getpid();

        /* Perform the searching for cell with assigned pid */
        for (i = 0; i < CONFIG_DEV_GPINT_NSIGNALS; i++)
        {
          /* It was founded */
          if (pid == dev->gp_signals[i].gp_pid)
          {
            /* Perform searching a first free cell in the array */
            for (j = i + 1; j < CONFIG_DEV_GPINT_NSIGNALS; j++)
            {
              /* It was founded */
              if (dev->gp_signals[j].gp_pid == 0)
              {
                break;
              }
            }

            /* Decrement j:
              *  - if the next cell after our was free (pid == 0 = 1)
              *      (i == --j = 1) then ignor this branch and just release
              *      the cell;
              *  - if the following cell(s) is busy (pid != 0 = 1)
              *      (i < --j = 1) then j is the index of the last busy cell,
              *      perform transfer this last cell to i (our pid) position
              *      and release j position.
              */
            if (i != --j)
            {
              memcpy(&dev->gp_signals[i], &dev->gp_signals[j],
                      sizeof(dev->gp_signals[i]));
            }

            dev->gp_signals[j].gp_pid = 0;
            nxsig_cancel_notification(&dev->gp_signals[j].gp_work);
            break;
          }
        }
#endif
        leave_critical_section(flags);

        /* Check if this cell already have the detached handler */
        if (dev->attached == false)
        {
          break;
        }

        /* Make sure that the pins interrupt is disabled */
        DEBUGASSERT(dev->ops->go_enable != NULL);
        for (i = 0; i < dev->int_number; i++)
        {
          ret = dev->ops->go_enable(dev, i, false);
          if (ret < 0)
          {
            break;
          }
        }

        /* Detach the handler */
        DEBUGASSERT(dev->ops->go_attach != NULL);
        ret = dev->ops->go_attach(dev, NULL);
        dev->attached = false;
        break;

      /* Command:     GPINT_BIT_DISABLE
       * Description: Disable the interrupt generating on dedicated pin.
       * Argument:    The index of desired interrupt pin.
       *
       * Command:     GPINT_BIT_ENABLE
       * Description: Enable the interrupt generating on dedicated pin.
       * Argument:    The pointer to gpint_enable_s structure.
       */
      case GPINT_BIT_DISABLE:
        cmdflag = false;
        /* Fallthrough */

      case GPINT_BIT_ENABLE:
        struct gpint_enable_s *order;

        DEBUGASSERT((void *)arg != NULL);
        order = (struct gpint_enable_s *)arg;

        switch (order->mode)
        {
          case GPINT_MODE_DIRECT:

            DEBUGASSERT(dev->ops->go_enable != NULL);
            ret = dev->ops->go_enable(dev, order->val.pos, cmdflag);
            break;

          case GPINT_MODE_MASKED:

            DEBUGASSERT(dev->ops->go_enable != NULL);

            for (i = 0; i < dev->int_number; i++)
            {
              if ((order->val.mask[i/8] & (1 << i%8)) != 0)
              {
                ret = dev->ops->go_enable(dev, i, cmdflag);
                if (ret < 0)
                {
                  break;
                }
              }
            }
            break;

          default:
            ret = -ENOTTY;
            break;
        }
        break;

      /* Unrecognized command */
      default:
        ret = -ENOTTY;
        break;
  }

  return ret;
}

/****************************************************************************
 * Name: gpint_poll
 *
 * Description:
 *    Poll method for gpint device.
 *
 * Arguments:
 *    filep  - A pointer to file identifier
 *    fds    - A pointer to the standard pollfd structure
 *    setup  - A value to setup or teardown polling
 *
 * Returned values:
 *  OK - if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

static int gpint_poll(FAR struct file *filep,
                      FAR struct pollfd *fds,
                      bool setup)
{
#if CONFIG_DEV_GPINT_NPOLLWAITERS > 0
  FAR struct inode *inode = filep->f_inode;
  FAR struct gpint_dev_s *dev = inode->i_private;
  int i;
#endif
  irqstate_t flags;
  int ret = OK;

  flags = enter_critical_section();

  /* Are we setting up the poll?  Or tearing it down? */
  if (setup)
  {
#if CONFIG_DEV_GPINT_NPOLLWAITERS > 0
    /* This is a request to set up the poll.  Find an available
     * slot for the poll structure reference
     */

    for (i = 0; i < CONFIG_DEV_GPINT_NPOLLWAITERS; i++)
    {
      /* Find an available slot */
      if (dev->fds[i] == NULL)
      {
        /* Bind the poll structure and this slot */
        dev->fds[i] = fds;
        fds->priv   = &dev->fds[i];

        /* Report if a event is pending */
        if (dev->int_count != (uintptr_t)(filep->f_priv))
        {
          poll_notify(&fds, 1, POLLIN);
        }

        break;
      }
    }

    if (i >= CONFIG_DEV_GPINT_NPOLLWAITERS)
#endif
    {
      fds->priv = NULL;
      ret       = -EBUSY;
    }
  }
  else if (fds->priv != NULL)
  {
    /* This is a request to tear down the poll. */
    FAR struct pollfd **slot = (FAR struct pollfd **)fds->priv;

    /* Remove all memory of the poll setup */
    *slot     = NULL;
    fds->priv = NULL;
  }

  leave_critical_section(flags);
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

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
                   int minor)
{

  char devname[32];

  DEBUGASSERT(dev != NULL && dev->ops != NULL);

  snprintf(devname, sizeof(devname), "/dev/gpint%u", (unsigned int)minor);

  gpioinfo("Registering %s\n", devname);

  if (dev->register_count != 0)
  {
    gpioinfo("Already registered %s\n", devname);
    return -EEXIST;
  }

  dev->register_count = 1;

  return register_driver(devname, &g_gpint_drvrops, 0666, dev);

}

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
                     int minor)
{
  char devname[32];

  snprintf(devname, sizeof(devname), "/dev/gpint%u", (unsigned int)minor);

  gpioinfo("Unregistering %s\n", devname);

  dev->register_count = 0;

  return unregister_driver(devname);
}

#endif /* CONFIG_DEV_GPINT */
