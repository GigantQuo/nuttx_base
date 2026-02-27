/****************************************************************************
 * apps/include/industry/apc3.h
 ****************************************************************************/

#ifndef __APPS_INCLUDE_INDUSTRY_APC3_H
#define __APPS_INCLUDE_INDUSTRY_APC3_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#if defined(CONFIG_INDUSTRY_APC3)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define APC3_NSYSTEMS               5

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  /* Main Power Status command buffer conversion */
  #define MNPWRST_CNV(raw)  (raw[0])
  /* Battery Power Status command buffer conversion */
  #define BSPWRST_CNV(raw)  (raw[1]) & ((1 << (BOARD_GPINP0_AC_OK_ACC-8))  |\
                                        (1 << (BOARD_GPINP0_BAT_LOW_ACC-8)))

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) || \
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  /* Main Power Status command buffer conversion */
  #define MNPWRST_CNV(raw)  (raw[0]) & ((1 << BOARD_GPINP0_PWR_PRE0) |\
                                        (1 << BOARD_GPINP0_PWR_PG0)  |\
                                        (1 << BOARD_GPINP0_PWR_PRE1) |\
                                        (1 << BOARD_GPINP0_PWR_PG1))
  /* Battery Power Status command buffer conversion */
  #define BSPWRST_CNV(raw)  ((raw[0]) & ((1 << BOARD_GPINP0_AC_OK_ACC)  | \
                                         (1 << BOARD_GPINP0_BAT_LOW_ACC)))\
                                         >> BOARD_GPINP0_AC_OK_ACC;
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef struct
{
  pid_t pids[APC3_NSYSTEMS];
  pid_t selfpid;
} task_data_t;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* The Process IDs of the systems */
extern task_data_t *g_system_pids;

/* The global buffers for drivers data */
extern uint8_t g_adc0_data_buffer[BOARD_ADC_NUM_CHANNELS];
extern uint8_t g_gpinp0_data_buffer[2];

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/


#endif /* CONFIG_INDUSTRY_APC3 */
#endif /* __APPS_INCLUDE_INDUSTRY_APC3_H */
