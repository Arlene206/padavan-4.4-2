#ifndef _RAETH_MII_MGR_TIMEOUT_POLICY_H
#define _RAETH_MII_MGR_TIMEOUT_POLICY_H

/*
 * MDIO polling runs with local interrupts disabled while mdio_lock is held.
 * jiffies cannot be relied on to advance in that context. Use elapsed
 * monotonic nanoseconds supplied by ktime_get_ns() instead.
 */
#define MDIO_BUSY_TIMEOUT_NS	100000000ULL

static inline int
mii_mgr_busy_timeout_expired(unsigned long long start_ns,
			     unsigned long long now_ns)
{
	return now_ns - start_ns >= MDIO_BUSY_TIMEOUT_NS;
}

#endif
