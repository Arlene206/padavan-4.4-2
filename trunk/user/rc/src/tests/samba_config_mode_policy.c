#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "samba_config_mode_policy.h"

int
main(void)
{
	int enabled = -1;

	assert(samba_config_mode_is_valid(0)); /* disabled */
	assert(samba_config_mode_is_valid(1)); /* guest/share mode */
	assert(samba_config_mode_is_valid(3)); /* media mode */
	assert(samba_config_mode_is_valid(4)); /* authenticated mode */
	assert(!samba_config_mode_is_valid(2));
	assert(!samba_config_mode_is_valid(-1));
	assert(!samba_config_mode_is_valid(5));
	assert(!samba_config_mode_is_valid(INT_MIN));
	assert(!samba_config_mode_is_valid(INT_MAX));

	assert(samba_boolean_value_status(EIO, 0, &enabled) == EIO);
	assert(enabled == 0);
	assert(samba_boolean_value_status(-1, 1, &enabled) == EIO);
	assert(samba_boolean_value_status(0, 2, &enabled) == EINVAL);
	assert(enabled == 0);
	assert(samba_boolean_value_status(0, 1, &enabled) == 0 && enabled == 1);
	assert(samba_boolean_value_status(0, 0, &enabled) == 0 && enabled == 0);
	assert(samba_boolean_value_status(0, 1, NULL) == EINVAL);

	/* Disabled global switch must not depend on an unreadable mode key. */
	assert(samba_runtime_enabled_status(0, 0, EIO, 99, &enabled) == 0);
	assert(enabled == 0);
	assert(samba_runtime_enabled_status(EIO, 1, 0, 1, &enabled) == EIO);
	assert(enabled == 0);
	assert(samba_runtime_enabled_status(0, 2, 0, 1, &enabled) == EINVAL);
	assert(enabled == 0);
	assert(samba_runtime_enabled_status(0, 1, EIO, 1, &enabled) == EIO);
	assert(enabled == 0);
	assert(samba_runtime_enabled_status(0, 1, 0, 2, &enabled) == EINVAL);
	assert(enabled == 0);
	assert(samba_runtime_enabled_status(0, 1, 0, 0, &enabled) == 0);
	assert(enabled == 0);
	assert(samba_runtime_enabled_status(0, 1, 0, 1, &enabled) == 0);
	assert(enabled == 1);
	assert(samba_runtime_enabled_status(0, 1, 0, 3, &enabled) == 0);
	assert(enabled == 1);
	assert(samba_runtime_enabled_status(0, 1, 0, 4, &enabled) == 0);
	assert(enabled == 1);
	assert(samba_runtime_enabled_status(0, 1, 0, 1, NULL) == EINVAL);

	puts("Samba configuration mode and checked enable policy: PASS");
	return 0;
}
