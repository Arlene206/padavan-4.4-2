#ifndef PADAVAN_PPP_CALLBACK_STATE_FILE_H
#define PADAVAN_PPP_CALLBACK_STATE_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ppp_callback_generation_policy.h"

/* Missing is an initial state only when explicitly allowed. An existing
 * empty/truncated file is corruption and must never reset the token. */
static int
ppp_callback_state_file_read(const char *path, int allow_missing,
			     unsigned long *generation_out)
{
	int fd, result = -1, saved_errno = 0;
	struct stat st;
	char state_text[sizeof(unsigned long) * CHAR_BIT + 2];
	size_t used = 0;
	ssize_t count;

	if (!path || !path[0] || !generation_out) {
		errno = EINVAL;
		return -1;
	}
	*generation_out = 0;
	fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
	if (fd < 0) {
		if (errno == ENOENT &&
		    ppp_callback_generation_initial_state_allowed(1, allow_missing))
			return 0;
		return -1;
	}
	if (fstat(fd, &st) != 0)
		goto out;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		errno = !S_ISREG(st.st_mode) ? EINVAL :
			(st.st_uid != geteuid() ? EPERM : EMLINK);
		goto out;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
		goto out;

	for (;;) {
		count = read(fd, state_text + used, sizeof(state_text) - 1 - used);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			goto out;
		}
		if (count == 0)
			break;
		used += (size_t)count;
		if (used >= sizeof(state_text) - 1) {
			errno = EOVERFLOW;
			goto out;
		}
	}
	state_text[used] = '\0';
	if (used > 0 && state_text[used - 1] == '\n')
		state_text[--used] = '\0';
	if (used == 0) {
		errno = ENOENT;
		goto out;
	}
	if (ppp_callback_generation_parse(state_text, generation_out) != 0)
		goto out;
	result = 0;

out:
	saved_errno = errno;
	if (close(fd) != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0)
		errno = saved_errno ? saved_errno : EIO;
	return result;
}

/* Write, fsync and close a same-directory inode before atomically replacing
 * the active generation. A failed publication removes only the stage. */
static int
ppp_callback_state_file_publish(const char *path, unsigned long generation)
{
	static const char suffix[] = ".tmp.XXXXXX";
	char generation_text[sizeof(unsigned long) * CHAR_BIT + 2];
	char *staged_path = NULL;
	size_t path_length, length, written;
	struct stat target_st;
	int fd = -1, result = -1, saved_errno = 0;
	ssize_t count;

	if (!path || !path[0] || generation == 0) {
		errno = EINVAL;
		return -1;
	}
	if (lstat(path, &target_st) == 0) {
		if (!S_ISREG(target_st.st_mode) || target_st.st_uid != geteuid() ||
		    target_st.st_nlink != 1) {
			errno = !S_ISREG(target_st.st_mode) ? EINVAL :
				(target_st.st_uid != geteuid() ? EPERM : EMLINK);
			return -1;
		}
	} else if (errno != ENOENT) {
		return -1;
	}
	if (ppp_callback_generation_format(generation, generation_text,
					  sizeof(generation_text)) != 0)
		return -1;
	length = strlen(generation_text);
	generation_text[length++] = '\n';

	path_length = strlen(path);
	if (path_length > (size_t)-1 - sizeof(suffix)) {
		errno = EOVERFLOW;
		return -1;
	}
	staged_path = (char *)malloc(path_length + sizeof(suffix));
	if (!staged_path)
		return -1;
	if (snprintf(staged_path, path_length + sizeof(suffix), "%s%s",
		     path, suffix) < 0) {
		errno = EIO;
		goto out;
	}
	fd = mkstemp(staged_path);
	if (fd < 0)
		goto out;
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
		goto out;
	written = 0;
	while (written < length) {
		count = write(fd, generation_text + written, length - written);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			goto out;
		}
		if (count == 0) {
			errno = EIO;
			goto out;
		}
		written += (size_t)count;
	}
	if (fsync(fd) != 0)
		goto out;
	if (close(fd) != 0) {
		fd = -1;
		goto out;
	}
	fd = -1;
	if (rename(staged_path, path) != 0)
		goto out;
	result = 0;

out:
	saved_errno = errno;
	if (fd >= 0 && close(fd) != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0 && staged_path)
		(void)unlink(staged_path);
	free(staged_path);
	if (result != 0)
		errno = saved_errno ? saved_errno : EIO;
	return result;
}

/* This sidecar inode is not replaced while callbacks are active. It serializes
 * readers with the atomic state-file rename and has independent validation. */
static int
ppp_callback_state_file_lock(const char *lock_path, const char *state_path,
			     unsigned long *generation_out, int allow_missing,
			     int attempts)
{
	int fd, attempt, locked = 0;
	struct stat st;
	struct flock lock;

	if (!lock_path || !lock_path[0] || !state_path || !state_path[0] ||
	    !generation_out || attempts <= 0) {
		errno = EINVAL;
		return -1;
	}
	fd = open(lock_path,
		  O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_NONBLOCK, 0600);
	if (fd < 0 && errno == EEXIST)
		fd = open(lock_path, O_RDWR | O_NOFOLLOW | O_NONBLOCK);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		errno = !S_ISREG(st.st_mode) ? EINVAL :
			(st.st_uid != geteuid() ? EPERM : EMLINK);
		goto failed;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
		goto failed;

	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	for (attempt = 0; attempt < attempts; ++attempt) {
		if (fcntl(fd, F_SETLK, &lock) == 0) {
			locked = 1;
			break;
		}
		if (errno == EINTR)
			continue;
		if (errno != EACCES && errno != EAGAIN)
			goto failed;
		if (attempt + 1 < attempts)
			(void)usleep(10000);
	}
	if (!locked) {
		errno = EBUSY;
		goto failed;
	}
	if (ppp_callback_state_file_read(state_path, allow_missing,
					 generation_out) != 0) {
		int saved_errno = errno ? errno : EIO;
		lock.l_type = F_UNLCK;
		(void)fcntl(fd, F_SETLK, &lock);
		(void)close(fd);
		errno = saved_errno;
		return -1;
	}
	return fd;

failed:
	{
		int saved_errno = errno ? errno : EIO;
		if (locked) {
			lock.l_type = F_UNLCK;
			(void)fcntl(fd, F_SETLK, &lock);
		}
		(void)close(fd);
		errno = saved_errno;
	}
	return -1;
}

static int
ppp_callback_state_file_unlock(int fd)
{
	struct flock lock;
	int result = 0, saved_errno = 0;

	if (fd < 0) {
		errno = EINVAL;
		return -1;
	}
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	if (fcntl(fd, F_SETLK, &lock) != 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (close(fd) != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0)
		errno = saved_errno;
	return result;
}

#endif /* PADAVAN_PPP_CALLBACK_STATE_FILE_H */
