#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ftp_mdtm_policy.h"

static void
expect_valid(const char *argument, int expected)
{
	assert(ftp_mdtm_timeval_is_valid(argument, strlen(argument)) == expected);
}

static void
test_valid_date_and_time_tokens(void)
{
	expect_valid("20240229 report.txt", 1);
	expect_valid("20000229 report.txt", 1);
	expect_valid("20240229123456 report.txt", 1);
	expect_valid("20240229123456.1 report.txt", 1);
	expect_valid("20240229123456.123456 report.txt", 1);
	expect_valid("19000228 report.txt", 1);
	expect_valid("20241130235959 archive.bin", 1);
}

static void
test_invalid_calendar_and_clock_values(void)
{
	expect_valid("00000229 file", 0);
	expect_valid("20230229 file", 0);
	expect_valid("19000229 file", 0);
	expect_valid("20241301 file", 0);
	expect_valid("20240001 file", 0);
	expect_valid("20240100 file", 0);
	expect_valid("20240431 file", 0);
	expect_valid("20240229123460 file", 0);
	expect_valid("20240229126000 file", 0);
	expect_valid("20240229240000 file", 0);
}

static void
test_invalid_token_grammar_is_rejected(void)
{
	expect_valid("2024x229 file", 0);
	expect_valid("2024022 file", 0);
	expect_valid("2024022912345 file", 0);
	expect_valid("20240229123456x123 file", 0);
	expect_valid("20240229123456. file", 0);
	expect_valid("20240229123456.x file", 0);
	expect_valid("20240229.123 file", 0);
	expect_valid("20240229123456.123", 0);
	expect_valid("20240229", 0);
	expect_valid("20240229 ", 0);
}

static void
test_delimiter_and_path_are_required(void)
{
	assert(!ftp_mdtm_timeval_is_valid(NULL, 8U));
	assert(!ftp_mdtm_timeval_is_valid("20240229xfile", 8U));
	assert(!ftp_mdtm_timeval_is_valid("20240229 file", 9U));
	assert(!ftp_mdtm_timeval_is_valid("20240229 ", 8U));
}

int
main(void)
{
	test_valid_date_and_time_tokens();
	test_invalid_calendar_and_clock_values();
	test_invalid_token_grammar_is_rejected();
	test_delimiter_and_path_are_required();
	puts("vsftpd MDTM timestamp policy: PASS");
	return 0;
}
