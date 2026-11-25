#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "openvpn_client_secret_file.h"

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

int
main(void)
{
	char directory[] = "/tmp/openvpn-secret-test.XXXXXX";
	char secret[256], victim[256], symlink_path[256];
	char blocked[256], sentinel[256];
	struct stat before, after;
	mode_t old_umask;
	FILE *fp;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(secret, sizeof(secret), "%s/secret", directory) > 0);
	assert(snprintf(victim, sizeof(victim), "%s/victim", directory) > 0);
	assert(snprintf(symlink_path, sizeof(symlink_path), "%s/symlink", directory) > 0);
	assert(snprintf(blocked, sizeof(blocked), "%s/blocked", directory) > 0);
	assert(snprintf(sentinel, sizeof(sentinel), "%s/blocked/sentinel", directory) > 0);

	old_umask = umask(0000);

	/* Reproduce the former fopen-then-chmod exposure under PID 1's umask. */
	fp = fopen(secret, "w+");
	assert(fp != NULL);
	assert(fputs("old-user\nold-password\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(stat(secret, &before) == 0);
	assert((before.st_mode & 0777) == 0666);

	/* Replacement must publish a new owner-only inode, even at umask 0000. */
	assert(openvpn_client_secret_publish(secret, "vpn-user", "vpn-password") == 0);
	assert(stat(secret, &after) == 0);
	assert((after.st_mode & 0777) == 0600);
	assert(after.st_nlink == 1);
	assert(before.st_ino != after.st_ino);
	read_exact(secret, "vpn-user\nvpn-password\n");

	/* Atomic rename replaces a symlink itself; it must not overwrite target. */
	fp = fopen(victim, "w");
	assert(fp != NULL);
	assert(fputs("preserve-victim\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(symlink(victim, symlink_path) == 0);
	assert(openvpn_client_secret_publish(symlink_path, "new-user", "new-pass") == 0);
	assert(lstat(symlink_path, &after) == 0 && S_ISREG(after.st_mode));
	assert((after.st_mode & 0777) == 0600);
	read_exact(victim, "preserve-victim\n");
	read_exact(symlink_path, "new-user\nnew-pass\n");

	/* A failed rename leaves the old destination untouched and removes stage. */
	assert(mkdir(blocked, 0700) == 0);
	fp = fopen(sentinel, "w");
	assert(fp != NULL);
	assert(fputs("keep\n", fp) >= 0);
	assert(fclose(fp) == 0);
	errno = 0;
	assert(openvpn_client_secret_publish(blocked, "unused", "unused") == -1);
	assert(errno != 0);
	read_exact(sentinel, "keep\n");
	assert(count_staged_files(directory, "blocked.tmp.") == 0);

	errno = 0;
	assert(openvpn_client_secret_publish(NULL, "user", "pass") == -1);
	assert(errno == EINVAL);

	assert(umask(old_umask) == 0000);
	assert(unlink(secret) == 0);
	assert(unlink(victim) == 0);
	assert(unlink(symlink_path) == 0);
	assert(unlink(sentinel) == 0);
	assert(rmdir(blocked) == 0);
	assert(rmdir(directory) == 0);

	puts("OpenVPN client credential staging policy: PASS");
	return 0;
}
