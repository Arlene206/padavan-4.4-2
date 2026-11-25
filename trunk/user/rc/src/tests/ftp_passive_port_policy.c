#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "../ftp_passive_port_policy.h"

int
main(void)
{
	int minimum, maximum;

	/* Missing NVRAM values retain the historic defaults. */
	assert(ftp_passive_port_range_status(0, 50000, 0, 50100,
					     &minimum, &maximum) == 0);
	assert(minimum == 50000 && maximum == 50100);

	/* One-port and full-range boundaries remain supported. */
	assert(ftp_passive_port_range_status(0, 50000, 0, 50000,
					     &minimum, &maximum) == 0);
	assert(minimum == 50000 && maximum == 50000);
	assert(ftp_passive_port_range_status(0, 1, 0, 65535,
					     &minimum, &maximum) == 0);
	assert(minimum == 1 && maximum == 65535);

	/* Retain the legacy numeric clamp, but reject an inverted range. */
	assert(ftp_passive_port_range_status(0, INT_MIN, 0, INT_MAX,
					     &minimum, &maximum) == 0);
	assert(minimum == 1 && maximum == 65535);
	minimum = 123;
	maximum = 456;
	assert(ftp_passive_port_range_status(0, 51000, 0, 50000,
					     &minimum, &maximum) == EINVAL);
	assert(minimum == 0 && maximum == 0);

	/* Backend/read errors cannot be replaced by defaults. */
	assert(ftp_passive_port_range_status(EIO, 50000, 0, 50100,
					     &minimum, &maximum) == EIO);
	assert(minimum == 0 && maximum == 0);
	assert(ftp_passive_port_range_status(0, 50000, EACCES, 50100,
					     &minimum, &maximum) == EACCES);
	assert(minimum == 0 && maximum == 0);
	assert(ftp_passive_port_range_status(0, 50000, 0, 50100,
					     NULL, &maximum) == EINVAL);

	puts("FTP passive-port range validation and checked-status propagation: PASS");
	return 0;
}
