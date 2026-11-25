#ifndef __OPENVPN_SCRIPT_LINK_H__
#define __OPENVPN_SCRIPT_LINK_H__

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define OPENVPN_SCRIPT_LINK_TARGET_SIZE 128

/* Verify both the link type and its literal target; never follow the link. */
static inline int
openvpn_script_link_matches(const char *path, const char *target)
{
	struct stat before, after;
	char actual[OPENVPN_SCRIPT_LINK_TARGET_SIZE];
	ssize_t length;

	if (!path || !path[0] || !target || !target[0]) {
		errno = EINVAL;
		return -1;
	}
	if (lstat(path, &before) != 0)
		return -1;
	if (!S_ISLNK(before.st_mode)) {
		errno = EEXIST;
		return -1;
	}
	length = readlink(path, actual, sizeof(actual) - 1);
	if (length < 0)
		return -1;
	if ((size_t)length >= sizeof(actual) - 1) {
		errno = ENAMETOOLONG;
		return -1;
	}
	actual[length] = '\0';
	if (lstat(path, &after) != 0)
		return -1;
	if (before.st_dev != after.st_dev || before.st_ino != after.st_ino ||
	    !S_ISLNK(after.st_mode)) {
		errno = EAGAIN;
		return -1;
	}
	if (strcmp(actual, target) != 0) {
		errno = EEXIST;
		return -1;
	}
	return 0;
}

/* Idempotently create the expected callback link; report ownership to caller. */
static inline int
openvpn_script_link_ensure(const char *path, const char *target, int *created)
{
	int saved_errno;

	if (!path || !path[0] || !target || !target[0] || !created) {
		errno = EINVAL;
		return -1;
	}
	*created = 0;
	if (openvpn_script_link_matches(path, target) == 0)
		return 0;
	saved_errno = errno;
	if (saved_errno != ENOENT) {
		errno = saved_errno;
		return -1;
	}
	if (symlink(target, path) == 0) {
		*created = 1;
		return 0;
	}
	saved_errno = errno;
	/* Another start may have installed the exact expected link. */
	if (saved_errno == EEXIST &&
	    openvpn_script_link_matches(path, target) == 0)
		return 0;
	errno = saved_errno;
	return -1;
}

/* Roll back only a link created by this start attempt. */
static inline int
openvpn_script_link_remove_created(const char *path, int created)
{
	if (!created)
		return 0;
	if (unlink(path) == 0 || errno == ENOENT)
		return 0;
	return -1;
}

#endif /* __OPENVPN_SCRIPT_LINK_H__ */
