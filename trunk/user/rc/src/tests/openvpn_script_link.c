#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "openvpn_script_link.h"

static void
test_expected_link_is_created_and_reused(const char *directory)
{
	char path[256];
	char actual[128];
	int created = 0;
	ssize_t length;

	snprintf(path, sizeof(path), "%s/callback", directory);
	assert(openvpn_script_link_ensure(path, "/sbin/rc", &created) == 0);
	assert(created == 1);
	length = readlink(path, actual, sizeof(actual) - 1);
	assert(length == (ssize_t)strlen("/sbin/rc"));
	actual[length] = '\0';
	assert(strcmp(actual, "/sbin/rc") == 0);
	assert(openvpn_script_link_matches(path, "/sbin/rc") == 0);

	created = 1;
	assert(openvpn_script_link_ensure(path, "/sbin/rc", &created) == 0);
	assert(created == 0);
	assert(openvpn_script_link_remove_created(path, created) == 0);
	assert(lstat(path, &(struct stat){0}) == 0);
	assert(openvpn_script_link_remove_created(path, 1) == 0);
	assert(lstat(path, &(struct stat){0}) != 0 && errno == ENOENT);
}

static void
test_wrong_link_and_regular_file_fail_closed(const char *directory)
{
	char path[256];
	FILE *file;
	int created = 0;

	snprintf(path, sizeof(path), "%s/wrong-link", directory);
	assert(symlink("wrong-target", path) == 0);
	assert(openvpn_script_link_ensure(path, "/sbin/rc", &created) != 0);
	assert(errno == EEXIST);
	assert(created == 0);
	assert(openvpn_script_link_matches(path, "wrong-target") == 0);
	assert(unlink(path) == 0);

	snprintf(path, sizeof(path), "%s/regular", directory);
	file = fopen(path, "w");
	assert(file != NULL);
	assert(fclose(file) == 0);
	assert(openvpn_script_link_ensure(path, "/sbin/rc", &created) != 0);
	assert(errno == EEXIST);
	assert(created == 0);
	assert(unlink(path) == 0);
}

static void
test_remove_only_owned_link(const char *directory)
{
	char path[256];
	int created = 0;

	snprintf(path, sizeof(path), "%s/owned", directory);
	assert(openvpn_script_link_ensure(path, "/sbin/rc", &created) == 0);
	assert(created == 1);
	assert(openvpn_script_link_remove_created(path, 0) == 0);
	assert(openvpn_script_link_matches(path, "/sbin/rc") == 0);
	assert(openvpn_script_link_remove_created(path, created) == 0);
}

int
main(void)
{
	char directory[] = "/tmp/openvpn-script-link.XXXXXX";

	assert(mkdtemp(directory) != NULL);
	test_expected_link_is_created_and_reused(directory);
	test_wrong_link_and_regular_file_fail_closed(directory);
	test_remove_only_owned_link(directory);
	assert(rmdir(directory) == 0);
	puts("OpenVPN callback script link lifecycle policy: PASS");
	return 0;
}
