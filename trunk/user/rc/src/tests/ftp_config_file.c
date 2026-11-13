#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../ftp_config_file.h"

static void
read_text_file(const char *path, char *buffer, size_t buffer_size)
{
	FILE *fp;
	size_t length;

	assert(buffer_size > 0);
	fp = fopen(path, "r");
	assert(fp != NULL);
	length = fread(buffer, 1, buffer_size - 1, fp);
	assert(!ferror(fp));
	buffer[length] = '\0';
	assert(fclose(fp) == 0);
}

int
main(void)
{
	char dir[] = "/tmp/ftp-config-file.XXXXXX";
	char target[512];
	char missing_target[512];
	char staged[512];
	char short_staged[4];
	char contents[128];
	FILE *fp;
	struct stat st;
	int path_len;

	assert(mkdtemp(dir) != NULL);
	path_len = snprintf(target, sizeof(target), "%s/vsftpd.conf", dir);
	assert(path_len > 0 && (size_t)path_len < sizeof(target));

	fp = fopen(target, "w");
	assert(fp != NULL);
	assert(fputs("old configuration\n", fp) >= 0);
	assert(fclose(fp) == 0);

	/* A complete new file replaces the old configuration atomically. */
	assert(ftp_config_file_open(target, staged, sizeof(staged), &fp) == 0);
	assert(fp != NULL);
	assert(strncmp(staged, target, strlen(target)) == 0);
	assert(fputs("listen=YES\n", fp) >= 0);
	assert(ftp_config_file_publish(&fp, staged, target) == 0);
	assert(fp == NULL);
	assert(staged[0] == '\0');
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "listen=YES\n") == 0);
	assert(stat(target, &st) == 0);
	assert((st.st_mode & 0777) == 0644);

	/* A buffered write error must not replace the last-known-good file. */
	assert(ftp_config_file_open(target, staged, sizeof(staged), &fp) == 0);
	assert(fclose(fp) == 0);
	fp = fopen("/dev/full", "w");
	assert(fp != NULL);
	(void)fputs("injected write failure\n", fp);
	assert(ftp_config_file_publish(&fp, staged, target) != 0);
	assert(fp == NULL);
	assert(staged[0] == '\0');
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "listen=YES\n") == 0);

	/* A failed staging open leaves the existing destination untouched. */
	path_len = snprintf(missing_target, sizeof(missing_target),
			    "%s/missing/vsftpd.conf", dir);
	assert(path_len > 0 && (size_t)path_len < sizeof(missing_target));
	assert(ftp_config_file_open(missing_target, staged,
				   sizeof(staged), &fp) != 0);
	assert(fp == NULL);
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "listen=YES\n") == 0);

	/* Reject a truncated temporary pathname before creating a file. */
	assert(ftp_config_file_open(target, short_staged,
				   sizeof(short_staged), &fp) != 0);
	assert(fp == NULL);
	assert(short_staged[0] == '\0');

	assert(unlink(target) == 0);
	assert(rmdir(dir) == 0);
	puts("vsftpd configuration atomic-publication regression: PASS");
	return 0;
}
