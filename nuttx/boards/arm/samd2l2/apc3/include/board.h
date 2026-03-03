/****************************************************************************
 * boards/arm/samd2l2/apc3/include/board.h
 ****************************************************************************/

#ifndef __BOARDS_ARM_SAMD2L2_SAMD20_APC3_BOARD_H
#define __BOARDS_ARM_SAMD2L2_SAMD20_APC3_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Clocking *****************************************************************/

/* Overview
 *
 * OSC8M         Output = 8MHz
 *  `- GCLK1     Input  = 8MHz   Prescaler    = 1   output         = 8MHz
 *    `- DFLL    Input  = 8MHz   Multiplier   = 6   output         = 48MHz
 *      `- GCLK0 Input  = 48MHz  Prescaler    = 1   output         = 48MHz
 *         `- PM Input  = 48Mhz  CPU divider  = 1   CPU frequency  = 48MHz
 *                               APBA divider = 1   APBA frequency = 48MHz
 *                               APBB divider = 1   APBB frequency = 48MHz
 *                               APBC divider = 1   APBC frequency = 48MHz
 *
 *  OSCULP32K Output = 32.768kHz
 *   `-GCLK2  Input  = 32.768kHz Prescaler    = 32  output         = 1024Hz
 *     `-WDT  Input  = 1024Hz
 */

/* OSC32 Configuration -- not used
 *
 *   BOARD_OSC32K_ENABLE        - Boolean (defined / not defined)
 *   BOARD_OSC32K_FREQUENCY     - In Hz
 *   BOARD_OSC32K_STARTUPTIME   - See SYSCTRL_OSC32K_STARTUP_* definitions
 *   BOARD_OSC32K_EN1KHZ        - Boolean (defined / not defined)
 *   BOARD_OSC32K_EN32KHZ       - Boolean (defined / not defined)
 *   BOARD_OSC32K_ONDEMAND      - Boolean (defined / not defined)
 *   BOARD_OSC32K_RUNINSTANDBY  - Boolean (defined / not defined)
 */

#undef  BOARD_OSC32K_ENABLE
#define BOARD_OSC32K_FREQUENCY        32768
#define BOARD_OSC32K_STARTUPTIME      SYSCTRL_OSC32K_STARTUP_4MS
#define BOARD_OSC32K_EN1KHZ           1
#define BOARD_OSC32K_EN32KHZ          1
#define BOARD_OSC32K_ONDEMAND         1
#undef  BOARD_OSC32K_RUNINSTANDBY

/* OSC8M Configuration -- always enabled
 *
 *   BOARD_OSC8M_PRESCALER      - See SYSCTRL_OSC8M_PRESC_DIV* definitions
 *   BOARD_OSC8M_ONDEMAND       - Boolean (defined / not defined)
 *   BOARD_OSC8M_RUNINSTANDBY   - Boolean (defined / not defined)
 */

#define BOARD_OSC8M_PRESCALER         SYSCTRL_OSC8M_PRESC_DIV1
#define BOARD_OSC8M_ONDEMAND          1
#undef  BOARD_OSC8M_RUNINSTANDBY

#define BOARD_OSC8M_FREQUENCY         8000000

/* OSCULP32K Configuration -- not used. */

#define BOARD_OSCULP32K_FREQUENCY     32768

/* Digital Frequency Locked Loop configuration.  In closed-loop mode, the
 * DFLL output frequency (Fdfll) is given by:
 *
 *  Fdfll = DFLLmul * Frefclk
 *        = 6 * 8000000 = 48MHz
 *
 * Where the reference clock is Generic Clock Channel 0 output of GLCK1.
 * GCLCK1 provides OSC8M, undivided.
 *
 * When operating in open-loop mode, the output frequency of the DFLL will
 * be determined by the values written to the DFLL Coarse Value bit group
 * and the DFLL Fine Value bit group in the DFLL Value register.
 *
 *   BOARD_DFLL_OPENLOOP            - Boolean (defined / not defined)
 *   BOARD_DFLL_TRACKAFTERFINELOCK  - Boolean (defined / not defined)
 *   BOARD_DFLL_KEEPLOCKONWAKEUP    - Boolean (defined / not defined)
 *   BOARD_DFLL_ENABLECHILLCYCLE    - Boolean (defined / not defined)
 *   BOARD_DFLL_QUICKLOCK           - Boolean (defined / not defined)
 *   BOARD_DFLL_ONDEMAND            - Boolean (defined / not defined)
 *
 * Closed loop mode only:
 *   BOARD_DFLL_GCLKGEN             - GCLK index
 *   BOARD_DFLL_MULTIPLIER          - Value
 *   BOARD_DFLL_MAXCOARSESTEP       - Value
 *   BOARD_DFLL_MAXFINESTEP         - Value
 *
 *   BOARD_DFLL_FREQUENCY           - The resulting frequency
 */

