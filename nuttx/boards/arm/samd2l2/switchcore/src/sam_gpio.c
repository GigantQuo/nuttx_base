/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_gpio.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <arch/irq.h>

#include <nuttx/ioexpander/gpio.h>

#include "arm_internal.h"
#include "sam_pinmap.h"
#include "sam_port.h"
#include "sam_eic.h"

#if defined(CONFIG_DEV_GPIO)
/****************************************************************************
 * Private Types
 ****************************************************************************/

struct samgpio_dev_s
{
	struct gpio_dev_s gpio;
	uint8_t id;
};

struct samgpint_dev_s
{
	struct samgpio_dev_s samgpio;
	pin_interrupt_t callback;
};


/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#if BOARD_NGPIOIN > 0
static int gpin_read(struct gpio_dev_s *dev, bool *value);
#endif
static int gpout_read(struct gpio_dev_s *dev, bool *value);
static int gpout_write(struct gpio_dev_s *dev, bool value);

static int gpint_read(struct gpio_dev_s *dev, bool *value);
static int gpint_attach(struct gpio_dev_s *dev,
                        pin_interrupt_t callback);
static int gpint_enable(struct gpio_dev_s *dev, bool enable);


/****************************************************************************
 * Private Data
 ****************************************************************************/

#if BOARD_NGPIOINT > 0
/* This array maps the GPIO pins used as INTERRUPT INPUTS */

static const uint32_t G_INT_INPUTS[BOARD_NGPIOINT] =
{
	PWR_FLT_A,				// /dev/gpio/gpio25	| PA04	| 13
	PWR_FLT_B,				// /dev/gpio/gpio26	| PA05	| 14
	FUSE_FLT_A,				// /dev/gpio/gpio27	| PA06	| 15
	FUSE_FLT_B,				// /dev/gpio/gpio28	| PA07	| 16
	SysRstOutn,				// /dev/gpio/gpio29	| PB08	| 11
	JTAG_nRST				// /dev/gpio/gpio30	| PB09	| 12
};

static const struct gpio_operations_s gpint_ops =
{
	.go_read   = gpint_read,
	.go_write  = NULL,
	.go_attach = gpint_attach,
	.go_enable = gpint_enable,
};

static const int G_EXTINTS[BOARD_NGPIOINT] =
{
	PWR_FLT_A_EXTINT,
	PWR_FLT_B_EXTINT,
	FUSE_FLT_A_EXTINT,
 	FUSE_FLT_B_EXTINT,
	SysRstOutn_EXTINT,
	JTAG_nRST_EXTINT
};

static struct samgpint_dev_s g_gpint[BOARD_NGPIOINT];
#endif

#if BOARD_NGPIOIN > 0
/* This array maps the GPIO pins used as INPUT */

static const uint32_t G_INPUTS[BOARD_NGPIOIN] =
{
	PGOOD					// /dev/gpio/gpio00	| PA13	| 30
};

static const struct gpio_operations_s gpin_ops =
{
	.go_read   = gpin_read,
	.go_write  = NULL,
	.go_attach = NULL,
	.go_enable = NULL,
};

static struct samgpio_dev_s g_gpin[BOARD_NGPIOIN];
#endif

#if BOARD_NGPIOOUT > 0
/* This array maps the GPIO pins used as OUTPUT */

