#ifndef PADAVAN_SAMBA_CONFIG_TRANSACTION_H
#define PADAVAN_SAMBA_CONFIG_TRANSACTION_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SAMBA_CONFIG_TRANSACTION_DIRECTORY "/etc/samba/.config-transaction"
#define SAMBA_CONFIG_TRANSACTION_LOCK_FILE "/var/run/samba-config.lock"
#define SAMBA_CONFIG_TRANSACTION_PASSDB "/etc/samba/smbpasswd"
#define SAMBA_CONFIG_TRANSACTION_CONFIG "/etc/smb.conf"
#define SAMBA_CONFIG_TRANSACTION_PATH_MAX 512

/* The kernel releases this process-scoped lock if the writer exits. */
static inline int
samba_config_transaction_lock_acquire(const char *lock_path)
{
	struct flock lock;
	int fd, result;

	if (!lock_path || !*lock_path) {
		errno = EINVAL;
		return -1;
	}
	fd = open(lock_path, O_RDWR | O_CREAT, 0600);
	if (fd < 0)
		return -1;
	if (fchmod(fd, 0600) != 0 ||
	    fcntl(fd, F_SETFD, FD_CLOEXEC) != 0) {
		int saved_errno = errno ? errno : EIO;
		close(fd);
		errno = saved_errno;
		return -1;
	}

	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	do {
		result = fcntl(fd, F_SETLKW, &lock);
	} while (result != 0 && errno == EINTR);
	if (result != 0) {
		int saved_errno = errno ? errno : EIO;
		close(fd);
		errno = saved_errno;
		return -1;
	}
	return fd;
}

/* Closing the sole descriptor releases the process-scoped advisory lock. */
static inline int
samba_config_transaction_lock_release(int lock_fd)
{
	if (lock_fd < 0)
		return 0;
	return close(lock_fd);
}

static inline int
samba_config_transaction_make_path(char *path, size_t path_size,
                                   const char *directory, const char *leaf)
{
	int length;

	if (!path || path_size == 0 || !directory || !*directory ||
	    !leaf || !*leaf) {
		errno = EINVAL;
		return -1;
	}
	length = snprintf(path, path_size, "%s/%s", directory, leaf);
	if (length < 0 || (size_t)length >= path_size) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return 0;
}

static inline int
samba_config_transaction_snapshot(const char *target_path,
                                  const char *snapshot_path,
                                  int *had_original)
{
	struct stat st;

	if (!target_path || !*target_path || !snapshot_path ||
	    !*snapshot_path || !had_original) {
		errno = EINVAL;
		return -1;
	}
	*had_original = 0;
	if (lstat(target_path, &st) != 0) {
		if (errno == ENOENT)
			return 0;
		return -1;
	}
	if (!S_ISREG(st.st_mode)) {
		errno = EINVAL;
		return -1;
	}
	if (link(target_path, snapshot_path) != 0)
		return -1;
	*had_original = 1;
	return 0;
}

