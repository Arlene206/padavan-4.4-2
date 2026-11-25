#include <stdio.h>
#include <errno.h>
#include "wireless_guest_schedule_policy.h"
#define CHECK(c) do { if (!(c)) { \
	fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #c); \
	return 1; } } while (0)

static int test_legacy_defaults(void)
{
	CHECK(wireless_guest_schedule_time_allowed("", "", "", 2, 12, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed("1111111", "00002359", "00002359", 0, 23, 59) == 1);
	CHECK(wireless_guest_schedule_time_allowed(NULL, "09001700", "10001100", 1, 12, 0) == 1);
	return 0;
}
static int test_weekday_and_weekend_windows(void)
{
	const char *weekdays = "0111110";
	const char *weekends = "1000001";
	CHECK(wireless_guest_schedule_time_allowed(weekdays, "09001700", "10001100", 1, 9, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed(weekdays, "09001700", "10001100", 1, 17, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed(weekdays, "09001700", "10001100", 1, 8, 59) == 0);
	CHECK(wireless_guest_schedule_time_allowed(weekdays, "09001700", "10001100", 6, 10, 30) == 0);
	CHECK(wireless_guest_schedule_time_allowed(weekends, "09001700", "10001100", 6, 10, 30) == 1);
	CHECK(wireless_guest_schedule_time_allowed(weekends, "09001700", "10001100", 0, 9, 59) == 0);
	CHECK(wireless_guest_schedule_time_allowed(weekends, "09001700", "10001100", 0, 10, 0) == 1);
	return 0;
}
static int test_weekday_overnight_window(void)
{
	const char *weekdays = "0111110";
	CHECK(wireless_guest_schedule_time_allowed(weekdays, "22000200", "10001100", 2, 23, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed(weekdays, "22000200", "10001100", 3, 0, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed(weekdays, "22000200", "10001100", 3, 2, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed(weekdays, "22000200", "10001100", 3, 2, 1) == 0);
	return 0;
}
static int test_weekend_and_week_boundary_overnight(void)
{
	CHECK(wireless_guest_schedule_time_allowed("1000001", "09001700", "22000200", 6, 23, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed("1000001", "09001700", "22000200", 0, 1, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed("1100000", "09001700", "22000200", 0, 23, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed("1100000", "09001700", "22000200", 1, 1, 0) == 1);
	CHECK(wireless_guest_schedule_time_allowed("0000011", "22000200", "10001100", 6, 1, 0) == 1);
	return 0;
}
static int test_checked_status_is_fail_closed(void)
{
	int allowed = 1;
	CHECK(wireless_guest_schedule_resolve(EIO, 0, 0, 1, &allowed) == EIO);
	CHECK(allowed == 0);
	CHECK(wireless_guest_schedule_resolve(0, 0, EACCES, 1, &allowed) == 0);
	CHECK(allowed == 0);
	CHECK(wireless_guest_schedule_resolve(0, 1, EACCES, 1, &allowed) == EACCES);
	CHECK(allowed == 0);
	CHECK(wireless_guest_schedule_resolve(0, 1, 0, 1, &allowed) == 0);
	CHECK(allowed == 1);
	CHECK(wireless_guest_schedule_resolve(0, 1, 0, 0, &allowed) == 0);
	CHECK(allowed == 0);
	CHECK(wireless_guest_schedule_resolve(0, 1, 0, 1, NULL) == EINVAL);
	return 0;
}
int main(void)
{
	CHECK(test_legacy_defaults() == 0);
	CHECK(test_weekday_and_weekend_windows() == 0);
	CHECK(test_weekday_overnight_window() == 0);
	CHECK(test_weekend_and_week_boundary_overnight() == 0);
	CHECK(test_checked_status_is_fail_closed() == 0);
	puts("checked wireless guest schedule policy: PASS");
	return 0;
}
