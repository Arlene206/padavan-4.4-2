#include <stdio.h>
#include "xl2tpd_start_policy.h"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); return 1; } } while (0)
int
main(void)
{
	CHECK(xl2tpd_start_policy_result(PROCESS_PRESENCE_RUNNING) == 1);
	CHECK(xl2tpd_start_policy_result(PROCESS_PRESENCE_ABSENT) == 0);
	CHECK(xl2tpd_start_policy_result(PROCESS_PRESENCE_UNKNOWN) == -1);
	CHECK(xl2tpd_start_policy_result((enum process_presence)99) == -1);
	puts("xl2tpd process-presence start gate: PASS");
	return 0;
}
