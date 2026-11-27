#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../modem_node_file.h"
#include "../modem_node_identity_policy.h"

static int test_metadata_write_calls;
static int test_metadata_remove_calls;
static int test_metadata_write_result;
static int test_metadata_remove_result;

static int
test_metadata_write(const char *directory, const char *name, const char *contents)
{
	assert(strcmp(directory, "/modem") == 0);
	assert(strcmp(name, "wwan0") == 0);
	assert(strcmp(contents, "pref=1\\n") == 0);
	test_metadata_write_calls++;
	return test_metadata_write_result;
}

static int
test_metadata_remove(const char *directory, const char *name)
{
	assert(strcmp(directory, "/modem") == 0);
	assert(strcmp(name, "wwan0") == 0);
	test_metadata_remove_calls++;
	return test_metadata_remove_result;
}

static void
test_metadata_publication_cleanup(void)
{
	test_metadata_write_calls = 0;
	test_metadata_remove_calls = 0;
	test_metadata_write_result = 0;
	test_metadata_remove_result = 0;

	/* Successful replacement keeps the newly published identity. */
	assert(modem_node_file_publish_with_cleanup("/modem", "wwan0",
					    "pref=1\\n", 1,
					    test_metadata_write,
					    test_metadata_remove) == 0);
	assert(test_metadata_write_calls == 1);
	assert(test_metadata_remove_calls == 0);

	/* Failed replacement invalidates the previous interface identity. */
	test_metadata_write_result = -1;
	assert(modem_node_file_publish_with_cleanup("/modem", "wwan0",
					    "pref=1\\n", 1,
					    test_metadata_write,
					    test_metadata_remove) == -1);
	assert(test_metadata_write_calls == 2);
	assert(test_metadata_remove_calls == 1);

	/* Invalid or truncated content is never written and clears stale identity. */
	assert(modem_node_file_publish_with_cleanup("/modem", "wwan0",
					    "pref=1\\n", 0,
					    test_metadata_write,
					    test_metadata_remove) == -1);
	assert(test_metadata_write_calls == 2);
	assert(test_metadata_remove_calls == 2);

	/* Distinguish failed cleanup so hotplug can log residual stale state. */
	test_metadata_remove_result = -1;
	assert(modem_node_file_publish_with_cleanup("/modem", "wwan0",
					    "pref=1\\n", 1,
					    test_metadata_write,
					    test_metadata_remove) == -2);
	assert(test_metadata_write_calls == 3);
	assert(test_metadata_remove_calls == 3);

	/* Without a remover, refuse to publish because failure cannot be cleaned up. */
	assert(modem_node_file_publish_with_cleanup("/modem", "wwan0",
					    "pref=1\\n", 1,
					    test_metadata_write, NULL) == -1);
	assert(test_metadata_write_calls == 3);
	assert(test_metadata_remove_calls == 3);
}


static void
test_manual_node_index_status_is_fail_closed(void)
{
	int manual_index = 99;

	assert(modem_node_manual_index_status(0, 0, &manual_index) == 0);
	assert(manual_index == -1);
	assert(modem_node_manual_index_status(0, -1, &manual_index) == 0);
	assert(manual_index == -1);
	assert(modem_node_manual_index_status(0, 1, &manual_index) == 0);
	assert(manual_index == 0);
	assert(modem_node_manual_index_status(0, 15, &manual_index) == 0);
	assert(manual_index == 14);
	assert(modem_node_manual_index_status(0, INT_MAX, &manual_index) == 0);
	assert(manual_index == INT_MAX - 1);
	assert(modem_node_manual_index_status(EIO, 7, &manual_index) == EIO);
	assert(manual_index == -1);
	assert(modem_node_manual_index_status(-1, 7, &manual_index) == EIO);
	assert(manual_index == -1);
	assert(modem_node_manual_index_status(0, 7, NULL) == EINVAL);
}


