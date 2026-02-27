/****************************************************************************
 * boards/arm/samd2l2/switchore/src/sam_max662.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/board/board.h>
#include "switchcore.h"

#include <debug.h>
#include <stdio.h>

#include <nuttx/i2c/i2c_master.h>
#include <nuttx/sensors/max662.h>

#include "arm_internal.h"
#include "sam_pinmap.h"
#include "sam_i2c_master.h"
#include "sam_port.h"

#ifdef CONFIG_SENSORS_MAX662
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
static int OT_attach (		struct max662_config_s *state,
							xcpt_t isr,
							FAR void *arg);

static void OT_enable (		const struct max662_config_s *state,
							bool enable);

static void OT_clear (		const struct max662_config_s *state);

static bool OT_get_state (	const struct max662_config_s *state);

/****************************************************************************
 * Private Data
 ****************************************************************************/
static max662_config_t max662_config =
{
	.irq = MAX662_EXTINT,			/* IRQ number */
	.irq_attach = OT_attach,	/* Attaching callback */
	.irq_enable = OT_enable,	/* Enabling interrupt */
	.irq_clear = OT_clear,		/* Clear pending interrupt */
	.get_state = OT_get_state		/* Turn on LPS22H */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: OT_attach
 *
 * Description:
 *	Attaching callback interrupt service routing
 *
 ****************************************************************************/

static int OT_attach (		struct max662_config_s *state,
							xcpt_t isr,
							FAR void *arg)
{
	struct max662_config_s* ot = (struct max662_config_s*)state;
	irqstate_t flags;

	gpioinfo("Attaching the callback\n");

	flags = enter_critical_section();

	int ret = irq_attach((int)ot->irq, isr, arg);
	if (ret == OK)
	{
		/* Configure the interrupt */
		sam_configport(PORT_MAX662_IRQ);
	}

	leave_critical_section(flags);
	gpioinfo("Attach %p\n", isr);

	return OK;
}

/****************************************************************************
 * Name: OT_enable
 *
 * Description:
 *	Enable corresponding interrupt
 *
 ****************************************************************************/

static void OT_enable (	const struct max662_config_s *state,
						bool enable)
{
	struct max662_config_s* ot = (struct max662_config_s*)state;
	irqstate_t flags;

	flags = enter_critical_section();

	if (enable)
	{
		if (ot != NULL)
		{
			gpioinfo("Enabling the interrupt\n");
			up_enable_irq((int)ot->irq);
		}
	}
	else
	{
			up_disable_irq((int)ot->irq);
			gpioinfo("Disable the interrupt\n");
	}

	leave_critical_section(flags);
}

/****************************************************************************
 * Name: OT_clear
 *
 * Description:
 *	Clear corresponding pending interrupt
 *
 ****************************************************************************/

static void OT_clear (const struct max662_config_s *state)
{
	struct max662_config_s* ot = (struct max662_config_s*)state;

	arm_ack_irq((int)ot->irq);
}

/****************************************************************************
 * Name: OT_get_state
 *
 * Description:
 *	Get current logic level on interrup OT input
 *
 ****************************************************************************/

static bool OT_get_state (	const struct max662_config_s *state)
{
	bool pin_state;

	pin_state = sam_portread(PORT_MAX662_IRQ);

	return pin_state;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_max662_initialize
 *
 * Description:
 *   Initialize and register the MAX662 Temperature Sensor driver.
 *
 * Input Parameters:
 *   devno - The device number, used to build the device
 *   busno - The I2C bus number
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int board_max662_initialize (	struct i2c_master_s* i2c,
								int devno,
								int busno)
{
	char devname[32];
	int ret;

	sninfo("\nInitializing MAX662!\n");

	/* Initialize MAX622 */
	if (i2c)
	{
		snprintf(devname, sizeof(devname), "/dev/uorb/sensor_ambient_temp%d", devno);

		ret = max662_register(	devname,
								i2c,
								&max662_config);
		if (ret < 0)
		{
			snerr("ERROR: Error registering MAX662 in I2C%d\n", busno);
		}
	}
	else
	{
		ret = -ENODEV;
	}
	return ret;
}

#endif /* CONFIG_SENSORS_MAX662 */