#define BOARD_DFLL_ENABLE             1
#define BOARD_DFLL_OPENLOOP           1
#undef  BOARD_DFLL_ONDEMAND
#undef  BOARD_DFLL_RUNINSTANDBY

/* DFLL closed loop mode configuration */

#define BOARD_DFLL_SRCGCLKGEN         1
#define BOARD_DFLL_MULTIPLIER         6
#define BOARD_DFLL_QUICKLOCK          1
#define BOARD_DFLL_TRACKAFTERFINELOCK 1
#define BOARD_DFLL_KEEPLOCKONWAKEUP   1
#define BOARD_DFLL_ENABLECHILLCYCLE   1
#define BOARD_DFLL_MAXCOARSESTEP     (0x1f / 4)
#define BOARD_DFLL_MAXFINESTEP       (0xff / 4)

#define BOARD_DFLL_FREQUENCY         (48000000)

/* GCLK definitions *********************************************************/

/* GCLK Configuration
 *
 * Global enable/disable.
 *
 *   BOARD_GCLK_ENABLE            - Boolean (defined / not defined)
 *
 * For n=1-7:
 *   BOARD_GCLKn_ENABLE           - Boolean (defined / not defined)
 *
 * For n=0-8:
 *   BOARD_GCLKn_RUN_IN_STANDBY   - Boolean (defined / not defined)
 *   BOARD_GCLKn_CLOCK_SOURCE     - See GCLK_GENCTRL_SRC_* definitions
 *   BOARD_GCLKn_PRESCALER        - Value
 *   BOARD_GCLKn_OUTPUT_ENABLE    - Boolean (defined / not defined)
 */

#define BOARD_GCLK_ENABLE             1

/* GCLK generator 0 (Main Clock) - Source is the DFLL */

#undef  BOARD_GCLK0_RUN_IN_STANDBY
#define BOARD_GCLK0_CLOCK_SOURCE      GCLK_GENCTRL_SRC_DFLL48M
#define BOARD_GCLK0_PRESCALER         1
#undef  BOARD_GCLK0_OUTPUT_ENABLE
#define BOARD_GCLK0_FREQUENCY        (BOARD_DFLL_FREQUENCY / \
                                      BOARD_GCLK0_PRESCALER)

/* Configure GCLK generator 1 - Drives the DFLL */

#define BOARD_GCLK1_ENABLE            1
#undef  BOARD_GCLK1_RUN_IN_STANDBY
#define BOARD_GCLK1_CLOCK_SOURCE      GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK1_PRESCALER         1
#undef  BOARD_GCLK1_OUTPUT_ENABLE
#define BOARD_GCLK1_FREQUENCY        (BOARD_OSC8M_FREQUENCY / \
                                      BOARD_GCLK1_PRESCALER)

/* Configure GCLK generator 2 (RTC) */

#define  BOARD_GCLK2_ENABLE            1
#undef  BOARD_GCLK2_RUN_IN_STANDBY
#define BOARD_GCLK2_CLOCK_SOURCE      GCLK_GENCTRL_SRC_OSCULP32K
#define BOARD_GCLK2_PRESCALER         31
#undef  BOARD_GCLK2_OUTPUT_ENABLE
#define BOARD_GCLK2_FREQUENCY        (BOARD_OSCULP32K_FREQUENCY / \
                                      BOARD_GCLK2_PRESCALER)

/* Configure GCLK generator 3 */

#undef  BOARD_GCLK3_ENABLE
#undef  BOARD_GCLK3_RUN_IN_STANDBY
#define BOARD_GCLK3_CLOCK_SOURCE      GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK3_PRESCALER         1
#undef  BOARD_GCLK3_OUTPUT_ENABLE
#define BOARD_GCLK3_FREQUENCY        (BOARD_OSC8M_FREQUENCY / \
                                      BOARD_GCLK3_PRESCALER)

/* Configure GCLK generator 4 */

#undef  BOARD_GCLK4_ENABLE
#undef  BOARD_GCLK4_RUN_IN_STANDBY
#define BOARD_GCLK4_CLOCK_SOURCE      GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK4_PRESCALER         1
#undef  BOARD_GCLK4_OUTPUT_ENABLE
#define BOARD_GCLK4_FREQUENCY        (BOARD_OSC8M_FREQUENCY / \
                                      BOARD_GCLK4_PRESCALER)

/* Configure GCLK generator 5 */

