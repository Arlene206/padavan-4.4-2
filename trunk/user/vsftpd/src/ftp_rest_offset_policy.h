#ifndef VSF_FTP_REST_OFFSET_POLICY_H
#define VSF_FTP_REST_OFFSET_POLICY_H

#ifndef VSF_FILESIZE_H
#include "filesize.h"
#endif

/*
 * Parse the decimal restart marker accepted by the FTP REST command.
 * Preserve the existing 15-digit range while rejecting signs, whitespace,
 * embedded NULs and non-decimal suffixes instead of silently mapping them
 * to offset zero.
 */
static int
ftp_rest_offset_parse(const char* p_input, unsigned int input_length,
                      filesize_t* p_offset)
{
  unsigned int i;
  filesize_t parsed = 0;
  if (p_input == 0 || p_offset == 0 ||
      input_length == 0U || input_length > 15U)
  {
    return 0;
  }
  for (i = 0; i < input_length; ++i)
  {
    unsigned char ch = (unsigned char) p_input[i];
    if (ch < '0' || ch > '9')
    {
      return 0;
    }
    parsed = parsed * 10 + (filesize_t) (ch - '0');
  }
  *p_offset = parsed;
  return 1;
}

#endif /* VSF_FTP_REST_OFFSET_POLICY_H */
