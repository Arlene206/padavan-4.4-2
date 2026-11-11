#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

#include "openvpn_config_file.h"

static void
read_exact(const char *path, const char *expected)
{
	char buffer[256];
	FILE *fp = fopen(path, "r");
	size_t length;

	assert(fp != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, fp);
	buffer[length] = '\0';
	assert(strcmp(buffer, expected) == 0);
	assert(fclose(fp) == 0);
}

static int
count_staged_files(const char *directory, const char *prefix)
{
	DIR *dir = opendir(directory);
	struct dirent *entry;
	int count = 0;
	size_t prefix_len = strlen(prefix);

	assert(dir != NULL);
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, prefix, prefix_len) == 0)
			count++;
	}
	assert(closedir(dir) == 0);
	return count;
}

static FILE *
begin_config(const char *path, char *staged, size_t staged_size)
{
	return openvpn_config_file_open(path, staged, staged_size);
}

int
main(void)
{
	char directory[] = "/tmp/openvpn-config-test.XXXXXX";
	char config[256], victim[256], symlink_path[256];
	char blocked[256], sentinel[256], staged[512], short_staged[8];
	struct stat before, after, staged_stat;
	struct rlimit old_limit, zero_limit;
	mode_t old_umask;
	FILE *fp;
	void (*old_signal)(int);
	int write_errno;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(config, sizeof(config), "%s/client.conf", directory) > 0);
	assert(snprintf(victim, sizeof(victim), "%s/victim", directory) > 0);
	assert(snprintf(symlink_path, sizeof(symlink_path), "%s/linked.conf", directory) > 0);
	assert(snprintf(blocked, sizeof(blocked), "%s/blocked", directory) > 0);
	assert(snprintf(sentinel, sizeof(sentinel), "%s/blocked/sentinel", directory) > 0);

	old_umask = umask(0000);
	fp = fopen(config, "w");
	assert(fp != NULL);
	assert(fputs("last-good-config\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(stat(config, &before) == 0);

	/* Staging is private and leaves the live file intact until rename. */
	fp = begin_config(config, staged, sizeof(staged));
	assert(fp != NULL);
	assert(fstat(fileno(fp), &staged_stat) == 0);
	assert((staged_stat.st_mode & 0777) == 0600);
	assert(fputs("new-config\n", fp) >= 0);
	read_exact(config, "last-good-config\n");
	assert(openvpn_config_file_publish(&fp, staged, config, 0644) == 0);
	assert(fp == NULL);
	assert(stat(config, &after) == 0);
	assert((after.st_mode & 0777) == 0644);
	assert(after.st_nlink == 1);
	assert(before.st_ino != after.st_ino);
	read_exact(config, "new-config\n");

	/* A failed flush must not replace the previous usable configuration. */
	assert(getrlimit(RLIMIT_FSIZE, &old_limit) == 0);
	fp = begin_config(config, staged, sizeof(staged));
	assert(fp != NULL);
	assert(fputs("must-not-publish\n", fp) >= 0);
	old_signal = signal(SIGXFSZ, SIG_IGN);
	assert(old_signal != SIG_ERR);
	zero_limit = old_limit;
	zero_limit.rlim_cur = 0;
	assert(setrlimit(RLIMIT_FSIZE, &zero_limit) == 0);
	errno = 0;
	assert(openvpn_config_file_publish(&fp, staged, config, 0644) == -1);
	write_errno = errno;
	assert(setrlimit(RLIMIT_FSIZE, &old_limit) == 0);
	assert(signal(SIGXFSZ, old_signal) != SIG_ERR);
	assert(write_errno == EFBIG);
	assert(fp == NULL);
	read_exact(config, "new-config\n");
	assert(count_staged_files(directory, "client.conf.tmp.") == 0);

	/* Replacing a symlink must replace the link itself, not its victim. */
	fp = fopen(victim, "w");
	assert(fp != NULL);
	assert(fputs("preserve-victim\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(symlink(victim, symlink_path) == 0);
	fp = begin_config(symlink_path, staged, sizeof(staged));
	assert(fp != NULL);
	assert(fputs("replacement\n", fp) >= 0);
	assert(openvpn_config_file_publish(&fp, staged, symlink_path, 0644) == 0);
	assert(lstat(symlink_path, &after) == 0 && S_ISREG(after.st_mode));
	read_exact(victim, "preserve-victim\n");
	read_exact(symlink_path, "replacement\n");

	/* Rename failure must preserve the destination and remove the stage. */
	assert(mkdir(blocked, 0700) == 0);
	fp = fopen(sentinel, "w");
	assert(fp != NULL);
	assert(fputs("keep\n", fp) >= 0);
	assert(fclose(fp) == 0);
	fp = begin_config(blocked, staged, sizeof(staged));
	assert(fp != NULL);
	assert(fputs("unused\n", fp) >= 0);
	errno = 0;
	assert(openvpn_config_file_publish(&fp, staged, blocked, 0644) == -1);
	assert(errno != 0);
	read_exact(sentinel, "keep\n");
	assert(count_staged_files(directory, "blocked.tmp.") == 0);

	/* Invalid and undersized path arguments fail before creating a stage. */
	errno = 0;
	assert(openvpn_config_file_open(NULL, staged, sizeof(staged)) == NULL);
	assert(errno == EINVAL);
	errno = 0;
	assert(openvpn_config_file_open(config, short_staged, sizeof(short_staged)) == NULL);
	assert(errno == ENAMETOOLONG);

	assert(umask(old_umask) == 0000);
	assert(unlink(config) == 0);
	assert(unlink(victim) == 0);
	assert(unlink(symlink_path) == 0);
	assert(unlink(sentinel) == 0);
	assert(rmdir(blocked) == 0);
	assert(rmdir(directory) == 0);

	puts("OpenVPN config atomic publication and failure preservation: PASS");
	return 0;
}
