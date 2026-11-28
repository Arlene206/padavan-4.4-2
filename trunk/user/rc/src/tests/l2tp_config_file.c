#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Inject one interrupted fsync without relying on GNU ld --wrap. */
static int l2tp_test_fsync_interrupt_call = -1;
static int l2tp_test_fsync_call_count;

static int
l2tp_test_fsync(int fd)
{
	l2tp_test_fsync_call_count++;
	if (l2tp_test_fsync_call_count == l2tp_test_fsync_interrupt_call) {
		errno = EINTR;
		return -1;
	}
	return fsync(fd);
}

#define L2TP_CONFIG_FILE_FSYNC(fd) l2tp_test_fsync(fd)
#include "l2tp_config_file.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s (errno=%d)\n", \
			__FILE__, __LINE__, #condition, errno); \
		return 1; \
	} \
} while (0)

static int
write_text(FILE *fp, void *context)
{
	const char *text = (const char *)context;

	if (fputs(text, fp) == EOF)
		return -1;
	return 0;
}

static int
write_partial_then_fail(FILE *fp, void *context)
{
	(void)context;
	if (fputs("partial replacement", fp) == EOF)
		return -1;
	errno = ENOSPC;
	return -1;
}

static int
write_path(const char *path, const char *text)
{
	FILE *fp = fopen(path, "w");

	if (!fp)
		return -1;
	if (fputs(text, fp) == EOF) {
		int saved_errno = errno ? errno : EIO;
		(void)fclose(fp);
		errno = saved_errno;
		return -1;
	}
	if (fclose(fp) != 0)
		return -1;
	return 0;
}

static int
read_path(const char *path, char *buffer, size_t capacity)
{
	FILE *fp;
	size_t length;

	if (!buffer || capacity == 0) {
		errno = EINVAL;
		return -1;
	}
	fp = fopen(path, "r");
	if (!fp)
		return -1;
	length = fread(buffer, 1, capacity - 1, fp);
	if (ferror(fp)) {
		int saved_errno = errno ? errno : EIO;
		(void)fclose(fp);
		errno = saved_errno;
		return -1;
	}
	buffer[length] = '\0';
	if (fclose(fp) != 0)
		return -1;
	return 0;
}

static int
count_temporary_files(const char *directory)
{
	DIR *dir;
	struct dirent *entry;
	int count = 0;
	const char prefix[] = ".l2tp.conf.tmp.";

	dir = opendir(directory);
	if (!dir)
		return -1;
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, prefix, sizeof(prefix) - 1) == 0)
			count++;
	}
	if (closedir(dir) != 0)
		return -1;
	return count;
}

static int
test_symlink_directory_is_rejected(const char *outside,
				   const char *link_directory)
{
	struct stat st;

	if (mkdir(outside, 0700) != 0 ||
	    symlink(outside, link_directory) != 0)
		return -1;
	if (l2tp_config_directory_prepare(link_directory) != -1)
		return -1;
	if (lstat(outside, &st) != 0 || !S_ISDIR(st.st_mode))
		return -1;
	return 0;
}

static int
test_file_symlink_is_replaced_without_touching_target(const char *directory,
						      const char *target,
						      const char *config)
{
	struct stat st;
	char content[128];

	if (write_path(target, "sentinel must survive") != 0 ||
	    symlink(target, config) != 0)
		return -1;
	if (l2tp_config_file_publish(directory, "l2tp.conf",
				     write_text, (void *)"new configuration") != 0)
		return -1;
	if (lstat(config, &st) != 0 || !S_ISREG(st.st_mode) ||
	    st.st_nlink != 1 || (st.st_mode & 0777) != 0644)
		return -1;
	if (read_path(target, content, sizeof(content)) != 0 ||
	    strcmp(content, "sentinel must survive") != 0)
		return -1;
	if (read_path(config, content, sizeof(content)) != 0 ||
	    strcmp(content, "new configuration") != 0)
		return -1;
	return 0;
}

static int
test_failed_publish_preserves_existing_file_and_cleans_temp(const char *directory,
							    const char *config)
{
	char content[128];

	if (write_path(config, "last good configuration") != 0)
		return -1;
	errno = 0;
	if (l2tp_config_file_publish(directory, "l2tp.conf",
				     write_partial_then_fail, NULL) != -1 ||
	    errno != ENOSPC)
		return -1;
	if (read_path(config, content, sizeof(content)) != 0 ||
	    strcmp(content, "last good configuration") != 0)
		return -1;
	if (count_temporary_files(directory) != 0)
		return -1;
	return 0;
}

static int
test_fsync_retries_eintr(const char *directory, const char *config)
{
	char content[128];
	int status;

	l2tp_test_fsync_call_count = 0;
	l2tp_test_fsync_interrupt_call = 1;
	status = l2tp_config_file_publish(directory, "l2tp.conf",
					  write_text, (void *)"resumed after EINTR");
	l2tp_test_fsync_interrupt_call = -1;
	if (status != 0 || l2tp_test_fsync_call_count != 2)
		return -1;
	if (read_path(config, content, sizeof(content)) != 0 ||
	    strcmp(content, "resumed after EINTR") != 0)
		return -1;
	return 0;
}

int
main(void)
{
	char root[256], directory[320], outside[320], link_directory[320];
	char target[384], config[384];
	struct stat st;
	mode_t original_umask;
	int length;

	length = snprintf(root, sizeof(root), "/tmp/padavan-l2tp-config-%ld",
			  (long)getpid());
	CHECK(length >= 0 && (size_t)length < sizeof(root));
	CHECK(mkdir(root, 0700) == 0);
	CHECK(snprintf(directory, sizeof(directory), "%s/l2tp", root) <
	      (int)sizeof(directory));
	CHECK(snprintf(outside, sizeof(outside), "%s/outside", root) <
	      (int)sizeof(outside));
	CHECK(snprintf(link_directory, sizeof(link_directory), "%s/link", root) <
	      (int)sizeof(link_directory));
	CHECK(snprintf(target, sizeof(target), "%s/target", outside) <
	      (int)sizeof(target));
	CHECK(snprintf(config, sizeof(config), "%s/l2tp.conf", directory) <
	      (int)sizeof(config));

	original_umask = umask(0000);
	CHECK(l2tp_config_directory_prepare(directory) == 0);
	CHECK(lstat(directory, &st) == 0 && S_ISDIR(st.st_mode));
	CHECK((st.st_mode & 07777) == 0755);

	/* Repair an old permissive directory despite PID 1's umask. */
	CHECK(chmod(directory, 0777) == 0);
	CHECK(l2tp_config_directory_prepare(directory) == 0);
	CHECK(lstat(directory, &st) == 0 && (st.st_mode & 07777) == 0755);

	CHECK(test_symlink_directory_is_rejected(outside, link_directory) == 0);
	CHECK(test_file_symlink_is_replaced_without_touching_target(
		      directory, target, config) == 0);
	CHECK(test_failed_publish_preserves_existing_file_and_cleans_temp(
		      directory, config) == 0);
	CHECK(test_fsync_retries_eintr(directory, config) == 0);

	CHECK(umask(original_umask) == 0000);
	CHECK(unlink(config) == 0);
	CHECK(unlink(link_directory) == 0);
	CHECK(unlink(target) == 0);
	CHECK(rmdir(outside) == 0);
	CHECK(rmdir(directory) == 0);
	CHECK(rmdir(root) == 0);

	puts("rp-l2tp private atomic configuration publication: PASS");
	return 0;
}
