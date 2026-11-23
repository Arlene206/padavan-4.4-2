#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "storage_service_enable_policy.h"

int
main(void)
{
	int enabled = 99;

	/* Missing/disabled values preserve nvram_match(..., "1") semantics. */
	assert(storage_service_enable_value_status(0, "", &enabled) == 0);
	assert(enabled == 0);
	assert(storage_service_enable_value_status(0, "0", &enabled) == 0);
	assert(enabled == 0);
	assert(storage_service_enable_value_status(0, "01", &enabled) == 0);
	assert(enabled == 0);

	assert(storage_service_enable_value_status(0, "1", &enabled) == 0);
	assert(enabled == 1);

	/* Backend/read/close failures never collapse into a successful disabled state. */
	assert(storage_service_enable_value_status(EIO, "", &enabled) == EIO);
	assert(enabled == 0);
	assert(storage_service_enable_value_status(-1, "1", &enabled) == EIO);
	assert(enabled == 0);
	assert(errno == EIO);

	assert(storage_service_enable_value_status(0, NULL, &enabled) == EINVAL);
	assert(enabled == 0);
	assert(storage_service_enable_value_status(0, "1", NULL) == EINVAL);

	puts("Storage service checked enable policy: PASS");
	return 0;
}