#undef  BOARD_GCLK5_ENABLE
#undef  BOARD_GCLK5_RUN_IN_STANDBY
#define BOARD_GCLK5_CLOCK_SOURCE      GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK5_PRESCALER         1
#undef  BOARD_GCLK5_OUTPUT_ENABLE
#define BOARD_GCLK5_FREQUENCY        (BOARD_OSC8M_FREQUENCY / \
                                      BOARD_GCLK5_PRESCALER)

/* Configure GCLK generator 6 */

#undef  BOARD_GCLK6_ENABLE
#undef  BOARD_GCLK6_RUN_IN_STANDBY
#define BOARD_GCLK6_CLOCK_SOURCE      GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK6_PRESCALER         1
#undef  BOARD_GCLK6_OUTPUT_ENABLE
#define BOARD_GCLK6_FREQUENCY        (BOARD_OSC8M_FREQUENCY / \
                                      BOARD_GCLK6_PRESCALER)

/* Configure GCLK generator 7 */

#undef  BOARD_GCLK7_ENABLE
#undef  BOARD_GCLK7_RUN_IN_STANDBY
#define BOARD_GCLK7_CLOCK_SOURCE      GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK7_PRESCALER         1
#undef  BOARD_GCLK7_OUTPUT_ENABLE
#define BOARD_GCLK7_FREQUENCY        (BOARD_OSC8M_FREQUENCY / \
                                      BOARD_GCLK7_PRESCALER)

/* The source of the main clock is always GCLK_MAIN.
 * Also called GCLKGEN[0], this is the clock feeding the Power Manager.
 * The Power Manager, in turn, generates main clock which is divided
 * down to produce the CPU, AHB, and APB clocks.
 *
 * The main clock is initially OSC8M divided by 8.
 */

#define BOARD_GCLK_MAIN_FREQUENCY     BOARD_GCLK0_FREQUENCY

/* Main clock dividers
 *
 *    BOARD_CPU_DIVIDER    - See PM_CPUSEL_CPUDIV_* definitions
 *    BOARD_CPU_FREQUENCY  - In Hz
 *    BOARD_CPU_FAILDECT   - Boolean (defined / not defined)
 *    BOARD_APBA_DIVIDER   - See M_APBASEL_APBADIV_* definitions
 *    BOARD_APBA_FREQUENCY - In Hz
 *    BOARD_APBB_DIVIDER   - See M_APBBSEL_APBBDIV_* definitions
 *    BOARD_APBB_FREQUENCY - In Hz
 *    BOARD_APBC_DIVIDER   - See M_APBCSEL_APBCDIV_* definitions
 *    BOARD_APBC_FREQUENCY - In Hz
 */

#define BOARD_CPU_FAILDECT            1
#define BOARD_CPU_DIVIDER             PM_CPUSEL_CPUDIV_1
#define BOARD_APBA_DIVIDER            PM_APBASEL_APBADIV_1
#define BOARD_APBB_DIVIDER            PM_APBBSEL_APBBDIV_1
#define BOARD_APBC_DIVIDER            PM_APBCSEL_APBCDIV_1

/* Resulting frequencies */

#define BOARD_MCK_FREQUENCY          (BOARD_GCLK_MAIN_FREQUENCY)
#define BOARD_CPU_FREQUENCY          (BOARD_MCK_FREQUENCY)
#define BOARD_PBA_FREQUENCY          (BOARD_MCK_FREQUENCY)
#define BOARD_PBB_FREQUENCY          (BOARD_MCK_FREQUENCY)
#define BOARD_PBC_FREQUENCY          (BOARD_MCK_FREQUENCY)
#define BOARD_PBD_FREQUENCY          (BOARD_MCK_FREQUENCY)

/* FLASH definitions ********************************************************/

/* FLASH wait states
 *
 * Vdd Range     Wait states    Maximum Operating Frequency
 * ------------- -------------- ---------------------------
 * 1.62V to 2.7V  0             14 MHz
 *                1             28 MHz
 *                2             42 MHz
 *                3             48 MHz
 * 2.7V to 3.63V  0             24 MHz
 *                1             48 MHz
 */

#if 0 /* REVISIT -- should not be necessary */
#  define BOARD_FLASH_WAITSTATES      1
#else
#  define BOARD_FLASH_WAITSTATES      2
#endif

/* WDT definitions **********************************************************/

#define BOARD_WDT_GCLKGEN             2
#define BOARD_WDT_FREQUENCY           BOARD_GCLK2_FREQUENCY

/* EIC definitions **********************************************************/

#define BOARD_GCLK_EIC                1         /* EIC GCLK index */

/* SERCOM definitions *******************************************************/

/***************************** SLAVE subsystem ******************************/

/* This is the source clock generator for the GCLK_SERCOM_SLOW clock that
 * is common to all SERCOM modules.
 */

