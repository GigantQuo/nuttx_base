/****************************************************************************
 * drivers/ioexpander/gpout.c
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
#include <nuttx/ioexpander/gpout.h>

#ifdef CONFIG_DEV_GPOUT

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int     gpout_open(FAR struct file *filep);
static int     gpout_close(FAR struct file *filep);
static ssize_t gpout_read(FAR struct file *filep, FAR char *buffer,
                          size_t buflen);
static ssize_t gpout_write(FAR struct file *filep, FAR const char *buffer,
                           size_t buflen);
static int     gpout_ioctl(FAR struct file *filep, int cmd,
                           unsigned long arg);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_gpout_drvrops =
{
  gpout_open,   /* open */
  gpout_close,  /* close */
  gpout_read,   /* read */
  gpout_write,  /* write */
  NULL,         /* seek */
  gpout_ioctl,  /* ioctl */
  NULL,         /* mmap */
  NULL,         /* truncate */
  NULL,         /* poll */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gpout_open
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

static int gpout_open(FAR struct file *filep)
{
  FAR struct inode *inode;
  FAR struct gpout_dev_s *dev;

  inode = filep->f_inode;
  DEBUGASSERT(inode->i_private != NULL);
  dev = inode->i_private;

  filep->f_priv = (FAR void *)dev->out_number;
  return OK;
}

/****************************************************************************
 * Name: gpout_close
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

static int gpout_close(FAR struct file *filep)
{
  filep->f_priv = (FAR void*)0;
  return OK;
}

/****************************************************************************
 * Name: gpout_read
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

static ssize_t gpout_read(FAR struct file *filep,
                          FAR char *buffer,
                          size_t buflen)
{
  FAR struct inode *inode;
  FAR struct gpout_dev_s *dev;
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

  for (i = 0; (i < dev->out_number) && (i < (buflen * 8)); i++)
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
 * Name: gpout_write
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
 *  ssize_t - size of writed bytes, if it is OK,
 *  Error code - if something went wrong.
 *
 ****************************************************************************/

static ssize_t gpout_write(FAR struct file *filep,
                           FAR const char *buffer,
                           size_t buflen)
{
  FAR struct inode *inode;
  FAR struct gpout_dev_s *dev;
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

  for (i = 0; (i < dev->out_number) && (i < (buflen * 8)); i++)
  {

    bit = (buffer[i/8] & (0x1 << i%8)) != 0;

    ret = dev->ops->go_write(dev, i, &bit);
    if (ret < 0)
    {
      return ret;
    }
  }

  return (ssize_t)((i/8) + 1);
}

/****************************************************************************
 * Name: gpout_ioctl
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

static int gpout_ioctl(FAR struct file *filep,
                       int cmd,
                       unsigned long arg)
{
  FAR struct inode *inode;
  FAR struct gpout_dev_s *dev;
  int ret = OK;

  inode = filep->f_inode;
  DEBUGASSERT(inode->i_private != NULL);
  dev = inode->i_private;
  DEBUGASSERT(dev->ops != NULL);

  switch (cmd)
  {
    /* Command:     GPOUT_BIT_READ
     * Description: Read the value of a dedicated output GPOUT
     * Argument:    A pointer to a struct bitval_s to receive the result
     *              and point on desired bit position.
     */
    case GPOUT_BIT_READ:
      struct bitval_s *rd_pull = (struct bitval_s *)arg;

      ret = dev->ops->go_read(dev, rd_pull->bit, rd_pull->val);
      if (ret < 0)
      {
        return ret;
      }
      break;

    /* Command:     GPOUT_BIT_WRITE
     * Description: Write the value to a dedicated output GPOUT
     * Argument:    A pointer to a struct bitval_s to write the value
     *              and point on desired bit position.
     */
    case GPOUT_BIT_WRITE:
      struct bitval_s *wr_pull = (struct bitval_s *)arg;

      ret = dev->ops->go_write(dev, wr_pull->bit, wr_pull->val);
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

int gpout_register(FAR struct gpout_dev_s *dev,
                   int minor)
{
  char devname[32];

  DEBUGASSERT(dev != NULL && dev->ops != NULL);

  snprintf(devname, sizeof(devname), "/dev/gpout%u", (unsigned int)minor);

  gpioinfo("Registering %s\n", devname);

  if (dev->register_count != 0)
  {
    gpioinfo("Already registered %s\n", devname);
    return -EEXIST;
  }

  dev->register_count = 1;

  return register_driver(devname, &g_gpout_drvrops, 0666, dev);
}

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

int gpout_unregister(FAR struct gpout_dev_s *dev,
                     int minor)
{
  char devname[32];

  snprintf(devname, sizeof(devname), "/dev/gpout%u", (unsigned int)minor);

  gpioinfo("Unregistering %s\n", devname);

  dev->register_count = 0;

  return unregister_driver(devname);
}

#endif /* CONFIG_DEV_GPOUT */
