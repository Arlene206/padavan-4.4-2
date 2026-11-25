#ifndef PADAVAN_DNSMASQ_FILE_H
#define PADAVAN_DNSMASQ_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "dnsmasq_domain_policy.h"
#include "dnsmasq_safe_read.h"

/* Preserve the staged file's preselected mode when publishing a related set. */
#define DNSMASQ_FILE_PRESERVE_MODE ((mode_t)-1)

/* Bit flags mirror the service options emitted below.  Only DHCP modes that
 * emit dhcp-leasefile make the persistent lease path part of startup. */
#define DNSMASQ_DHCP_FEATURE_LEASES 0x01
#define DNSMASQ_DHCP_FEATURE_RA     0x02

/* Declared here because dnsmasq_file_open() uses it during fdopen failure cleanup. */
static inline int dnsmasq_file_discard(char *path);

static int
dnsmasq_lease_file_required(int dhcp_features)
{
	return (dhcp_features & DNSMASQ_DHCP_FEATURE_LEASES) != 0;
}

typedef int (*dnsmasq_file_unlink_fn)(const char *path);
typedef FILE *(*dnsmasq_file_fdopen_fn)(int fd, const char *mode);
typedef int (*dnsmasq_file_close_fn)(int fd);

/* Keep the unique path when unlink fails, allowing a caller to retry/log it. */
static inline int
dnsmasq_file_discard_with_unlink(char *path, dnsmasq_file_unlink_fn unlink_fn)
{
	int status, saved_errno;

	if (!path || !path[0])
		return 0;
	if (!unlink_fn) {
		errno = EINVAL;
		return -1;
	}
	do {
		errno = 0;
		status = unlink_fn(path);
	} while (status != 0 && errno == EINTR);
	if (status == 0) {
		path[0] = '\0';
		return 0;
	}
	saved_errno = errno ? errno : EIO;
	if (saved_errno == ENOENT) {
		path[0] = '\0';
		return 0;
	}
	errno = saved_errno;
	return -1;
}

static inline int
dnsmasq_file_discard(char *path)
{
	return dnsmasq_file_discard_with_unlink(path, unlink);
}

/* fdopen() can fail after mkstemp() has created a file. Preserve its name if
 * the cleanup unlink also fails so the caller can retry and report the leak. */
static inline FILE *
dnsmasq_file_open_with_ops(char *path, dnsmasq_file_fdopen_fn fdopen_fn,
			   dnsmasq_file_close_fn close_fn,
			   dnsmasq_file_unlink_fn unlink_fn)
{
	int fd, saved_errno;
	FILE *fp;

	if (!path || !path[0] || !fdopen_fn || !close_fn || !unlink_fn) {
		errno = EINVAL;
		return NULL;
	}
	fd = mkstemp(path);
	if (fd < 0) {
		saved_errno = errno ? errno : EIO;
		path[0] = '\0';
		errno = saved_errno;
		return NULL;
	}
	fp = fdopen_fn(fd, "w");
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		(void)close_fn(fd);
		(void)dnsmasq_file_discard_with_unlink(path, unlink_fn);
		errno = saved_errno;
		return NULL;
	}
	return fp;
}

/* Build a complete replacement beside its destination; never truncate a
 * predictable shared-/tmp pathname or follow a pre-existing destination link. */
static FILE *
dnsmasq_file_open(char *path)
{
	return dnsmasq_file_open_with_ops(path, fdopen, close, unlink);
}

/* Flush/close the staged file before atomically replacing the published path.
 * rename() replaces a destination symlink itself; it never follows its target. */
