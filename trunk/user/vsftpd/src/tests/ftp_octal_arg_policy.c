#include <limits.h>
#include <stdio.h>

#include "ftp_octal_arg_policy.h"

static int
expect_valid(const char* text, unsigned int expected)
{
  unsigned int parsed = 0x13579U;
  if (!ftp_octal_arg_parse(text, 0777U, &parsed) || parsed != expected)
  {
    fprintf(stderr, "valid octal argument rejected or misparsed: %s\n",
            text == 0 ? "(null)" : text);
    return 0;
  }
  return 1;
}

static int
expect_invalid(const char* text)
{
  unsigned int parsed = 0x13579U;
  if (ftp_octal_arg_parse(text, 0777U, &parsed) ||
      parsed != 0x13579U)
  {
    fprintf(stderr, "invalid octal argument accepted or changed output: %s\n",
            text == 0 ? "(null)" : text);
    return 0;
  }
  return 1;
}

int
main(void)
{
  static const char* invalid[] = {
    "", "8", "078", "-1", "+755", "755x", "755 ", " 755",
    "1000", "4000", "7777", "0008", 0
  };
  unsigned int value = 0x13579U;
  unsigned int i;

  if (!expect_valid("0", 0U) ||
      !expect_valid("000", 0U) ||
      !expect_valid("7", 7U) ||
      !expect_valid("755", 0755U) ||
      !expect_valid("0755", 0755U) ||
      !expect_valid("0777", 0777U) ||
      !expect_valid("0000000000000755", 0755U) ||
      !expect_valid("0000000000000777", 0777U))
  {
    return 1;
  }
  for (i = 0; invalid[i] != 0; ++i)
  {
    if (!expect_invalid(invalid[i]))
    {
      return 1;
    }
  }
  if (ftp_octal_arg_parse(0, 0777U, &value) || value != 0x13579U ||
      ftp_octal_arg_parse("755", 0777U, 0))
  {
    fprintf(stderr, "null octal parser input/output was accepted\n");
    return 1;
  }
  value = 0x13579U;
  if (ftp_octal_arg_parse("777777777777777777777777777777", UINT_MAX,
                          &value) ||
      value != 0x13579U)
  {
    fprintf(stderr, "octal arithmetic overflow was accepted\n");
    return 1;
  }

  puts("vsftpd SITE CHMOD/UMASK strict octal argument policy: PASS");
  return 0;
}
