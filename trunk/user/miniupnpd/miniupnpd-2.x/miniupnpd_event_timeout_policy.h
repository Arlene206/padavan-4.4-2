#ifndef MINIUPNPD_EVENT_TIMEOUT_POLICY_H
#define MINIUPNPD_EVENT_TIMEOUT_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <strings.h>
#include <time.h>

/*
 * Parse the UPnP event TIMEOUT grammar without atoi():
 *   Second-<unsigned decimal seconds>
 *   Second-infinite
 * A zero duration preserves the caller's existing infinite/default policy.
 */
static inline int
miniupnpd_event_timeout_parse(const char *value, size_t length,
                              int *timeout_out)
{
	size_t first = 0;
	size_t last = length;
	unsigned long parsed = 0;

	if (!value || !timeout_out) {
		errno = EINVAL;
		return -1;
	}

	while (first < last &&
	       (value[first] == ' ' || value[first] == '\t'))
		++first;
	while (last > first &&
	       (value[last - 1U] == ' ' || value[last - 1U] == '\t'))
		--last;

	if (last - first < 7U ||
	    strncasecmp(value + first, "Second-", 7U) != 0) {
		errno = EINVAL;
		return -1;
	}
	first += 7U;

	if (last - first == 8U &&
	    strncasecmp(value + first, "infinite", 8U) == 0) {
		*timeout_out = 0;
		return 0;
	}
	if (first == last) {
		errno = EINVAL;
		return -1;
	}

	for (; first < last; ++first) {
		unsigned int digit;

		if (value[first] < '0' || value[first] > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned int)(value[first] - '0');
		if (parsed > ((unsigned long)INT_MAX - (unsigned long)digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		parsed = parsed * 10UL + (unsigned long)digit;
	}

	*timeout_out = (int)parsed;
	return 0;
}

/* Return the largest positive value representable by a signed time_t. */
static inline time_t
miniupnpd_event_time_t_max(void)
{
	unsigned int bits = (unsigned int)(sizeof(time_t) * CHAR_BIT);
	unsigned int uintmax_bits = (unsigned int)(sizeof(uintmax_t) * CHAR_BIT);
	uintmax_t maximum;

	if (bits >= uintmax_bits)
		maximum = UINTMAX_MAX >> 1;
	else
		maximum = ((uintmax_t)1U << (bits - 1U)) - 1U;
	return (time_t)maximum;
}

/*
 * Compute an absolute subscription expiry without signed time_t overflow.
 * The limit-taking form makes 32-bit boundary behavior testable on 64-bit
 * hosts; production uses the actual signed time_t maximum.
 */
static inline int
miniupnpd_event_timeout_deadline_for_limit(time_t now, int timeout,
                                           time_t maximum,
                                           time_t *deadline_out)
{
	uintmax_t remaining;

	if (!deadline_out) {
		errno = EINVAL;
		return -1;
	}
	if (timeout < 0) {
		errno = EINVAL;
		return -1;
	}
	if (timeout == 0) {
		*deadline_out = (time_t)0;
		return 0;
	}
	if (now < (time_t)0 || maximum <= (time_t)0 || now > maximum) {
		errno = ERANGE;
		return -1;
	}

	remaining = (uintmax_t)maximum - (uintmax_t)now;
	if ((uintmax_t)timeout > remaining)
		*deadline_out = maximum;
	else
		*deadline_out = now + (time_t)timeout;
	return 0;
}

static inline int
miniupnpd_event_timeout_deadline(time_t now, int timeout,
                                 time_t *deadline_out)
{
	return miniupnpd_event_timeout_deadline_for_limit(
		now, timeout, miniupnpd_event_time_t_max(), deadline_out);
}

#endif /* MINIUPNPD_EVENT_TIMEOUT_POLICY_H */
