/****************************************************************************
 * apps/industry/apc3/system_2/system_2.h
 ****************************************************************************/

#ifndef __APPS_INDUSTRY_APC3_SYSTEM_2_H
#define __APPS_INDUSTRY_APC3_SYSTEM_2_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include "industry/apc3.h"

#include <arch/board/board.h>

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Power is OUT OF CONTROL error extension */
#define EPWROUT               2001
#define EPWROUT_STR           "Power system is out of control"
#define EUNKNOWN              2002
#define EUNKNOWN_STR          "Unknown code"


/* State of Power Unit presence */
#define SYSTEM_2_NO_PU        (0xFE)
#define SYSTEM_2_BOTH_PU      (0xFF)
#define SYSTEM_2_MAIN_PU      (0x00)
#define SYSTEM_2_ACC_PU       (0x01)


/* Blocks of power definitions */
#define SYSTEM_2_BLOCK_1      (0x1 << 0)
#define SYSTEM_2_BLOCK_2      (0x1 << 1)
#define SYSTEM_2_BLOCK_3      (0x1 << 2)
#define SYSTEM_2_ALL_BLOCKS   (0x7 << 0)


/* Voltage definitions */
/* Block 1 */
#define SYSTEM_2_3V3          (0x1 << 0)
#define SYSTEM_2_1V8          (0x1 << 1)
#define SYSTEM_2_1V5          (0x1 << 2)
#define SYSTEM_2_1V02_0       (0x1 << 3)
#define SYSTEM_2_1V02_1       (0x1 << 4)
#define SYSTEM_2_0V9          (0x1 << 5)
/* Block 2 */
#define SYSTEM_2_1V0_0        (0x1 << 6)
#define SYSTEM_2_1V0_1        (0x1 << 7)
#define SYSTEM_2_3V3_0        (0x1 << 8)
#define SYSTEM_2_3V3_1        (0x1 << 9)
/* Block 3 */
#define SYSTEM_2_5V0          (0x1 << 10)
/* ACC definition */
#define SYSTEM_2_ACC          (0x1 << 11)
/* PU definitions */
#define SYSTEM_2_PU_1         (0x1 << 12)
#define SYSTEM_2_PU_2         (0x1 << 13)

/* Reset definition */
#define SYSTEM_2_SW0          (0x1 << 14)
#define SYSTEM_2_SW1          (0x1 << 15)
#define SYSTEM_2_TCA          (0x1 << 16)

/* Indication definitions */
#define SYSTEM_2_POWER_OK     (0x1 << 0)
#define SYSTEM_2_ALARM        (0x1 << 1)
#define SYSTEM_2_ALL_IND      (0x3 << 0)

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
  #define SYSTEM_2_B1_NOUTS     (6)
  #define SYSTEM_2_RST_NOUTS    (3)
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
  #define SYSTEM_2_B1_NOUTS     (5)
  #define SYSTEM_2_B2_NOUTS     (4)
  #define SYSTEM_2_RST_NOUTS    (2)
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  #define SYSTEM_2_B1_NOUTS     (5)
  #define SYSTEM_2_RST_NOUTS    (2)
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  #define SYSTEM_2_B1_NOUTS     (4)
  #define SYSTEM_2_B2_NOUTS     (2)
  #define SYSTEM_2_RST_NOUTS    (1)
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */


/* Alarm state definitions */
#define SYSTEM_2_NORMAL_STATE       false
#define SYSTEM_2_ALARM_STATE        true


/* ADC units to voltage conversion definition */
#define ADC2MVOLT(adc)         ((((adc) * 1650) / 65536) * 2)

/* The close-file procedure definition */
#define CLOSE(fd)              close(fd); SYS2_leave_critical_section()

/* Returned values of retfl in system_2_start function
 * from main switch-case branch:
 * retfl < 0          - software / hardware error
 * retfl == -EPWROUT  - the worse script situation - power out of control
 * retfl = 0          - there is no errors, but power is not OK:
 *                      0) PU       -;
 *                      1) Block 1  -;
 *                      2) Block 2  -;
 *                      3) Block 3  -;
 * retfl = 1          - there is no errors, but power is not OK:
 *                      0) PU       P;
 *                      1) Block 1  -;
 *                      2) Block 2  -;
 *                      3) Block 3  -;
 * retfl = 2          - there is no errors, but power is not OK:
 *                      0) PU       P;
 *                      1) Block 1  P;
 *                      2) Block 2  -;
 *                      3) Block 3  -;
 * retfl = 3          - there is no errors, but power is not OK:
 *                      0) PU       P;
 *                      1) Block 1  P;
 *                      2) Block 2  P;
 *                      3) Block 3  -;
 * retfl = 4          - there is no errors and power is OK:
 *                      0) PU       P;
 *                      1) Block 1  P;
 *                      2) Block 2  P;
 *                      3) Block 3  P;
 * - Failed,
 * P Passed.
 */

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef struct
{
  int *fd;
  bool *opened;
  void *data_ptr;
} sig_ctx_s2_t;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* From system_2_main.c */
extern const uint8_t block_1_pull[SYSTEM_2_B1_NOUTS];
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
extern const uint8_t block_2_pull[SYSTEM_2_B2_NOUTS];
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */
extern const uint8_t reset_pull[SYSTEM_2_RST_NOUTS];
extern pthread_t alrm_thr_id;
extern pthread_attr_t alrm_thr_attr;
extern volatile bool alarm_st;
extern sigset_t g_wait_mask;

