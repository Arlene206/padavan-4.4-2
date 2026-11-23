#ifndef PADAVAN_SAMBA_PASSDB_POLICY_H
#define PADAVAN_SAMBA_PASSDB_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * The legacy integer getter clamps account counts into 0..MAX_ACCOUNT_NUM,
 * but it also turns NVRAM backend/parse failures into a plausible zero. Keep
 * the clamp for compatibility while propagating every failed checked read.
 */
static inline int
samba_passdb_account_count_status(int read_status, int value,
				  int maximum, int *account_count)
{
	if (!account_count || maximum < 0)
		return EINVAL;
	*account_count = 0;
	if (read_status != 0)
		return read_status > 0 ? read_status : EIO;
	if (value < 0)
		value = 0;
	else if (value > maximum)
		value = maximum;
	*account_count = value;
	return 0;
}


/* Match the firmware's 127-byte password limit and reject line controls. */
static int
samba_passdb_password_is_safe(const char *password)
{
	const unsigned char *p;
	size_t len;

	if (!password || !*password)
		return 0;

	len = strlen(password);
	if (len >= 128)
		return 0;

	for (p = (const unsigned char *)password; *p; ++p) {
		if (*p < 0x20 || *p == 0x7f)
			return 0;
	}

	return 1;
}


/*
 * Preserve the previous passdb with a same-directory hard link before
 * replacing it. A symlink or special file is an ambiguous passdb target and
 * fails closed; the hard link retains the original inode, mode and contents.
 */
static inline int
samba_passdb_backup_file(const char *passdb_path,
                         const char *backup_template,
                         char *backup_path, size_t backup_path_size,
                         int *had_original)
{
	struct stat st;
	int fd, length, saved_errno;

	if (!passdb_path || !*passdb_path || !backup_template ||
	    !*backup_template || !backup_path || backup_path_size == 0 ||
	    !had_original) {
		errno = EINVAL;
		return -1;
	}

	backup_path[0] = '\0';
	*had_original = 0;
	if (lstat(passdb_path, &st) != 0) {
		if (errno == ENOENT)
			return 0;
		return -1;
	}
	if (!S_ISREG(st.st_mode)) {
		errno = EINVAL;
		return -1;
	}

	length = snprintf(backup_path, backup_path_size, "%s", backup_template);
	if (length < 0 || (size_t)length >= backup_path_size) {
		backup_path[0] = '\0';
		errno = ENAMETOOLONG;
		return -1;
	}

	fd = mkstemp(backup_path);
	if (fd < 0) {
		backup_path[0] = '\0';
		return -1;
	}
	if (close(fd) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(backup_path);
		backup_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (unlink(backup_path) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(backup_path);
		backup_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (link(passdb_path, backup_path) != 0) {
		saved_errno = errno ? errno : EIO;
		backup_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}

	*had_original = 1;
	return 0;
}

/* Restore the last-known-good passdb or remove a newly-created first-run DB. */
static inline int
samba_passdb_restore_file(const char *passdb_path,
                          const char *backup_path, int had_original)
{
	if (!passdb_path || !*passdb_path) {
		errno = EINVAL;
		return -1;
	}

	if (had_original) {
		if (!backup_path || !*backup_path) {
			errno = EINVAL;
			return -1;
		}
		return rename(backup_path, passdb_path);
	}

	if (unlink(passdb_path) == 0 || errno == ENOENT)
		return 0;
	return -1;
}

/*
 * Select the account-name source for smb.conf. run_samba() supplies names
 * from its credential transaction; standalone writers fetch their own list.
 */
static inline int
samba_passdb_account_list_source_status(int has_snapshot,
				       int snapshot_count, int maximum,
				       int *use_snapshot)
{
	if (!use_snapshot || maximum < 0 ||
	    (has_snapshot != 0 && has_snapshot != 1))
		return EINVAL;
	*use_snapshot = 0;
	if (has_snapshot) {
		if (snapshot_count < 0 || snapshot_count > maximum)
			return EINVAL;
		*use_snapshot = 1;
		return 0;
	}
	return snapshot_count == 0 ? 0 : EINVAL;
}

/* Preserve a credential-read failure over a later transaction-close error. */
static inline int
samba_passdb_snapshot_end_status(int snapshot_status, int end_status,
				 int end_errno)
{
	if (snapshot_status != 0)
		return snapshot_status > 0 ? snapshot_status : EIO;
	if (end_status == 0)
		return 0;
	if (end_errno > 0)
		return end_errno;
	return end_status > 0 ? end_status : EIO;
}

#endif /* PADAVAN_SAMBA_PASSDB_POLICY_H */
