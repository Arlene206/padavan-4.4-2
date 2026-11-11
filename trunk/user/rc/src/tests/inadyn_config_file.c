#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "inadyn_config_file.h"

static void
read_exact(const char *path, const char *expected)
{
	char buffer[256];
	FILE *fp;
	size_t length;

	fp = fopen(path, "r");
	assert(fp != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, fp);
	buffer[length] = '\0';
	assert(strcmp(buffer, expected) == 0);
	assert(!ferror(fp));
	assert(fclose(fp) == 0);
}

static int
count_staged_files(const char *directory, const char *prefix)
{
	DIR *dir;
	struct dirent *entry;
	int count = 0;
	size_t prefix_length = strlen(prefix);

	dir = opendir(directory);
	assert(dir != NULL);
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, prefix, prefix_length) == 0)
			++count;
	}
	assert(closedir(dir) == 0);
	return count;
}

int
main(void)
{
	char directory[] = "/tmp/inadyn-config-test.XXXXXX";
	char config[256], victim[256], link_path[256], blocked[256], sentinel[256];
	char staged_path[512];
	struct stat before, after;
	mode_t old_umask;
	FILE *fp;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(config, sizeof(config), "%s/inadyn.conf", directory) > 0);
	assert(snprintf(victim, sizeof(victim), "%s/victim", directory) > 0);
	assert(snprintf(link_path, sizeof(link_path), "%s/config-link", directory) > 0);
	assert(snprintf(blocked, sizeof(blocked), "%s/blocked", directory) > 0);
	assert(snprintf(sentinel, sizeof(sentinel), "%s/blocked/sentinel", directory) > 0);

	old_umask = umask(0000);

	/* Reproduce the original fopen("w") mode under PID 1's zero umask. */
	fp = fopen(config, "w");
	assert(fp != NULL);
	assert(fputs("old-user\nold-password\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(stat(config, &before) == 0);
	assert((before.st_mode & 0777) == 0666);

	/* The staged inode is private before any DDNS credential is written. */
	fp = inadyn_config_file_open(config, staged_path, sizeof(staged_path));
	assert(fp != NULL && staged_path[0] != '\0');
	assert(stat(staged_path, &after) == 0);
	assert((after.st_mode & 0777) == 0600);
	assert(fputs("new-user\nnew-password\n", fp) >= 0);
	assert(inadyn_config_file_publish(fp, staged_path, config) == 0);
	assert(staged_path[0] == '\0');
	assert(stat(config, &after) == 0);
	assert((after.st_mode & 0777) == 0600);
	assert(after.st_nlink == 1);
	assert(before.st_ino != after.st_ino);
	read_exact(config, "new-user\nnew-password\n");

	/* Atomic publication replaces a destination symlink, not its target. */
	fp = fopen(victim, "w");
	assert(fp != NULL);
	assert(fputs("keep-victim\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(symlink(victim, link_path) == 0);
	fp = inadyn_config_file_open(link_path, staged_path, sizeof(staged_path));
	assert(fp != NULL);
	assert(fputs("private-link-replacement\n", fp) >= 0);
	assert(inadyn_config_file_publish(fp, staged_path, link_path) == 0);
	assert(lstat(link_path, &after) == 0 && S_ISREG(after.st_mode));
	assert((after.st_mode & 0777) == 0600);
	read_exact(victim, "keep-victim\n");
	read_exact(link_path, "private-link-replacement\n");

	/* Aborting a partial build leaves the previous config untouched. */
	fp = inadyn_config_file_open(config, staged_path, sizeof(staged_path));
	assert(fp != NULL);
	assert(fputs("partial-secret\n", fp) >= 0);
	assert(inadyn_config_file_discard(fp, staged_path) == 0);
	assert(staged_path[0] == '\0');
	read_exact(config, "new-user\nnew-password\n");
	assert(count_staged_files(directory, "inadyn.conf.tmp.") == 0);

	/* A rename failure cleans the stage and preserves the existing directory. */
	assert(mkdir(blocked, 0700) == 0);
	fp = fopen(sentinel, "w");
	assert(fp != NULL);
	assert(fputs("sentinel\n", fp) >= 0);
	assert(fclose(fp) == 0);
	fp = inadyn_config_file_open(blocked, staged_path, sizeof(staged_path));
	assert(fp != NULL);
	assert(fputs("unpublishable\n", fp) >= 0);
	errno = 0;
	assert(inadyn_config_file_publish(fp, staged_path, blocked) == -1);
	assert(errno != 0);
	assert(staged_path[0] == '\0');
	read_exact(sentinel, "sentinel\n");
	assert(count_staged_files(directory, "blocked.tmp.") == 0);

	staged_path[0] = 'x';
	errno = 0;
	assert(inadyn_config_file_open(NULL, staged_path, sizeof(staged_path)) == NULL);
	assert(errno == EINVAL && staged_path[0] == 'x');

	assert(umask(old_umask) == 0000);
	assert(unlink(config) == 0);
	assert(unlink(victim) == 0);
	assert(unlink(link_path) == 0);
	assert(unlink(sentinel) == 0);
	assert(rmdir(blocked) == 0);
	assert(rmdir(directory) == 0);

	puts("Inadyn private atomic configuration publication: PASS");
	return 0;
}
