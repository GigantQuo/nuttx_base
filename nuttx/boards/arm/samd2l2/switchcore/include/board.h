/****************************************************************************
 * boards/arm/samd2l2/switchcore/include/board.h
 ****************************************************************************/

#ifndef __BOARDS_ARM_SAMD2L2_SWITCHCORE_PRIVATE_INCLUDE_BOARD_H
#define __BOARDS_ARM_SAMD2L2_SWITCHCORE_PRIVATE_INCLUDE_BOARD_H

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
/* 32.768kHz internal oscillator */
#define BOARD_OSC32K_FREQUENCY			32768
#define BOARD_OSC32K_STARTUPTIME		SYSCTRL_OSC32K_STARTUP_4MS
#define BOARD_OSC32K_EN1KHZ				1
#define BOARD_OSC32K_EN32KHZ			1
#define BOARD_OSC32K_ONDEMAND			1
#undef  BOARD_OSC32K_RUNINSTANDBY

/* OSC8M Configuration -- always enabled
 *
 *   BOARD_OSC8M_PRESCALER      - See SYSCTRL_OSC8M_PRESC_DIV* definitions
 *   BOARD_OSC8M_ONDEMAND       - Boolean (defined / not defined)
 *   BOARD_OSC8M_RUNINSTANDBY   - Boolean (defined / not defined)
 */

#define BOARD_OSC8M_PRESCALER			SYSCTRL_OSC8M_PRESC_DIV1
#define BOARD_OSC8M_ONDEMAND			1
#undef  BOARD_OSC8M_RUNINSTANDBY

/* 8MHz high-accuracy internal oscillator */
#define BOARD_OSC8M_FREQUENCY			8000000

/* OSCULP32K Configuration -- not used. */

/* 32kHz ultra-low-power internal oscillator */
#define BOARD_OSCULP32K_FREQUENCY		32768

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

#define BOARD_DFLL_ENABLE				1
#define BOARD_DFLL_OPENLOOP				1
#undef  BOARD_DFLL_ONDEMAND
#undef  BOARD_DFLL_RUNINSTANDBY

/* DFLL closed loop mode configuration */

#define BOARD_DFLL_SRCGCLKGEN			1
#define BOARD_DFLL_MULTIPLIER			6
#define BOARD_DFLL_QUICKLOCK			1
#define BOARD_DFLL_TRACKAFTERFINELOCK	1
#define BOARD_DFLL_KEEPLOCKONWAKEUP		1
#define BOARD_DFLL_ENABLECHILLCYCLE		1
#define BOARD_DFLL_MAXCOARSESTEP		(0x1f / 4)
#define BOARD_DFLL_MAXFINESTEP			(0xff / 4)

#define BOARD_DFLL_FREQUENCY			(48000000)

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

#define BOARD_GCLK_ENABLE				1

/* GCLK generator 0 (Main Clock) - Source is the DFLL */

#undef  BOARD_GCLK0_RUN_IN_STANDBY
#define BOARD_GCLK0_CLOCK_SOURCE		GCLK_GENCTRL_SRC_DFLL48M
#define BOARD_GCLK0_PRESCALER			1
#undef  BOARD_GCLK0_OUTPUT_ENABLE
#define BOARD_GCLK0_FREQUENCY			(BOARD_DFLL_FREQUENCY / \
										BOARD_GCLK0_PRESCALER)

/* Configure GCLK generator 1 - Drives the DFLL */

#define BOARD_GCLK1_ENABLE				1
#undef  BOARD_GCLK1_RUN_IN_STANDBY
#define BOARD_GCLK1_CLOCK_SOURCE		GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK1_PRESCALER			1
#undef  BOARD_GCLK1_OUTPUT_ENABLE
#define BOARD_GCLK1_FREQUENCY			(BOARD_OSC8M_FREQUENCY / \
										BOARD_GCLK1_PRESCALER)

/* Configure GCLK generator 2 (WDT) */

#define BOARD_GCLK2_ENABLE				1
#undef  BOARD_GCLK2_RUN_IN_STANDBY
#define BOARD_GCLK2_CLOCK_SOURCE		GCLK_GENCTRL_SRC_OSCULP32K
#define BOARD_GCLK2_PRESCALER			31
#undef  BOARD_GCLK2_OUTPUT_ENABLE
#define BOARD_GCLK2_FREQUENCY			(BOARD_OSCULP32K_FREQUENCY / \
										BOARD_GCLK2_PRESCALER)