#define BOARD_SERCOM05_SLOW_GCLKGEN   0

/* SERCOM1 I2C_slave
 *
 *  PIN PORT SERCOM        FUNCTION
 *  --- ------------------ -----------
 *  17  PA8  SERCOM0 PAD0  I2C SDA
 *  18  PA9  SERCOM0 PAD1  I2C SCL
 */

#define BOARD_SERCOM1_GCLKGEN         0
#define BOARD_SERCOM1_SLOW_GCLKGEN    BOARD_SERCOM05_SLOW_GCLKGEN
#define BOARD_SERCOM1_MUXCONFIG       0
#define BOARD_SERCOM1_PINMAP_PAD0     PORT_SERCOM1_PAD0_1 /* SDA */
#define BOARD_SERCOM1_PINMAP_PAD1     PORT_SERCOM1_PAD1_1 /* SCL */
#define BOARD_SERCOM1_PINMAP_PAD2     0
#define BOARD_SERCOM1_PINMAP_PAD3     0

/***************************** DEBUG subsystem ******************************/

#define BOARD_SERCOM1_FREQUENCY      BOARD_GCLK0_FREQUENCY

/*  SERCOM2 debug UART
 *
 *   PA12 SERCOM2 / USART TXD
 *   PA13 SERCOM2 / USART RXD
 */

#define BOARD_SERCOM2_GCLKGEN         0
#define BOARD_SERCOM2_SLOW_GCLKGEN    BOARD_SERCOM05_SLOW_GCLKGEN
#define BOARD_SERCOM2_MUXCONFIG      (USART_CTRLA_RXPAD1 | \
                                      USART_CTRLA_TXPAD0)
#define BOARD_SERCOM2_PINMAP_PAD0     PORT_SERCOM2_PAD0_1 /* USART TX */
#define BOARD_SERCOM2_PINMAP_PAD1     PORT_SERCOM2_PAD1_1 /* USART RX */
#define BOARD_SERCOM2_PINMAP_PAD2     0
#define BOARD_SERCOM2_PINMAP_PAD3     0


#define BOARD_SERCOM2_FREQUENCY       BOARD_GCLK0_FREQUENCY

/* ADC definitions **********************************************************/

/***************************** POWER subsystem ******************************/

/* adc0 driver */
#define BOARD_ADC_GCLKGEN             0

