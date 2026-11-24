#ifndef VSF_FTP_OCTAL_ARG_POLICY_H
#define VSF_FTP_OCTAL_ARG_POLICY_H

/*
 * Strict parser for FTP SITE CHMOD and SITE UMASK arguments.
 * The generic str_octal_to_uint() helper stops at the first invalid byte;
 * this parser accepts only a complete ASCII-octal token within the caller's
 * mode range and leaves *value unchanged on failure.
 */
static inline int
ftp_octal_arg_parse(const char* text, unsigned int maximum,
                    unsigned int* value)
{
  const unsigned char* cursor;
  unsigned int parsed = 0;

  if (text == 0 || value == 0 || *text == '\0')
  {
    return 0;
  }
  cursor = (const unsigned char*) text;
  while (*cursor != '\0')
  {
    unsigned int digit;
    if (*cursor < (unsigned char) '0' ||
        *cursor > (unsigned char) '7')
    {
      return 0;
    }
    digit = (unsigned int) (*cursor - (unsigned char) '0');
    if (digit > maximum || parsed > (maximum - digit) / 8U)
    {
      return 0;
    }
    parsed = parsed * 8U + digit;
    cursor++;
  }
  *value = parsed;
  return 1;
}

#endif /* VSF_FTP_OCTAL_ARG_POLICY_H */