/* Configure GCLK generator 3 */

#undef  BOARD_GCLK3_ENABLE
#undef  BOARD_GCLK3_RUN_IN_STANDBY
#define BOARD_GCLK3_CLOCK_SOURCE		GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK3_PRESCALER			1
#undef  BOARD_GCLK3_OUTPUT_ENABLE
#define BOARD_GCLK3_FREQUENCY			(BOARD_OSC8M_FREQUENCY / \
										BOARD_GCLK3_PRESCALER)

/* Configure GCLK generator 4 */

#undef  BOARD_GCLK4_ENABLE
#undef  BOARD_GCLK4_RUN_IN_STANDBY
#define BOARD_GCLK4_CLOCK_SOURCE		GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK4_PRESCALER			1
#undef  BOARD_GCLK4_OUTPUT_ENABLE
#define BOARD_GCLK4_FREQUENCY			(BOARD_OSC8M_FREQUENCY / \
										BOARD_GCLK4_PRESCALER)

/* Configure GCLK generator 5 */

#undef  BOARD_GCLK5_ENABLE
#undef  BOARD_GCLK5_RUN_IN_STANDBY
#define BOARD_GCLK5_CLOCK_SOURCE		GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK5_PRESCALER			1
#undef  BOARD_GCLK5_OUTPUT_ENABLE
#define BOARD_GCLK5_FREQUENCY			(BOARD_OSC8M_FREQUENCY / \
										BOARD_GCLK5_PRESCALER)

/* Configure GCLK generator 6 */

#undef  BOARD_GCLK6_ENABLE
#undef  BOARD_GCLK6_RUN_IN_STANDBY
#define BOARD_GCLK6_CLOCK_SOURCE		GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK6_PRESCALER			1
#undef  BOARD_GCLK6_OUTPUT_ENABLE
#define BOARD_GCLK6_FREQUENCY			(BOARD_OSC8M_FREQUENCY / \
										BOARD_GCLK6_PRESCALER)

/* Configure GCLK generator 7 */

#undef  BOARD_GCLK7_ENABLE
#undef  BOARD_GCLK7_RUN_IN_STANDBY
#define BOARD_GCLK7_CLOCK_SOURCE		GCLK_GENCTRL_SRC_OSC8M
#define BOARD_GCLK7_PRESCALER			1
#undef  BOARD_GCLK7_OUTPUT_ENABLE
#define BOARD_GCLK7_FREQUENCY			(BOARD_OSC8M_FREQUENCY / \
										BOARD_GCLK7_PRESCALER)

/* The source of the main clock is always GCLK_MAIN.  Also called GCLKGEN[0],
 * this is the clock feeding the Power Manager.
 * The Power Manager, in turn, generates main clock which is divided down to
 * produce the CPU, AHB, and APB clocks.
 *
 * The main clock is initially OSC8M divided by 8.
 */

#define BOARD_GCLK_MAIN_FREQUENCY		BOARD_GCLK0_FREQUENCY

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

#define BOARD_CPU_FAILDECT				1
#define BOARD_CPU_DIVIDER				PM_CPUSEL_CPUDIV_1
#define BOARD_APBA_DIVIDER				PM_APBASEL_APBADIV_1
#define BOARD_APBB_DIVIDER				PM_APBBSEL_APBBDIV_1
#define BOARD_APBC_DIVIDER				PM_APBCSEL_APBCDIV_1

/* Resulting frequencies */

#define BOARD_MCK_FREQUENCY				(BOARD_GCLK_MAIN_FREQUENCY)
#define BOARD_CPU_FREQUENCY				(BOARD_MCK_FREQUENCY)
#define BOARD_PBA_FREQUENCY				(BOARD_MCK_FREQUENCY)
#define BOARD_PBB_FREQUENCY				(BOARD_MCK_FREQUENCY)
#define BOARD_PBC_FREQUENCY				(BOARD_MCK_FREQUENCY)
#define BOARD_PBD_FREQUENCY				(BOARD_MCK_FREQUENCY)

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
#  define BOARD_FLASH_WAITSTATES		1
#else
#  define BOARD_FLASH_WAITSTATES		2
#endif /* 0 */