/* We are using this pins:
 * PB0,
 * PB1,
 * PB2,
 * PB3,
 * PB4,
 * PB5,
 * PB6,
 * PB7
 * as an analog inputs.
 */

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
  #define BOARD_ADC_INPUT8              1 /* PB0 - 5.0V */
  #define BOARD_ADC_INPUT9              1 /* PB1 - 3.3V */
  #define BOARD_ADC_INPUT10             1 /* PB2 - 1.8V */
  #define BOARD_ADC_INPUT11             1 /* PB3 - 1.5V */
  #define BOARD_ADC_INPUT12             1 /* PB4 - 1.0V_0 */
  #define BOARD_ADC_INPUT13             1 /* PB5 - 1.0V_1 */
  #define BOARD_ADC_INPUT14             1 /* PB6 - 0.9V */
  #define BOARD_ADC_INPUT15             1 /* PB7 - 12.0V BAT */

  #define BOARD_ADC_VBAT_CH             15
  #define BOARD_ADC_0V9_CH              14
  #define BOARD_ADC_1V02_1_CH           13
  #define BOARD_ADC_1V02_0_CH           12
  #define BOARD_ADC_1V5_CH              11
  #define BOARD_ADC_1V8_CH              10
  #define BOARD_ADC_3V3_CH              9
  #define BOARD_ADC_5V0_CH              8

  #define BOARD_ADC_NUM_CHANNELS        8

  /* adc0 channels offsets in g_adc0_data_buffer */
  #define BOARD_ADC_5V0_CH_OFFSET      (0x0)
  #define BOARD_ADC_3V3_CH_OFFSET      (0x1)
  #define BOARD_ADC_1V8_CH_OFFSET      (0x2)
  #define BOARD_ADC_1V5_CH_OFFSET      (0x3)
  #define BOARD_ADC_VBAT_CH_OFFSET     (0x4)
  #define BOARD_ADC_1V02_0_CH_OFFSET   (0x5)
  #define BOARD_ADC_1V02_1_CH_OFFSET   (0x6)
  #define BOARD_ADC_0V9_CH_OFFSET      (0x7)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
  #define PORT_AIN2                     PORT_AIN2_2
  #define PORT_AIN3                     PORT_AIN3_2

  #define BOARD_ADC_INPUT2              1 /* PB8 - 1.0V_0 */
  #define BOARD_ADC_INPUT3              1 /* PB9 - 1.0V_1 */
  #define BOARD_ADC_INPUT6              1 /* PA6 - 12.0V VBAT */
  #define BOARD_ADC_INPUT8              1 /* PB0 - 5.0V */
  #define BOARD_ADC_INPUT9              1 /* PB1 - 3.3V */
  #define BOARD_ADC_INPUT10             1 /* PB2 - 3.3V_0 */
  #define BOARD_ADC_INPUT11             1 /* PB3 - 3.3V_1 */
  #define BOARD_ADC_INPUT12             1 /* PB4 - 1.8V */
  #define BOARD_ADC_INPUT13             1 /* PB5 - 1.5V */
  #define BOARD_ADC_INPUT14             1 /* PB6 - 1.02V_0 */
  #define BOARD_ADC_INPUT15             1 /* PB7 - 1.02V_1 */

  #define BOARD_ADC_1V02_0_CH           15
  #define BOARD_ADC_1V02_1_CH           14
  #define BOARD_ADC_1V5_CH              13
  #define BOARD_ADC_1V8_CH              12
  #define BOARD_ADC_3V3_1_CH            11
  #define BOARD_ADC_3V3_0_CH            10
  #define BOARD_ADC_3V3_CH              9
  #define BOARD_ADC_5V0_CH              8
  #define BOARD_ADC_VBAT_CH             6
  #define BOARD_ADC_1V0_1_CH            3
  #define BOARD_ADC_1V0_0_CH            2

  #define BOARD_ADC_NUM_CHANNELS        11

  /* adc0 channels offsets in g_adc0_data_buffer */
  #define BOARD_ADC_5V0_CH_OFFSET      (0x0)
  #define BOARD_ADC_3V3_CH_OFFSET      (0x1)
  #define BOARD_ADC_1V8_CH_OFFSET      (0x2)
  #define BOARD_ADC_1V5_CH_OFFSET      (0x3)
  #define BOARD_ADC_VBAT_CH_OFFSET     (0x4)
  #define BOARD_ADC_1V02_0_CH_OFFSET   (0x5)
  #define BOARD_ADC_1V02_1_CH_OFFSET   (0x6)
  #define BOARD_ADC_3V3_0_CH_OFFSET    (0x7)
  #define BOARD_ADC_3V3_1_CH_OFFSET    (0x8)
  #define BOARD_ADC_1V0_0_CH_OFFSET    (0x9)
  #define BOARD_ADC_1V0_1_CH_OFFSET    (0xA)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  #define BOARD_ADC_INPUT8              1 /* PB0 - 5.0V */
  #define BOARD_ADC_INPUT9              1 /* PB1 - 3.3V */
  #define BOARD_ADC_INPUT10             1 /* PB2 - 1.8V */
  #define BOARD_ADC_INPUT11             1 /* PB3 - 1.5V */
  #define BOARD_ADC_INPUT12             1 /* PB4 - 1.0V_0 */
  #define BOARD_ADC_INPUT13             1 /* PB5 - 0.9V */
  #define BOARD_ADC_INPUT14             1 /* PB6 - 12.0V BAT */

  #define BOARD_ADC_VBAT_CH             14
  #define BOARD_ADC_0V9_CH              13
  #define BOARD_ADC_1V02_0_CH           12
  #define BOARD_ADC_1V5_CH              11
  #define BOARD_ADC_1V8_CH              10
  #define BOARD_ADC_3V3_CH              9
  #define BOARD_ADC_5V0_CH              8

  #define BOARD_ADC_NUM_CHANNELS        7

  /* adc0 channels offsets in g_adc0_data_buffer */
  #define BOARD_ADC_5V0_CH_OFFSET      (0x0)
  #define BOARD_ADC_3V3_CH_OFFSET      (0x1)
  #define BOARD_ADC_1V8_CH_OFFSET      (0x2)
  #define BOARD_ADC_1V5_CH_OFFSET      (0x3)
  #define BOARD_ADC_VBAT_CH_OFFSET     (0x4)
  #define BOARD_ADC_1V02_0_CH_OFFSET   (0x5)
  #define BOARD_ADC_0V9_CH_OFFSET      (0x6)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  #define PORT_AIN2                     PORT_AIN2_2

  #define BOARD_ADC_INPUT2              1 /* PB8 - 1.0V_0 */
  #define BOARD_ADC_INPUT6              1 /* PA6 - 12.0V VBAT */
  #define BOARD_ADC_INPUT8              1 /* PB0 - 5.0V */
  #define BOARD_ADC_INPUT9              1 /* PB1 - 3.3V */
  #define BOARD_ADC_INPUT11             1 /* PB3 - 3.3V_0 */
  #define BOARD_ADC_INPUT12             1 /* PB4 - 1.8V */
  #define BOARD_ADC_INPUT13             1 /* PB5 - 1.5V */
  #define BOARD_ADC_INPUT14             1 /* PB6 - 1.02V_0 */

  #define BOARD_ADC_1V02_0_CH           14
  #define BOARD_ADC_1V5_CH              13
  #define BOARD_ADC_1V8_CH              12
  #define BOARD_ADC_3V3_0_CH            11
  #define BOARD_ADC_3V3_CH              9
  #define BOARD_ADC_5V0_CH              8
  #define BOARD_ADC_VBAT_CH             6
  #define BOARD_ADC_1V0_0_CH            2

  #define BOARD_ADC_NUM_CHANNELS        8

  /* adc0 channels offsets in g_adc0_data_buffer */
  #define BOARD_ADC_5V0_CH_OFFSET      (0x0)
  #define BOARD_ADC_3V3_CH_OFFSET      (0x1)
  #define BOARD_ADC_1V8_CH_OFFSET      (0x2)
  #define BOARD_ADC_1V5_CH_OFFSET      (0x3)
  #define BOARD_ADC_VBAT_CH_OFFSET     (0x4)
  #define BOARD_ADC_1V02_0_CH_OFFSET   (0x5)
  #define BOARD_ADC_3V3_0_CH_OFFSET    (0x6)
  #define BOARD_ADC_1V0_0_CH_OFFSET    (0x7)
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

