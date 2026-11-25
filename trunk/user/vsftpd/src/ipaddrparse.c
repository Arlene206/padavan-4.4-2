/*
 * Part of Very Secure FTPd
 * Licence: GPL v2
 * Author: Chris Evans
 * ipaddrparse.c
 *
 * A routine to parse ip addresses. I'm paranoid and don't want to use
 * inet_pton.
 */

#include "ipaddrparse.h"
#include "sysutil.h"
#include "str.h"

static int ipv6_parse_main(struct mystr* p_out_str,
                           const struct mystr* p_in_str);
static int ipv6_parse_hex(struct mystr* p_out_str,
                          const struct mystr* p_in_str);
static int ipv4_parse_dotquad(struct mystr* p_out_str,
                              const struct mystr* p_in_str);

const unsigned char*
vsf_sysutil_parse_ipv6(const struct mystr* p_str)
{
  static struct mystr s_ret;
  static struct mystr s_rhs_ret;
  static struct mystr s_lhs_str;
  static struct mystr s_rhs_str;
  struct str_locate_result compression_ret;
  unsigned int lhs_len;
  unsigned int rhs_len;
  if (p_str == 0)
  {
    return 0;
  }
  compression_ret = str_locate_text(p_str, "::");
  str_empty(&s_ret);
  str_empty(&s_rhs_ret);
  str_copy(&s_lhs_str, p_str);
  str_split_text(&s_lhs_str, &s_rhs_str, "::");
  /* An IPv4 tail must be the final part of the whole IPv6 address. */
  if (compression_ret.found &&
      str_locate_char(&s_lhs_str, '.').found)
  {
    return 0;
  }
  if (!ipv6_parse_main(&s_ret, &s_lhs_str))
  {
    return 0;
  }
  if (!ipv6_parse_main(&s_rhs_ret, &s_rhs_str))
  {
    return 0;
  }
  lhs_len = str_getlen(&s_ret);
  rhs_len = str_getlen(&s_rhs_ret);
  if (compression_ret.found)
  {
    /* A double colon must compress at least one 16-bit group. */
    if (lhs_len + rhs_len >= 16)
    {
      return 0;
    }
    {
      unsigned int add_nulls = 16 - (lhs_len + rhs_len);
      while (add_nulls--)
      {
        str_append_char(&s_ret, '\0');
      }
    }
    str_append_str(&s_ret, &s_rhs_ret);
  }
  else if (lhs_len != 16 || rhs_len != 0)
  {
    /* Without "::", exactly eight 16-bit groups are required. */
    return 0;
  }
  return (const unsigned char*) str_getbuf(&s_ret);
}

const unsigned char*
vsf_sysutil_parse_ipv4(const struct mystr* p_str)
{
  static unsigned char items[4];
  return vsf_sysutil_parse_uchar_string_sep(p_str, '.', items, sizeof(items));
}

int
vsf_sysutil_parse_uint(const struct mystr* p_str, unsigned int maximum,
                       unsigned int* p_value)
{
  unsigned int length;
  unsigned int i;
  unsigned int value = 0;
  if (p_str == 0 || p_value == 0)
  {
    return 0;
  }
  length = str_getlen(p_str);
  if (length == 0)
  {
    return 0;
  }
  for (i = 0; i < length; ++i)
  {
    int ch = (unsigned char) str_get_char_at(p_str, i);
    unsigned int digit;
    if (ch < '0' || ch > '9')
    {
      return 0;
    }
    digit = (unsigned int) (ch - '0');
    if (digit > maximum || value > (maximum - digit) / 10U)
    {
      return 0;
    }
    value = value * 10U + digit;
  }
  *p_value = value;
  return 1;
}


/*
 * RFC 2428 permits any printable ASCII EPRT delimiter, not only '|'.
 * Require the opening delimiter and exactly four occurrences so each field
 * boundary is unambiguous before post-login parsing starts.
 */
int
vsf_sysutil_parse_eprt_delimiter(const struct mystr* p_str,
                                 char* p_delimiter)
{
  unsigned int length;
  unsigned int i;
  unsigned int delimiter_count = 0;
  int delimiter;
  if (p_str == 0 || p_delimiter == 0)
  {
    return 0;
  }
  length = str_getlen(p_str);
  if (length < 7U)
  {
    return 0;
  }
  delimiter = (unsigned char) str_get_char_at(p_str, 0);
  if (delimiter < 33 || delimiter > 126)
  {
    return 0;
  }
  for (i = 0; i < length; ++i)
  {
    if ((unsigned char) str_get_char_at(p_str, i) ==
        (unsigned int) delimiter)
    {
      ++delimiter_count;
    }
  }
  if (delimiter_count != 4U)
  {
    return 0;
  }
  *p_delimiter = (char) delimiter;
  return 1;
}

