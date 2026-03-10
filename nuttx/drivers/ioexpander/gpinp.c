/****************************************************************************
 * drivers/ioexpander/gpinp.c
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

#include <nuttx/fs/fs.h>
#include <nuttx/ioexpander/gpinp.h>

#ifdef CONFIG_DEV_GPINP

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int     gpinp_open(FAR struct file *filep);
static int     gpinp_close(FAR struct file *filep);
static ssize_t gpinp_read(FAR struct file *filep,
                          FAR char *buffer,
                          size_t buflen);
static ssize_t gpinp_write(FAR struct file *filep,
                           FAR const char *buffer,
                           size_t buflen);
static int     gpinp_ioctl(FAR struct file *filep,
                           int cmd,
                           unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_gpinp_drvrops =
{
  gpinp_open,   /* open */
  gpinp_close,  /* close */
  gpinp_read,   /* read */
  gpinp_write,  /* write */
  NULL,         /* seek */
  gpinp_ioctl,  /* ioctl */
  NULL,         /* mmap */
  NULL,         /* truncate */
  NULL,         /* poll */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gpinp_open
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

static int gpinp_open(FAR struct file *filep)
{
  FAR struct inode *inode;
  FAR struct gpinp_dev_s *dev;

  inode = filep->f_inode;
  DEBUGASSERT(inode->i_private != NULL);
  dev = inode->i_private;

  filep->f_priv = (FAR void *)dev->inp_number;
  return OK;
}

/****************************************************************************
 * Name: gpinp_close
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

static int gpinp_close(FAR struct file *filep)
{
  filep->f_priv = (FAR void*)0;
  return OK;
}

/****************************************************************************
 * Name: gpinp_read
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

static ssize_t gpinp_read(FAR struct file *filep,
                          FAR char *buffer,
                          size_t buflen)
{
  FAR struct inode *inode;
  FAR struct gpinp_dev_s *dev;
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

  memset((void *)buffer, 0x0, buflen);

  for (i = 0; (i < dev->inp_number) && (i < (buflen * 8)); i++)
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
 * Name: gpinp_write
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

static ssize_t gpinp_write(FAR struct file *filep,
                           FAR const char *buffer,
                           size_t buflen)
{
  return (ssize_t)buflen;
}

/****************************************************************************
 * Name: gpinp_ioctl
 *
 * Description:
 *   Standard character driver ioctl method.
 *
 * Arguments:
 *   filep  - A pointer to file identifier
 *   cmd    - A command uniq number
 *   arg    - An argument, needed by command
 *
 * Returned values:
 *  OK - if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

static int gpinp_ioctl(FAR struct file *filep,
                       int cmd,
                       unsigned long arg)
{
  FAR struct inode *inode;
  FAR struct gpinp_dev_s *dev;
  int ret = OK;

  inode = filep->f_inode;
  DEBUGASSERT(inode->i_private != NULL);
  dev = inode->i_private;
  DEBUGASSERT(dev->ops != NULL);

  switch (cmd)
  {
      /* Command:     GPINP_BIT_READ
       * Description: Read the value of an input
       * Argument:    A pointer to an bool value to receive the result:
       *              false=low value; true=high value.
       */
      case GPINP_BIT_READ:

        struct bitval_s *pull = (struct bitval_s *)arg;

        ret = dev->ops->go_read(dev, pull->bit, pull->val);
        if (ret < 0)
        {
          return ret;
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
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gpinp_register
 *
 * Description:
 *   Register gpinp device driver.
 *
 * Arguments:
 *   dev    - A pointer to a gpinp_dev_s
 *   minor  - A number of the registering driver
 *
 * Returned values:
 *  OK - if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

int gpinp_register(FAR struct gpinp_dev_s *dev,
                   int minor)
{
  char devname[32];

  DEBUGASSERT(dev != NULL && dev->ops != NULL);

  snprintf(devname, sizeof(devname), "/dev/gpinp%u", (unsigned int)minor);

  gpioinfo("Registering %s\n", devname);

  if (dev->register_count != 0)
  {
    gpioinfo("Already registered %s\n", devname);
    return -EEXIST;
  }

  dev->register_count = 1;

  return register_driver(devname, &g_gpinp_drvrops, 0666, dev);
}

/****************************************************************************
 * Name: gpinp_unregister
 *
 * Description:
 *   Unregister gpinp device driver at /dev/gpinp(minor).
 *
 * Arguments:
 *   dev      - A pointer to a gpinp_dev_s
 *   minor  - A number of the unregistering driver
 *
 * Returned values:
 *  OK - if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

int gpinp_unregister(FAR struct gpinp_dev_s *dev,
                     int minor)
{
  char devname[32];

  snprintf(devname, sizeof(devname), "/dev/gpinp%u", (unsigned int)minor);

  gpioinfo("Unregistering %s\n", devname);

  dev->register_count = 0;

  return unregister_driver(devname);
}

#endif /* CONFIG_DEV_GPINP */
