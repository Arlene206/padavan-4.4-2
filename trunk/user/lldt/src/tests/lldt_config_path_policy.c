#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "lldt_config_path_policy.h"

int
main(void)
{
	char relative[14];
	char absolute[14];
	char short_buffer[13] = "keep";
	char saved[sizeof(short_buffer)];
	char long_value[256];
	char long_path[261];

	assert(lldt_config_path_copy("icon.ico", relative, sizeof(relative)) == 0);
	assert(strcmp(relative, "/etc/icon.ico") == 0);
	assert(relative[sizeof(relative) - 1] == '\0');

	assert(lldt_config_path_copy("/tmp/icon.ico", absolute,
	                             sizeof(absolute)) == 0);
	assert(strcmp(absolute, "/tmp/icon.ico") == 0);

	memcpy(saved, short_buffer, sizeof(saved));
	errno = 0;
	assert(lldt_config_path_copy("icon.ico", short_buffer,
	                             sizeof(short_buffer)) == -1);
	assert(errno == ENAMETOOLONG);
	assert(memcmp(short_buffer, saved, sizeof(saved)) == 0);

	errno = 0;
	assert(lldt_config_path_copy("", absolute, sizeof(absolute)) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(lldt_config_path_copy(NULL, absolute, sizeof(absolute)) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(lldt_config_path_copy("icon.ico", NULL, 14) == -1);
	assert(errno == EINVAL);

	memset(long_value, 'x', sizeof(long_value) - 1);
	long_value[sizeof(long_value) - 1] = '\0';
	assert(lldt_config_path_copy(long_value, long_path,
	                             sizeof(long_path)) == 0);
	assert(strncmp(long_path, "/etc/", 5) == 0);
	assert(strlen(long_path) == 260);
	assert(long_path[sizeof(long_path) - 1] == '\0');

	puts("LLTD bounded config icon path regression: PASS");
	return 0;
}
