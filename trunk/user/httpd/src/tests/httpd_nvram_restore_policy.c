#include <assert.h>

#include "httpd_nvram_restore_policy.h"

int
main(void)
{
	/* Reboot is allowed only after both profile replay and flash commit pass. */
	assert(httpd_nvram_restore_is_committed(0, 0));
	assert(!httpd_nvram_restore_is_committed(-1, 0));
	assert(!httpd_nvram_restore_is_committed(1, 0));
	assert(!httpd_nvram_restore_is_committed(0, -1));
	assert(!httpd_nvram_restore_is_committed(0, 1));
	assert(!httpd_nvram_restore_is_committed(-1, -1));
	return 0;
}
