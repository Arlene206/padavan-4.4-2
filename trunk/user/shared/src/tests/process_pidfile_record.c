#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "process_pidfile_record.h"

static void
expect_invalid(const char *text)
{
	unsigned long long start_time = 0;
	assert(process_pidfile_record_parse_start_time(
		text, text ? strlen(text) : 0, &start_time) == -1);
}

int
main(void)
{
	static const char sample[] = "321 (udhcpc tricky ) comm) S 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 98765 23\n";
	unsigned long long start_time = 0;
	char record[64];
	int length;

	assert(process_pidfile_record_parse_start_time(
		sample, sizeof(sample) - 1, &start_time) == 0);
	assert(start_time == 98765ULL);
	expect_invalid(NULL);
	expect_invalid("321 udhcpc) S 4 5");
	expect_invalid("321 (udhcpc) S 4 5");
	expect_invalid("321 (udhcpc) S 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 0");
	expect_invalid("321 (udhcpc) S 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 x");
	expect_invalid("321 (udhcpc) S 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 18446744073709551616");

	length = process_pidfile_record_format((pid_t)321, 98765ULL,
					       record, sizeof(record));
	assert(length > 0);
	assert(strcmp(record, "321 98765\n") == 0);
	assert(process_pidfile_record_format((pid_t)1, 98765ULL,
					     record, sizeof(record)) == -1);
	assert(process_pidfile_record_format((pid_t)321, 0,
					     record, sizeof(record)) == -1);
	assert(process_pidfile_record_format((pid_t)321, 98765ULL,
					     record, 2) == -1);
	assert(process_pidfile_record_read_start_time(getpid(), &start_time) == 0);
	assert(start_time != 0);
	puts("shared PID-file record policy: PASS");
	return 0;
}
