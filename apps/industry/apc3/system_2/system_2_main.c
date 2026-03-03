/****************************************************************************
 * apps/industry/apc3/system_2/system_2_main.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>

#include <arch/board/board.h>

#include <nuttx/ioexpander/gpout.h>

#include "system_2.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SYSTEM_2_PANIC_CHECK(ret)   \
                            if ((ret) == -EPWROUT) \
                                 SYS2_out_of_control(); \
                            else SYS2_under_control()

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static inline int SYS2_request_init(struct sigaction *act_rstrq,
                                    sig_ctx_s2_t *sig_ctx_rstrq);

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* gpout0 driver */
const uint8_t block_1_pull[SYSTEM_2_B1_NOUTS] =
{
    BOARD_GPOUT0_3V3
  , BOARD_GPOUT0_1V8
  , BOARD_GPOUT0_1V5
  , BOARD_GPOUT0_1V02_0
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
  , BOARD_GPOUT0_1V02_1
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_-S */

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  , BOARD_GPOUT0_0V9
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
};

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
const uint8_t block_2_pull[SYSTEM_2_B2_NOUTS] =
{
  BOARD_GPOUT0_3V3_0,
  BOARD_GPOUT0_3V3_1,
  BOARD_GPOUT0_1V0_0,
  BOARD_GPOUT0_1V0_1
};
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
const uint8_t block_2_pull[SYSTEM_2_B2_NOUTS] =
{
  BOARD_GPOUT0_3V3_0,
  BOARD_GPOUT0_1V0_0
};
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

/* gpout1 driver */
const uint8_t reset_pull[SYSTEM_2_RST_NOUTS] =
{
    BOARD_GPOUT1_SW0
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
  , BOARD_GPOUT1_SW1
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_-S */

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  , BOARD_GPOUT1_TCA
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
};

pthread_t alrm_thr_id;
pthread_attr_t alrm_thr_attr;
volatile bool alarm_st;

sigset_t g_wait_mask;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_request_init
 *
 * Description:
 *   Initalize the signals and its handlers to handle the SIGTTOU signal.
 *      SIGTTOU - is necessary for the requesting task to perform the
 *       desired action (reset the PC3).
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS2_request_init(struct sigaction *act_rstrq,
                                    sig_ctx_s2_t *sig_ctx_rstrq)
{
  int ret;

  ret = OK;

  /* sigaction struct initialization */
  memset(act_rstrq, 0x0, sizeof(struct sigaction));

  /* Initialize the signal reset request handler */
  act_rstrq->sa_sigaction = SYS2_rstrq_handler;
  act_rstrq->sa_flags = SA_SIGINFO;
  act_rstrq->sa_user = (void *)sig_ctx_rstrq;

  /* Initalize the masks */
  ret = sigemptyset(&act_rstrq->sa_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to empty sigset (act_rstrq): %d\n\r",
         ret);
    return ret;
  }
  ret = sigemptyset(&g_wait_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to empty sigset (g_wait_mask): %d\n\r",
         ret);
    return ret;
  }

  /* Block all signals */
  ret = sigfillset(&act_rstrq->sa_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to fill sigset (act_rstrq): %d\n\r",
         ret);
    return ret;
  }
  ret = sigfillset(&g_wait_mask);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to fill sigset (g_wait_mask): %d\n\r",
         ret);
    return ret;
  }

  /* Only for sigprocmask */

  /* Unblock SIGTTOU signal */
  ret = sigdelset(&g_wait_mask,
                  SIGTTOU);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to delete signal (SIGTTOU): %d\n\r",
         ret);
    return ret;
  }
  /* Unblock SIGTTIN signal */
  ret = sigdelset(&g_wait_mask,
                  SIGTTIN);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to delete signal (SIGTTIN): %d\n\r",
         ret);
    return ret;
  }

  /* Now the g_wait_mask contents all signals
   * except SIGTTOU (reset request)
   * and SIGTTIN (info request)
   */

  /* Apply the mask on this task */
  ret = sigprocmask(SIG_SETMASK,
                    &g_wait_mask,
                    NULL);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to apply sigset (g_wait_mask): %d\n\r",
         ret);
    return ret;
  }

  /* Attach the signal handler to reset request */
  ret = sigaction(SIGTTOU,
                  act_rstrq,
                  NULL);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to set up sigaction (SIGTTOU): %d\n\r",
         ret);
    return ret;
  }

  /* If there is SIGTTOU or SIGTTIN
   * signals on this task then
   * the corresponding handler will be
   * executed by interrupting the main
   * task
   */

  return ret;
}

