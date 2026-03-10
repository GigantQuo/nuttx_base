/****************************************************************************
* arch/arm/src/samd2l2/sam_nvmctrl.c
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <sys/param.h>

#include <arch/board/board.h>

#include <nuttx/arch.h>
#include <nuttx/mutex.h>
#include <nuttx/progmem.h>

#include "arm_internal.h"
#include "chip.h"

#include "sam_periphclks.h"
#include "sam_nvmctrl.h"

#if defined(CONFIG_SAMD2L2_NVMCTRL)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#ifndef CONFIG_SAMD2L2_FLASH_NROWS
  #error CONFIG_SAMD2L2_FLASH_NROWS is not defined
#endif

/* The program memory start address */
#define SAMD2_FLASH_START     (SAM_FLASH_BASE)

#define SAMD2_ERASEDVAL       (0xFFU)

/* Pages definitions */

/* How many pages one row contains */
#define SAMD2_PAGES_PER_ROW   (4)
/* The page size of internal Flash memory */
#define SAMD2_PAGE_SIZE       ((1 << ((getreg32(SAM_NVMCTRL_PARAM) & \
                              NVMCTRL_PARAM_PSZ_MASK) >> \
                              NVMCTRL_PARAM_PSZ_SHIFT)) * 8)
/* The max number of pages in the internal Flash volume */
#define SAMD2_FLASH_NPAGES    ((getreg32(SAM_NVMCTRL_PARAM) & \
                              NVMCTRL_PARAM_NVMP_MASK) >> \
                              NVMCTRL_PARAM_NVMP_SHIFT)
/* The current number of pages */
#define SAMD2_CUR_NPAGES      (CONFIG_SAMD2L2_FLASH_NROWS * \
                              SAMD2_PAGES_PER_ROW)
/* The first user-defined page */
#define SAMD2_FIRST_PAGE      (SAMD2_FLASH_NPAGES - SAMD2_CUR_NPAGES)
/* Page mask */
#define SAMD2_PAGE_MASK       (SAMD2_PAGE_SIZE - 1)
/* The number of word contained in a page */
#define SAMD2_PAGE_WORDS      (SAMD2_PAGE_SIZE / 4)


/* The entire Flash memory definitions */

/* The size of internal Flash memory */
#define SAMD2_FLASH_SIZE      (SAMD2_PAGE_SIZE * SAMD2_FLASH_NPAGES)
/* The number of bytes in the internal Flash memory */
#define SAMD2_FLASH_NBYTES    (SAMD2_FLASH_SIZE)
/* The current number of bytes */
#define SAMD2_CUR_NBYTES      (SAMD2_CUR_NROWS * SAMD2_ROW_SIZE)
/* The first user-defined available byte address */
#define SAMD2_FIRST_BYTE      (SAMD2_FLASH_NBYTES - SAMD2_CUR_NBYTES)


/* The rows memory definitions */

/* The minimal eraseble size equal 4 pages named row */
#define SAMD2_ROW_SIZE        (SAMD2_PAGE_SIZE * SAMD2_PAGES_PER_ROW)
/* The max number of row in the internal Flash volume */
#define SAMD2_FLASH_NROWS     (SAMD2_FLASH_NPAGES / SAMD2_PAGES_PER_ROW)
/* The current number of row */
#define SAMD2_CUR_NROWS       (CONFIG_SAMD2L2_FLASH_NROWS)
/* The first user-defined row */
#define SAMD2_FIRST_ROW       (SAMD2_FLASH_NROWS - SAMD2_CUR_NROWS)


/* The Flash memory lock region definitions */

#ifdef USE_LOCK
  /* The minimal size that can be lock */
  #define SAMD2_LOCK_SIZE     (SAMD2_FLASH_SIZE / 16)
  /* The max number of lock regions in the internal Flash volume */
  #define SAMD2_FLASH_NLOCK   (16)
  /* The current number of lock regions */
  #define SAMD2_CUR_NLOCK     ((SAMD2_CUR_NBYTES / SAMD2_LOCK_SIZE) + \
                              (SAMD2_CUR_NBYTES % SAMD2_LOCK_SIZE > 0))
  /* The first user-defined lock region */
  #define SAMD2_FIRST_LOCK    (SAMD2_FLASH_NLOCK - SAMD2_CUR_NLOCK)
#endif


/* Conversions */

