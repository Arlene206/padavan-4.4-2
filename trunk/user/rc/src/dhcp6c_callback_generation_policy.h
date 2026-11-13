#ifndef PADAVAN_DHCP6C_CALLBACK_GENERATION_POLICY_H
#define PADAVAN_DHCP6C_CALLBACK_GENERATION_POLICY_H

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DHCP6C_CALLBACK_LOCK_ATTEMPTS 500

static int
dhcp6c_callback_generation_parse(const char *text, unsigned long *generation_out)
{
	const unsigned char *p;
	unsigned long generation, digit;

	if (generation_out)
		*generation_out = 0;
	if (!text || !text[0] || !generation_out) {
		errno = EINVAL;
		return -1;
	}
	p = (const unsigned char *)text;
	if (*p == '0' && p[1] != '\0') {
		errno = EINVAL;
		return -1;
	}
	generation = 0;
	for (; *p; ++p) {
		if (*p < '0' || *p > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned long)(*p - '0');
		if (generation > (ULONG_MAX - digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		generation = generation * 10UL + digit;
	}
	if (generation == 0) {
		errno = EINVAL;
		return -1;
	}
	*generation_out = generation;
	return 0;
}

static int
dhcp6c_callback_generation_next(unsigned long current, unsigned long *next_out)
{
	if (!next_out || current == ULONG_MAX) {
		errno = current == ULONG_MAX ? ERANGE : EINVAL;
		return -1;
	}
	*next_out = current + 1UL;
	return 0;
}

static int
dhcp6c_callback_generation_format(unsigned long generation, char *out,
				    size_t out_size)
{
	char reversed[sizeof(unsigned long) * CHAR_BIT + 1];
	size_t used, i;
	unsigned long value;

	if (!out || out_size == 0 || generation == 0) {
		errno = EINVAL;
		return -1;
	}
	used = 0;
	value = generation;
	while (value > 0) {
		reversed[used++] = (char)('0' + (value % 10UL));
		value /= 10UL;
	}
	if (used + 1 > out_size) {
		out[0] = '\0';
		errno = EOVERFLOW;
		return -1;
	}
	for (i = 0; i < used; ++i)
		out[i] = reversed[used - i - 1];
	out[used] = '\0';
	return 0;
}

/* The file is both the generation record and a cross-process callback lock.
 * A callback retains the lock while publishing state; start/stop retain it
 * while changing generations and launching/terminating the client. */
static int
dhcp6c_callback_session_lock(const char *path, unsigned long *generation,
			      int allow_empty)
{
	int fd, attempt, locked, created;
	struct stat st;
	struct flock file_lock;
	char state_text[sizeof(unsigned long) * CHAR_BIT + 2];
	size_t used;
	ssize_t count;

	if (!path || !path[0] || !generation) {
		errno = EINVAL;
		return -1;
	}

	created = 0;
	if (allow_empty) {
		fd = open(path, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_NONBLOCK, 0600);
		if (fd >= 0) {
			created = 1;
		} else if (errno == EEXIST) {
			fd = open(path, O_RDWR | O_NOFOLLOW | O_NONBLOCK);
		}
	} else {
		/* A callback must never create an empty generation file while trying
		 * to validate itself; doing so could poison later start/stop recovery. */
		fd = open(path, O_RDWR | O_NOFOLLOW | O_NONBLOCK);
	}
	if (fd < 0)
		return -1;

	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		errno = !S_ISREG(st.st_mode) ? EINVAL :
			(st.st_uid != geteuid() ? EPERM : EMLINK);
		goto failed;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0 ||
	    fcntl(fd, F_SETFD, FD_CLOEXEC) != 0)
		goto failed;

	memset(&file_lock, 0, sizeof(file_lock));
	file_lock.l_type = F_WRLCK;
	file_lock.l_whence = SEEK_SET;
	file_lock.l_start = 0;
	file_lock.l_len = 0;
	locked = 0;
	for (attempt = 0; attempt < DHCP6C_CALLBACK_LOCK_ATTEMPTS; ++attempt) {
		if (fcntl(fd, F_SETLK, &file_lock) == 0) {
			locked = 1;
			break;
		}
		if (errno == EINTR)
			continue;
		if (errno != EACCES && errno != EAGAIN)
			goto failed;
		usleep(10000);
	}
	if (!locked) {
		errno = EBUSY;
		goto failed;
	}

	if (lseek(fd, 0, SEEK_SET) < 0)
		goto failed_locked;
	used = 0;
	for (;;) {
		count = read(fd, state_text + used, sizeof(state_text) - 1 - used);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			goto failed_locked;
		}
		if (count == 0)
			break;
		used += (size_t)count;
		if (used >= sizeof(state_text) - 1) {
			errno = EOVERFLOW;
			goto failed_locked;
		}
	}
	state_text[used] = '\0';
	if (used > 0 && state_text[used - 1] == '\n')
		state_text[--used] = '\0';
	if (used == 0) {
		if (!created || !allow_empty) {
			errno = ENOENT;
			goto failed_locked;
		}
		*generation = 0;
	} else if (dhcp6c_callback_generation_parse(state_text, generation) != 0) {
		goto failed_locked;
	}

	return fd;

failed_locked:
	file_lock.l_type = F_UNLCK;
	(void)fcntl(fd, F_SETLK, &file_lock);
failed:
	{
		int saved_errno = errno ? errno : EIO;
		(void)close(fd);
		errno = saved_errno;
	}
	return -1;
}

static int
dhcp6c_callback_session_write(int fd, unsigned long generation)
{
	char state_text[sizeof(unsigned long) * CHAR_BIT + 2];
	size_t length, written;
	ssize_t count;

	if (fd < 0 || dhcp6c_callback_generation_format(generation,
							 state_text, sizeof(state_text)) != 0) {
		if (fd < 0)
			errno = EINVAL;
		return -1;
	}
	length = strlen(state_text);
	state_text[length++] = '\n';
	if (lseek(fd, 0, SEEK_SET) < 0 || ftruncate(fd, 0) != 0)
		return -1;
	written = 0;
	while (written < length) {
		count = write(fd, state_text + written, length - written);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (count == 0) {
			errno = EIO;
			return -1;
		}
		written += (size_t)count;
	}
	if (fsync(fd) != 0)
		return -1;
	return 0;
}

static int
dhcp6c_callback_generation_matches(const char *text, unsigned long current,
				    const char *expected_ifname,
				    const char *callback_ifname)
{
	unsigned long supplied;

	if (current == 0 || !expected_ifname || !expected_ifname[0] ||
	    !callback_ifname || !callback_ifname[0] ||
	    strcmp(expected_ifname, callback_ifname) != 0 ||
	    dhcp6c_callback_generation_parse(text, &supplied) != 0)
		return 0;
	return supplied == current;
}

static int
dhcp6c_callback_session_unlock(int fd)
{
	struct flock file_lock;
	int result = 0;
	int saved_errno = 0;

	if (fd < 0) {
		errno = EINVAL;
		return -1;
	}
	memset(&file_lock, 0, sizeof(file_lock));
	file_lock.l_type = F_UNLCK;
	file_lock.l_whence = SEEK_SET;
	file_lock.l_start = 0;
	file_lock.l_len = 0;
	if (fcntl(fd, F_SETLK, &file_lock) != 0) {
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

#endif /* PADAVAN_DHCP6C_CALLBACK_GENERATION_POLICY_H */