const unsigned char*
vsf_sysutil_parse_uchar_string_sep(
  const struct mystr* p_str, char sep, unsigned char* p_items,
  unsigned int items)
{
  static struct mystr s_tmp_str;
  static struct mystr s_rhs_sep_str;
  unsigned int i;
  unsigned int length;
  unsigned int separator_count = 0;
  if (p_str == 0 || p_items == 0 || items == 0 || sep == '\0')
  {
    return 0;
  }
  length = str_getlen(p_str);
  for (i = 0; i < length; ++i)
  {
    if (str_get_char_at(p_str, i) == sep)
    {
      ++separator_count;
    }
  }
  if (separator_count != items - 1)
  {
    return 0;
  }
  str_copy(&s_tmp_str, p_str);
  for (i = 0; i < items; ++i)
  {
    unsigned int this_number;
    /* Split one field; the preflight above requires exactly items - 1 separators. */
    str_split_char(&s_tmp_str, &s_rhs_sep_str, sep);
    if ((i < items - 1 && str_isempty(&s_rhs_sep_str)) ||
        (i == items - 1 && !str_isempty(&s_rhs_sep_str)))
    {
      return 0;
    }
    if (!vsf_sysutil_parse_uint(&s_tmp_str, 255U, &this_number))
    {
      return 0;
    }
    p_items[i] = (unsigned char) this_number;
    str_copy(&s_tmp_str, &s_rhs_sep_str);
  }
  return p_items;
}

static int
ipv6_parse_main(struct mystr* p_out_str, const struct mystr* p_in_str)
{
  static struct mystr s_lhs_str;
  static struct mystr s_rhs_str;
  struct str_locate_result loc_ret;
  struct str_locate_result colon_ret;
  str_copy(&s_lhs_str, p_in_str);
  while (!str_isempty(&s_lhs_str))
  {
    colon_ret = str_locate_char(&s_lhs_str, ':');
    str_split_char(&s_lhs_str, &s_rhs_str, ':');
    if (str_isempty(&s_lhs_str) ||
        (colon_ret.found && str_isempty(&s_rhs_str)))
    {
      return 0;
    }
    loc_ret = str_locate_char(&s_lhs_str, '.');
    if (loc_ret.found)
    {
      if (!str_isempty(&s_rhs_str) ||
          !ipv4_parse_dotquad(p_out_str, &s_lhs_str))
      {
        return 0;
      }
    }
    else if (!ipv6_parse_hex(p_out_str, &s_lhs_str))
    {
      return 0;
    }
    str_copy(&s_lhs_str, &s_rhs_str);
  }
  return 1;
}

static int
ipv6_parse_hex(struct mystr* p_out_str, const struct mystr* p_in_str)
{
  unsigned int len = str_getlen(p_in_str);
  unsigned int i;
  unsigned int val = 0;
  if (len == 0 || len > 4)
  {
    return 0;
  }
  for (i=0; i<len; ++i)
  {
    int ch = vsf_sysutil_toupper(str_get_char_at(p_in_str, i));
    if (ch >= '0' && ch <= '9')
    {
      ch -= '0';
    }
    else if (ch >= 'A' && ch <= 'F')
    {
      ch -= 'A';
      ch += 10;
    }
    else
    {
      return 0;
    }
    val <<= 4;
    val |= ch;
    if (val > 0xFFFF)
    {
      return 0;
    }
  }
  str_append_char(p_out_str, (val >> 8));
  str_append_char(p_out_str, (val & 0xFF));
  return 1;
}

static int
ipv4_parse_dotquad(struct mystr* p_out_str, const struct mystr* p_in_str)
{
  unsigned int len = str_getlen(p_in_str);
  unsigned int i;
  unsigned int val = 0;
  unsigned int final_val = 0;
  int seen_char = 0;
  int dots = 0;
  for (i=0; i<len; ++i)
  {
    int ch = str_get_char_at(p_in_str, i);
    if (ch == '.')
    {
      if (!seen_char || dots == 3)
      {
        return 0;
      }
      seen_char = 0;
      dots++;
      final_val <<= 8;
      final_val |= val;
      val = 0;
    }
    else if (ch >= '0' && ch <= '9')
    {
      ch -= '0';
      val *= 10;
      val += ch;
      if (val > 255)
      {
        return 0;
      }
      seen_char = 1;
    }
    else
    {
      return 0;
    }
  }
  if (dots != 3 || !seen_char)
  {
    return 0;
  }
  final_val <<= 8;
  final_val |= val;
  str_append_char(p_out_str, (final_val >> 24));
  str_append_char(p_out_str, ((final_val >> 16) & 0xFF));
  str_append_char(p_out_str, ((final_val >> 8) & 0xFF));
  str_append_char(p_out_str, (final_val & 0xFF));
  return 1;
}