int
main(void)
{
	char root[] = "/tmp/modem-node-policy.XXXXXX";
	char directory[PATH_MAX], outside[PATH_MAX], outside_file[PATH_MAX];
	char node_path[PATH_MAX], link_path[PATH_MAX], escape_path[PATH_MAX];
	char missing_path[PATH_MAX], linked_parent[PATH_MAX], linked_path[PATH_MAX];
	struct stat st;
	FILE *fp;
	char line[64];
	int n;

	test_metadata_publication_cleanup();
	test_manual_node_index_status_is_fail_closed();

	assert(modem_node_manual_index_matches(-1, 0));
	assert(modem_node_manual_index_matches(4, 4));
	assert(!modem_node_manual_index_matches(4, 3));
	assert(!modem_node_manual_index_matches(4, -1));
	assert(modem_node_usb_identity_is_valid("1-1", 7));
	assert(modem_node_usb_identity_is_valid("1-2.3", 127));
	assert(!modem_node_usb_identity_is_valid(NULL, 7));
	assert(!modem_node_usb_identity_is_valid("", 7));
	assert(!modem_node_usb_identity_is_valid("1-1", 0));
	assert(!modem_node_usb_identity_is_valid("1-1", -1));
	assert(!modem_node_usb_identity_is_valid("1-1", 128));
	assert(modem_node_usb_identity_matches("1-1", 7, "1-1", 7));
	assert(!modem_node_usb_identity_matches("1-1", 7, "1-2", 7));
	assert(!modem_node_usb_identity_matches("1-1", 7, "1-1", 8));
	assert(!modem_node_usb_identity_matches("1-1", 0, "1-1", 0));
	assert(!modem_node_usb_identity_matches(NULL, 7, "1-1", 7));
	assert(!modem_node_usb_identity_node_is_eligible("1-1", 7, "1-1", 7, 2, 0, -1, 1));
	assert(modem_node_usb_identity_node_is_eligible("1-1", 7, "1-1", 7, 2, 1, -1, 1));
	assert(modem_node_usb_identity_node_is_eligible("1-1", 7, "1-1", 7, 2, 0, -1, 0));
	assert(!modem_node_usb_identity_node_is_eligible("1-1", 7, "1-2", 7, 2, 1, -1, 0));
	assert(!modem_node_usb_identity_node_is_eligible("1-1", 7, "1-1", 8, 2, 1, -1, 0));
	/* Manual selection pins an index but bypasses preferred-port ranking. */
	assert(modem_node_usb_identity_node_is_eligible("1-1", 7, "1-1", 7, 4, 0, 4, 1));
	assert(!modem_node_usb_identity_node_is_eligible("1-1", 7, "1-1", 7, 5, 1, 4, 0));
	assert(!modem_node_usb_identity_node_is_eligible("1-1", 7, "1-2", 7, 4, 1, 4, 0));
	assert(modem_node_name_is_safe("ttyUSB0"));
	assert(modem_node_name_is_safe("ttyACM12"));
	assert(modem_node_name_is_safe("cdc-wdm0"));
	assert(modem_node_name_is_safe("wwan0"));
	assert(modem_node_name_is_safe("weth0"));
	assert(!modem_node_name_is_safe(NULL));
	assert(!modem_node_name_is_safe(""));
	assert(!modem_node_name_is_safe("."));
	assert(!modem_node_name_is_safe(".."));
	assert(!modem_node_name_is_safe("../passwd"));
	assert(!modem_node_name_is_safe("tty/USB0"));
	assert(!modem_node_name_is_safe("tty USB0"));
	assert(!modem_node_name_is_safe("tty\nUSB0"));
	assert(!modem_node_name_is_safe("9ttyUSB0"));

	assert(mkdtemp(root) != NULL);
	n = snprintf(directory, sizeof(directory), "%s/modem", root);
	assert(n > 0 && n < (int)sizeof(directory));
	n = snprintf(outside, sizeof(outside), "%s/outside", root);
	assert(n > 0 && n < (int)sizeof(outside));
	n = snprintf(outside_file, sizeof(outside_file), "%s/sentinel", root);
	assert(n > 0 && n < (int)sizeof(outside_file));
	n = snprintf(node_path, sizeof(node_path), "%s/ttyUSB0", directory);
	assert(n > 0 && n < (int)sizeof(node_path));
	n = snprintf(link_path, sizeof(link_path), "%s/ttyUSB1", directory);
	assert(n > 0 && n < (int)sizeof(link_path));
	n = snprintf(escape_path, sizeof(escape_path), "%s/escape", root);
	assert(n > 0 && n < (int)sizeof(escape_path));
	n = snprintf(missing_path, sizeof(missing_path), "%s/missing/ttyUSB2", root);
	assert(n > 0 && n < (int)sizeof(missing_path));
	n = snprintf(linked_parent, sizeof(linked_parent), "%s/linked-parent", root);
	assert(n > 0 && n < (int)sizeof(linked_parent));
	n = snprintf(linked_path, sizeof(linked_path), "%s/ttyUSB3", linked_parent);
	assert(n > 0 && n < (int)sizeof(linked_path));

	assert(modem_node_file_publish_identity(directory, "ttyUSB0", "pref=1\ndevnum=7\n", 1) == 0);
	assert(config_file_sync_parent(node_path) == 0);
	errno = 0;
	assert(config_file_sync_parent(missing_path) != 0);
	assert(errno == ENOENT);
	assert(symlink(root, linked_parent) == 0);
	errno = 0;
	assert(config_file_sync_parent(linked_path) != 0);
	assert(unlink(linked_parent) == 0);
	assert(lstat(directory, &st) == 0 && S_ISDIR(st.st_mode));
	assert((st.st_mode & 0777) == 0755);
	assert(st.st_uid == geteuid());
	assert(lstat(node_path, &st) == 0 && S_ISREG(st.st_mode));
	assert((st.st_mode & 0777) == 0644);
	assert(st.st_uid == geteuid());
	fp = fopen(node_path, "r");
	assert(fp != NULL);
	assert(fgets(line, sizeof(line), fp) != NULL);
	assert(strcmp(line, "pref=1\n") == 0);
	assert(fclose(fp) == 0);

	fp = fopen(outside_file, "w");
	assert(fp != NULL);
	assert(fputs("sentinel\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(symlink(outside_file, link_path) == 0);
	assert(modem_node_file_write(directory, "ttyUSB1", "pref=0\ndevnum=9\n") == 0);
	assert(lstat(link_path, &st) == 0 && S_ISREG(st.st_mode));
	fp = fopen(outside_file, "r");
	assert(fp != NULL);
	assert(fgets(line, sizeof(line), fp) != NULL);
	assert(strcmp(line, "sentinel\n") == 0);
	assert(fclose(fp) == 0);

	assert(mkdir(outside, 0700) == 0);
	assert(symlink(outside, escape_path) == 0);
	assert(modem_node_dir_prepare(escape_path) == -1);
	assert(modem_node_file_write(escape_path, "ttyUSB2", "pref=1\n") == -1);

	assert(modem_node_file_remove(directory, "ttyUSB0") == 0);
	assert(lstat(node_path, &st) != 0);
	assert(modem_node_file_remove(directory, "../sentinel") == -1);

	unlink(link_path);
	unlink(outside_file);
	unlink(escape_path);
	rmdir(outside);
	rmdir(directory);
	rmdir(root);
	puts("USB modem node identity and private metadata staging: PASS");
	return 0;
}