static const uint32_t G_OUTPUTS[BOARD_NGPIOOUT] =
{
	IsOn,					// /dev/gpio/gpio01	| PA10	| 19
	IsReady,				// /dev/gpio/gpio02	| PA11	| 20
	PWR_EN_block1,			// /dev/gpio/gpio03	| PA18	| 37
	PWR_EN_block2,			// /dev/gpio/gpio04	| PA19	| 38
	PWR_EN_block3,			// /dev/gpio/gpio05	| PB16	| 39
	PWR_EN_block4,			// /dev/gpio/gpio06	| PB17	| 40
	PWR_EN_block5,			// /dev/gpio/gpio07	| PA20	| 41
	PWR_EN_block6,			// /dev/gpio/gpio08	| PA21	| 42
	PWR_EN_block7,			// /dev/gpio/gpio09	| PA22	| 43
	EN_CLK_PHY_block1,		// /dev/gpio/gpio10	| PB10	| 23
	EN_CLK_PHY_block2,		// /dev/gpio/gpio11	| PB11	| 24
	EN_CLK_PHY_block3,		// /dev/gpio/gpio12	| PB12	| 25
	EN_CLK_PHY_block4,		// /dev/gpio/gpio13	| PB13	| 26
	EN_CLK_PHY_block5,		// /dev/gpio/gpio14	| PB14	| 27
	EN_CLK_PHY_block6,		// /dev/gpio/gpio15	| PB15	| 28
	En3V3_switch,			// /dev/gpio/gpio16	| PB02	| 63
	QPHY_RESETn_block1,		// /dev/gpio/gpio17	| PA00	| 1
	QPHY_RESETn_block2,		// /dev/gpio/gpio18	| PA01	| 2
	QPHY_RESETn_block3,		// /dev/gpio/gpio19	| PA02	| 3
	QPHY_RESETn_block4,		// /dev/gpio/gpio20	| PA03	| 4
	QPHY_RESETn_block5,		// /dev/gpio/gpio21	| PB04	| 5
	QPHY_RESETn_block6,		// /dev/gpio/gpio22	| PB05	| 6
	PHY_RESETn,				// /dev/gpio/gpio23	| PB00	| 61
	RST_switch				// /dev/gpio/gpio24	| PB01	| 62
};

static const struct gpio_operations_s gpout_ops =
{
	.go_read   = gpout_read,
	.go_write  = gpout_write,
	.go_attach = NULL,
	.go_enable = NULL,
};

static struct samgpio_dev_s g_gpout[BOARD_NGPIOOUT];
#endif


/****************************************************************************
 * Private Functions
 ****************************************************************************/

#if BOARD_NGPIOIN > 0
/****************************************************************************
 * Name: gpin_read
 *
 * Description:
 *	Read input pin data (true/false)
 *
 ****************************************************************************/

static int gpin_read(	struct gpio_dev_s *dev,
						bool *value)
{
	struct samgpio_dev_s *samgpio = (struct samgpio_dev_s *)dev;

	DEBUGASSERT(samgpio != NULL && value != NULL);
	DEBUGASSERT(samgpio->id < BOARD_NGPIOIN);
	gpioinfo("Reading...\n");

	*value = sam_portread(G_INPUTS[samgpio->id]);
	return OK;
}
#endif


#if BOARD_NGPIOOUT > 0
/****************************************************************************
 * Name: gpout_read
 *
 * Description:
 *	Read output pin data (true/false)
 *
 ****************************************************************************/

static int gpout_read(	struct gpio_dev_s *dev,
						bool *value)
{
	struct samgpio_dev_s *samgpio = (struct samgpio_dev_s *)dev;

	DEBUGASSERT(samgpio != NULL && value != NULL);
	DEBUGASSERT(samgpio->id < BOARD_NGPIOOUT);
	gpioinfo("Reading...\n");

	*value = sam_portread(G_OUTPUTS[samgpio->id]);
	return OK;
}


/****************************************************************************
 * Name: gpout_write
 *
 * Description:
 *	Write output pin data (true/false)
 *
 ****************************************************************************/

static int gpout_write(	struct gpio_dev_s *dev,
						bool value)
{
	struct samgpio_dev_s *samgpio = (struct samgpio_dev_s *)dev;

	DEBUGASSERT(samgpio != NULL);
	DEBUGASSERT(samgpio->id < BOARD_NGPIOOUT);
	gpioinfo("Writing %d\n", (int)value);

	sam_portwrite(G_OUTPUTS[samgpio->id], value);
	return OK;
}
#endif


#if BOARD_NGPIOINT > 0
/****************************************************************************
 * Name: samgpio_interrupt
 *
 * Description:
 *	Information wrapper for interrupt callback
 *
 ****************************************************************************/

