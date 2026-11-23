#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "logger_start_policy.h"

int
main(void)
{
	char address[16];
	int port;

	assert(rc_logger_start_status(0, 0) == 0);
	assert(rc_logger_start_status(17, 0) == 17);
	assert(rc_logger_start_status(0, 23) == 23);
	assert(rc_logger_start_status(17, 23) == 17);
	assert(rc_logger_start_status(-1, 0) == -1);
	assert(rc_logger_start_status(0, -1) == -1);
	assert(rc_logger_start_status_with_wait(0, 0, 0) == 0);
	assert(rc_logger_start_status_with_wait(17, 23, 29) == 17);
	assert(rc_logger_start_status_with_wait(0, 23, 29) == 23);
	assert(rc_logger_start_status_with_wait(0, 0, 29) == 29);
	assert(rc_logger_start_status_with_wait(-1, 23, 29) == -1);
	assert(rc_logger_start_status_with_wait(0, -1, 29) == -1);

	assert(rc_logger_nvram_address_status(1, 0, "192.0.2.10", address, sizeof(address)) == 0);
	assert(strcmp(address, "192.0.2.10") == 0);
	assert(rc_logger_nvram_address_status(0, 0, NULL, address, sizeof(address)) == 0);
	assert(address[0] == '\0');
	assert(rc_logger_nvram_address_status(-1, EACCES, "ignored", address, sizeof(address)) == EACCES);
	assert(address[0] == '\0');
	assert(rc_logger_nvram_address_status(-1, 0, NULL, address, sizeof(address)) == EIO);
	assert(rc_logger_nvram_address_status(0, 0, "unexpected", address, sizeof(address)) == EIO);
	assert(rc_logger_nvram_address_status(1, 0, NULL, address, sizeof(address)) == EIO);
	assert(rc_logger_nvram_address_status(1, 0, "192.0.2.10-extra", address, sizeof(address)) == 0);
	assert(address[0] == '\0');
	assert(rc_logger_nvram_address_status(1, 0, "192.0.2.10", NULL, 16) == EINVAL);
	assert(rc_logger_nvram_address_status(1, 0, "192.0.2.10", address, 0) == EINVAL);

	assert(rc_logger_nvram_port_status(0, 0, NULL, 514, 1, 65535, &port) == 0);
	assert(port == 514);
	assert(rc_logger_nvram_port_status(1, 0, "1514", 514, 1, 65535, &port) == 0);
	assert(port == 1514);
	assert(rc_logger_nvram_port_status(1, 0, "1514tail", 514, 1, 65535, &port) == 0);
	assert(port == 1514);
	assert(rc_logger_nvram_port_status(1, 0, "", 514, 1, 65535, &port) == 0);
	assert(port == 514);
	assert(rc_logger_nvram_port_status(1, 0, "invalid", 514, 1, 65535, &port) == 0);
	assert(port == 514);
	assert(rc_logger_nvram_port_status(1, 0, "0", 514, 1, 65535, &port) == 0);
	assert(port == 514);
	assert(rc_logger_nvram_port_status(1, 0, "65536", 514, 1, 65535, &port) == 0);
	assert(port == 514);
	assert(rc_logger_nvram_port_status(1, 0, "-1", 514, 1, 65535, &port) == 0);
	assert(port == 514);
	assert(rc_logger_nvram_port_status(1, 0, "99999999999999999999", 514, 1, 65535, &port) == 0);
	assert(port == 514);
	assert(rc_logger_nvram_port_status(-1, EIO, "1514", 514, 1, 65535, &port) == EIO);
	assert(port == 514);
	assert(rc_logger_nvram_port_status(-1, 0, NULL, 514, 1, 65535, &port) == EIO);
	assert(rc_logger_nvram_port_status(0, 0, "bad", 514, 1, 65535, &port) == EIO);
	assert(rc_logger_nvram_port_status(1, 0, NULL, 514, 1, 65535, &port) == EIO);
	assert(rc_logger_nvram_port_status(7, 0, "1514", 514, 1, 65535, &port) == EIO);
	assert(rc_logger_nvram_port_status(1, 0, "1514", 0, 1, 65535, &port) == EINVAL);
	assert(rc_logger_nvram_port_status(1, 0, "1514", 514, 65535, 1, &port) == EINVAL);
	assert(rc_logger_nvram_port_status(1, 0, "1514", 514, 1, 65535, NULL) == EINVAL);

	puts("RC logger startup, remote NVRAM, and first-failure policies: PASS");
	return 0;
}
