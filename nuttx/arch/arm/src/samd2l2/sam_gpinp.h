/****************************************************************************
 * arch/arm/src/samd2l2/sam_gpinp.h
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

#ifndef __ARCH_ARM_SRC_SAMD2L2_SAM_GPINP_H
#define __ARCH_ARM_SRC_SAMD2L2_SAM_GPINP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <stdint.h>
#  include <stdbool.h>
#endif /* __ASSEMBLY__ */

#include "chip.h"

#if defined(CONFIG_SAMD2L2_GPINP)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__
  #undef EXTERN

  #if defined(__cplusplus)
    #define EXTERN extern "C"
extern "C"
{
  #else
    #define EXTERN extern
  #endif /* __cplusplus */

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
   *    struct gpinp_dev_s - if it is OK
   *    NULL - if something went wrong
   *
   ****************************************************************************/

  struct gpinp_dev_s *sam_gpinp_initialize(const volatile uint32_t *pull,
                                           const uint32_t ngpinp);

/****************************************************************************
 * Name: sam_gpinp_uninitialize
 *
 * Arguments:
 *  const uint32_t *pull - The array contains the values of the gpinputs
 *                         initialization data.
 *
 * Description:
 *  Perform sam_gpinp structure and harware uninitialization.
 *
 ****************************************************************************/

void sam_gpinp_uninitialize(FAR struct gpinp_dev_s *dev);

  #undef EXTERN
  #if defined(__cplusplus)
}
  #endif /* __cplusplus */

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_SAMD2L2_GPINP */
#endif /* __ARCH_ARM_SRC_SAMD2L2_SAM_GPINP_H */
