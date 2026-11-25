#ifndef IPV6_RESTART_POLICY_H
#define IPV6_RESTART_POLICY_H

/* Keep the last applied IPv6 mode until the whole restart succeeds. */
static inline int
ipv6_restart_should_publish_type(int restart_status)
{
	return restart_status == 0;
}

#endif /* IPV6_RESTART_POLICY_H */