/* WDT definitions **********************************************************/

#define BOARD_WDT_GCLKGEN				2
#define BOARD_WDT_FREQUENCY				BOARD_GCLK2_FREQUENCY



/* EIC definitions **********************************************************/

#define BOARD_GCLK_EIC					1         /* EIC GCLK index */



/* SERCOM definitions *******************************************************/

/* This is the source clock generator for the GCLK_SERCOM_SLOW clock that is
 * common to all SERCOM modules.
 */
#define BOARD_SERCOM05_SLOW_GCLKGEN		0

/* SERCOM0 I2C_slave is available on EXT1
 *
 *  PIN PORT SERCOM        FUNCTION
 *  --- ------------------ -----------
 *  17  PA8  SERCOM0 PAD0  I2C SDA
 *  18  PA9  SERCOM0 PAD1  I2C SCL
 */

#define BOARD_SERCOM0_GCLKGEN			0
#define BOARD_SERCOM0_SLOW_GCLKGEN		BOARD_SERCOM05_SLOW_GCLKGEN
#define BOARD_SERCOM0_MUXCONFIG			0
#define BOARD_SERCOM0_PINMAP_PAD0		PORT_SERCOM0_PAD0_1
#define BOARD_SERCOM0_PINMAP_PAD1		PORT_SERCOM0_PAD1_1

#define BOARD_SERCOM0_FREQUENCY			BOARD_GCLK0_FREQUENCY

/* SERCOM1 I2C_master is available on EXT2
 *
 *  PIN PORT SERCOM        FUNCTION
 *  --- ------------------ -----------
 *  35  PA16 SERCOM1 PAD0  I2C SDA
 *  36  PA17 SERCOM1 PAD1  I2C SCL
 */

#define BOARD_SERCOM1_GCLKGEN			0
#define BOARD_SERCOM1_SLOW_GCLKGEN		BOARD_SERCOM05_SLOW_GCLKGEN
#define BOARD_SERCOM1_MUXCONFIG			0
#define BOARD_SERCOM1_PINMAP_PAD0		PORT_SERCOM1_PAD0_1
#define BOARD_SERCOM1_PINMAP_PAD1		PORT_SERCOM1_PAD1_1

#define BOARD_SERCOM1_FREQUENCY			BOARD_GCLK0_FREQUENCY

/* SERCOM2 is not used */

/* SERCOM3 is not used */

/* SERCOM4 is not used */

/* SERCOM5 is not used */


/* USB definitions **********************************************************/

/* This is the source clock generator for the GCLK_USB clock
 */

#define BOARD_USB_GCLKGEN				0
#define BOARD_USB_FREQUENCY				BOARD_GCLK0_FREQUENCY

/* default USB Pad calibration (not used yet by USB driver) */

#define BOARD_USB_PADCAL_P				29
#define BOARD_USB_PADCAL_N				5
#define BOARD_USB_PADCAL_TRIM			3

#if defined(CONFIG_DEV_GPIO)

/* GPIO definitions *********************************************************/

#define BOARD_NGPIOINT					6
#define BOARD_NGPIOIN					1
#define BOARD_NGPIOOUT					24


/*******************POWER SYSTEM*******************/

/* Interrupt pins */
#define PWR_FLT_A						(PORT_INTERRUPT | PORT_PULL_UP | \
										PORT_INT_LOW | PORTA | PORT_PIN4 | \
										PORT_FUNCA)

#define PWR_FLT_B						(PORT_INTERRUPT | PORT_PULL_UP | \
										PORT_INT_LOW | PORTA | PORT_PIN5 | \
										PORT_FUNCA)

#define FUSE_FLT_A						(PORT_INTERRUPT | PORT_PULL_UP | \
										PORT_INT_LOW | PORTA | PORT_PIN6 | \
										PORT_FUNCA)

#define FUSE_FLT_B						(PORT_INTERRUPT | PORT_PULL_UP | \
										PORT_INT_LOW | PORTA | PORT_PIN7 | \
										PORT_FUNCA)

/* Input pins */
#define PGOOD							(PORT_INPUT | PORT_PULL_UP | \
										PORT_INT_LOW | PORTA | PORT_PIN13)

