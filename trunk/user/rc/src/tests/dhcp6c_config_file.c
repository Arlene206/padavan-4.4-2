#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dhcp6c_config_file.h"

int
main(void)
{
	char directory[] = "/tmp/dhcp6c-config-test.XXXXXX";
	char target[256], staged[256], failed_stage[256], bad_stage[256];
	char buffer[128];
	FILE *stream = NULL;
	struct stat st;
	mode_t saved_umask;
	int fd;
	size_t length;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(target, sizeof(target), "%s/dhcp6c.conf", directory) > 0);
	assert(snprintf(staged, sizeof(staged), "%s/.dhcp6c.conf.XXXXXX", directory) > 0);
	assert(snprintf(failed_stage, sizeof(failed_stage),
			"%s/.dhcp6c-failed.XXXXXX", directory) > 0);
	assert(snprintf(bad_stage, sizeof(bad_stage),
			"%s/missing/.dhcp6c.conf.XXXXXX", directory) > 0);

	/* Even a permissive PPP callback umask must not expose the new config. */
	saved_umask = umask(0000);
	fd = open(target, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	assert(write(fd, "old configuration\n", 18) == 18);
	assert(close(fd) == 0);

	assert(dhcp6c_config_file_open_staged(staged, &stream) == 0);
	assert(stream != NULL);
	assert(lstat(staged, &st) == 0);
	assert(S_ISREG(st.st_mode));
	assert((st.st_mode & 0777) == 0600);
	assert(fputs("new configuration\n", stream) >= 0);
	assert(dhcp6c_config_file_publish(stream, staged, target) == 0);
	stream = NULL;

	assert(lstat(target, &st) == 0);
	assert(S_ISREG(st.st_mode));
	assert((st.st_mode & 0777) == 0600);
	assert(access(staged, F_OK) != 0 && errno == ENOENT);
	stream = fopen(target, "r");
	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	assert(fclose(stream) == 0);
	buffer[length] = '\0';
	assert(strcmp(buffer, "new configuration\n") == 0);

	/* A failed rename must discard the staged file, not the active config. */
	assert(dhcp6c_config_file_open_staged(failed_stage, &stream) == 0);
	assert(fputs("unpublished configuration\n", stream) >= 0);
	assert(dhcp6c_config_file_publish(stream, failed_stage, directory) == -1);
	stream = NULL;
	assert(access(failed_stage, F_OK) != 0 && errno == ENOENT);
	stream = fopen(target, "r");
	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	assert(fclose(stream) == 0);
	buffer[length] = '\0';
	assert(strcmp(buffer, "new configuration\n") == 0);

	/* Invalid staged locations fail before publishing anything. */
	assert(dhcp6c_config_file_open_staged(bad_stage, &stream) == -1);
	assert(stream == NULL);

	assert(unlink(target) == 0);
	assert(rmdir(directory) == 0);
	umask(saved_umask);

	puts("DHCPv6 config private staging, atomic replacement and failure cleanup: PASS");
	return 0;
}
