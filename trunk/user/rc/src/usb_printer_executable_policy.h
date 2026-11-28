#ifndef USB_PRINTER_EXECUTABLE_POLICY_H
#define USB_PRINTER_EXECUTABLE_POLICY_H

#include <errno.h>
#include <string.h>

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

#endif /* USB_PRINTER_EXECUTABLE_POLICY_H */