/* Output pins */
#define IsOn							(PORT_OUTPUT | PORT_PULL_DOWN | \
										PORT_OUTPUT_CLEAR | \
										PORTA | PORT_PIN10)

#define IsReady							(PORT_OUTPUT | PORT_PULL_DOWN | \
										PORT_OUTPUT_CLEAR | \
										PORTA | PORT_PIN11)


#define PWR_EN_block1					(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_CLEAR | \
										PORTA | PORT_PIN18)

#define PWR_EN_block2					(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_CLEAR | \
										PORTA | PORT_PIN19)

#define PWR_EN_block3					(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_CLEAR | \
										PORTB | PORT_PIN16)

#define PWR_EN_block4					(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_CLEAR | \
										PORTB | PORT_PIN17)

#define PWR_EN_block5					(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_CLEAR | \
										PORTA | PORT_PIN20)

#define PWR_EN_block6					(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_CLEAR | \
										PORTA | PORT_PIN21)

#define PWR_EN_block7					(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_CLEAR | \
										PORTA | PORT_PIN22)


#define EN_CLK_PHY_block1				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN10)

#define EN_CLK_PHY_block2				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN11)

#define EN_CLK_PHY_block3				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN12)

#define EN_CLK_PHY_block4				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN13)

#define EN_CLK_PHY_block5				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN14)

#define EN_CLK_PHY_block6				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN15)


#define En3V3_switch					(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN2)

/* Interrupts numbers */
#define PWR_FLT_A_EXTINT				SAM_IRQ_EXTINT4
#define PWR_FLT_B_EXTINT				SAM_IRQ_EXTINT5

#define FUSE_FLT_A_EXTINT				SAM_IRQ_EXTINT6
#define FUSE_FLT_B_EXTINT				SAM_IRQ_EXTINT7


/*******************RESET SYSTEM*******************/

/* Interrupt pins */
#define SysRstOutn						(PORT_INTERRUPT | PORT_PULL_UP | \
										PORT_INT_LOW | PORTB | PORT_PIN8 | \
										PORT_FUNCA)

#define JTAG_nRST						(PORT_INTERRUPT | PORT_PULL_UP | \
										PORT_INT_LOW | PORTB | PORT_PIN9 | \
										PORT_FUNCA)

/* Output pins */
#define QPHY_RESETn_block1				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTA | PORT_PIN0)

#define QPHY_RESETn_block2				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTA | PORT_PIN1)

#define QPHY_RESETn_block3				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTA | PORT_PIN2)

#define QPHY_RESETn_block4				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTA | PORT_PIN3)

#define QPHY_RESETn_block5				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN4)

#define QPHY_RESETn_block6				(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN5)

#define PHY_RESETn						(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN0)

#define RST_switch						(PORT_OUTPUT | PORT_PULL_NONE | \
										PORT_OUTPUT_SET | \
										PORTB | PORT_PIN1)

/* Interrupts numbers */
#define SysRstOutn_EXTINT				SAM_IRQ_EXTINT8
#define JTAG_nRST_EXTINT				SAM_IRQ_EXTINT14

#endif /* CONFIG_DEV_GPIO */


/*******************I2C MASTER SYSTEM*******************/

#ifdef CONFIG_I2C_DRIVER

#if defined(CONFIG_SENSORS_LPS25H)

/* LPS22H definitions *******************************************************/

#define LPS22H_EXTINT					SAM_IRQ_EXTINT14

#define PORT_LPS22H_IRQ					(PORT_INTERRUPT | PORT_PULL_DOWN | \
										PORT_INT_RISING | PORT_EXTINT14_3)

#endif /* CONFIG_SENSORS_LPS25H */


#if defined(CONFIG_SENSORS_MAX662)

/* MAX662 definitions *******************************************************/

#define MAX662_EXTINT					SAM_IRQ_EXTINT15

#define PORT_MAX662_IRQ					(PORT_INTERRUPT | PORT_PULL_NONE | \
										PORT_INT_FALLING | PORT_EXTINT15_4)
#endif /* CONFIG_SENSORS_MAX662 */

#endif /* CONFIG_I2C_DRIVER */

#endif /* __BOARDS_ARM_SAMD2L2_SWITCHCORE_PRIVATE_INCLUDE_BOARD_H */
