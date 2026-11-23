#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "../mii_mgr_timeout_policy.h"

static void
test_timeout_not_expired_before_deadline(void)
{
	assert(!mii_mgr_busy_timeout_expired(0ULL, 0ULL));
	assert(!mii_mgr_busy_timeout_expired(0ULL, 99999999ULL));
	assert(!mii_mgr_busy_timeout_expired(123456789ULL, 223456788ULL));
}

static void
test_timeout_expires_at_deadline(void)
{
	assert(mii_mgr_busy_timeout_expired(0ULL, MDIO_BUSY_TIMEOUT_NS));
	assert(mii_mgr_busy_timeout_expired(123456789ULL,
					    123456789ULL + MDIO_BUSY_TIMEOUT_NS));
	assert(mii_mgr_busy_timeout_expired(0ULL, MDIO_BUSY_TIMEOUT_NS + 1ULL));
}

static void
test_unsigned_elapsed_time_survives_counter_wrap(void)
{
	unsigned long long start_ns = ULLONG_MAX - 50ULL;

	/* A monotonic u64 counter can wrap; unsigned subtraction remains valid. */
	assert(!mii_mgr_busy_timeout_expired(start_ns, 48ULL));
	assert(mii_mgr_busy_timeout_expired(start_ns, 49ULL));
}

int
main(void)
{
	test_timeout_not_expired_before_deadline();
	test_timeout_expires_at_deadline();
	test_unsigned_elapsed_time_survives_counter_wrap();
	puts("RAETH IRQ-safe MDIO monotonic timeout policy: PASS");
	return 0;
}
