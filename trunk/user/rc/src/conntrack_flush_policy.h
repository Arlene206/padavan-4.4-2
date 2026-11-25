#ifndef RC_CONNTRACK_FLUSH_POLICY_H
#define RC_CONNTRACK_FLUSH_POLICY_H
#include <errno.h>
typedef int (*rc_conntrack_flush_write_fn)(const char *path, const char *value);
static inline int
rc_conntrack_flush_apply(const char *ip, rc_conntrack_flush_write_fn write_file)
{
	if (!write_file)
		return EINVAL;
	return write_file("/proc/net/nf_conntrack", ip ? ip : "f");
}
#endif