#define PAGE2ROW(p)     (p / SAMD2_PAGES_PER_ROW)
#define PAGE2ADDR(p)    (p * SAMD2_PAGE_SIZE)
#ifdef USE_LOCK
  #define PAGE2LOCK(p)  (p / (SAMD2_LOCK_SIZE / SAMD2_PAGE_SIZE))
#endif

#define ROW2PAGE(r)     (r * SAMD2_PAGES_PER_ROW)
#define ROW2ADDR(r)     (r * SAMD2_ROW_SIZE)
#ifdef USE_LOCK
  #define ROW2LOCK(r)   (r / (SAMD2_LOCK_SIZE / SAMD2_ROW_SIZE))
#endif

#define ADDR2PAGE(a)    (a / SAMD2_PAGE_SIZE)
#define ADDR2ROW(a)     (a / SAMD2_ROW_SIZE)
#ifdef USE_LOCK
  #define ADDR2LOCK(a)  (a / SAMD2_LOCK_SIZE)
#endif

#ifdef USE_LOCK
  #define LOCK2PAGE(l)  (l * (SAMD2_LOCK_SIZE / SAMD2_PAGE_SIZE))
  #define LOCK2ROW(l)   (l * (SAMD2_LOCK_SIZE / SAMD2_ROW_SIZE))
  #define LOCK2ADDR(l)  (l * SAMD2_LOCK_SIZE)
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int nvm_command(uint16_t cmd, uint32_t arg);

#ifdef USE_LOCK
static int nvm_unlock(size_t page, size_t npages);
static int nvm_lock(size_t page, size_t npages);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint32_t g_page_buffer[0x400];             /* 1024B, 1KB */
static mutex_t g_page_lock = NXMUTEX_INITIALIZER;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nvm_command
 *
 * Description:
 *    Send a FLASH command
 *
 * Arguments:
 *    cmd - The FLASH command to be sent
 *    arg - The argument to accompany the command
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

static int nvm_command(uint16_t cmd, uint32_t arg)
{
  uint16_t regval;
  uint16_t status;

  /* Wait until this module isn't busy */
  status = (uint16_t)getreg8(SAM_NVMCTRL_INTFLAG);
  while (!(status & NVMCTRL_INT_READY));

  /* Check for errors */
  if (status & NVMCTRL_INT_ERROR)
  {
    status = getreg16(SAM_NVMCTRL_STATUS);
    if ((status & (NVMCTRL_STATUS_PROGE |        /* Programming error */
#ifdef USE_LOCK
                  NVMCTRL_STATUS_LOCKE |        /* Lock error */
#endif
                  NVMCTRL_STATUS_NVME)) != 0)    /* NVM error */
    {
      ferr("ERROR: cmd=0x%x regval=0x%x status=0x%x\n",
           cmd,
           regval,
           status);
      return -EIO;
    }
  }

  /* Clear status */
  putreg8(NVMCTRL_INT_READY, SAM_NVMCTRL_INTFLAG);

  /* Set address */
  if (arg)
    putreg32((arg/2), SAM_NVMCTRL_ADDR);

  /* Write the command to the flash command register */
  putreg16(cmd | NVMCTRL_CTRLA_CMDEX, SAM_NVMCTRL_CTRLA);

  /* Wait until this module isn't busy */
  status = (uint16_t)getreg8(SAM_NVMCTRL_INTFLAG);
  while (!(status & NVMCTRL_INT_READY));

  return OK;
}

/****************************************************************************
 * Name: nvm_unlock
 *
 * Description:
 *    Make sure that the FLASH is unlocked
 *
 * Arguments:
 *    page  - The first page to unlock
 *    npages - The number of consecutive pages to unlock
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

#ifdef USE_LOCK
static int nvm_unlock(size_t page, size_t npages)
{
  size_t start_region;
  size_t end_region;
  size_t unlockregion;
  int ret;

  ret = OK;

  /* Align the page to the unlock region */
  end_region   = PAGE2LOCK(page + npages);
  end_region   = LOCK2ADDR(end_region);

  start_region = PAGE2LOCK(page);
  start_region = LOCK2ADDR(start_region);

  unlockregion = start_region;

  do
  {
    finfo("INFO: unlock region=%d address=0x%x\n",
          ADDR2LOCK(unlockregion),
          unlockregion);
    ret = nvm_command(NVMCTRL_CTRLA_CMD_UR, unlockregion);
    if (ret < 0)
    {
      return ret;
    }

    unlockregion += SAMD2_LOCK_SIZE;
  }
  while (unlockregion < end_region);

  return ret;
}


