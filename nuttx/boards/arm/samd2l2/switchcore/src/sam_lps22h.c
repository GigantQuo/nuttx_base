/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_lps22h.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>

#include <nuttx/arch.h>

#include <nuttx/i2c/i2c_master.h>
#include <nuttx/sensors/lps25h.h>

#include "arm_internal.h"
#include "sam_pinmap.h"
#include "sam_i2c_master.h"
#include "sam_port.h"

#ifdef CONFIG_SENSORS_LPS25H
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
static int int_sen_attach (	struct lps25h_config_s *state,
							xcpt_t isr,
							FAR void *arg);

static void int_sen_enable (	const struct lps25h_config_s *state,
								bool enable);

static void int_sen_clear (const struct lps25h_config_s *state);

static int dum_set_power (	const struct lps25h_config_s *state,
							bool on);

/****************************************************************************
 * Private Data
 ****************************************************************************/
static lps25h_config_t lps22h_config =
{
	.irq = LPS22H_EXTINT,			/* IRQ number */
	.irq_attach = int_sen_attach,	/* Attaching callback */
	.irq_enable = int_sen_enable,	/* Enabling interrupt */
	.irq_clear = int_sen_clear,		/* Clear pending interrupt */
	.set_power = dum_set_power		/* Turn on LPS22H */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: int_sen_attach
 *
 * Description:
 *	Attaching callback interrupt service routing
 *
 ****************************************************************************/

static int int_sen_attach (	struct lps25h_config_s *state,
							xcpt_t isr,
							FAR void *arg)
{
	struct lps25h_config_s* int_sen = (struct lps25h_config_s*)state;
	irqstate_t flags;

	gpioinfo("Attaching the callback\n");

	flags = enter_critical_section();

	int ret = irq_attach((int)int_sen->irq, isr, arg);
	if (ret == OK)
	{
		/* Configure the interrupt */
		sam_configport(PORT_LPS22H_IRQ);
	}

	leave_critical_section(flags);
	gpioinfo("Attach %p\n", isr);

	return OK;
}

/****************************************************************************
 * Name: int_sen_enable
 *
 * Description:
 *	Enable corresponding interrupt
 *
 ****************************************************************************/

static void int_sen_enable(	const struct lps25h_config_s *state,
								bool enable)
{
	struct lps25h_config_s* int_sen = (struct lps25h_config_s*)state;
	irqstate_t flags;

	flags = enter_critical_section();

	if (enable)
	{
		if (int_sen != NULL)
		{
			gpioinfo("Enabling the interrupt\n");
			up_enable_irq((int)int_sen->irq);
		}
	}
	else
	{
			up_disable_irq((int)int_sen->irq);
			gpioinfo("Disable the interrupt\n");
	}

	leave_critical_section(flags);
}

/****************************************************************************
 * Name: int_sen_clear
 *
 * Description:
 *	Clear corresponding pending interrupt
 *
 ****************************************************************************/

static void int_sen_clear(const struct lps25h_config_s *state)
{
	struct lps25h_config_s* int_sen = (struct lps25h_config_s*)state;

	arm_ack_irq((int)int_sen->irq);
}

/****************************************************************************
 * Name: dum_set_power
 *
 * Description:
 *	Dummy function does nothing, because the LPS22H always powered on
 *
 ****************************************************************************/

static int dum_set_power (	const struct lps25h_config_s *state,
							bool on)
{
	return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_lps22h_initialize
 *
 * Description:
 *   Initialize and register the LPS22H Pressure Sensor driver.
 *
 * Input Parameters:
 *   devno - The device number, used to build the device
 *   busno - The I2C bus number
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int board_lps22h_initialize (	struct i2c_master_s* i2c,
								int devno,
								int busno)
{
	char devname[32];
	int ret;

	sninfo("\nInitializing LPS22H!\n");

	/* Initialize LPS22H */
	if (i2c)
	{
		snprintf(devname, sizeof(devname), "/dev/uorb/sensor_press%d", devno);
		ret = lps25h_register(	devname,
								i2c,
								(uint8_t)0x5D,
								&lps22h_config);
		if (ret < 0)
		{
			snerr("ERROR: Error registering LPS22H in I2C%d\n", busno);
		}
	}
	else
	{
		ret = -ENODEV;
	}
	return ret;
}

#endif /* CONFIG_SENSORS_LPS25H */
