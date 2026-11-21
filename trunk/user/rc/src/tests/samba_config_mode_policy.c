#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "samba_config_mode_policy.h"

int
main(void)
{
	assert(samba_config_mode_is_valid(0)); /* disabled */
	assert(samba_config_mode_is_valid(1)); /* guest/share mode */
	assert(samba_config_mode_is_valid(3)); /* media mode */
	assert(samba_config_mode_is_valid(4)); /* authenticated mode */

	assert(!samba_config_mode_is_valid(2));
	assert(!samba_config_mode_is_valid(-1));
	assert(!samba_config_mode_is_valid(5));
	assert(!samba_config_mode_is_valid(INT_MIN));
	assert(!samba_config_mode_is_valid(INT_MAX));

	puts("Samba configuration mode allowlist: PASS");
	return 0;
}