/* The negative input is the internal GND */
#define BOARD_ADC_NEG                   ADC_INPUTCTRL_MUXNEG_GND

/* The VREF is the INTVCC1 = 1/2 VDDANA */
#define BOARD_ADC_REF                   ADC_REFCTRL_REFSEL_INTVCC1

/* We need to measure up to 3.3V voltage, becose use the divider: /2 */
#define BOARD_ADC_GAIN                  ADC_INPUTCTRL_GAIN_DIV2

/*  */
#define BOARD_ADC_SAMPLEN               0x3F

#define BOARD_ADC_AVERAGING             ADC_AVGCTRL_SAMPLENUM_1024

#define BOARD_ADC_PRESCALER             ADC_CTRLB_PRESCALER_DIV32


/* GPIO definitions *********************************************************/

/* gpinp0 driver */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) ||\
    defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  #define BOARD_NGPINP0                 10
  #define PWR1_PRESENT_CLAMP           (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN0)
  #define PWR2_PRESENT_CLAMP           (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN1)
  #define PWR1_AC_OK_CLAMP             (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN2)
  #define PWR2_AC_OK_CLAMP             (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN3)
  #define PWR1_ALERT_CLAMP             (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN4)
  #define PWR2_ALERT_CLAMP             (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN5)
  #define PWR1_PW_OK_CLAMP             (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN6)
  #define PWR2_PW_OK_CLAMP             (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN7)
  #define PWR_AC_OK                    (PORT_INPUT | PORT_PULL_DOWN| \
                                        PORT_INT_LOW | PORTA | PORT_PIN10)
  #define PWR_BAT_LOW                  (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN11)
  /* gpinp0 input bit offsets */
  #define BOARD_GPINP0_PRESENT_1       (0x0)
  #define BOARD_GPINP0_AC_OK_1         (0x1)
  #define BOARD_GPINP0_ALERT_1         (0x2)
  #define BOARD_GPINP0_PW_OK_1         (0x3)
  #define BOARD_GPINP0_PRESENT_2       (0x4)
  #define BOARD_GPINP0_AC_OK_2         (0x5)
  #define BOARD_GPINP0_ALERT_2         (0x6)
  #define BOARD_GPINP0_PW_OK_2         (0x7)
  #define BOARD_GPINP0_AC_OK_ACC       (0x8)
  #define BOARD_GPINP0_BAT_LOW_ACC     (0x9)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) || \
      defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  #define BOARD_NGPINP0                 6
  #define PWR_PRE0                     (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN0)
  #define PWR_PRE1                     (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN1)
  #define PWR_PG0                      (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN2)
  #define PWR_PG1                      (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN3)
  #define PWR_AC_OK                    (PORT_INPUT | PORT_PULL_DOWN| \
                                        PORT_INT_LOW | PORTA | PORT_PIN10)
  #define PWR_BAT_LOW                  (PORT_INPUT | PORT_PULL_NONE | \
                                        PORT_INT_LOW | PORTA | PORT_PIN11)
  /* gpinp0 input bit offsets */
  #define BOARD_GPINP0_PWR_PRE0        (0x0)
  #define BOARD_GPINP0_PWR_PG0         (0x1)
  #define BOARD_GPINP0_PWR_PRE1        (0x2)
  #define BOARD_GPINP0_PWR_PG1         (0x3)
  #define BOARD_GPINP0_AC_OK_ACC       (0x4)
  #define BOARD_GPINP0_BAT_LOW_ACC     (0x5)
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

