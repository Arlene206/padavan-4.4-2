#ifndef WIRELESS_GUEST_SCHEDULE_POLICY_H
#define WIRELESS_GUEST_SCHEDULE_POLICY_H
#include <errno.h>
#include <string.h>

/* Keep the legacy unrestricted default for absent or malformed schedules. */
static inline int
wireless_guest_schedule_time_allowed(const char *date, const char *weekday_time,
				     const char *weekend_time, int weekday,
				     int hour, int minute)
{
	enum {
		DOW_MASK_SUN = 1 << 0, DOW_MASK_MON = 1 << 1,
		DOW_MASK_TUE = 1 << 2, DOW_MASK_WED = 1 << 3,
		DOW_MASK_THU = 1 << 4, DOW_MASK_FRI = 1 << 5,
		DOW_MASK_SAT = 1 << 6
	};
	int current_min, current_dow, scheduled_dow;
	int weekday_begin, weekday_end, weekend_begin, weekend_end;
	int i;

	if (!date || !weekday_time || !weekend_time ||
	    strlen(date) != 7 || strlen(weekday_time) != 8 ||
	    strlen(weekend_time) != 8)
		return 1;
	if (strcmp(date, "1111111") == 0 &&
	    strcmp(weekday_time, "00002359") == 0 &&
	    strcmp(weekend_time, "00002359") == 0)
		return 1;
	if (weekday < 0 || weekday > 6 || hour < 0 || hour > 23 ||
	    minute < 0 || minute > 59)
		return 0;

	weekday_begin = ((weekday_time[0] - '0') * 10 +
			 (weekday_time[1] - '0')) * 60 +
			(weekday_time[2] - '0') * 10 + (weekday_time[3] - '0');
	weekday_end = ((weekday_time[4] - '0') * 10 +
		       (weekday_time[5] - '0')) * 60 +
		      (weekday_time[6] - '0') * 10 + (weekday_time[7] - '0');
	weekend_begin = ((weekend_time[0] - '0') * 10 +
			 (weekend_time[1] - '0')) * 60 +
			(weekend_time[2] - '0') * 10 + (weekend_time[3] - '0');
	weekend_end = ((weekend_time[4] - '0') * 10 +
		       (weekend_time[5] - '0')) * 60 +
		      (weekend_time[6] - '0') * 10 + (weekend_time[7] - '0');

	current_min = hour * 60 + minute;
	current_dow = 1 << weekday;
	scheduled_dow = 0;
	for (i = 0; i < 7; ++i)
		if (date[i] == '1')
			scheduled_dow |= (1 << i);

	if (current_dow & DOW_MASK_SAT) {
		if (scheduled_dow & DOW_MASK_SAT) {
			if (weekend_begin < weekend_end) {
				if (current_min >= weekend_begin && current_min <= weekend_end)
					return 1;
			} else {
				if (current_min >= weekend_begin)
					return 1;
				if ((scheduled_dow & DOW_MASK_FRI) &&
				    weekday_begin < weekday_end && current_min <= weekend_end)
					return 1;
			}
		}
		if ((scheduled_dow & DOW_MASK_FRI) &&
		    weekday_begin >= weekday_end && current_min <= weekday_end)
			return 1;
	} else if (current_dow & DOW_MASK_SUN) {
		if (scheduled_dow & DOW_MASK_SUN) {
			if (weekend_begin < weekend_end) {
				if (current_min >= weekend_begin && current_min <= weekend_end)
					return 1;
			} else if (current_min >= weekend_begin) {
				return 1;
			}
		}
		if ((scheduled_dow & DOW_MASK_SAT) &&
		    weekend_begin >= weekend_end && current_min <= weekend_end)
			return 1;
	} else if (current_dow & DOW_MASK_MON) {
		if (scheduled_dow & DOW_MASK_MON) {
			if (weekday_begin < weekday_end) {
				if (current_min >= weekday_begin && current_min <= weekday_end)
					return 1;
			} else {
				if (current_min >= weekday_begin)
					return 1;
				if ((scheduled_dow & DOW_MASK_SUN) &&
				    weekend_begin < weekend_end && current_min <= weekday_end)
					return 1;
			}
		}
		if ((scheduled_dow & DOW_MASK_SUN) &&
		    weekend_begin >= weekend_end && current_min <= weekend_end)
			return 1;
	} else {
		if (scheduled_dow & current_dow) {
			if (weekday_begin < weekday_end) {
				if (current_min >= weekday_begin && current_min <= weekday_end)
					return 1;
			} else if (current_min >= weekday_begin) {
				return 1;
			}
		}
		if ((scheduled_dow & (current_dow >> 1)) &&
		    weekday_begin >= weekday_end && current_min <= weekday_end)
			return 1;
	}
	return 0;
}

/* Do not collapse UNKNOWN NVRAM state into "guest AP may be started". */
static inline int
wireless_guest_schedule_resolve(int enable_status, int enabled,
				int schedule_status, int schedule_allowed,
				int *allowed)
{
	if (!allowed)
		return EINVAL;
	*allowed = 0;
	if (enable_status != 0)
		return enable_status;
	if (!enabled)
		return 0;
	if (schedule_status != 0)
		return schedule_status;
	*allowed = !!schedule_allowed;
	return 0;
}
#endif