int samgpio_interrupt(	int irq,
						void *context,
						void *arg)
{
	struct samgpint_dev_s *samgpint = (struct samgpint_dev_s *)arg;
	/*
	bool val;
	int pin;
	int pinset = G_INT_INPUTS[samgpint->samgpio.id];
	uint32_t config;

	val = sam_portread(pinset);
	pin = (pinset & PORT_PIN_MASK) >> PORT_PIN_SHIFT;

	if (val)
	{
		sam_eic_irq_disable(G_EXTINTS[samgpint->samgpio.id]);

		gpioinfo("HIGH: pinset before: %x\n", pinset);

		config	= getreg32(SAM_EIC_CONFIG0);
		config &= ~EIC_CONFIG0_SENSE_MASK(4);
		config |= EIC_CONFIG0_SENSE_LOW(4);
		putreg32(config, SAM_EIC_CONFIG0);;
		//sam_eic_config(pin, pinset);

		gpioinfo("HIGH:pinset after: %x\n", pinset);

		gpioinfo("HIGH: INTENSET: %08x\n", (unsigned int)getreg32(SAM_EIC_INTENSET));
		gpioinfo("HIGH: INTFLAG:  %08x\n", (unsigned int)getreg32(SAM_EIC_INTFLAG));
		gpioinfo("HIGH: CONFIG0:  %08x\n", (unsigned int)getreg32(SAM_EIC_CONFIG0));
		gpioinfo("HIGH: CONFIG1:  %08x\n", (unsigned int)getreg32(SAM_EIC_CONFIG1));

		sam_eic_irq_enable(G_EXTINTS[samgpint->samgpio.id]);
	}
	else
	{

		sam_eic_irq_disable(G_EXTINTS[samgpint->samgpio.id]);


		config	= getreg32(SAM_EIC_CONFIG0);
		config &= ~EIC_CONFIG0_SENSE_MASK(4);
		config |= EIC_CONFIG0_SENSE_HIGH(4);
		putreg32(config, SAM_EIC_CONFIG0);


		s am_eic_irq_enable(G_EXTINTS[samgpint->samgpio.id]);                         *

		gpioinfo("LOW: INTENSET: %08x\n", (unsigned int)getreg32(SAM_EIC_INTENSET));
		gpioinfo("LOW: INTFLAG:  %08x\n", (unsigned int)getreg32(SAM_EIC_INTFLAG));
		gpioinfo("LOW: CONFIG0:  %08x\n", (unsigned int)getreg32(SAM_EIC_CONFIG0));
		gpioinfo("LOW: CONFIG1:  %08x\n", (unsigned int)getreg32(SAM_EIC_CONFIG1));
*/
	DEBUGASSERT(samgpint != NULL && samgpint->callback != NULL);
	gpioinfo("Interrupt! callback=%p\n", samgpint->callback);

	samgpint->callback(&samgpint->samgpio.gpio, samgpint->samgpio.id);
	//}


	return OK;
}


/****************************************************************************
 * Name: gpint_read
 *
 * Description:
 *	Read interrupt pin data (true/false)
 *
 ****************************************************************************/

static int gpint_read(	struct gpio_dev_s *dev,
						bool *value)
{
	struct samgpint_dev_s *samgpint = (struct samgpint_dev_s *)dev;

	DEBUGASSERT(samgpint != NULL && value != NULL);
	DEBUGASSERT(samgpint->samgpio.id < BOARD_NGPIOINT);
	gpioinfo("Reading int pin...\n");

	*value = sam_portread(G_INT_INPUTS[samgpint->samgpio.id]);
	return OK;
}


/****************************************************************************
 * Name: gpint_attach
 *
 * Description:
 *	Attaching callback interrupt service routing
 *
 ****************************************************************************/

