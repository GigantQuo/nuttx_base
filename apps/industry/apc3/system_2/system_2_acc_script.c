/****************************************************************************
 * apps/industry/apc3/system_2/system_2_acc_script.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <debug.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <arch/board/board.h>

#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>

#include "system_2.h"

#if defined(CONFIG_INDUSTRY_APC3_SYSTEM_2)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) || defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
#define AC_OK_BAT(raw) (raw[1] & (1 << (BOARD_GPINP0_AC_OK_ACC - 8)))
#define BAT_LOW(raw) (raw[1] & (1 << (BOARD_GPINP0_BAT_LOW_ACC - 8)))
#define VALID_BYTE(raw) (raw[1])
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) || defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
#define AC_OK_BAT(raw) (raw[0] & (1 << BOARD_GPINP0_AC_OK_ACC))
#define BAT_LOW(raw) (raw[0] & (1 << BOARD_GPINP0_BAT_LOW_ACC))
#define VALID_BYTE(raw) (raw[0])

#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static inline int SYS2_check_ACC(void);
static inline int SYS2_check_VBAT(void);

/****************************************************************************
 * Public Data
 ****************************************************************************/

volatile bool g_acc_enabled = 0x0;

/****************************************************************************
 * Private Data
 ****************************************************************************/

static volatile char g_acc_pwout_cntr = 0x0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_check_ACC
 *
 * Description:
 *   Check the ACC state from inputs.
 *
 * Returned values:
 *   0          - if it is OK, but no one PU is functional
 *   1          - if it is OK and at least one PU is functional
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS2_check_ACC(void)
{
    char* devpath;
    char raw[(BOARD_NGPINP0 / 8) + 1];
    int ret;
    int fd;
    static char prev_raw = 0xF0;

    ret = OK;
    devpath = "/dev/gpinp0";

    /* Enter the task critical section */
    ret = SYS2_enter_critical_section();
    if (ret < 0) {
        _err("ERROR: SYSTEM_2: Failed to enter critical section: %d\n\r",
            ret);
        return ret;
    }

    fd = open(devpath, O_RDONLY);
    if (fd < 0) {
        ret = fd;
        _err("ERROR: SYSTEM_2: Failed to open /dev/gpinp0: %d\n\r",
            ret);
        SYS2_leave_critical_section();
        return ret;
    }

    ret = read(fd, raw, sizeof(raw));
    if (ret < 0) {
        _err("ERROR: SYSTEM_2: Failed to read /dev/gpinp0: %d\n\r",
            ret);
        CLOSE(fd);
        return ret;
    }

    /* Update the global buffer */
    g_gpinp0_data_buffer[0] = MNPWRST_CNV(raw);
    g_gpinp0_data_buffer[1] = BSPWRST_CNV(raw);

    ret = 1;

    CLOSE(fd);

    if (AC_OK_BAT(raw)) /* AC_OK_BAT */
    {
        if (prev_raw != VALID_BYTE(raw)) {
            _info("ACC AC_OK is gone!");
        }
    } else if (!(BAT_LOW(raw))) /* BAT_LOW */
    {
        if (prev_raw != VALID_BYTE(raw)) {
            _info("ACC battery low!");
        }
    }

    prev_raw = VALID_BYTE(raw);

    return ret;
}

