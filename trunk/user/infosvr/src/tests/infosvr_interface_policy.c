#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "infosvr_interface_policy.h"

int
main(void)
{
	char destination[16];
	char maximum[16];

	memset(destination, 0xa5, sizeof(destination));
	assert(infosvr_interface_name_copy(
		destination, sizeof(destination), "br0") == 0);
	assert(strcmp(destination, "br0") == 0);
	assert(destination[3] == '\0');

	memset(maximum, 'x', sizeof(maximum));
	maximum[sizeof(maximum) - 1] = '\0';
	assert(infosvr_interface_name_copy(
		destination, sizeof(destination), maximum) == 0);
	assert(memcmp(destination, maximum, sizeof(destination)) == 0);

	errno = 0;
	assert(infosvr_interface_name_copy(
		destination, sizeof(destination), "1234567890123456") == -1);
	assert(errno == ENAMETOOLONG);
	errno = 0;
	assert(infosvr_interface_name_copy(destination, sizeof(destination), "") == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(infosvr_interface_name_copy(destination, sizeof(destination), NULL) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(infosvr_interface_name_copy(NULL, sizeof(destination), "br0") == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(infosvr_interface_name_copy(destination, 0, "br0") == -1);
	assert(errno == EINVAL);

	puts("infosvr interface-name bounded copy policy: PASS");
	return 0;
}
