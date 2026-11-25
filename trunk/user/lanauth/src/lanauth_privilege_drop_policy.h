#ifndef LANAUTH_PRIVILEGE_DROP_POLICY_H
#define LANAUTH_PRIVILEGE_DROP_POLICY_H
#include <errno.h>
#include <sys/types.h>
enum lanauth_privilege_drop_stage {
    LANAUTH_PRIVILEGE_DROP_STAGE_NONE = 0,
    LANAUTH_PRIVILEGE_DROP_STAGE_GID,
    LANAUTH_PRIVILEGE_DROP_STAGE_UID
};
typedef int (*lanauth_setgid_fn)(void *context, gid_t gid);
typedef int (*lanauth_setuid_fn)(void *context, uid_t uid);
/* Load secrets before this policy; group reduction always precedes user reduction. */
static int
lanauth_apply_privilege_drop_with_ops(int change_gid, gid_t gid,
                                      int change_uid, uid_t uid,
                                      lanauth_setgid_fn set_gid,
                                      lanauth_setuid_fn set_uid,
                                      void *context,
                                      enum lanauth_privilege_drop_stage *failure_stage)
{
    int status;
    if (failure_stage)
        *failure_stage = LANAUTH_PRIVILEGE_DROP_STAGE_NONE;
    if ((change_gid && !set_gid) || (change_uid && !set_uid))
        return EINVAL;
    if (change_gid) {
        errno = 0;
        status = set_gid(context, gid);
        if (status != 0) {
            if (failure_stage)
                *failure_stage = LANAUTH_PRIVILEGE_DROP_STAGE_GID;
            return errno ? errno : EACCES;
        }
    }
    if (change_uid) {
        errno = 0;
        status = set_uid(context, uid);
        if (status != 0) {
            if (failure_stage)
                *failure_stage = LANAUTH_PRIVILEGE_DROP_STAGE_UID;
            return errno ? errno : EACCES;
        }
    }
    return 0;
}
#endif