/* gpout0 driver */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
  #define BOARD_NGPOUT0                 10
  #define PWR1_ON                      (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_SET | PORTA | PORT_PIN8)
  #define PWR2_ON                      (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_SET | PORTA | PORT_PIN9)
  #define VDD5V0_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN12)
  #define VDD3V3_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN13)
  #define VDD1V8_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN14)
  #define VDD1V5_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN15)
  #define VDD1V02_EN_0                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN16)
  #define VDD1V02_EN_1                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN17)
  #define VDD0V9_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN22)
  #define CONTROL                      (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN14)
  /* gpout0 output bit offsets */
  #define BOARD_GPOUT0_3V3             (0x0)
  #define BOARD_GPOUT0_1V8             (0x1)
  #define BOARD_GPOUT0_1V5             (0x2)
  #define BOARD_GPOUT0_1V02_0          (0x3)
  #define BOARD_GPOUT0_1V02_1          (0x4)
  #define BOARD_GPOUT0_0V9             (0x5)
  #define BOARD_GPOUT0_5V0             (0x6)
  #define BOARD_GPOUT0_PWR1            (0x7)
  #define BOARD_GPOUT0_PWR2            (0x8)
  #define BOARD_GPOUT0_CONTROL         (0x9)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
  #define BOARD_NGPOUT0                 10
  #define VDD5V0_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN14)
  #define VDD3V3_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN15)
  #define VDD3V3_0_EN                  (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN16)
  #define VDD3V3_1_EN                  (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN22)
  #define VDD1V8_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN10)
  #define VDD1V5_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN11)
  #define VDD1V02_EN_0                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN13)
  #define VDD1V02_EN_1                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN23)
  #define VDD1V0_EN_0                  (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN12)
  #define VDD1V0_EN_1                  (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN17)
  /* gpout0 output bit offsets */
  #define BOARD_GPOUT0_3V3             (0x0)
  #define BOARD_GPOUT0_1V8             (0x1)
  #define BOARD_GPOUT0_1V5             (0x2)
  #define BOARD_GPOUT0_1V02_0          (0x3)
  #define BOARD_GPOUT0_1V02_1          (0x4)
  #define BOARD_GPOUT0_5V0             (0x5)
  #define BOARD_GPOUT0_3V3_0           (0x6)
  #define BOARD_GPOUT0_3V3_1           (0x7)
  #define BOARD_GPOUT0_1V0_0           (0x8)
  #define BOARD_GPOUT0_1V0_1           (0x9)
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  #define BOARD_NGPOUT0                 9
  #define PWR1_ON                      (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_SET | PORTA | PORT_PIN8)
  #define PWR2_ON                      (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_SET | PORTA | PORT_PIN9)
  #define VDD5V0_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN12)
  #define VDD3V3_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN13)
  #define VDD1V8_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN14)
  #define VDD1V5_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN15)
  #define VDD1V02_EN_0                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN16)
  #define VDD0V9_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN17)
  #define CONTROL                      (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN14)
  /* gpout0 output bit offsets */
  #define BOARD_GPOUT0_3V3             (0x0)
  #define BOARD_GPOUT0_1V8             (0x1)
  #define BOARD_GPOUT0_1V5             (0x2)
  #define BOARD_GPOUT0_1V02_0          (0x3)
  #define BOARD_GPOUT0_0V9             (0x4)
  #define BOARD_GPOUT0_5V0             (0x5)
  #define BOARD_GPOUT0_PWR1            (0x6)
  #define BOARD_GPOUT0_PWR2            (0x7)
  #define BOARD_GPOUT0_CONTROL         (0x8)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  #define BOARD_NGPOUT0                 7
  #define VDD5V0_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN14)
  #define VDD3V3_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN15)
  #define VDD3V3_0_EN                  (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN22)
  #define VDD1V8_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN10)
  #define VDD1V5_EN                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN11)
  #define VDD1V02_EN_0                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN13)
  #define VDD1V0_EN_0                  (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN12)
  /* gpout0 output bit offsets */
  #define BOARD_GPOUT0_3V3             (0x0)
  #define BOARD_GPOUT0_1V8             (0x1)
  #define BOARD_GPOUT0_1V5             (0x2)
  #define BOARD_GPOUT0_1V02_0          (0x3)
  #define BOARD_GPOUT0_5V0             (0x4)
  #define BOARD_GPOUT0_3V3_0           (0x5)
  #define BOARD_GPOUT0_1V0_0           (0x6)
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

