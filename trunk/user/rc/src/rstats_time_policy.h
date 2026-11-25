#ifndef RSTATS_TIME_POLICY_H
#define RSTATS_TIME_POLICY_H

#include <stddef.h>
#include <time.h>

/*
 * localtime() is permitted to return NULL when time_t cannot be represented.
 * Keep this check in a shared policy so the production path and host
 * regression exercise the same pointer/year guard.
 */
static inline int rstats_time_has_valid_year(const struct tm *tms, int start_year)
{
	if (tms == NULL)
		return 0;

	return tms->tm_year > (start_year - 1900);
}

#endif /* RSTATS_TIME_POLICY_H */