/****************************************************************************
 * Name: nvm_lock
 *
 * Description:
 *    Make sure that the FLASH is locked
 *
 * Arguments:
 *    page  - The first page to lock
 *    npages - The number of consecutive pages to lock
 *
 * Returned values:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/


static int nvm_lock(size_t page, size_t npages)
{
  size_t start_region;
  size_t end_region;
  size_t lockregion;
  int ret;

  ret = OK;

  /* Align the page to the unlock region */
  end_region   = PAGE2LOCK(page + npages);
  end_region   = LOCK2ADDR(end_region);

  start_region = PAGE2LOCK(page);
  start_region = LOCK2ADDR(start_region);

  lockregion = start_region;

  do
  {
    finfo("INFO: lock region=%d address=0x%x\n",
          ADDR2LOCK(lockregion),
          lockregion);
    ret = nvm_command(NVMCTRL_CTRLA_CMD_LR, lockregion);
    if (ret < 0)
    {
      return ret;
    }

    lockregion += SAMD2_LOCK_SIZE;
  }
  while (lockregion < end_region);

  return ret;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_nvmctrl_initialize
 *
 * Description:
 *    Call to initialize FLASH programming memory access
 *
 * Returned value:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

int sam_nvmctrl_initialize(void)
{
  uint32_t ctrlb;

  /* 0 wait states,
   * Manual write enable,
   * Wakeup-on-access mode,
   * No-miss-penalty mode,
   * Cache is enabled.
   */

  ctrlb = (NVMCTRL_CTRLB_RWS(BOARD_FLASH_WAITSTATES)  |
           NVMCTRL_CTRLB_MANW                         |
           NVMCTRL_CTRLB_SLEEPPRM_WAKEONACCESS        |
           NVMCTRL_CTRLB_READMODE_NO_MISS_PENALTY);

  ctrlb &= ~NVMCTRL_CTRLB_CACHEDIS;

  putreg32(ctrlb, SAM_NVMCTRL_CTRLB);

  return OK;
}

/****************************************************************************
 * Name: sam_nvmctrl_uninitialize
 *
 * Description:
 *    Call to initialize FLASH programming memory access
 *
 * Returned value:
 *    OK - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

int sam_nvmctrl_uninitialize(void)
{
  /* Clear and disable interrupts */
  putreg32(0x3, SAM_NVMCTRL_INTFLAG);
  putreg32(0x3, SAM_NVMCTRL_INTENCLR);

  /* Clear status registers */
  putreg32(0x1E, SAM_NVMCTRL_STATUS);

  /* Put to the registers their reset values */
  putreg32(0x0, SAM_NVMCTRL_CTRLA);
  putreg32(0x80, SAM_NVMCTRL_CTRLB);
  putreg32(0x0, SAM_NVMCTRL_ADDR);

  return OK;
}

/****************************************************************************
 * Name: up_progmem_neraseblocks
 *
 * Description:
 *    Get number of rows in the available FLASH memory.
 *
 * Returned values:
 *    Return number of rows
 *
 ****************************************************************************/

size_t up_progmem_neraseblocks(void)
{
  return (size_t)(SAMD2_CUR_NROWS > SAMD2_FLASH_NROWS ?
                  SAMD2_FLASH_NROWS : SAMD2_CUR_NROWS);
}

/****************************************************************************
 * Name: up_progmem_isuniform
 *
 * Description:
 *   Define the row size is uniform
 *
 * Returned values:
 *    No
 *
 ****************************************************************************/

bool up_progmem_isuniform(void)
{
  return false;
}

/****************************************************************************
 * Name: up_progmem_pagesize
 *
 * Description:
 *    Return page size
 *
 * Arguments:
 *    page - The page number
 *
 * Returned values:
 *    Page size
 *
 ****************************************************************************/

size_t up_progmem_pagesize(size_t page)
{
  return (size_t)(SAMD2_PAGE_SIZE);
}

/****************************************************************************
 * Name: up_progmem_erasesize
 *
 * Description:
 *    Return erased size
 *
 * Arguments:
 *    page - The page number
 *
 * Returned values:
 *    Page size
 *
 ****************************************************************************/

size_t up_progmem_erasesize(size_t page)
{
  return (size_t)(SAMD2_ROW_SIZE);
}

/****************************************************************************
 * Name: up_progmem_getpage
 *
 * Description:
 *    Address to page conversion
 *
 * Arguments:
 *    address - An absolute or relative address
 *
 * Returned values:
 *    Page size - if it is OK
 *    Error code - if something went wrong
 *
 ****************************************************************************/

