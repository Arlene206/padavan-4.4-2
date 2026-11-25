#ifndef RC_UDHCPC_SCRIPT_LINK_POLICY_H
#define RC_UDHCPC_SCRIPT_LINK_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#define RC_UDHCPC_SCRIPT_LINK_TARGET_MAX 256

typedef struct {
	int (*create)(const char *target, const char *link_path);
	ssize_t (*read)(const char *link_path, char *buffer, size_t buffer_size);
} rc_udhcpc_script_link_ops_t;

/*
 * Create the event-handler link, accepting an existing link only when its
 * target is exactly the expected executable. Never replace an unrelated file
 * or symlink at the fixed /tmp pathname.
 *
 * Return zero on success, otherwise a positive errno-style status.
 */
static inline int
rc_udhcpc_script_link_ensure(const char *link_path, const char *target,
			     const rc_udhcpc_script_link_ops_t *ops)
{
	char existing_target[RC_UDHCPC_SCRIPT_LINK_TARGET_MAX];
	size_t target_length;
	ssize_t existing_length;
	int saved_errno;

	if (!link_path || !*link_path || !target || !*target || !ops ||
	    !ops->create || !ops->read)
		return EINVAL;

	target_length = strlen(target);
	if (target_length >= sizeof(existing_target))
		return ENAMETOOLONG;

	errno = 0;
	if (ops->create(target, link_path) == 0)
		return 0;
	saved_errno = errno ? errno : EIO;
	if (saved_errno != EEXIST)
		return saved_errno;

	/* EEXIST is idempotent only for the exact symlink we require. */
	errno = 0;
	existing_length = ops->read(link_path, existing_target,
				    sizeof(existing_target));
	if (existing_length < 0)
		return errno ? errno : EIO;
	if ((size_t)existing_length != target_length ||
	    memcmp(existing_target, target, target_length) != 0)
		return EEXIST;

	return 0;
}

#endif /* RC_UDHCPC_SCRIPT_LINK_POLICY_H */