static inline int
samba_config_transaction_write_state(const char *directory,
                                     int passdb_had_original,
                                     int config_had_original,
                                     const char *phase)
{
	char state_path[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	char next_path[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	char content[128];
	size_t offset;
	int fd, length, saved_errno;
	ssize_t written;

	if (!phase || (strcmp(phase, "PREPARED") != 0 &&
		       strcmp(phase, "COMMITTED") != 0) ||
	    samba_config_transaction_make_path(state_path, sizeof(state_path),
						directory, "state") != 0 ||
	    samba_config_transaction_make_path(next_path, sizeof(next_path),
						directory, "state.new") != 0)
		return -1;

	length = snprintf(content, sizeof(content),
			  "PADAVAN_SAMBA_CONFIG_TXN_V1\nPASSDB=%d\nCONFIG=%d\n%s\n",
			  passdb_had_original ? 1 : 0,
			  config_had_original ? 1 : 0, phase);
	if (length < 0 || (size_t)length >= sizeof(content)) {
		errno = EOVERFLOW;
		return -1;
	}
	if (unlink(next_path) != 0 && errno != ENOENT)
		return -1;
	fd = open(next_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (fd < 0)
		return -1;

	offset = 0;
	while (offset < (size_t)length) {
		written = write(fd, content + offset, (size_t)length - offset);
		if (written < 0 && errno == EINTR)
			continue;
		if (written <= 0) {
			saved_errno = errno ? errno : EIO;
			close(fd);
			unlink(next_path);
			errno = saved_errno;
			return -1;
		}
		offset += (size_t)written;
	}
	if (fsync(fd) != 0) {
		saved_errno = errno ? errno : EIO;
		close(fd);
		unlink(next_path);
		errno = saved_errno;
		return -1;
	}
	if (close(fd) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(next_path);
		errno = saved_errno;
		return -1;
	}
	if (rename(next_path, state_path) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(next_path);
		errno = saved_errno;
		return -1;
	}
	return 0;
}

/* Return 1 for a valid state, 0 when no state was published, -1 on error. */
static inline int
samba_config_transaction_read_state(const char *directory,
                                    int *passdb_had_original,
                                    int *config_had_original,
                                    int *is_committed)
{
	char state_path[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	char content[128], expected[128];
	size_t offset;
	int fd, passdb, config, phase, length;
	ssize_t count;

	if (!passdb_had_original || !config_had_original || !is_committed ||
	    samba_config_transaction_make_path(state_path, sizeof(state_path),
						directory, "state") != 0)
		return -1;
	fd = open(state_path, O_RDONLY);
	if (fd < 0) {
		if (errno == ENOENT)
			return 0;
		return -1;
	}

	offset = 0;
	while (offset < sizeof(content) - 1) {
		count = read(fd, content + offset, sizeof(content) - 1 - offset);
		if (count < 0 && errno == EINTR)
			continue;
		if (count < 0) {
			int saved_errno = errno ? errno : EIO;
			close(fd);
			errno = saved_errno;
			return -1;
		}
		if (count == 0)
			break;
		offset += (size_t)count;
	}
	if (offset == sizeof(content) - 1) {
		char extra;
		count = read(fd, &extra, 1);
		if (count != 0) {
			close(fd);
			errno = EINVAL;
			return -1;
		}
	}
	if (close(fd) != 0)
		return -1;
	content[offset] = '\0';

	for (passdb = 0; passdb <= 1; ++passdb) {
		for (config = 0; config <= 1; ++config) {
			for (phase = 0; phase <= 1; ++phase) {
				length = snprintf(expected, sizeof(expected),
						  "PADAVAN_SAMBA_CONFIG_TXN_V1\nPASSDB=%d\nCONFIG=%d\n%s\n",
						  passdb, config,
						  phase ? "COMMITTED" : "PREPARED");
				if (length < 0 || (size_t)length >= sizeof(expected))
					continue;
				if (strcmp(content, expected) == 0) {
					*passdb_had_original = passdb;
					*config_had_original = config;
					*is_committed = phase;
					return 1;
				}
			}
		}
	}
	errno = EINVAL;
	return -1;
}

static inline int
samba_config_transaction_restore_file(const char *target_path,
                                      const char *snapshot_path,
                                      const char *restore_path,
                                      int had_original)
{
	struct stat st;
	int saved_errno;

	if (!target_path || !*target_path || !snapshot_path ||
	    !*snapshot_path || !restore_path || !*restore_path) {
		errno = EINVAL;
		return -1;
	}
	if (!had_original) {
		if (unlink(target_path) == 0 || errno == ENOENT)
			return 0;
		return -1;
	}
	if (lstat(snapshot_path, &st) != 0)
		return -1;
	if (!S_ISREG(st.st_mode)) {
		errno = EINVAL;
		return -1;
	}
	if (unlink(restore_path) != 0 && errno != ENOENT)
		return -1;
	if (link(snapshot_path, restore_path) != 0)
		return -1;
	if (rename(restore_path, target_path) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(restore_path);
		errno = saved_errno;
		return -1;
	}
	return 0;
}

static inline int
samba_config_transaction_cleanup(const char *directory)
{
	static const char *const files[] = {
		"passdb.old", "config.old", "passdb.restore", "config.restore",
		"state.new", "state", NULL
	};
	char path[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	int i, result = 0;

	for (i = 0; files[i] != NULL; ++i) {
		if (samba_config_transaction_make_path(path, sizeof(path),
						       directory, files[i]) != 0)
			return -1;
		if (unlink(path) != 0 && errno != ENOENT)
			result = -1;
	}
	if (result != 0)
		return -1;
	if (rmdir(directory) != 0 && errno != ENOENT)
		return -1;
	return 0;
}

/*
 * PREPARED is published after both snapshots and before either live rename.
 * Recovery is idempotent: snapshots remain until both original files return.
 */
static inline int
samba_config_transaction_recover(const char *directory,
                                 const char *passdb_path,
                                 const char *config_path)
{
	struct stat st;
	char passdb_snapshot[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	char config_snapshot[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	char passdb_restore[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	char config_restore[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	int passdb_had_original, config_had_original, is_committed;
	int state_result;

	if (!directory || !*directory || !passdb_path || !*passdb_path ||
	    !config_path || !*config_path) {
		errno = EINVAL;
		return -1;
	}
	if (lstat(directory, &st) != 0) {
		if (errno == ENOENT)
			return 0;
		return -1;
	}
	if (!S_ISDIR(st.st_mode) || (st.st_mode & 0777) != 0700) {
		errno = EINVAL;
		return -1;
	}
	state_result = samba_config_transaction_read_state(directory,
			&passdb_had_original, &config_had_original, &is_committed);
	if (state_result < 0)
		return -1;
	if (state_result == 0)
		return samba_config_transaction_cleanup(directory);

	if (!is_committed) {
		if (samba_config_transaction_make_path(passdb_snapshot,
				sizeof(passdb_snapshot), directory, "passdb.old") != 0 ||
		    samba_config_transaction_make_path(config_snapshot,
				sizeof(config_snapshot), directory, "config.old") != 0 ||
		    samba_config_transaction_make_path(passdb_restore,
				sizeof(passdb_restore), directory, "passdb.restore") != 0 ||
		    samba_config_transaction_make_path(config_restore,
				sizeof(config_restore), directory, "config.restore") != 0)
			return -1;
		if (samba_config_transaction_restore_file(passdb_path,
				passdb_snapshot, passdb_restore,
				passdb_had_original) != 0)
			return -1;
		if (samba_config_transaction_restore_file(config_path,
				config_snapshot, config_restore,
				config_had_original) != 0)
			return -1;
	}
	return samba_config_transaction_cleanup(directory);
}

static inline int
samba_config_transaction_begin(const char *directory,
                               const char *passdb_path,
                               const char *config_path)
{
	char passdb_snapshot[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	char config_snapshot[SAMBA_CONFIG_TRANSACTION_PATH_MAX];
	int passdb_had_original = 0, config_had_original = 0;
	int saved_errno;

	if (samba_config_transaction_recover(directory, passdb_path,
					     config_path) != 0)
		return -1;
	if (mkdir(directory, 0700) != 0)
		return -1;
	if (samba_config_transaction_make_path(passdb_snapshot,
			sizeof(passdb_snapshot), directory, "passdb.old") != 0 ||
	    samba_config_transaction_make_path(config_snapshot,
			sizeof(config_snapshot), directory, "config.old") != 0 ||
	    samba_config_transaction_snapshot(passdb_path, passdb_snapshot,
					      &passdb_had_original) != 0 ||
	    samba_config_transaction_snapshot(config_path, config_snapshot,
					      &config_had_original) != 0 ||
	    samba_config_transaction_write_state(directory, passdb_had_original,
						 config_had_original, "PREPARED") != 0) {
		saved_errno = errno ? errno : EIO;
		samba_config_transaction_cleanup(directory);
		errno = saved_errno;
		return -1;
	}
	return 0;
}

/* Return 1 when COMMITTED cleanup remains, 0 when completely cleaned. */
static inline int
samba_config_transaction_commit(const char *directory,
                                const char *passdb_path,
                                const char *config_path)
{
	int passdb_had_original, config_had_original, is_committed;
	int state_result;

	(void)passdb_path;
	(void)config_path;
	state_result = samba_config_transaction_read_state(directory,
			&passdb_had_original, &config_had_original, &is_committed);
	if (state_result != 1 || is_committed) {
		if (state_result == 0)
			errno = EINVAL;
		return -1;
	}
	if (samba_config_transaction_write_state(directory,
			passdb_had_original, config_had_original,
			"COMMITTED") != 0)
		return -1;
	return samba_config_transaction_cleanup(directory) == 0 ? 0 : 1;
}

static inline int
samba_config_transaction_abort(const char *directory,
                               const char *passdb_path,
                               const char *config_path)
{
	return samba_config_transaction_recover(directory, passdb_path,
						 config_path);
}

#endif /* PADAVAN_SAMBA_CONFIG_TRANSACTION_H */
