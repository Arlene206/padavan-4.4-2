#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "ftp_transfer_rate_policy.h"

int
main(void)
{
	int rate = 99;

	/* Missing key defaults to unlimited (zero), preserving legacy behavior. */
	assert(ftp_transfer_rate_value_status(0, 0, 10000, &rate) == 0);
	assert(rate == 0);

	/* Preserve in-range values and the legacy numeric clamps. */
	assert(ftp_transfer_rate_value_status(0, 1, 10000, &rate) == 0);
	assert(rate == 1);
	assert(ftp_transfer_rate_value_status(0, 10000, 10000, &rate) == 0);
	assert(rate == 10000);
	assert(ftp_transfer_rate_value_status(0, -1, 10000, &rate) == 0);
	assert(rate == 0);
	assert(ftp_transfer_rate_value_status(0, 20000, 10000, &rate) == 0);
	assert(rate == 10000);

	/* Backend/read/close errors must never become unlimited transfer. */
	assert(ftp_transfer_rate_value_status(EIO, 0, 10000, &rate) == EIO);
	assert(rate == 0);
	assert(ftp_transfer_rate_value_status(-1, 0, 10000, &rate) == EIO);
	assert(rate == 0);
	assert(ftp_transfer_rate_value_status(0, 10, -1, &rate) == EINVAL);
	assert(rate == 0);
	assert(ftp_transfer_rate_value_status(0, 10, 10000, NULL) == EINVAL);

	puts("FTP anonymous transfer-rate checked-read policy: PASS");
	return 0;
}
