#ifndef FTP_STOU_COLLISION_POLICY_H
#define FTP_STOU_COLLISION_POLICY_H

#include <errno.h>

/*
 * A STOU candidate may become occupied after its nofollow preflight but
 * before openat(O_CREAT | O_EXCL). Only EEXIST is a name collision; all
 * authorization, resolution and I/O failures must remain fail-closed.
 */
#define FTP_STOU_COLLISION_MAX_RETRIES 16U

static inline int
ftp_stou_collision_should_retry(int open_result, int saved_errno,
                                unsigned int collision_retries)
{
  return open_result < 0 && saved_errno == EEXIST &&
         collision_retries < FTP_STOU_COLLISION_MAX_RETRIES;
}

#endif
