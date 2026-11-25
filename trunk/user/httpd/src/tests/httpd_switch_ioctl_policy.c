#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "../httpd_switch_ioctl_policy.h"

int main(void)
{
	uint64_t rx = 0x1122334455667788ULL;
	uint64_t tx = 0x8877665544332211ULL;

	assert(httpd_switch_normalize_result(0, 0) == 0);
	assert(httpd_switch_normalize_result(1, 0) == 0);
	assert(httpd_switch_normalize_result(-1, EACCES) == -EACCES);
	assert(httpd_switch_normalize_result(-1, 0) == -EIO);

	assert(httpd_switch_choose_result(-EIO, -EBADF) == -EIO);
	assert(httpd_switch_choose_result(0, -EBADF) == -EBADF);
	assert(httpd_switch_choose_result(0, 0) == 0);

	assert(httpd_switch_publish_port_bytes(-EIO, 10, 20, &rx, &tx) == -EIO);
	assert(rx == 0x1122334455667788ULL);
	assert(tx == 0x8877665544332211ULL);
	assert(httpd_switch_publish_port_bytes(0, 10, 20, &rx, &tx) == 0);
	assert(rx == 10 && tx == 20);
	assert(httpd_switch_publish_port_bytes(0, 30, 40, NULL, &tx) == -EINVAL);
	assert(httpd_switch_publish_port_bytes(0, 30, 40, &rx, NULL) == -EINVAL);

	puts("HTTPD switch ioctl errors and counter publication policy: PASS");
	return 0;
}