/****************************************************************************
 * Name: SYS2_check_VBAT
 *
 * Description:
 *   Check the ACC 12V voltage.
 *
 * Returned values:
 *   0          - if it is OK, but no one PU is functional
 *   1          - if it is OK and at least one PU is functional
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

static inline int SYS2_check_VBAT(void)
{
    struct adc_msg_s voltage;
    char* devpath;
    char pwout_cntr;
    int ret;
    int fd;
    uint8_t i;

    ret = 0;
    pwout_cntr = 0;
    devpath = "/dev/adc0";

    /* Enter the task critical section */
    ret = SYS2_enter_critical_section();
    if (ret < 0) {
        _err("ERROR: SYSTEM_2: Failed to enter critical section: %d\n\r",
            ret);
        return ret;
    }

    /* We need to check the 12.0V
     * and turn in on if it is OK.
     */

    fd = open(devpath, O_RDONLY);
    if (fd < 0) {
        ret = fd;
        _err("ERROR: SYSTEM_2: Failed to open /dev/adc0: %d\n\r",
            ret);
        SYS2_leave_critical_section();
        return ret;
    }

    for (i = 0; i < (BOARD_ADC_NUM_CHANNELS + 1); i++) {
        /* Check this voltage */
        ret = read(fd, &voltage, sizeof(voltage));
        if (ret <= 0) {
            _err("ERROR: SYSTEM_2: Failed to read /dev/adc0: %d\n\r",
                ret);
            pwout_cntr++;
            break;
        }

        /* First conversion in the group is false - ignore it */
        if ((voltage.am_channel == BOARD_ADC_VBAT_CH) && (i != 0)) {
            if (g_acc_enabled) /* If ACC enabled then apply typical range */
            {
                /* Check +- 10% from 12V0:
                 * There is a input external resistive divider /11
                 * and internal input divider /2, but reference is
                 * VDDANA/2 = 1.65V.
                 * Then 12.0V/22 = 0.55V <=> 1.65V
                 * 12.0V-10% : 18200 (10.8V)
                 * 12.0V     : 20270
                 * 12.0V+10% : 22350 (13.2V)
                 *
                 * NOTE: Empirical values due to
                 * inaccuracy of divider resistors and
                 * additional inaccurate voltage drop
                 * across diodes.
                 */
                if ((voltage.am_data < 15500) || (voltage.am_data > 24500)) {
                    _info("12.0V (ena) voltage out of range (9.6V-14.2V): %d mV",
                        (int)ADC2MVOLT(voltage.am_data + 600) * 11);

                    pwout_cntr++;
                }
            } else /* If ACC disabled then apply more strict range */
            {
                /* Check +- 5% from 12V0:
                 * There is a input external resistive divider /11
                 * and internal input divider /2, but reference is
                 * VDDANA/2 = 1.65V.
                 * Then 12.0V/22 = 0.55V <=> 1.65V
                 * 12.0V-5% : 19960 (11.4V)
                 * 12.0V    : 20270
                 * 12.0V+5% : 21900 (12.6V)
                 *
                 * NOTE: Empirical values due to
                 * inaccuracy of divider resistors and
                 * additional inaccurate voltage drop
                 * across diodes.
                 */
                if ((voltage.am_data < 17000) || (voltage.am_data > 24000)) {
                    _info("12.0V (dis) voltage out of range (9.6V-14.2V): %d mV",
                        (int)ADC2MVOLT(voltage.am_data + 600) * 11);

                    pwout_cntr++;
                }
            }

            /* Update the global buffer */
            g_adc0_data_buffer[BOARD_ADC_VBAT_CH_OFFSET] = (uint8_t)((ADC2MVOLT(voltage.am_data + 600) * 11) / 100);
        }
    }

    CLOSE(fd);

    /* If global power out counter > 2 PWOUT iteration
     * then turn OFF Block,
     * If there is no PWOUT states, then decrement global counter
     */
    if (pwout_cntr == 0) {
        g_acc_pwout_cntr = (g_acc_pwout_cntr > 0) ? (g_acc_pwout_cntr - 1) : 0;
    } else {
        g_acc_pwout_cntr++;
    }

    if (g_acc_pwout_cntr >= 2) {
        ret = 0;
        return ret;
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) || defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
    if (!(g_acc_enabled)) {
        ret = SYS2_pwrup(SYSTEM_2_ACC);
        if (ret < 0) {
            _err("ERROR: SYSTEM_2: Failed to turn on 12.0V ACC: %d\n\r",
                ret);
            return ret;
        }
    }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_S */

    g_acc_enabled = 0x1;

    ret = 1;

    return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: SYS2_acc_script
 *
 * Description:
 *   The function describes the algorithm that operates when the BCF-150 PU:
 *   accumulator power present in the device.
 *
 * Returned values:
 *   OK         - if it is OK
 *   Error code - if sometheing wents wrong
 *
 ****************************************************************************/

int SYS2_acc_script(void)
{
    int ret;

    ret = OK;

    /* Perform the inputs checking */
    ret = SYS2_check_ACC();
    if (ret < 0) {
        _err("ERROR: SYSTEM_2: Failed to check ACC: %d\n\r",
            ret);
        return ret;
    } else if (ret == 1) {
        /* ACC check is passed */

        /* Fall through */
    } else if (ret == 0) {
        /* ACC check is failed */
        return ret;
    } else {
        /* Unknown code */
        ret = -EUNKNOWN;
        return ret;
    }

    /* Perform the VBAT voltage checking */
    ret = SYS2_check_VBAT();
    if (ret < 0) {
        _err("ERROR: SYSTEM_2: Failed to check VBAT: %d\n\r",
            ret);
        return ret;
    } else if (ret == 1) {
        /* ACC check is passed */

        /* Fall through */
    } else if (ret == 0) {
        /* ACC check is failed */
        return ret;
    } else {
        /* Unknown code */
        ret = -EUNKNOWN;
        return ret;
    }

    /* Blocks checking */

    /* Check Block 1 */
    ret = SYS2_check_block_1();
    if (ret < 0) {
        /* Software or hardware error */
        _err("ERROR: SYSTEM_2: Failed to check Block 1: %d\n\r",
            ret);
        return ret;
    } else if (ret == 1) {
        /* Block 1 check is failed */
        return ret;
    } else if (ret == 2) {
        /* Block 1 check is passed */

        /* Fall through */
    } else {
        /* Unknown code */
        ret = -EUNKNOWN;
        return ret;
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) || defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
    /* Check Block 2 */
    ret = SYS2_check_block_2();
    if (ret < 0) {
        /* Software or hardware error */
        _err("ERROR: SYSTEM_2: Failed to check Block 2: %d\n\r",
            ret);
        return ret;
    } else if (ret == 2) {
        /* Block 2 check is failed */
        return ret;
    } else if (ret == 3) {
        /* Block 2 check is passed */

        /* Fall through */
    } else {
        /* Unknown code */
        ret = -EUNKNOWN;
        return ret;
    }
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_--GE_FS */

    /* Check Block 3 */
    ret = SYS2_check_block_3();
    if (ret < 0) {
        /* Software or hardware error */
        _err("ERROR: SYSTEM_2: Failed to check Block 3: %d\n\r",
            ret);
        return ret;
    } else if (ret == 3) {
        /* Block 3 check is failed */
        return ret;
    } else if (ret == 4) {
        /* Block 2 check is passed */

        /* Fall through */
    } else {
        /* Unknown code */
        ret = -EUNKNOWN;
        return ret;
    }

    return ret;
}

#endif /* CONFIG_INDUSTRY_APC3_SYSTEM_2 */