ssize_t up_progmem_getpage(size_t address)
{
  /* Check if address is absolute or relative */
  if (address >= SAMD2_FIRST_BYTE)
  {
    /* It is absolute. Move to relative */
    address -= SAMD2_FIRST_BYTE;
  }
  /* address is relative */

  if (address >= SAMD2_FLASH_NBYTES)
  {
    return -EFAULT;
  }

  /* We should return a relative page number, not absolute */
  return (ssize_t)ADDR2PAGE(address);
}

/****************************************************************************
 * Name: up_progmem_getaddress
 *
 * Description:
 *    Row to address conversion
 *
 * Arguments:
 *    page - The relative page index
 *
 * Returned values:
 *    Base address of given page, maximum size if page index is not valid.
 *
 ****************************************************************************/

size_t up_progmem_getaddress(size_t page)
{
  if (page >= SAMD2_FLASH_NPAGES)
  {
    return (size_t)(SAMD2_FLASH_NBYTES);
  }

  return (size_t)(PAGE2ADDR(page) + SAMD2_FLASH_START);
}

/****************************************************************************
 * Name: up_progmem_ispageerased
 *
 * Description:
 *    Checks whether row is erased
 *
 * Arguments:
 *    row - The row to be checked
 *
 * Returned values:
 *    Number of bytes erased
 *    Error code - if something went wrong
 *
 ****************************************************************************/

ssize_t up_progmem_ispageerased(size_t row)
{
  size_t address;
  size_t nwritten;
  int nleft;

  finfo("INFO: row=%d\n", row);
  if (row >= SAMD2_FLASH_NROWS)
  {
    return -EFAULT;
  }

  /* Flush and invalidate D-Cache for this address range */
  address = ROW2ADDR(row) + SAMD2_FIRST_BYTE;
  up_flush_dcache(address, address + SAMD2_FIRST_BYTE);

  /* Verify that the row is erased (i.e., all 0xffU) */
  for (nleft = SAMD2_ROW_SIZE, nwritten = 0;
       nleft > 0;
  nleft--, address++)
       {
         if (getreg8(address) != SAMD2_ERASEDVAL)
         {
           nwritten++;
         }
       }

       if (nwritten)
         fwarn("WARN: non written=%d\n", nwritten);

  return nwritten;
}

/****************************************************************************
 * Name: up_progmem_eraseblock
 *
 * Description:
 *    Erase selected row.
 *
 * Arguments:
 *    row - The relative row index to be erased
 *
 * Returned values:
 *    Page size
 *    Error code - if something went wrong
 *
 ****************************************************************************/

ssize_t up_progmem_eraseblock(size_t row)
{
#ifdef USE_LOCK
  uint32_t page;
#endif
  int ret;

  ret = OK;

  finfo("INFO: row=%d\n", row);
  if (row >= SAMD2_FLASH_NROWS)
  {
    return -EFAULT;
  }

#ifdef USE_LOCK
  /* Get the absolute page number */
  page = ROW2PAGE(row) + SAMD2_FIRST_PAGE;
  /* Unlock all pages in this row */
  nvm_unlock(page, SAMD2_PAGES_PER_ROW);
#endif

  finfo("INFO: erase row=%d address=0x%x\n",
        row,
        ROW2ADDR(row));
  ret = nvm_command(NVMCTRL_CTRLA_CMD_ER, ROW2ADDR(row));

#ifdef USE_LOCK
  /* Lock it back */
  nvm_lock(page, SAMD2_PAGES_PER_ROW);
#endif

  if (ret < 0)
  {
    return ret;
  }

  /* Aftercheck that all pages is really erased */
  if (up_progmem_ispageerased(row) == 0)
  {
    return SAMD2_ROW_SIZE;
  }

  /* Failure */
  return -EIO;
}

/****************************************************************************
 * Name: up_progmem_write
 *
 * Description:
 *    Program data at given address
 *
 * Input Parameters:
 *    address  - An address with or without flash offset
 *    buffer   - A pointer to buffer
 *    buflen   - A number of bytes to write
 *
 * Returned Value:
 *    Bytes written
 *    Error code - if something went wrong
 *
 ****************************************************************************/

