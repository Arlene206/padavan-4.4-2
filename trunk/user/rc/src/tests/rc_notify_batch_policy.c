#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include "rc_notify_batch_policy.h"


static void
test_deferred_hotplug_event_retry_mapping(void)
{
	assert(rc_notify_hotplug_retry_flag("on_hotplug_usb_printer") == RC_NOTIFY_HOTPLUG_RETRY_PRINTER_PLUG);
	assert(rc_notify_hotplug_retry_flag("on_unplug_usb_printer") == RC_NOTIFY_HOTPLUG_RETRY_PRINTER_UNPLUG);
	assert(rc_notify_hotplug_retry_flag("on_hotplug_usb_modem") == RC_NOTIFY_HOTPLUG_RETRY_MODEM_PLUG);
	assert(rc_notify_hotplug_retry_flag("on_unplug_usb_modem") == RC_NOTIFY_HOTPLUG_RETRY_MODEM_UNPLUG);
	assert(rc_notify_hotplug_retry_flag("on_hotplug_mass_storage") == RC_NOTIFY_HOTPLUG_RETRY_STORAGE_PLUG);
	assert(rc_notify_hotplug_retry_flag("on_unplug_mass_storage") == 0);
	assert(rc_notify_hotplug_retry_flag("restart_wan") == 0);
	assert(rc_notify_hotplug_retry_flag(NULL) == 0);
}

static void
test_deferred_hotplug_retry_lifecycle(void)
{
	unsigned int pending = 0;
	unsigned int printer, modem;

	pending = rc_notify_hotplug_retry_record(
		pending, "on_hotplug_usb_printer", EIO);
	pending = rc_notify_hotplug_retry_record(
		pending, "on_unplug_usb_modem", -1);
	assert(pending == (RC_NOTIFY_HOTPLUG_RETRY_PRINTER_PLUG |
			   RC_NOTIFY_HOTPLUG_RETRY_MODEM_UNPLUG));

	pending = rc_notify_hotplug_retry_record(
		pending, "on_hotplug_usb_printer", 0);
	assert(pending == (RC_NOTIFY_HOTPLUG_RETRY_PRINTER_PLUG |
			   RC_NOTIFY_HOTPLUG_RETRY_MODEM_UNPLUG));

	printer = RC_NOTIFY_HOTPLUG_RETRY_PRINTER_PLUG;
	modem = RC_NOTIFY_HOTPLUG_RETRY_MODEM_UNPLUG;
	assert(rc_notify_hotplug_retry_complete(pending, printer, 0) == modem);
	assert(rc_notify_hotplug_retry_complete(pending, printer, -1) == pending);
	assert(rc_notify_hotplug_retry_complete(pending, printer | modem, 0) == 0);
}

int
main(void)
{
	assert(!rc_notify_batch_should_requeue(0, 1, 0));
	assert(!rc_notify_batch_should_requeue(9, 1, 0));
	assert(!rc_notify_batch_should_requeue(10, 0, 0));
	assert(rc_notify_batch_should_requeue(10, 1, 0));
	assert(rc_notify_batch_should_requeue(11, 1, 0));
	assert(!rc_notify_batch_should_requeue(10, 1, 1));
	assert(!rc_notify_batch_should_requeue(100, 1, 1));
	assert(rc_notify_merge_action_result(0, 0) == 0);
	assert(rc_notify_merge_action_result(0, EBUSY) == -1);
	assert(rc_notify_merge_action_result(0, EIO) == -1);
	assert(rc_notify_merge_action_result(0, 256) == -1);
	assert(rc_notify_merge_action_result(0, -1) == -1);
	assert(rc_notify_merge_action_result(-1, 0) == -1);
	assert(rc_notify_merge_action_result(-1, EBUSY) == -1);
	test_deferred_hotplug_event_retry_mapping();
	test_deferred_hotplug_retry_lifecycle();

	return 0;
}
