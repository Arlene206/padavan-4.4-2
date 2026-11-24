/*
 * Host regression for the production vsftpd decimal and address parsers.
 * Minimal mystr functions keep this test independent of firmware sysutil.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipaddrparse.h"
#include "str.h"

static void
test_string_set(struct mystr* p_str, const char* text, unsigned int length)
{
  char* copy = (char*) malloc(length + 1U);
  if (copy == 0)
  {
    fprintf(stderr, "allocation failed in string set stub\\n");
    exit(2);
  }
  if (length != 0)
  {
    memcpy(copy, text, length);
  }
  copy[length] = '\0';
  free(p_str->PRIVATE_HANDS_OFF_p_buf);
  p_str->PRIVATE_HANDS_OFF_p_buf = copy;
  p_str->PRIVATE_HANDS_OFF_len = length;
  p_str->PRIVATE_HANDS_OFF_alloc_bytes = length + 1U;
}

void
str_empty(struct mystr* p_str)
{
  free(p_str->PRIVATE_HANDS_OFF_p_buf);
  p_str->PRIVATE_HANDS_OFF_p_buf = 0;
  p_str->PRIVATE_HANDS_OFF_len = 0;
  p_str->PRIVATE_HANDS_OFF_alloc_bytes = 0;
}

const char*
str_getbuf(const struct mystr* p_str)
{
  return p_str->PRIVATE_HANDS_OFF_p_buf;
}

void
str_append_char(struct mystr* p_str, char the_char)
{
  unsigned int old_length = p_str->PRIVATE_HANDS_OFF_len;
  char* copy = (char*) malloc(old_length + 2U);
  if (copy == 0)
  {
    fprintf(stderr, "allocation failed in append char stub\\n");
    exit(2);
  }
  if (old_length != 0)
  {
    memcpy(copy, p_str->PRIVATE_HANDS_OFF_p_buf, old_length);
  }
  copy[old_length] = the_char;
  copy[old_length + 1U] = '\0';
  free(p_str->PRIVATE_HANDS_OFF_p_buf);
  p_str->PRIVATE_HANDS_OFF_p_buf = copy;
  p_str->PRIVATE_HANDS_OFF_len = old_length + 1U;
  p_str->PRIVATE_HANDS_OFF_alloc_bytes = old_length + 2U;
}

void
str_append_str(struct mystr* p_str, const struct mystr* p_other)
{
  unsigned int old_length = p_str->PRIVATE_HANDS_OFF_len;
  unsigned int other_length = p_other->PRIVATE_HANDS_OFF_len;
  char* copy = (char*) malloc(old_length + other_length + 1U);
  if (copy == 0)
  {
    fprintf(stderr, "allocation failed in append string stub\\n");
    exit(2);
  }
  if (old_length != 0)
  {
    memcpy(copy, p_str->PRIVATE_HANDS_OFF_p_buf, old_length);
  }
  if (other_length != 0)
  {
    memcpy(copy + old_length, p_other->PRIVATE_HANDS_OFF_p_buf, other_length);
  }
  copy[old_length + other_length] = '\0';
  free(p_str->PRIVATE_HANDS_OFF_p_buf);
  p_str->PRIVATE_HANDS_OFF_p_buf = copy;
  p_str->PRIVATE_HANDS_OFF_len = old_length + other_length;
  p_str->PRIVATE_HANDS_OFF_alloc_bytes = old_length + other_length + 1U;
}

struct str_locate_result
str_locate_char(const struct mystr* p_str, char look_char)
{
  struct str_locate_result result;
  unsigned int i;
  result.found = 0;
  result.index = 0;
  result.char_found = 0;
  for (i = 0; i < p_str->PRIVATE_HANDS_OFF_len; ++i)
  {
    if (p_str->PRIVATE_HANDS_OFF_p_buf[i] == look_char)
    {
      result.found = 1;
      result.index = i;
      result.char_found = look_char;
      break;
    }
  }
  return result;
}

struct str_locate_result
str_locate_text(const struct mystr* p_str, const char* p_text)
{
  struct str_locate_result result;
  unsigned int i;
  unsigned int text_length = (unsigned int) strlen(p_text);
  result.found = 0;
  result.index = 0;
  result.char_found = 0;
  if (text_length == 0 || text_length > p_str->PRIVATE_HANDS_OFF_len)
  {
    return result;
  }
  for (i = 0; i <= p_str->PRIVATE_HANDS_OFF_len - text_length; ++i)
  {
    if (memcmp(p_str->PRIVATE_HANDS_OFF_p_buf + i, p_text, text_length) == 0)
    {
      result.found = 1;
      result.index = i;
      result.char_found = p_text[0];
      break;
    }
  }
  return result;
}

void
str_split_text(struct mystr* p_src, struct mystr* p_rhs, const char* p_text)
{
  struct str_locate_result result = str_locate_text(p_src, p_text);
  unsigned int text_length = (unsigned int) strlen(p_text);
  if (!result.found)
  {
    str_empty(p_rhs);
    return;
  }
  {
    struct mystr right = INIT_MYSTR;
    struct mystr left = INIT_MYSTR;
    test_string_set(&right, p_src->PRIVATE_HANDS_OFF_p_buf +
                    result.index + text_length,
                    p_src->PRIVATE_HANDS_OFF_len - result.index - text_length);
    test_string_set(&left, p_src->PRIVATE_HANDS_OFF_p_buf, result.index);
    str_empty(p_src);
    str_empty(p_rhs);
    *p_src = left;
    *p_rhs = right;
  }
}

int
vsf_sysutil_toupper(int the_char)
{
  return toupper((unsigned char) the_char);
}

void
str_copy(struct mystr* p_dest, const struct mystr* p_src)
{
  char* copy;
  unsigned int length;
  if (p_dest == p_src)
  {
    return;
  }
  length = p_src->PRIVATE_HANDS_OFF_len;
  copy = (char*) malloc(length + 1U);
  if (copy == 0)
  {
    fprintf(stderr, "allocation failed in str_copy stub\n");
    exit(2);
  }
  if (length != 0)
  {
    memcpy(copy, p_src->PRIVATE_HANDS_OFF_p_buf, length);
  }
  copy[length] = '\0';
  free(p_dest->PRIVATE_HANDS_OFF_p_buf);
  p_dest->PRIVATE_HANDS_OFF_p_buf = copy;
  p_dest->PRIVATE_HANDS_OFF_len = length;
  p_dest->PRIVATE_HANDS_OFF_alloc_bytes = length + 1U;
}

void
str_split_char(struct mystr* p_src, struct mystr* p_rhs, char separator)
{
  unsigned int i;
  unsigned int length = p_src->PRIVATE_HANDS_OFF_len;
  for (i = 0; i < length; ++i)
  {
    if (p_src->PRIVATE_HANDS_OFF_p_buf[i] == separator)
    {
      struct mystr right = INIT_MYSTR;
      struct mystr left = INIT_MYSTR;
      right.PRIVATE_HANDS_OFF_len = length - i - 1U;
      right.PRIVATE_HANDS_OFF_alloc_bytes = right.PRIVATE_HANDS_OFF_len + 1U;
      right.PRIVATE_HANDS_OFF_p_buf = (char*) malloc(right.PRIVATE_HANDS_OFF_alloc_bytes);
      left.PRIVATE_HANDS_OFF_len = i;
      left.PRIVATE_HANDS_OFF_alloc_bytes = i + 1U;
      left.PRIVATE_HANDS_OFF_p_buf = (char*) malloc(left.PRIVATE_HANDS_OFF_alloc_bytes);
      if (right.PRIVATE_HANDS_OFF_p_buf == 0 || left.PRIVATE_HANDS_OFF_p_buf == 0)
      {
        fprintf(stderr, "allocation failed in str_split_char stub\n");
        exit(2);
      }
      if (right.PRIVATE_HANDS_OFF_len != 0)
      {
        memcpy(right.PRIVATE_HANDS_OFF_p_buf,
               p_src->PRIVATE_HANDS_OFF_p_buf + i + 1U,
               right.PRIVATE_HANDS_OFF_len);
      }
      right.PRIVATE_HANDS_OFF_p_buf[right.PRIVATE_HANDS_OFF_len] = '\0';
      if (left.PRIVATE_HANDS_OFF_len != 0)
      {
        memcpy(left.PRIVATE_HANDS_OFF_p_buf,
               p_src->PRIVATE_HANDS_OFF_p_buf, left.PRIVATE_HANDS_OFF_len);
      }
      left.PRIVATE_HANDS_OFF_p_buf[left.PRIVATE_HANDS_OFF_len] = '\0';
      free(p_src->PRIVATE_HANDS_OFF_p_buf);
      free(p_rhs->PRIVATE_HANDS_OFF_p_buf);
      *p_src = left;
      *p_rhs = right;
      return;
    }
  }
  free(p_rhs->PRIVATE_HANDS_OFF_p_buf);
  p_rhs->PRIVATE_HANDS_OFF_p_buf = (char*) malloc(1);
  if (p_rhs->PRIVATE_HANDS_OFF_p_buf == 0)
  {
    fprintf(stderr, "allocation failed in str_split_char stub\n");
    exit(2);
  }
  p_rhs->PRIVATE_HANDS_OFF_p_buf[0] = '\0';
  p_rhs->PRIVATE_HANDS_OFF_len = 0;
  p_rhs->PRIVATE_HANDS_OFF_alloc_bytes = 1;
}

unsigned int
str_getlen(const struct mystr* p_str)
{
  return p_str->PRIVATE_HANDS_OFF_len;
}

int
str_isempty(const struct mystr* p_str)
{
  return p_str->PRIVATE_HANDS_OFF_len == 0;
}

char
str_get_char_at(const struct mystr* p_str, const unsigned int indexx)
{
  return p_str->PRIVATE_HANDS_OFF_p_buf[indexx];
}

static void
test_string_init(struct mystr* p_str, const char* text)
{
  size_t length = strlen(text);
  p_str->PRIVATE_HANDS_OFF_p_buf = (char*) malloc(length + 1U);
  if (p_str->PRIVATE_HANDS_OFF_p_buf == 0)
  {
    fprintf(stderr, "allocation failed in test_string_init\n");
    exit(2);
  }
  memcpy(p_str->PRIVATE_HANDS_OFF_p_buf, text, length + 1U);
  p_str->PRIVATE_HANDS_OFF_len = (unsigned int) length;
  p_str->PRIVATE_HANDS_OFF_alloc_bytes = (unsigned int) length + 1U;
}

static void
test_string_free(struct mystr* p_str)
{
  free(p_str->PRIVATE_HANDS_OFF_p_buf);
  p_str->PRIVATE_HANDS_OFF_p_buf = 0;
  p_str->PRIVATE_HANDS_OFF_len = 0;
  p_str->PRIVATE_HANDS_OFF_alloc_bytes = 0;
}

static int
expect_uint(const char* text, unsigned int maximum, int expected_status,
            unsigned int expected_value)
{
  struct mystr value = INIT_MYSTR;
  unsigned int parsed = 123456U;
  int status;
  test_string_init(&value, text);
  status = vsf_sysutil_parse_uint(&value, maximum, &parsed);
  test_string_free(&value);
  if (status != expected_status ||
      (expected_status && parsed != expected_value) ||
      (!expected_status && parsed != 123456U))
  {
    fprintf(stderr, "decimal parser mismatch for [%s]: status=%d value=%u\n",
            text, status, parsed);
    return 1;
  }
  return 0;
}

static int
expect_octets(const char* text, char separator,
              const unsigned char* expected, unsigned int count,
              int expected_success)
{
  struct mystr input = INIT_MYSTR;
  unsigned char values[8];
  const unsigned char* result;
  unsigned int i;
  int failed = 0;
  memset(values, 0xA5, sizeof(values));
  test_string_init(&input, text);
  result = vsf_sysutil_parse_uchar_string_sep(&input, separator, values, count);
  test_string_free(&input);
  if ((result != 0) != expected_success)
  {
    fprintf(stderr, "octet parser status mismatch for [%s]\n", text);
    return 1;
  }
  if (expected_success)
  {
    for (i = 0; i < count; ++i)
    {
      if (values[i] != expected[i])
      {
        fprintf(stderr, "octet %u mismatch for [%s]: got %u expected %u\n",
                i, text, values[i], expected[i]);
        failed = 1;
      }
    }
  }
  return failed;
}

static int
expect_ipv6(const char* text, const unsigned char* expected,
            int expected_success)
{
  struct mystr input = INIT_MYSTR;
  const unsigned char* parsed;
  int failed = 0;
  test_string_init(&input, text);
  parsed = vsf_sysutil_parse_ipv6(&input);
  test_string_free(&input);
  if ((parsed != 0) != expected_success)
  {
    fprintf(stderr, "IPv6 parser status mismatch for [%s]\\n", text);
    return 1;
  }
  if (expected_success && memcmp(parsed, expected, 16) != 0)
  {
    fprintf(stderr, "IPv6 parser returned incorrect bytes for [%s]\\n", text);
    failed = 1;
  }
  return failed;
}

int
main(void)
{
  static const unsigned char ipv4_expected[4] = { 192, 0, 2, 1 };
  static const unsigned char port_expected[6] = { 192, 0, 2, 1, 4, 20 };
  static const unsigned char zero_padded_expected[4] = { 1, 2, 3, 4 };
  static const unsigned char ipv6_full_expected[16] =
    { 0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0,
      0, 0, 0xff, 0, 0, 0x42, 0x83, 0x29 };
  static const unsigned char ipv6_zero_expected[16] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  static const unsigned char ipv6_loopback_expected[16] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
  static const unsigned char ipv6_one_prefix_expected[16] =
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  static const unsigned char ipv6_mapped_expected[16] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 192, 0, 2, 1 };
  struct mystr ipv4 = INIT_MYSTR;
  const unsigned char* parsed_ipv4;
  int failed = 0;

  failed |= expect_uint("0", 0U, 1, 0U);
  failed |= expect_uint("000255", 255U, 1, 255U);
  failed |= expect_uint("65535", 65535U, 1, 65535U);
  failed |= expect_uint("", 65535U, 0, 0U);
  failed |= expect_uint("256", 255U, 0, 0U);
  failed |= expect_uint("1tail", 65535U, 0, 0U);
  failed |= expect_uint("+1", 65535U, 0, 0U);
  failed |= expect_uint("-1", 65535U, 0, 0U);
  failed |= expect_uint(" 1", 65535U, 0, 0U);
  failed |= expect_uint("1 ", 65535U, 0, 0U);

  failed |= expect_octets("192,0,2,1,4,20", ',', port_expected, 6, 1);
  failed |= expect_octets("192,0,2,1,4tail,20", ',', port_expected, 6, 0);
  failed |= expect_octets("192,0,2,1,4,20,", ',', port_expected, 6, 0);
  failed |= expect_octets("192,0,2,1,,20", ',', port_expected, 6, 0);
  failed |= expect_octets("192,0,2,1,4,256", ',', port_expected, 6, 0);
  failed |= expect_octets("192,0,2,1,4", ',', port_expected, 6, 0);
  failed |= expect_octets("192,0,2,1,4,20,21", ',', port_expected, 6, 0);

  test_string_init(&ipv4, "192.0.2.1");
  parsed_ipv4 = vsf_sysutil_parse_ipv4(&ipv4);
  if (parsed_ipv4 == 0 || memcmp(parsed_ipv4, ipv4_expected, 4) != 0)
  {
    fprintf(stderr, "valid IPv4 address was rejected or decoded incorrectly\n");
    failed = 1;
  }
  test_string_free(&ipv4);

  test_string_init(&ipv4, "001.002.003.004");
  parsed_ipv4 = vsf_sysutil_parse_ipv4(&ipv4);
  if (parsed_ipv4 == 0 || memcmp(parsed_ipv4, zero_padded_expected, 4) != 0)
  {
    fprintf(stderr, "zero-padded decimal IPv4 compatibility changed\n");
    failed = 1;
  }
  test_string_free(&ipv4);

  test_string_init(&ipv4, "192.0.2.1junk");
  parsed_ipv4 = vsf_sysutil_parse_ipv4(&ipv4);
  if (parsed_ipv4 != 0)
  {
    fprintf(stderr, "IPv4 parser accepted a junk suffix\n");
    failed = 1;
  }
  test_string_free(&ipv4);

  test_string_init(&ipv4, "192..2.1");
  parsed_ipv4 = vsf_sysutil_parse_ipv4(&ipv4);
  if (parsed_ipv4 != 0)
  {
    fprintf(stderr, "IPv4 parser accepted an empty octet\n");
    failed = 1;
  }
  test_string_free(&ipv4);

  test_string_init(&ipv4, "192.0.2.256");
  parsed_ipv4 = vsf_sysutil_parse_ipv4(&ipv4);
  if (parsed_ipv4 != 0)
  {
    fprintf(stderr, "IPv4 parser accepted an octet above 255\n");
    failed = 1;
  }
  test_string_free(&ipv4);

  failed |= expect_ipv6(
      "2001:0db8:0000:0000:0000:ff00:0042:8329",
      ipv6_full_expected, 1);
  failed |= expect_ipv6("2001:db8::ff00:42:8329", ipv6_full_expected, 1);
  failed |= expect_ipv6("::", ipv6_zero_expected, 1);
  failed |= expect_ipv6("::1", ipv6_loopback_expected, 1);
  failed |= expect_ipv6("1::", ipv6_one_prefix_expected, 1);
  failed |= expect_ipv6("::ffff:192.0.2.1", ipv6_mapped_expected, 1);
  failed |= expect_ipv6("2001:db8:1", ipv6_full_expected, 0);
  failed |= expect_ipv6("00000:0:0:0:0:0:0:1",
      ipv6_loopback_expected, 0);
  failed |= expect_ipv6("1:2:3:4:5:6:7:8:", ipv6_full_expected, 0);
  failed |= expect_ipv6("1:2:3:4:5:6:7:8::", ipv6_full_expected, 0);
  failed |= expect_ipv6("1::192.0.2.1:3", ipv6_full_expected, 0);
  failed |= expect_ipv6("192.0.2.1::", ipv6_full_expected, 0);

  if (failed)
  {
    return 1;
  }
  puts("vsftpd strict decimal, PORT tuple, IPv4 and IPv6 parsing: PASS");
  return 0;
}