ssize_t up_progmem_write(size_t address, const void *buffer, size_t buflen)
{
  irqstate_t flags;
  uint32_t *dest;
  const uint32_t *src;
  size_t written;
  size_t xfrsize;
  size_t offset;
  size_t page;
  size_t i;
  int ret;
#ifdef USE_UNLOCK
  size_t lock;
  size_t locksize;
#endif

  finfo("INFO: address=0x%x buflen=%d\n", address, buflen);

  /* Convert the address into a FLASH byte offset, if necessary */
  offset = address;
  if (address >= SAMD2_FIRST_BYTE)
  {
    /* Convert address to an offset relative to be beginning of the
     * writable FLASH region.
     */
    offset -= SAMD2_FIRST_BYTE;
  }

  /* Check for valid address range */
  if ((offset + buflen) > SAMD2_FLASH_NBYTES)
  {
    return -EFAULT;
  }

  /* Get exclusive access to the global page buffer */
  nxmutex_lock(&g_page_lock);

  /* Get the page number corresponding to the flash offset and the byte
   * offset into the page.
   */
  page = ADDR2PAGE((uint32_t)offset) + SAMD2_FIRST_PAGE;
  offset &= SAMD2_PAGE_MASK;

#ifdef USE_UNLOCK
  /* Make sure that the FLASH is unlocked */
  lock = page;
  locksize = ADDR2PAGE(buflen);
  nvm_unlock(lock, locksize);
#endif

  flags = enter_critical_section();

  /* Loop until all of the data has been written */
  dest = (uint32_t *)(address & ~SAMD2_PAGE_MASK);
  written = 0;
  while (buflen > 0)
  {
    /* How much can we write into this page? */
    xfrsize = MIN((size_t)SAMD2_PAGE_SIZE - offset, buflen);

    /* Do we need to use the intermediate buffer? */
    if (offset == 0 && xfrsize == SAMD2_PAGE_SIZE)
    {
      /* No, we can take the data directly from the user buffer */
      src = (const uint32_t *)buffer;
    }
    else
    {
      /* Yes, copy data into global page buffer */
      if (offset > 0)
      {
        memcpy((uint8_t *)g_page_buffer, (uint8_t *)dest, offset);
      }

      memcpy((uint8_t *)g_page_buffer + offset,
             (uint8_t *)buffer, xfrsize);

      if (offset + xfrsize < SAMD2_PAGE_SIZE)
      {
        memcpy((uint8_t *)g_page_buffer + offset + xfrsize,
               (const uint8_t *)dest + offset + xfrsize,
               SAMD2_PAGE_SIZE - offset - xfrsize);
      }

      src = g_page_buffer;
    }

      nvm_command(NVMCTRL_CTRLA_CMD_PBC, 0);

      /* Write the page buffer */
      for (i = 0; i < SAMD2_PAGE_WORDS; i++)
      {
        *dest++ = *src++;
      }

      /* Flush the data cache to memory */
      up_clean_dcache(address & ~SAMD2_PAGE_MASK,
                      (address & ~SAMD2_PAGE_MASK) + SAMD2_PAGE_SIZE);

      /* Send the write command */
      finfo("INFO: WP address=0x%x\n", address & ~SAMD2_PAGE_MASK);
      ret = nvm_command(NVMCTRL_CTRLA_CMD_WP, 0);
      if (ret >= 0)
      {
        written += xfrsize;
      }

      dest -= i;
      src -= i;

      /* Compare page data */
      for (i = 0; i < SAMD2_PAGE_WORDS; i++)
      {
        if (*dest != *src)
        {
          fwarn("offset=0x%x xfrsize=%d buflen=%d STATUS=0x%x\n",
                (unsigned int)offset,
                (unsigned int)xfrsize,
                (unsigned int)buflen,
                (unsigned int)getreg32(SAM_NVMCTRL_STATUS));
        }

        dest++;
        src++;
      }



    /* Adjust pointers and counts for the next time through the loop */
    address += xfrsize;
    dest     = (uint32_t *)address;
    buffer   = (void *)((uintptr_t)buffer + xfrsize);
    buflen  -= xfrsize;
    offset   = 0;
    page++;
  }

#ifdef USE_LOCK
  nvm_lock(lock, locksize);
#endif

  leave_critical_section(flags);
  nxmutex_unlock(&g_page_lock);
  return written;
}

/****************************************************************************
 * Name: up_progmem_erasestate
 *
 * Description:
 *    Check the value of erase state.
 *
 * Returned Value:
 *    The number of erase state
 *
 ****************************************************************************/

uint8_t up_progmem_erasestate(void)
{
  return SAMD2_ERASEDVAL;
}


#endif /* CONFIG_SAMD2L2_NVMCTRL */