/****************************************************************************
 * Public Functions
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

int system_2_start(int argc, char* argv[])
{
  int ret;
  char script;
  char prev_scr;
  char indication;
  char indication_mask;
  uint16_t power;
  char *blocks_message;
  int retfl;
  char prev_retfl;

  struct sigaction act_rstrq;
  sig_ctx_s2_t sig_ctx_rstrq;

  _info("SYSTEM_2 Started successfully!\n\r");

  ret = OK;

  /* Needed for NO SPAM */
  script      = SYSTEM_2_NO_PU;
  prev_scr    = 0x10;
  retfl       = 0x00;
  prev_retfl  = 0x10;

  /* Initialization of global ALARM stare */
  alarm_st = SYSTEM_2_NORMAL_STATE;

  /* Initialization of alarm thread attributes and id */
  pthread_attr_init(&alrm_thr_attr);
  alrm_thr_id = 0;

  /* Reset request handler initialization */
  ret = SYS2_request_init(&act_rstrq,
                          &sig_ctx_rstrq);
  if (ret < 0)
  {
    _err("ERROR: SYSTEM_2: Failed to initialize reset request: %d\n\r",
         ret);
  }

  while (1)
  {
     /* Reset the indication and power every interation */
    indication_mask = 0x0;
    indication = 0x0;
    power = 0x0;

    /* First we need to define
     * the current Power Unit in the device.
     * It can be:
     * UPxx01R or BCF-150S12  - mains power,
     * BCF-150                - accumulator power.
     *
     * Then script can be:
     * -2 - No any PU in the device,
     * -1 - Both PU are present in the device,
     * 0  - UPxx01R or BCF-150S12 PU is present,
     * 1  - BCF-159 PU is present.
     */

    ret = SYS2_define_PU(&script);
    if (ret < 0)
    {
      _err("ERROR: SYSTEM_2: Failed to define PU: %d\n\r",
           ret);
    }

    /* Logic to execute suitable script depending on
     * script value.
     * Indication initial values:
     * POWER OK - OFF,
     * ALARM    - OFF
     * Power initial values:
     * PU 1     - OFF,
     * PU 2     - OFF,
     * Block 1  - OFF,
     * Block 2  - OFF,
     * Block 3  - OFF
     */
    switch (script)
    {
      /* No valid PU in the device - call PANIC */
      case (SYSTEM_2_NO_PU):
        if (script != prev_scr)
        {
           printf("\n\rPANIC!: Illegal situation\n\r");
           printf("There is no Power Unit in the device!\n\r");
        }
        /* Indication:
         * POWER OK - OFF,
         * ALARM    - ON
         */
        indication_mask |= SYSTEM_2_ALL_IND;
        indication |= SYSTEM_2_ALARM;

        /* Power:
         * PU 1     - OFF,
         * PU 2     - OFF,
         * ACC      - OFF,
         * Block 1  - OFF,
         * Block 2  - OFF,
         * Block 3  - OFF
         */
        power |=   SYSTEM_2_ALL_BLOCKS
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                 | SYSTEM_2_PU_1
                 | SYSTEM_2_PU_2
                 | SYSTEM_2_ACC
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                 ;
        break;

      /* There is both PU in the device - call PANIC */
      case (SYSTEM_2_BOTH_PU):
        if (script != prev_scr)
        {
          printf("\n\rPANIC!: Illegal situation\n\r");
          printf("There is both Power Unit in the device!\n\r");
        }
        /* Indication:
         * POWER OK - OFF,
         * ALARM    - ON
         */
        indication_mask |= SYSTEM_2_ALL_IND;
        indication |= SYSTEM_2_ALARM;

        /* Power:
         * PU 1     - OFF,
         * PU 2     - OFF,
         * ACC      - OFF,
         * Block 1  - OFF,
         * Block 2  - OFF,
         * Block 3  - OFF
         */
        power |=   SYSTEM_2_ALL_BLOCKS
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                 | SYSTEM_2_PU_1
                 | SYSTEM_2_PU_2
                 | SYSTEM_2_ACC
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                 ;
        break;

      /* Only Mains PU in the device - call MAIN script */
      case (SYSTEM_2_MAIN_PU):
        char *pu_message;
        char bad_pu;
        char prev_bad_pu;

        blocks_message = "\n\rSYSTEM ERROR\r\n";
        pu_message = "\n\rAll PU is broken!\n\r";

        if (script != prev_scr)
        {
          printf("\n\rMains power script running!\n\r");

          /* Reset to new message */
          retfl       = 0x00;
          prev_retfl  = 0x10;

          bad_pu      = 0x03;
          prev_bad_pu = 0x10;
        }

        /* Execute the Mains Power algorithm */
        retfl = SYS2_main_script(&bad_pu);
        if (retfl < 0)
        {
          _err("ERROR: SYSTEM_2: mains power script failed\n\r");
          SYSTEM_2_PANIC_CHECK(retfl);
          continue;
        }

        /* Handle the result */
        switch (retfl)
        {
          /* All blocks and PU is BAD */
          case (0):
            case_1: /* Enter point */

            blocks_message = "\n\rFAILED:\n\rPU - BAD\n\rB1 - BAD\
\n\rB2 - BAD\n\rB3 - BAD\n\n\r";

            /* Indication:
             * POWER OK - OFF,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_ALARM;

            /* Power:
             * PU 1     - OFF,
             * PU 2     - OFF,
             * ACC      - OFF,
             * Block 1  - OFF,
             * Block 2  - OFF,
             * Block 3  - OFF
             */
            power |=   SYSTEM_2_ALL_BLOCKS
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                 | SYSTEM_2_PU_1
                 | SYSTEM_2_PU_2
                 | SYSTEM_2_ACC
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                 ;
            break;

          /* All blocks is BAD, but PU is OK */
          case (1):
            blocks_message = "\n\rFAILED:\n\rPU - OK\n\rB1 - BAD\
\n\rB2 - BAD\n\rB3 - BAD\n\n\r";

            /* Indication:
             * POWER OK - OFF,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_ALARM;

            /* Power:
             * PU 1     - no change,
             * PU 2     - no change,
             * ACC      - OFF,
             * Block 1  - OFF,
             * Block 2  - OFF,
             * Block 3  - OFF
             */
            power |=   SYSTEM_2_ALL_BLOCKS
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                     | SYSTEM_2_ACC
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                     ;
            break;

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
          /* Blocks 2-3 is BAD, but PU and Block 1 is OK */
          case (2):
            blocks_message = "\n\rFAILED:\n\rPU - OK\n\rB1 - OK\
\n\rB2 - BAD\n\rB3 - BAD\n\n\r";

            /* Indication:
             * POWER OK - OFF,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_ALARM;

            /* Power:
             * PU 1     - no change,
             * PU 2     - no change,
             * ACC      - OFF,
             * Block 1  - OFF,
             * Block 2  - OFF,
             * Block 3  - OFF
             */
            power |= SYSTEM_2_ALL_BLOCKS;
            break;
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */

          /* Block 3 is BAD, but PU and Blocks 1-2 is OK */
          case (3):
            blocks_message = "\n\rFAILED:\n\rPU - OK\n\rB1 - OK\
\n\rB2 - OK\n\rB3 - BAD\n\n\r";

            /* Indication:
             * POWER OK - OFF,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_ALARM;

            /* Power:
             * PU 1     - no change,
             * PU 2     - no change,
             * ACC      - OFF,
             * Block 1  - no change,
             * Block 2  - no change,
             * Block 3  - OFF
             */
            power |=   SYSTEM_2_BLOCK_3
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                     | SYSTEM_2_ACC
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                     ;
            break;

          /* All blocks and PU is OK */
          case (4):
            blocks_message = "\n\rPASSED:\n\rPU - OK\n\rB1 - OK\
\n\rB2 - OK\n\rB3 - OK\n\n\r";

            /* Indication:
             * POWER OK - ON,
             * ALARM    - no change
             */
            indication_mask |= SYSTEM_2_POWER_OK;
            indication |= SYSTEM_2_POWER_OK;

            /* Power:
             * PU 1     - no change,
             * PU 2     - no change,
             * ACC      - OFF,
             * Block 1  - no change,
             * Block 2  - no change,
             * Block 3  - no change
             */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
            power |= SYSTEM_2_ACC;
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
            break;

          /* Unknown code */
          default:
            /* Like case (1) */
            goto case_1;
        }

        /* Handle the PU state */
        switch (bad_pu & 0x3)
        {
          case (0):
            /* "\n\r All PU is broken!\n\r" */

            /* Indication:
             * No change, becouse
             * of it is already set properly
             * by previous switch-case
             */
            break;


          case (1):
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
            pu_message = "\n\rRight PU is broken!\n\r";
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
            pu_message = "\n\rLeft PU is broken!\n\r";
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

            /* Power:
             * PU 1     - no change,
             * PU 2     - OFF,
             * ACC      - no change,
             * Block 1  - no change,
             * Block 2  - no change,
             * Block 3  - no change
             */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
            power |= SYSTEM_2_PU_2;
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

            /* Indication:
             * POWER OK - no change,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALARM;
            indication |= SYSTEM_2_ALARM;
            break;

          case (2):
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
            pu_message = "\n\rLeft PU is broken!\n\r";
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) ||\
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
            pu_message = "\n\rRight PU is broken!\n\r";
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

            /* Wait until the PU 2 voltage become stable */
            sleep(2);

            /* Power:
             * PU 1     - OFF,
             * PU 2     - no change,
             * ACC      - no change,
             * Block 1  - no change,
             * Block 2  - no change,
             * Block 3  - no change
             */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
            power |= SYSTEM_2_PU_1;
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

            /* Indication:
             * POWER OK - no change,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALARM;
            indication |= SYSTEM_2_ALARM;
            break;

          case (3):
            pu_message = "\n\rAll PU is functional!\n\r";

/* Not used now: reserved algorithm */
#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2_RESERVED)
            /* Wait until the PU 1 voltage become stable */
            sleep(2);

            /* Power:
             * PU 1     - no change,
             * PU 2     - OFF,
             * ACC      - no change,
             * Block 1  - no change,
             * Block 2  - no change,
             * Block 3  - no change
             */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
            power |= SYSTEM_2_PU_2;
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2_RESERVED */

            /* Indication:
             * POWER OK - ON,
             * ALARM    - OFF
             */

            if ((power & SYSTEM_2_ALL_BLOCKS) != 0)
            {
              indication_mask |= 0x0;
            }
            else
            {
              indication_mask |= SYSTEM_2_ALL_IND;
            }
            indication |= SYSTEM_2_POWER_OK;

            break;

          default:
            /* "\n\r All PU is broken!\n\r" */

            /* Shutdown all power due to unkown code */
            goto case_0;
        }

        if (bad_pu != prev_bad_pu)
        {
          printf(pu_message);
          prev_bad_pu = bad_pu;
        }

        if (retfl != prev_retfl)
        {
          printf(blocks_message);
          prev_retfl = retfl;
        }

        break;

      /* Only Accumulator PU in the device - call ACC script */
      case (SYSTEM_2_ACC_PU):
        if (script != prev_scr)
        {
          printf("\n\rAccumulator power script running!\n\r");
        }

        retfl = SYS2_acc_script();
        if (ret < 0)
        {
          _err("ERROR: SYSTEM_2: accumulator power script failed\n\r");
          SYSTEM_2_PANIC_CHECK(retfl);
          continue;
        }

        /* Handle the result */
        switch (retfl)
        {
          /* All blocks and ACC is BAD */
          case (0):
            case_0: /* Enter point */

            blocks_message = "\n\rFAILED:\n\rACC - BAD\n\rB1 - BAD\
\n\rB2 - BAD\n\rB3 - BAD\n\n\r";

            /* Indication:
             * POWER OK - OFF,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_ALARM;

            /* Power:
             * PU 1     - OFF,
             * PU 2     - OFF,
             * ACC      - OFF,
             * Block 1  - OFF,
             * Block 2  - OFF,
             * Block 3  - OFF
             */
            power |=   SYSTEM_2_ALL_BLOCKS
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                     | SYSTEM_2_PU_1
                     | SYSTEM_2_PU_2
                     | SYSTEM_2_ACC
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                     ;
          break;

          /* All blocks is BAD, but ACC is OK */
          case (1):
            blocks_message = "\n\rFAILED:\n\rACC - OK\n\rB1 - BAD\
\n\rB2 - BAD\n\rB3 - BAD\n\n\r";

            /* Indication:
             * POWER OK - OFF,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_ALARM;

            /* Power:
             * PU 1     - OFF,
             * PU 2     - OFF,
             * ACC      - no change,
             * Block 1  - OFF,
             * Block 2  - OFF,
             * Block 3  - OFF
             */
            power |=   SYSTEM_2_ALL_BLOCKS
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                     | SYSTEM_2_PU_1
                     | SYSTEM_2_PU_2
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                     ;
            break;

            /* Blocks 2-3 is BAD, but ACC and Block 1 is OK */
          case (2):
            blocks_message = "\n\rFAILED:\n\rACC - OK\n\rB1 - OK\
\n\rB2 - BAD\n\rB3 - BAD\n\n\r";

            /* Indication:
             * POWER OK - OFF,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_ALARM;

            /* Power:
             * PU 1     - OFF,
             * PU 2     - OFF,
             * ACC      - no change,
             * Block 1  - OFF,
             * Block 2  - OFF,
             * Block 3  - OFF
             */
            power |=   SYSTEM_2_ALL_BLOCKS
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                     | SYSTEM_2_PU_1
                     | SYSTEM_2_PU_2
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                     ;
            break;

            /* Block 3 is BAD, but ACC and Blocks 1-2 is OK */
          case (3):
            blocks_message = "\n\rFAILED:\n\rACC - OK\n\rB1 - OK\
\n\rB2 - OK\n\rB3 - BAD\n\n\r";

            /* Indication:
             * POWER OK - OFF,
             * ALARM    - ON
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_ALARM;

            /* Power:
             * PU 1     - OFF,
             * PU 2     - OFF,
             * ACC      - no change,
             * Block 1  - no change,
             * Block 2  - no change,
             * Block 3  - OFF
             */
            power |=   SYSTEM_2_BLOCK_3
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                     | SYSTEM_2_PU_1
                     | SYSTEM_2_PU_2
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                     ;
            break;

          /* All blocks and ACC is OK */
          case (4):
            blocks_message = "\n\rPASSED:\n\rACC - OK\n\rB1 - OK\
\n\rB2 - OK\n\rB3 - OK\n\n\r";

            /* Indication:
             * POWER OK - ON,
             * ALARM    - no change
             */
            indication_mask |= SYSTEM_2_ALL_IND;
            indication |= SYSTEM_2_POWER_OK;

            /* Power:
             * PU 1     - OFF,
             * PU 2     - OFF,
             * ACC      - no change,
             * Block 1  - no change,
             * Block 2  - no change,
             * Block 3  - no change
             */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
            power |= SYSTEM_2_PU_1    |
                     SYSTEM_2_PU_2;
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
            break;

          /* Unknown code */
          default:
            /* Like case (0) */
            goto case_0;
        }

        if (retfl != prev_retfl)
        {
          printf(blocks_message);
          prev_retfl = retfl;
        }

        break;

      /* Unknown code due to a hardware error - call PANIC */
      default:
        if (script != prev_scr)
        {
          printf("\n\rPANIC!: Unknown situation\n\r");
        }

        /* Indication:
         * POWER OK - OFF,
         * ALARM    - ON
         */
        indication_mask |= SYSTEM_2_ALL_IND;
        indication |= SYSTEM_2_ALARM;

        /* Power:
         * PU 1     - OFF,
         * PU 2     - OFF,
         * ACC      - OFF,
         * Block 1  - OFF,
         * Block 2  - OFF,
         * Block 3  - OFF
         */
        power |=   SYSTEM_2_ALL_BLOCKS
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
                 | SYSTEM_2_PU_1
                 | SYSTEM_2_PU_2
                 | SYSTEM_2_ACC
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */
                 ;
        break;
    }

    prev_scr = script;

    /* Apply indication changing */
    ret = SYS2_indication(indication_mask,
                          indication);
    SYSTEM_2_PANIC_CHECK(ret);

    /* Apply power changing */
    ret = SYS2_shutdown(power);
    SYSTEM_2_PANIC_CHECK(ret);

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
    if (power & SYSTEM_2_ACC)
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) || \
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
    if ((power & SYSTEM_2_ALL_BLOCKS) == SYSTEM_2_ALL_BLOCKS)
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */
    {
      g_acc_enabled = 0x0;
    }

    /* Check once in 0.2 sec */
    usleep(400000);
  }

  return ret;
}


#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
