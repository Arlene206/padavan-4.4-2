#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "lanauth_privilege_drop_policy.h"
struct fixture {
    int calls[4];
    int call_count;
    gid_t received_gid;
    uid_t received_uid;
    int gid_error;
    int uid_error;
};
static void
record_call(struct fixture *fixture, int call)
{
    if (fixture->call_count < (int)(sizeof(fixture->calls) / sizeof(fixture->calls[0])))
        fixture->calls[fixture->call_count++] = call;
}
static int
fake_setgid(void *context, gid_t gid)
{
    struct fixture *fixture = (struct fixture *)context;
    record_call(fixture, 1);
    fixture->received_gid = gid;
    if (fixture->gid_error) {
        errno = fixture->gid_error;
        return -1;
    }
    return 0;
}
static int
fake_setuid(void *context, uid_t uid)
{
    struct fixture *fixture = (struct fixture *)context;
    record_call(fixture, 2);
    fixture->received_uid = uid;
    if (fixture->uid_error) {
        errno = fixture->uid_error;
        return -1;
    }
    return 0;
}
static int
test_group_is_applied_before_user_and_option_order_is_irrelevant(void)
{
    struct fixture fixture;
    enum lanauth_privilege_drop_stage stage;
    int status;
    memset(&fixture, 0, sizeof(fixture));
    status = lanauth_apply_privilege_drop_with_ops(
        1, (gid_t)73, 1, (uid_t)29, fake_setgid, fake_setuid, &fixture, &stage);
    if (status != 0 || stage != LANAUTH_PRIVILEGE_DROP_STAGE_NONE ||
        fixture.call_count != 2 || fixture.calls[0] != 1 || fixture.calls[1] != 2 ||
        fixture.received_gid != (gid_t)73 || fixture.received_uid != (uid_t)29)
        return 1;
    return 0;
}
static int
test_group_failure_prevents_user_drop(void)
{
    struct fixture fixture;
    enum lanauth_privilege_drop_stage stage;
    int status;
    memset(&fixture, 0, sizeof(fixture));
    fixture.gid_error = EPERM;
    status = lanauth_apply_privilege_drop_with_ops(
        1, (gid_t)73, 1, (uid_t)29, fake_setgid, fake_setuid, &fixture, &stage);
    if (status != EPERM || stage != LANAUTH_PRIVILEGE_DROP_STAGE_GID ||
        fixture.call_count != 1 || fixture.calls[0] != 1)
        return 1;
    return 0;
}
static int
test_user_failure_is_reported_after_group_drop(void)
{
    struct fixture fixture;
    enum lanauth_privilege_drop_stage stage;
    int status;
    memset(&fixture, 0, sizeof(fixture));
    fixture.uid_error = EACCES;
    status = lanauth_apply_privilege_drop_with_ops(
        1, (gid_t)73, 1, (uid_t)29, fake_setgid, fake_setuid, &fixture, &stage);
    if (status != EACCES || stage != LANAUTH_PRIVILEGE_DROP_STAGE_UID ||
        fixture.call_count != 2 || fixture.calls[0] != 1 || fixture.calls[1] != 2)
        return 1;
    return 0;
}
static int
test_unrequested_ids_are_not_changed(void)
{
    struct fixture fixture;
    enum lanauth_privilege_drop_stage stage;
    int status;
    memset(&fixture, 0, sizeof(fixture));
    status = lanauth_apply_privilege_drop_with_ops(
        0, (gid_t)0, 0, (uid_t)0, NULL, NULL, &fixture, &stage);
    if (status != 0 || stage != LANAUTH_PRIVILEGE_DROP_STAGE_NONE ||
        fixture.call_count != 0)
        return 1;
    return 0;
}
static int
test_missing_requested_callback_fails_closed(void)
{
    struct fixture fixture;
    enum lanauth_privilege_drop_stage stage;
    int status;
    memset(&fixture, 0, sizeof(fixture));
    status = lanauth_apply_privilege_drop_with_ops(
        1, (gid_t)73, 0, (uid_t)0, NULL, fake_setuid, &fixture, &stage);
    if (status != EINVAL || fixture.call_count != 0)
        return 1;
    return 0;
}
int
main(void)
{
    if (test_group_is_applied_before_user_and_option_order_is_irrelevant() != 0 ||
        test_group_failure_prevents_user_drop() != 0 ||
        test_user_failure_is_reported_after_group_drop() != 0 ||
        test_unrequested_ids_are_not_changed() != 0 ||
        test_missing_requested_callback_fails_closed() != 0) {
        fprintf(stderr, "LAN authentication privilege-drop policy regression failed\n");
        return 1;
    }
    puts("LAN authentication checked group-before-user privilege drop: PASS");
    return 0;
}
