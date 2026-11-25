#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "ftp_transfer_rate_policy.h"

static void
test_rate_setting_is_read_only_for_modes_that_emit_anon_max_rate(void)
{
	assert(ftp_transfer_rate_setting_required(3));
	assert(ftp_transfer_rate_setting_required(4));
	assert(!ftp_transfer_rate_setting_required(1));
	assert(!ftp_transfer_rate_setting_required(2));
	assert(!ftp_transfer_rate_setting_required(0));
	assert(!ftp_transfer_rate_setting_required(5));
}

int
main(void)
{
	int rate = 99;

	test_rate_setting_is_read_only_for_modes_that_emit_anon_max_rate();

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