/* gpout1 driver */
#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
  #define BOARD_NGPOUT1                 5
  #define POWER_GOOD                   (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN24)
  #define ALARM                        (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN23)
  #define PW_OK_SW0                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN23)
  #define PW_OK_SW1                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN30)
  #define PW_OK_TCA                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN31)
  /* gpout1 output bit offsets */
  #define BOARD_GPOUT1_POWERGOOD        (0x0)
  #define BOARD_GPOUT1_ALARM            (0x1)
  #define BOARD_GPOUT1_SW0              (0x2)
  #define BOARD_GPOUT1_SW1              (0x3)
  #define BOARD_GPOUT1_TCA              (0x4)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
  #define BOARD_NGPOUT1                 4
  #define POWER_GOOD                   (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN24)
  #define ALARM                        (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN23)
  #define PW_OK_SW0                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN30)
  #define PW_OK_SW1                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN31)
  /* gpout1 output bit offsets */
  #define BOARD_GPOUT1_POWERGOOD        (0x0)
  #define BOARD_GPOUT1_ALARM            (0x1)
  #define BOARD_GPOUT1_SW0              (0x2)
  #define BOARD_GPOUT1_SW1              (0x3)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
  #define BOARD_NGPOUT1                 4
  #define POWER_GOOD                   (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN24)
  #define ALARM                        (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN23)
  #define PW_OK_SW0                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN23)
  #define PW_OK_TCA                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN31)
  /* gpout1 output bit offsets */
  #define BOARD_GPOUT1_POWERGOOD        (0x0)
  #define BOARD_GPOUT1_ALARM            (0x1)
  #define BOARD_GPOUT1_SW0              (0x2)
  #define BOARD_GPOUT1_TCA              (0x3)

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
  #define BOARD_NGPOUT1                 3
  #define POWER_GOOD                   (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN24)
  #define ALARM                        (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTA | PORT_PIN23)
  #define PW_OK_SW0                    (PORT_OUTPUT | PORT_PULL_NONE | \
                                        PORT_OUTPUT_CLEAR | PORTB | PORT_PIN30)
  /* gpout1 output bit offsets */
  #define BOARD_GPOUT1_POWERGOOD        (0x0)
  #define BOARD_GPOUT1_ALARM            (0x1)
  #define BOARD_GPOUT1_SW0              (0x2)
#endif /* CONFIG_ARCH_BOARD_APC3_ARLAN_ */

/***************************** INLED subsystem ******************************/

/* gpint0 driver */
#define BOARD_NGPINT0                 2

#define BUTTON_1                     (PORT_INTERRUPT | PORT_PULL_UP | \
                                      PORT_INT_FALLING | PORTA | PORT_PIN18 | \
                                      PORT_FUNCA)
#define BUTTON_2                     (PORT_INTERRUPT | PORT_PULL_UP | \
                                      PORT_INT_FALLING | PORTA | PORT_PIN20 | \
                                      PORT_FUNCA)
/* gpint0 input bit offsets */
#define BOARD_GPINT0_BTN1            (0x0)
#define BOARD_GPINT0_BTN2            (0x1)

#define BUTTON_1_EXTINT               SAM_IRQ_EXTINT2
#define BUTTON_2_EXTINT               SAM_IRQ_EXTINT4

/* gpout2 driver */
#define BOARD_NGPOUT2                 2

#define LED_BUTTON_1                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                      PORT_OUTPUT_CLEAR | PORTA | PORT_PIN19)
#define LED_BUTTON_2                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                      PORT_OUTPUT_CLEAR | PORTA | PORT_PIN21)
/* gpout2 output bit offsets */
#define BOARD_GPOUT2_LED1            (0x0)
#define BOARD_GPOUT2_LED2            (0x1)

/***************************** RESET subsystem ******************************/

/* gpint1 driver */
#define BOARD_NGPINT1                 1

#define STANDBY                      (PORT_INTERRUPT | PORT_PULL_UP | \
                                      PORT_INT_FALLING | PORTA | PORT_PIN22 | \
                                      PORT_FUNCA)
/* gpint1 input bit offsets */
#define BOARD_GPINT1_STANDBY         (0x0)


#define STANDBY_EXTINT                SAM_IRQ_EXTINT6

/* gpout3 driver */
#define BOARD_NGPOUT3                 1

#define RESET_SYSTEM                 (PORT_OUTPUT | PORT_PULL_NONE | \
                                      PORT_OUTPUT_SET | PORTA | PORT_PIN25)
/* gpout3 output bit offsets */
#define BOARD_GPOUT3_RESET           (0x0)



#endif /* __BOARDS_ARM_SAMD2L2_SAMD20_APC3_BOARD_H */
