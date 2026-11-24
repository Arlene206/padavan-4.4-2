#include <stdio.h>

#include "ftp_rest_offset_policy.h"

static int
expect_rest_offset(const char* p_input, unsigned int input_length,
                   int expected_status, filesize_t expected_offset)
{
  filesize_t parsed = 314159;
  int status = ftp_rest_offset_parse(p_input, input_length, &parsed);
  if (status != expected_status ||
      (expected_status && parsed != expected_offset) ||
      (!expected_status && parsed != 314159))
  {
    fprintf(stderr, "REST offset policy mismatch for [%s]\n",
            p_input == 0 ? "<null>" : p_input);
    return 1;
  }
  return 0;
}

int
main(void)
{
  const char embedded_nul[] = { '1', '\0', '2' };
  int failed = 0;

  failed |= expect_rest_offset("0", 1U, 1, 0);
  failed |= expect_rest_offset("42", 2U, 1, 42);
  failed |= expect_rest_offset("00042", 5U, 1, 42);
  failed |= expect_rest_offset("999999999999999", 15U, 1,
                               999999999999999LL);
  failed |= expect_rest_offset("", 0U, 0, 0);
  failed |= expect_rest_offset(0, 1U, 0, 0);
  failed |= expect_rest_offset("-1", 2U, 0, 0);
  failed |= expect_rest_offset("+1", 2U, 0, 0);
  failed |= expect_rest_offset("12x", 3U, 0, 0);
  failed |= expect_rest_offset(" 1", 2U, 0, 0);
  failed |= expect_rest_offset("1 ", 2U, 0, 0);
  failed |= expect_rest_offset("1000000000000000", 16U, 0, 0);
  failed |= expect_rest_offset("0000000000000000", 16U, 0, 0);
  failed |= expect_rest_offset(embedded_nul, (unsigned int) sizeof(embedded_nul),
                               0, 0);

  if (!ftp_rest_offset_is_within_file(0, 0) ||
      !ftp_rest_offset_is_within_file(0, 4096) ||
      !ftp_rest_offset_is_within_file(4096, 4096) ||
      ftp_rest_offset_is_within_file(4097, 4096) ||
      ftp_rest_offset_is_within_file(1, 0) ||
      ftp_rest_offset_is_within_file(-1, 4096) ||
      ftp_rest_offset_is_within_file(0, -1))
  {
    fprintf(stderr, "REST upload offset/file-size policy mismatch\n");
    failed = 1;
  }

  if (!ftp_rest_offset_mode_is_valid(0, 0) ||
      !ftp_rest_offset_mode_is_valid(0, 42) ||
      !ftp_rest_offset_mode_is_valid(1, 0) ||
      ftp_rest_offset_mode_is_valid(1, 1) ||
      ftp_rest_offset_mode_is_valid(1, 42) ||
      ftp_rest_offset_mode_is_valid(0, -1))
  {
    fprintf(stderr, "REST ASCII/binary resume policy mismatch\n");
    failed = 1;
  }

  if (failed)
  {
    return 1;
  }
  puts("FTP REST offset syntax/range policy: PASS");
  return 0;
}
