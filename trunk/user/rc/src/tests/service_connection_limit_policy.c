#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "service_connection_limit_policy.h"

int
main(void)
{
	int connections = -1;

	/* Preserve the historical clamp for valid and out-of-range values. */
	assert(service_connection_limit_value_status(0, 7, 16, &connections) == 0);
	assert(connections == 7);
	assert(service_connection_limit_value_status(0, 0, 16, &connections) == 0);
	assert(connections == 1);
	assert(service_connection_limit_value_status(0, -1, 16, &connections) == 0);
	assert(connections == 1);
	assert(service_connection_limit_value_status(0, 99, 16, &connections) == 0);
	assert(connections == 16);

	/* Read/API/close failures must not be mistaken for a one-client limit. */
	assert(service_connection_limit_value_status(EIO, 0, 16, &connections) == EIO);
	assert(connections == 0);
	assert(service_connection_limit_value_status(-1, 10, 16, &connections) == EIO);
	assert(connections == 0);

	/* Invalid metadata and missing output pointers fail closed. */
	assert(service_connection_limit_value_status(0, 10, 0, &connections) == EINVAL);
	assert(service_connection_limit_value_status(0, 10, -1, &connections) == EINVAL);
	assert(service_connection_limit_value_status(0, 10, 16, NULL) == EINVAL);

	puts("Shared FTP/Samba connection-limit read policy: PASS");
	return 0;
}