static int gpint_attach(	struct gpio_dev_s *dev,
							pin_interrupt_t callback)
{
	struct samgpint_dev_s *samgpint = (struct samgpint_dev_s *)dev;
	irqstate_t flags;
	int ret;

	flags = enter_critical_section();

	samgpint->callback = callback;
	if (callback != NULL)
	{
		gpioinfo("Attaching the callback\n");

		/* Make sure the interrupt is disabled */
		irq_detach(G_EXTINTS[samgpint->samgpio.id]);


		ret = irq_attach(	G_EXTINTS[samgpint->samgpio.id],
							samgpio_interrupt,
							&g_gpint[samgpint->samgpio.id]);
		if (ret < 0)
		{
			gpioerr("Callback is not attached!\n");
			leave_critical_section(flags);
			return ret;
		}
		gpioinfo("Attach %p\n", callback);
	}
	else
	{
		gpioinfo("Detaching the callback\n");
		irq_detach(G_EXTINTS[samgpint->samgpio.id]);
	}

	leave_critical_section(flags);
	return OK;
}


/****************************************************************************
 * Name: gpint_enable
 *
 * Description:
 *	Enable corresponding interrupt
 *
 ****************************************************************************/

static int gpint_enable(	struct gpio_dev_s *dev,
							bool enable)
{
	struct samgpint_dev_s *samgpint = (struct samgpint_dev_s *)dev;
	irqstate_t flags;

	flags = enter_critical_section();
	if (enable)
	{
		if (samgpint->callback != NULL)
		{
			gpioinfo("Enabling the interrupt\n");
			sam_eic_irq_enable(G_EXTINTS[samgpint->samgpio.id]);
			//irq_dispatch(SAM_IRQ_EXTINT4, NULL);
		}
		else
		{
			gpioerr("Cannot enable - no callback registered\n");
			leave_critical_section(flags);
			return -EINVAL;
		}
	}
	else
	{
		gpioinfo("Disable the interrupt\n");
		sam_eic_irq_disable(G_EXTINTS[samgpint->samgpio.id]);
	}

	leave_critical_section(flags);
	return OK;
}
#endif


/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sam_gpio_initialize
 *
 * Description:
 *   Initialize GPIO drivers
 *
 ****************************************************************************/

int sam_gpio_initialize(void)
{
	int i;
	int pincount = 0;

	gpioinfo("\nInitializing GPIO!\n");

#if BOARD_NGPIOIN > 0
	for (i = 0; i < BOARD_NGPIOIN; i++)
	{
		/* Setup and register the GPIO pin */

		g_gpin[i].gpio.gp_pintype = GPIO_INPUT_PIN;
		g_gpin[i].gpio.gp_ops     = &gpin_ops;
		g_gpin[i].id              = i;
		gpio_pin_register(&g_gpin[i].gpio, pincount);

		/* Configure the pin that will be used as input */

		sam_configport(G_INPUTS[i]);

		pincount++;
	}
	gpioinfo("Inputs success!\n");
#endif

#if BOARD_NGPIOOUT > 0
	for (i = 0; i < BOARD_NGPIOOUT; i++)
	{
		/* Setup and register the GPIO pin */

		g_gpout[i].gpio.gp_pintype = GPIO_OUTPUT_PIN;
		g_gpout[i].gpio.gp_ops     = &gpout_ops;
		g_gpout[i].id              = i;
		gpio_pin_register(&g_gpout[i].gpio, pincount);

		/* Configure the pin that will be used as output */

		sam_configport(G_OUTPUTS[i]);
		pincount++;
	}
	gpioinfo("Outputs success!\n");
#endif

#if BOARD_NGPIOINT > 0
	for (i = 0; i < BOARD_NGPIOINT; i++)
	{
		memset(&g_gpint[i], 0, sizeof(struct samgpint_dev_s));

		/* Setup and register the GPIO pin */
		g_gpint[i].samgpio.gpio.gp_pintype = GPIO_INTERRUPT_PIN;
		g_gpint[i].samgpio.gpio.gp_ops     = &gpint_ops;
		g_gpint[i].samgpio.id              = i;
		gpio_pin_register(&g_gpint[i].samgpio.gpio, pincount);

		/* Configure the pin that will be used as interrupt input */

		sam_configport(G_INT_INPUTS[i]);

		pincount++;
	}
	gpioinfo("Interrupts success!\n");
#endif

	return 0;
}
#endif /* CONFIG_DEV_GPIO && !CONFIG_GPIO_LOWER_HALF */
