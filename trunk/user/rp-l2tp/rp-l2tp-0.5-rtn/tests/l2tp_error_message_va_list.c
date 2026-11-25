#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

static char captured_log[512];
static int captured_priority = -1;

void
vsyslog(int priority, const char *format, va_list args)
{
	va_list copy;

	va_copy(copy, args);
	vsnprintf(captured_log, sizeof(captured_log), format, copy);
	va_end(copy);
	captured_priority = priority;
}

extern void l2tp_set_errmsg(char const *fmt, ...);
extern char const *l2tp_get_errmsg(void);

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_multiple_argument_types(void)
{
	static const char expected[] = "peer=wan0 state=2 errno=104";

	l2tp_set_errmsg("peer=%s state=%d errno=%u", "wan0", 2, 104U);
	CHECK(strcmp(l2tp_get_errmsg(), expected) == 0);
	CHECK(strcmp(captured_log, expected) == 0);
	CHECK(captured_priority == LOG_ERR);
	return 0;
}

static int
test_repeated_calls_get_fresh_argument_lists(void)
{
	static const char expected[] = "retry=37 mode=ready";

	l2tp_set_errmsg("retry=%u mode=%s", 37U, "ready");
	CHECK(strcmp(l2tp_get_errmsg(), expected) == 0);
	CHECK(strcmp(captured_log, expected) == 0);
	CHECK(captured_priority == LOG_ERR);
	return 0;
}

int
main(void)
{
	CHECK(test_multiple_argument_types() == 0);
	CHECK(test_repeated_calls_get_fresh_argument_lists() == 0);
	puts("rp-l2tp error-message va_list regression: PASS");
	return 0;
}
