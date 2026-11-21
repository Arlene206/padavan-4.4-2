#ifndef __IPV6_GLOBAL_CONF_POLICY_H__
#define __IPV6_GLOBAL_CONF_POLICY_H__
#include <errno.h>
#include <stddef.h>
struct rc_ipv6_conf_operation { const char *ifname; const char *param; int value; };
typedef int (*rc_ipv6_conf_write_fn)(void *, const char *, const char *, int);
/* Attempt every requested write and retain the first failure. */
static inline int rc_ipv6_conf_apply(const struct rc_ipv6_conf_operation *operations,
 size_t operation_count, rc_ipv6_conf_write_fn write_operation, void *context)
{
 size_t i; int first_status = 0;
 if (operation_count == 0) return 0;
 if (!operations || !write_operation) return EINVAL;
 for (i = 0; i < operation_count; ++i) {
  int status = write_operation(context, operations[i].ifname, operations[i].param, operations[i].value);
  if (first_status == 0 && status != 0) first_status = status;
 }
 return first_status;
}
#endif
