#ifndef VSF_FTP_REST_OFFSET_POLICY_H
#define VSF_FTP_REST_OFFSET_POLICY_H

#ifndef VSF_FILESIZE_H
#include "filesize.h"
#endif

/* Reject upload restart positions that would create sparse holes. A marker
 * equal to EOF is valid; a marker beyond EOF is not a resumable upload offset.
 */
static int
ftp_rest_offset_is_within_file(filesize_t offset, filesize_t file_size)
{
  return offset >= 0 && file_size >= 0 && offset <= file_size;
}

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

/* ASCII conversions change byte counts, so REST offsets are binary-only. */
static int
ftp_rest_offset_mode_is_valid(int is_ascii, filesize_t offset)
{
  return offset >= 0 && (!is_ascii || offset == 0);
}

/* Preserve APPE append semantics only when no REST marker was supplied. */
static int
ftp_rest_append_is_active(int is_append, int restart_pos_set)
{
  return is_append && !restart_pos_set;
}

#endif /* VSF_FTP_REST_OFFSET_POLICY_H */
