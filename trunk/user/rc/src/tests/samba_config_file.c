#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../samba_config_file.h"

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
	char dir[] = "/tmp/samba-config-file.XXXXXX";
	char target[512], missing_target[512], directory_target[512];
	char staged[512], short_staged[4], contents[128];
	FILE *fp;
	struct stat st;
	int path_len;

	assert(mkdtemp(dir) != NULL);
	path_len = snprintf(target, sizeof(target), "%s/smb.conf", dir);
	assert(path_len > 0 && (size_t)path_len < sizeof(target));

	fp = fopen(target, "w");
	assert(fp != NULL);
	assert(fputs("old configuration\n", fp) >= 0);
	assert(fclose(fp) == 0);

	/* A complete staged file atomically replaces the old configuration. */
	assert(samba_config_file_open(target, staged, sizeof(staged), &fp) == 0);
	assert(fp != NULL);
	assert(strncmp(staged, target, strlen(target)) == 0);
	assert(fputs("new configuration\n", fp) >= 0);
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "old configuration\n") == 0);
	assert(samba_config_file_publish(&fp, staged, target) == 0);
	assert(fp == NULL);
	assert(staged[0] == '\0');
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "new configuration\n") == 0);
	assert(stat(target, &st) == 0);
	assert((st.st_mode & 0777) == 0644);

	/* A buffered ENOSPC error must preserve the last-known-good file. */
	assert(samba_config_file_open(target, staged, sizeof(staged), &fp) == 0);
	assert(fclose(fp) == 0);
	fp = fopen("/dev/full", "w");
	assert(fp != NULL);
	assert(fputs("injected write failure\n", fp) >= 0);
	errno = 0;
	assert(samba_config_file_publish(&fp, staged, target) != 0);
	assert(fp == NULL);
	assert(staged[0] == '\0');
	assert(errno == ENOSPC);
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "new configuration\n") == 0);

	/* A failed rename discards the candidate and keeps the active file. */
	path_len = snprintf(directory_target, sizeof(directory_target),
			    "%s/not-a-file", dir);
	assert(path_len > 0 && (size_t)path_len < sizeof(directory_target));
	assert(mkdir(directory_target, 0700) == 0);
	assert(samba_config_file_open(target, staged, sizeof(staged), &fp) == 0);
	assert(fputs("must-not-publish\n", fp) >= 0);
	assert(samba_config_file_publish(&fp, staged, directory_target) != 0);
	assert(fp == NULL);
	assert(staged[0] == '\0');
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "new configuration\n") == 0);
	assert(rmdir(directory_target) == 0);

	/* Explicit discard also closes and removes an unpublished candidate. */
	assert(samba_config_file_open(target, staged, sizeof(staged), &fp) == 0);
	assert(fputs("discard me\\n", fp) >= 0);
	samba_config_file_discard(&fp, staged);
	assert(fp == NULL);
	assert(staged[0] == '\\0');
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "new configuration\\n") == 0);

	/* Missing parent and a short staging buffer fail without truncation. */
	path_len = snprintf(missing_target, sizeof(missing_target),
			    "%s/missing/smb.conf", dir);
	assert(path_len > 0 && (size_t)path_len < sizeof(missing_target));
	assert(samba_config_file_open(missing_target, staged,
				      sizeof(staged), &fp) != 0);
	assert(fp == NULL);
	assert(samba_config_file_open(target, short_staged,
				      sizeof(short_staged), &fp) != 0);
	assert(fp == NULL);
	assert(short_staged[0] == '\0');
	read_text_file(target, contents, sizeof(contents));
	assert(strcmp(contents, "new configuration\n") == 0);

	assert(unlink(target) == 0);
	assert(rmdir(dir) == 0);
	puts("Samba config atomic-publication and fail-closed I/O: PASS");
	return 0;
}