/* From system_2_acc_script.c */
extern volatile bool g_acc_enabled;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: system_2_start
 *
 * Description:
 *   The Arlan PonCat3 Power Manager start function.
 *
 * Arguments:
 *   argc - the volume of argc
 *   argv - any input array
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int system_2_start(int argc, char* argv[]);

/****************************************************************************
 * Name: SYS2_out_of_control
 *
 * Description:
 *   Execute the PANIC algorithm.
 *
 ****************************************************************************/

void SYS2_out_of_control(void);

/****************************************************************************
 * Name: SYS2_under_control
 *
 * Description:
 *   Stop the PANIC algorithm.
 *
 ****************************************************************************/

void SYS2_under_control(void);

/****************************************************************************
 * Name: SYS2_define_PU
 *
 * Description:
 *   The function to define what the current Power Unit in the device
 *
 * Arguments:
 *   script - a pointer to a buffer to write a results
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_define_PU(char* script);

/****************************************************************************
 * Name: SYS2_main_script
 *
 * Description:
 *   The function describes the algorithm that operates when the UPxxxxR PU:
 *   mains power present in the device.
 *
 * Returned values:
 *   Described above
 *
 ****************************************************************************/

int SYS2_main_script(char *bad_pu);

/****************************************************************************
 * Name: SYS2_acc_script
 *
 * Description:
 *   The function describes the algorithm that operates when the BCF-150 PU:
 *   accumulator power present in the device.
 *
 * Returned values:
 *   Described above
 *
 ****************************************************************************/

int SYS2_acc_script(void);

/****************************************************************************
 * Name: SYS2_gpout_hw
 *
 * Description:
 *   Perform hardware gpout driver operations.
 *
 * Arguments:
 *   fd  - the opened file descriptor
 *   bit - the bit position
 *   val - the value to be written
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_gpout_hw(const int fd,
                  const char bit,
                  bool val);

/****************************************************************************
 * Name: SYS2_shutdown
 *
 * Description:
 *   Shut down the selected block
 *
 * Arguments:
 *   block - the mask of selected blocks
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_shutdown(const uint32_t block);

/****************************************************************************
 * Name: SYS2_pwrup
 *
 * Description:
 *   Turn on the specified block
 *
 * Arguments:
 *   rvoltage - the mask of selected voltage and reset
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_pwrup(const uint32_t rvoltage);

/****************************************************************************
 * Name: SYS2_indication
 *
 * Description:
 *   Install the indication
 *
 * Arguments:
 *   mask   - the mask of selected outputs
 *   value  - the output value to be written
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_indication(const char mask,
                    const char value);

/****************************************************************************
 * Name: SYS2_check_block_1
 *
 * Description:
 *   Perform the checking of block 1
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_check_block_1(void);

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
/****************************************************************************
 * Name: SYS2_check_block_2
 *
 * Description:
 *   Perform the checking of block 2
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_check_block_2(void);
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */

/****************************************************************************
 * Name: SYS2_check_block_3
 *
 * Description:
 *   Perform the checking of block 3
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_check_block_3(void);

/****************************************************************************
 * Name: SYS2_rstrq_handler
 *
 * Description:
 *   The signal handler function that performs the requested reset.
 *
 * Arguments:
 *   signo   - the number of received signal
 *   info    - a pointer to a signal info structure with a necessry data
 *   context - the context of this task
 *
 ****************************************************************************/

void SYS2_rstrq_handler(int signo,
                        siginfo_t *info,
                        void *context);

/****************************************************************************
 * Name: SYS2_leave_critical_section
 *
 * Description:
 *   Leave from critical section by enabling the receiving signals.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

void SYS2_leave_critical_section(void);

/****************************************************************************
 * Name: SYS2_enter_critical_section
 *
 * Description:
 *   Protect a critical sections by blocking all signals receiving
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_enter_critical_section(void);

#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
#endif /* __APPS_INDUSTRY_APC3_SYSTEM_2_H */
