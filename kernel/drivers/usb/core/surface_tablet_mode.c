/*
 * Tablet mode detection support for Microsoft Surface tablets.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#include <linux/dmi.h>
#include <linux/input.h>

#include "hub.h"

struct port_info {
	int bus;
	int port;
};

static const struct port_info surface_3 = {
	.bus = 1,
	.port = 3,
};

static const struct port_info surface_pro_3 = {
	.bus = 1,
	.port = 3,
};

static const struct port_info surface_pro_4 = {
	.bus = 1,
	.port = 7,
};

static const struct port_info surface_pro_5 = {
	.bus = 1,
	.port = 7,
};

static const struct port_info surface_pro_6 = {
	.bus = 1,
	.port = 7,
};

static const struct port_info surface_pro_7 = {
	.bus = 3,
	.port = 8,
};

static const struct port_info surface_book = {
	.bus = 1,
	.port = 1,
};

static const struct dmi_system_id devices[] = {
	{
		.ident = "Microsoft Surface 3",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "Surface 3"),
		},
		.driver_data = (void *)&surface_3,
	},
	{
		.ident = "Microsoft Surface Pro 3",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "Surface Pro 3"),
		},
		.driver_data = (void *)&surface_pro_3,
	},
	{
		.ident = "Microsoft Surface Pro 4",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "Surface Pro 4"),
		},
		.driver_data = (void *)&surface_pro_4,
	},
	{
		.ident = "Microsoft Surface Pro 2017",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "Surface Pro"),
		},
		.driver_data = (void *)&surface_pro_5,
	},
	{
		.ident = "Microsoft Surface Pro 6",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "Surface Pro 6"),
		},
		.driver_data = (void *)&surface_pro_6,
	},
	{
		.ident = "Microsoft Surface Pro 7",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "Surface Pro 7"),
		},
		.driver_data = (void *)&surface_pro_7,
	},
	{
		.ident = "Microsoft Surface Book",
		.matches = {
			DMI_EXACT_MATCH(DMI_SYS_VENDOR, "Microsoft Corporation"),
			DMI_EXACT_MATCH(DMI_PRODUCT_NAME, "Surface Book"),
		},
		.driver_data = (void *)&surface_book,
	},
	{}
};

static int surface_tablet_mode_bus = 0;
static int surface_tablet_mode_port = 0;
static struct input_dev *input = NULL;

void surface_tablet_mode_notify(int bus, int port, bool tablet_mode)
{
	if (input && surface_tablet_mode_bus == bus && surface_tablet_mode_port == port) {
		dev_info(input->dev.parent, "Surface Pro Tablet Mode: %d\n", tablet_mode);
		input_report_switch(input, SW_TABLET_MODE, tablet_mode);
		input_sync(input);
	}
}

void surface_tablet_mode_init(struct usb_hub *hub, int bus)
{
	const struct dmi_system_id *dmi_match;
	char *input_name = "Surface Pro Tablet Mode";
	struct port_info *port_data;
	int error;

	dmi_match = dmi_first_match(devices);
	if (!dmi_match)
		return;

	port_data = dmi_match->driver_data;

	if (port_data->bus != bus)
		return;

	surface_tablet_mode_bus = port_data->bus;
	surface_tablet_mode_port = port_data->port;

	input = input_allocate_device();
	if (!input)
		return;

	input->id.bustype = BUS_VIRTUAL;
	input->dev.parent = hub->intfdev;
	input->name = input_name;

	input_set_capability(input, EV_SW, SW_TABLET_MODE);

	error = input_register_device(input);
	if (error)
		goto err_free_input;

	dev_info(input->dev.parent, "Surface Pro Tablet Mode: activated\n");

	surface_tablet_mode_notify(surface_tablet_mode_bus, surface_tablet_mode_port, true);

	return;

 err_free_input:
	input_free_device(input);
}

void surface_tablet_mode_fini(struct usb_hub *hub, int bus)
{
	if (input && surface_tablet_mode_bus == bus)
		input_unregister_device(input);
}