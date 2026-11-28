#ifndef PADAVAN_WATCHDOG_PROCESS_EXECUTABLE_POLICY_H
#define PADAVAN_WATCHDOG_PROCESS_EXECUTABLE_POLICY_H

#include <errno.h>
#include <string.h>

/* /sbin/watchdog is installed as an alias to the single /sbin/rc binary. */
static inline int
watchdog_process_executable_path_matches(const char *actual)
{
	static const char *const expected[] = {
		"/sbin/rc",
		"/sbin/watchdog"
	};
	static const char deleted_suffix[] = " (deleted)";
	size_t i, expected_length, actual_length;

	if (!actual) {
		errno = EINVAL;
		return -1;
	}
	actual_length = strlen(actual);
	for (i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
		expected_length = strlen(expected[i]);
		if (actual_length == expected_length &&
		    memcmp(actual, expected[i], expected_length) == 0)
			return 1;
		if (actual_length == expected_length + sizeof(deleted_suffix) - 1 &&
		    memcmp(actual, expected[i], expected_length) == 0 &&
		    memcmp(actual + expected_length, deleted_suffix,
			   sizeof(deleted_suffix)) == 0)
			return 1;
	}
	return 0;
}

/* A process can choose argv[0] independently of the executable it runs. */
static inline int
watchdog_process_argv0_matches(const char *argv0)
{
	const char *base;

	if (!argv0) {
		errno = EINVAL;
		return -1;
	}
	if (!argv0[0])
		return 0;
	base = strrchr(argv0, '/');
	if (base)
		base++;
	else
		base = argv0;
	return strcmp(base, "watchdog") == 0;
}

#endif /* PADAVAN_WATCHDOG_PROCESS_EXECUTABLE_POLICY_H */
