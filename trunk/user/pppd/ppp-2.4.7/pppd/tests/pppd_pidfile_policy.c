#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pppd_pidfile_policy.h"

static void
expect_invalid_stat(const char *text)
{
	unsigned long long start_time = 0;
	assert(pppd_pidfile_parse_start_time(text, &start_time) == -1);
}

int
main(void)
{
	unsigned long long start_time = 0;
	char record[64];
	int length;

	/* comm contains both spaces and ')' characters. */
	assert(pppd_pidfile_parse_start_time(
		"321 (ppp tricky ) comm) S 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 98765 23\n",
		&start_time) == 0);
	assert(start_time == 98765ULL);
	expect_invalid_stat(NULL);
	expect_invalid_stat("321 ppp) S 4 5");
	expect_invalid_stat("321 (ppp) S 4 5 6");
	expect_invalid_stat("321 (ppp) S 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 0");
	expect_invalid_stat("321 (ppp) S 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 x");
	expect_invalid_stat("321 (ppp) S 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 18446744073709551616");

	length = pppd_pidfile_format_record((pid_t)321, 98765ULL,
					    record, sizeof(record));
	assert(length > 0);
	assert(strcmp(record, "321 98765\n") == 0);
	assert(pppd_pidfile_format_record((pid_t)1, 98765ULL,
					  record, sizeof(record)) == -1);
	assert(pppd_pidfile_format_record((pid_t)321, 0,
					  record, sizeof(record)) == -1);
	assert(pppd_pidfile_format_record((pid_t)321, 98765ULL,
					  record, 2) == -1);
	length = pppd_pidfile_format_link_record((pid_t)321, "ppp12",
						  98765ULL, record, sizeof(record));
	assert(length > 0);
	assert(strcmp(record, "321\nppp12\n98765\n") == 0);
	assert(pppd_pidfile_format_link_record((pid_t)321, "", 98765ULL,
						       record, sizeof(record)) > 0);
	assert(pppd_pidfile_format_link_record((pid_t)321, "eth0", 98765ULL,
						       record, sizeof(record)) == -1);
	assert(pppd_pidfile_format_link_record((pid_t)321, "pppA", 98765ULL,
						       record, sizeof(record)) == -1);
	puts("PPPD PID-file identity policy: PASS");
	return 0;
}
