/****************************************************************************
 * arch/arm/src/samd2l2/sam_wdt.h
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

#ifndef __ARCH_ARM_SRC_SAMD2L2_SAM_WDT_H
#define __ARCH_ARM_SRC_SAMD2L2_SAM_WDT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "chip.h"
#include "hardware/samd_wdt.h"
#include "hardware/samd_gclk.h"
#include "hardware/samd21_memorymap.h"

#ifdef CONFIG_SAMD2L2_WDT

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#define SAM_WDT_CLEAR_VALUE			0xA5

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: sam_wdt_initialize
 *
 * Description:
 *   Initialize the WDT watchdog timer.  The watchdog timer
 *   is initialized and registers as 'devpath'.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   watchdog_lowerhalf_s* structure
 *
 ****************************************************************************/

struct watchdog_lowerhalf_s* sam_wdt_initialize(void);


#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_SAMD2L2_WDT */
#endif /* __ARCH_ARM_SRC_SAMD2L2_SAM_WDT_H */