static int
dnsmasq_file_publish(FILE *fp, char *path, const char *destination, mode_t mode)
{
	int failed = 0;
	int saved_errno = 0;

	if (!fp || !path || !path[0] || !destination || !destination[0]) {
		if (fp)
			fclose(fp);
		dnsmasq_file_discard(path);
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	if (fflush(fp) != 0) {
		failed = 1;
		saved_errno = errno ? errno : EIO;
	}
	if (ferror(fp) && !failed) {
		failed = 1;
		saved_errno = errno ? errno : EIO;
	}
	if (!failed && mode != DNSMASQ_FILE_PRESERVE_MODE &&
	    fchmod(fileno(fp), mode) != 0) {
		failed = 1;
		saved_errno = errno ? errno : EIO;
	}
	if (fclose(fp) != 0) {
		if (!saved_errno)
			saved_errno = errno ? errno : EIO;
		failed = 1;
	}

	if (failed) {
		dnsmasq_file_discard(path);
		errno = saved_errno ? saved_errno : EIO;
		return -1;
	}

	if (rename(path, destination) != 0) {
		saved_errno = errno ? errno : EIO;
		dnsmasq_file_discard(path);
		errno = saved_errno;
		return -1;
	}

	path[0] = '\0';
	return 0;
}


/* POSIX has no two-path atomic rename. Preserve both old directory entries so
 * a failed second rename can restore the first without leaving mixed files. */
static inline int
dnsmasq_file_backup_create(const char *destination, char *backup_path)
{
	int fd, saved_errno;
	struct stat original, backup;

	if (!destination || !destination[0] || !backup_path || !backup_path[0]) {
		errno = EINVAL;
		return -1;
	}
	if (lstat(destination, &original) != 0) {
		if (errno == ENOENT) {
			backup_path[0] = '\0';
			return 0;
		}
		return -1;
	}
	fd = mkstemp(backup_path);
	if (fd < 0)
		return -1;
	if (close(fd) != 0) {
		saved_errno = errno ? errno : EIO;
		if (unlink(backup_path) == 0 || errno == ENOENT)
			backup_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (unlink(backup_path) != 0)
		return -1;
	if (link(destination, backup_path) != 0) {
		saved_errno = errno ? errno : EIO;
		backup_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (lstat(backup_path, &backup) != 0) {
		saved_errno = errno ? errno : EIO;
		if (unlink(backup_path) == 0 || errno == ENOENT)
			backup_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (original.st_dev != backup.st_dev || original.st_ino != backup.st_ino) {
		if (unlink(backup_path) == 0 || errno == ENOENT)
			backup_path[0] = '\0';
		errno = EAGAIN;
		return -1;
	}
	return 1;
}

static inline int
dnsmasq_file_backup_discard_with_unlink(char *backup_path,
					dnsmasq_file_unlink_fn unlink_fn)
{
	return dnsmasq_file_discard_with_unlink(backup_path, unlink_fn);
}

static inline int
dnsmasq_file_backup_discard(char *backup_path)
{
	return dnsmasq_file_backup_discard_with_unlink(backup_path, unlink);
}

/* An empty backup pathname means the destination did not exist at snapshot. */
static inline int
dnsmasq_file_backup_restore(const char *destination, char *backup_path)
{
	if (backup_path && backup_path[0]) {
		if (rename(backup_path, destination) != 0)
			return -1;
		backup_path[0] = '\0';
		return 0;
	}
	if (unlink(destination) != 0 && errno != ENOENT)
		return -1;
	return 0;
}


#define DNSMASQ_FILE_SNAPSHOT_SET_MAX 3

/*
 * Keep a hard-link snapshot of a small set of published outputs while a
 * higher-level service transition is in progress. Empty backup names record
 * destinations that did not exist and must therefore be removed on rollback.
 */
static inline int
dnsmasq_file_snapshot_set_create(const char **destinations,
				 char **backup_paths, size_t count)
{
	size_t i, j;
	int saved_errno;

	if (!destinations || !backup_paths || count == 0 ||
	    count > DNSMASQ_FILE_SNAPSHOT_SET_MAX) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < count; ++i) {
		if (!destinations[i] || !destinations[i][0] ||
		    !backup_paths[i] || !backup_paths[i][0]) {
			errno = EINVAL;
			return -1;
		}
		for (j = 0; j < i; ++j) {
			if (strcmp(destinations[i], destinations[j]) == 0) {
				errno = EINVAL;
				return -1;
			}
		}
	}
	for (i = 0; i < count; ++i) {
		if (dnsmasq_file_backup_create(destinations[i], backup_paths[i]) < 0) {
			saved_errno = errno ? errno : EIO;
			for (j = 0; j < count; ++j) {
				if (backup_paths[j] && backup_paths[j][0])
					(void)dnsmasq_file_backup_discard(backup_paths[j]);
			}
			errno = saved_errno;
			return -1;
		}
	}
	return 0;
}

static inline int
dnsmasq_file_snapshot_set_restore(const char **destinations,
				  char **backup_paths, size_t count)
{
	size_t i;
	int result = 0, saved_errno = 0;

	if (!destinations || !backup_paths || count == 0 ||
	    count > DNSMASQ_FILE_SNAPSHOT_SET_MAX) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < count; ++i) {
		if (!destinations[i] || !destinations[i][0] || !backup_paths[i]) {
			errno = EINVAL;
			return -1;
		}
	}
	for (i = count; i > 0; --i) {
		if (dnsmasq_file_backup_restore(destinations[i - 1],
						backup_paths[i - 1]) != 0) {
			if (result == 0)
				saved_errno = errno ? errno : EIO;
			result = -1;
		}
	}
	if (result != 0)
		errno = saved_errno ? saved_errno : EIO;
	return result;
}

static inline int
dnsmasq_file_snapshot_set_discard(const char **destinations,
				  char **backup_paths, size_t count)
{
	size_t i;
	int result = 0, saved_errno = 0;

	if (!destinations || !backup_paths || count == 0 ||
	    count > DNSMASQ_FILE_SNAPSHOT_SET_MAX) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < count; ++i) {
		if (!destinations[i] || !destinations[i][0] || !backup_paths[i]) {
			errno = EINVAL;
			return -1;
		}
	}
	for (i = count; i > 0; --i) {
		if (dnsmasq_file_backup_discard(backup_paths[i - 1]) != 0) {
			if (result == 0)
				saved_errno = errno ? errno : EIO;
			result = -1;
		}
	}
	if (result != 0)
		errno = saved_errno ? saved_errno : EIO;
	return result;
}

static inline int
dnsmasq_file_publish_pair(FILE *first_fp, char *first_stage_path,
			  const char *first_destination, char *first_backup_path,
			  FILE *second_fp, char *second_stage_path,
			  const char *second_destination, char *second_backup_path,
			  mode_t mode)
{
	int saved_errno = 0, preserve_first_backup = 0;

	if (!first_fp || !first_stage_path || !first_stage_path[0] ||
	    !first_destination || !first_destination[0] ||
	    !first_backup_path || !first_backup_path[0] ||
	    !second_fp || !second_stage_path || !second_stage_path[0] ||
	    !second_destination || !second_destination[0] ||
	    !second_backup_path || !second_backup_path[0] ||
	    strcmp(first_destination, second_destination) == 0) {
		saved_errno = EINVAL;
		goto failed;
	}
	if (dnsmasq_file_backup_create(first_destination, first_backup_path) < 0 ||
	    dnsmasq_file_backup_create(second_destination, second_backup_path) < 0)
		goto failed;

	if (dnsmasq_file_publish(first_fp, first_stage_path,
				 first_destination, mode) != 0) {
		first_fp = NULL;
		goto failed;
	}
	first_fp = NULL;
	if (dnsmasq_file_publish(second_fp, second_stage_path,
				 second_destination, mode) != 0) {
		second_fp = NULL;
		saved_errno = errno ? errno : EIO;
		if (dnsmasq_file_backup_restore(first_destination,
						first_backup_path) != 0)
			preserve_first_backup = first_backup_path[0] != '\0';
		goto failed;
	}
	second_fp = NULL;
	dnsmasq_file_backup_discard(first_backup_path);
	dnsmasq_file_backup_discard(second_backup_path);
	return 0;

failed:
	if (!saved_errno)
		saved_errno = errno ? errno : EIO;
	if (first_fp)
		(void)fclose(first_fp);
	if (second_fp)
		(void)fclose(second_fp);
	dnsmasq_file_discard(first_stage_path);
	dnsmasq_file_discard(second_stage_path);
	if (!preserve_first_backup)
		dnsmasq_file_backup_discard(first_backup_path);
	dnsmasq_file_backup_discard(second_backup_path);
	errno = saved_errno;
	return -1;
}


/* Publish up to three related outputs as one rollback-capable transaction.
 * POSIX lacks a multi-path atomic rename; snapshots restore every path already
 * replaced if a later rename fails. If restoration fails, keep all remaining
 * snapshots for manual recovery rather than deleting the only old copies. */
static inline int
dnsmasq_file_publish_set(FILE **staged_files, char **staged_paths,
			 const char **destinations, char **backup_paths,
			 size_t count, mode_t mode)
{
	size_t i, published = 0;
	int saved_errno = 0;
	int rollback_errno = 0;
	int rollback_failed = 0;

	if (!staged_files || !staged_paths || !destinations || !backup_paths ||
	    count == 0 || count > 3) {
		errno = EINVAL;
		return -1;
	}

	for (i = 0; i < count; i++) {
		size_t j;

		if (!staged_files[i] || !staged_paths[i] || !staged_paths[i][0] ||
		    !destinations[i] || !destinations[i][0] ||
		    !backup_paths[i] || !backup_paths[i][0]) {
			saved_errno = EINVAL;
			goto failed;
		}
		for (j = 0; j < i; j++) {
			if (strcmp(destinations[i], destinations[j]) == 0) {
				saved_errno = EINVAL;
				goto failed;
			}
		}
	}

	for (i = 0; i < count; i++) {
		if (dnsmasq_file_backup_create(destinations[i], backup_paths[i]) < 0)
			goto failed;
	}

	for (i = 0; i < count; i++) {
		FILE *staged_fp = staged_files[i];

		staged_files[i] = NULL;
		if (dnsmasq_file_publish(staged_fp, staged_paths[i],
					 destinations[i], mode) != 0) {
			saved_errno = errno ? errno : EIO;
			goto rollback;
		}
		published++;
	}
	for (i = 0; i < count; i++)
		dnsmasq_file_backup_discard(backup_paths[i]);
	return 0;

rollback:
	while (published > 0) {
		published--;
		if (dnsmasq_file_backup_restore(destinations[published],
						backup_paths[published]) != 0) {
			rollback_failed = 1;
			if (!rollback_errno)
				rollback_errno = errno ? errno : EIO;
		}
	}
	if (rollback_errno)
		saved_errno = rollback_errno;

failed:
	if (!saved_errno)
		saved_errno = errno ? errno : EIO;
	for (i = 0; i < count; i++) {
		if (staged_files[i]) {
			(void)fclose(staged_files[i]);
			staged_files[i] = NULL;
		}
		dnsmasq_file_discard(staged_paths[i]);
		if (!rollback_failed)
			dnsmasq_file_backup_discard(backup_paths[i]);
	}
	errno = saved_errno;
	return -1;
}

/* Validate a last-known-good server file without following the final path. */
static int
dnsmasq_file_is_safe_regular(const char *path)
{
	struct stat st;

	if (!path || !path[0]) {
		errno = EINVAL;
		return -1;
	}
	if (lstat(path, &st) != 0)
		return -1;
	if (!S_ISREG(st.st_mode)) {
		errno = EINVAL;
		return -1;
	}
	if (st.st_uid != geteuid()) {
		errno = EPERM;
		return -1;
	}
	if (st.st_nlink != 1) {
		errno = EMLINK;
		return -1;
	}
	if (st.st_mode & (S_IWGRP | S_IWOTH)) {
		errno = EPERM;
		return -1;
	}

	return 0;
}

/* Compare byte-for-byte only after validating both paths.  Any open, read or
 * close error means "different", so an uncertain comparison cannot suppress
 * publication of a complete resolver file. */
static int
dnsmasq_file_contents_equal(const char *path, const char *staged_path)
{
	FILE *current, *staged;
	int current_char, staged_char;
	int equal = 1;

	/* Validate the descriptors actually read, not pathnames checked earlier. */
	current = dnsmasq_file_open_safe_read(path);
	if (!current)
		return 0;
	staged = dnsmasq_file_open_safe_read(staged_path);
	if (!staged) {
		fclose(current);
		return 0;
	}

	for (;;) {
		current_char = fgetc(current);
		staged_char = fgetc(staged);
		if (current_char != staged_char) {
			equal = 0;
			break;
		}
		if (current_char == EOF) {
			if (ferror(current) || ferror(staged))
				equal = 0;
			break;
		}
	}

	if (fclose(current) != 0)
		equal = 0;
	if (fclose(staged) != 0)
		equal = 0;

	return equal;
}

/* dnsmasq opens this predictable lease pathname as part of service startup.
 * Refuse links, non-regular files and multiply-linked files before changing
 * permissions; opening never truncates existing lease records. */
static int
dnsmasq_lease_file_ensure(const char *path)
{
	int fd;
	int saved_errno;
	struct stat st;
	mode_t lease_mode;

	if (!path || !path[0]) {
		errno = EINVAL;
		return -1;
	}

	fd = open(path, O_RDWR | O_CREAT | O_NOFOLLOW | O_NONBLOCK,
		   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0)
		return -1;

	if (fstat(fd, &st) != 0) {
		saved_errno = errno ? errno : EIO;
		close(fd);
		errno = saved_errno;
		return -1;
	}
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		close(fd);
		errno = !S_ISREG(st.st_mode) ? EINVAL :
			(st.st_uid != geteuid() ? EPERM : EMLINK);
		return -1;
	}
	/* Preserve a restrictive umask/mode while ensuring the owner can maintain leases. */
	lease_mode = (st.st_mode & (S_IRUSR | S_IRGRP | S_IROTH)) | S_IRUSR | S_IWUSR;
	if (fchmod(fd, lease_mode) != 0) {
		saved_errno = errno ? errno : EIO;
		close(fd);
		errno = saved_errno;
		return -1;
	}
	if (close(fd) != 0)
		return -1;

	return 0;
}

#endif /* PADAVAN_DNSMASQ_FILE_H */
