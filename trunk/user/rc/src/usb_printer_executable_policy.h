#ifndef USB_PRINTER_EXECUTABLE_POLICY_H
#define USB_PRINTER_EXECUTABLE_POLICY_H

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <process_inventory.h>

/*
 * These are dedicated ROMFS executables. p910nd intentionally changes argv[0]
 * to p9100d..p9109d, so the requested process title is not always the
 * executable name. Resolve that alias explicitly instead of trusting argv[0]
 * as proof that an unrelated process is a printer daemon.
 */
static inline const char *
usb_printer_executable_path(const char *process_name)
{
	if (!process_name || !process_name[0]) {
		errno = EINVAL;
		return NULL;
	}
	if (strcmp(process_name, "u2ec") == 0)
		return "/usr/sbin/u2ec";
	if (strcmp(process_name, "lpd") == 0)
		return "/usr/sbin/lpd";
	if (strcmp(process_name, "p910nd") == 0)
		return "/usr/sbin/p910nd";
	if (process_name[0] == 'p' && process_name[1] == '9' &&
	    process_name[2] == '1' && process_name[3] == '0' &&
	    process_name[4] >= '0' && process_name[4] <= '9' &&
	    process_name[5] == 'd' && process_name[6] == '\0')
		return "/usr/sbin/p910nd";

	errno = ENOENT;
	return NULL;
}

/* readlink("/proc/PID/exe") appends this suffix for a still-running,
 * unlinked executable; it remains the same daemon binary for identity. */
static inline int
usb_printer_executable_path_matches(const char *expected, const char *actual)
{
	static const char deleted_suffix[] = " (deleted)";
	size_t expected_length, actual_length;

	if (!expected || !actual) {
		errno = EINVAL;
		return -1;
	}
	if (strcmp(expected, actual) == 0)
		return 1;

	expected_length = strlen(expected);
	actual_length = strlen(actual);
	if (actual_length == expected_length + sizeof(deleted_suffix) - 1 &&
	    memcmp(actual, expected, expected_length) == 0 &&
	    memcmp(actual + expected_length, deleted_suffix,
		   sizeof(deleted_suffix)) == 0)
		return 1;
	return 0;
}


typedef int (*usb_printer_identity_match_fn)(
	void *context, const struct process_identity *identity, int *check_error);

/*
 * Filter a process-name inventory down to processes whose executable identity
 * has been confirmed. Any unreadable candidate makes the result UNKNOWN: a
 * stop transaction must not signal a partially understood process set.
 */
static inline enum process_presence
usb_printer_filter_process_snapshot(
	const struct process_snapshot *candidates,
	struct process_snapshot *verified,
	usb_printer_identity_match_fn identity_matches,
	void *context)
{
	struct process_identity *items;
	size_t i, matched_count = 0;
	int matched, check_error, unknown_error = 0;

	if (!verified) {
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	memset(verified, 0, sizeof(*verified));
	verified->state = PROCESS_PRESENCE_UNKNOWN;
	if (!candidates || candidates == verified || !identity_matches) {
		verified->error = EINVAL;
		errno = EINVAL;
		return verified->state;
	}
	if (candidates->state == PROCESS_PRESENCE_UNKNOWN) {
		verified->error = candidates->error > 0 ? candidates->error : EIO;
		errno = verified->error;
		return verified->state;
	}
	if (candidates->state == PROCESS_PRESENCE_ABSENT) {
		if (candidates->count != 0 || candidates->items) {
			verified->error = EIO;
			errno = EIO;
			return verified->state;
		}
		verified->state = PROCESS_PRESENCE_ABSENT;
		return verified->state;
	}
	if (candidates->state != PROCESS_PRESENCE_RUNNING ||
	    !candidates->items || candidates->count == 0 ||
	    candidates->count > (size_t)-1 / sizeof(*items)) {
		verified->error = candidates->count > (size_t)-1 / sizeof(*items) ?
			EOVERFLOW : EIO;
		errno = verified->error;
		return verified->state;
	}

	items = (struct process_identity *)calloc(candidates->count,
						 sizeof(*items));
	if (!items) {
		verified->error = ENOMEM;
		errno = ENOMEM;
		return verified->state;
	}
	for (i = 0; i < candidates->count; i++) {
		check_error = 0;
		matched = identity_matches(context, &candidates->items[i],
					   &check_error);
		if (matched > 0)
			items[matched_count++] = candidates->items[i];
		else if (matched < 0 && unknown_error == 0)
			unknown_error = check_error > 0 ? check_error : EIO;
	}
	if (unknown_error != 0) {
		free(items);
		verified->error = unknown_error;
		errno = unknown_error;
		return verified->state;
	}
	if (matched_count == 0) {
		free(items);
		verified->state = PROCESS_PRESENCE_ABSENT;
		return verified->state;
	}
	verified->items = items;
	verified->count = matched_count;
	verified->state = PROCESS_PRESENCE_RUNNING;
	verified->error = 0;
	return verified->state;
}

#endif /* USB_PRINTER_EXECUTABLE_POLICY_H */
