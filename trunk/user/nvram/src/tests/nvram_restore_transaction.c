#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "nvram_restore_transaction.h"

struct fake_nvram {
    char persistent[256], temporary[256];
    int clear_calls, set_calls, fail_clear_call, fail_set_call_a, fail_set_call_b;
    char attempted[16][32];
};
static int fake_clear(void *context)
{
    struct fake_nvram *f = (struct fake_nvram *)context;
    ++f->clear_calls;
    if (f->clear_calls == f->fail_clear_call) { errno = EBUSY; return -1; }
    memset(f->persistent, 0, sizeof(f->persistent));
    memset(f->temporary, 0, sizeof(f->temporary));
    return 0;
}
static int fake_set(void *context, const char *name, const char *value, int is_temp)
{
    struct fake_nvram *f = (struct fake_nvram *)context;
    char *target = is_temp ? f->temporary : f->persistent;
    size_t nl = strlen(name), vl = strlen(value), pos = 0, need = nl + 1 + vl + 1;
    ++f->set_calls;
    if (f->set_calls <= 16)
        snprintf(f->attempted[f->set_calls - 1], sizeof(f->attempted[0]), "%s", name);
    if (f->set_calls == f->fail_set_call_a || f->set_calls == f->fail_set_call_b) {
        errno = EIO; return -1;
    }
    while (pos < sizeof(f->persistent) && target[pos] != '\0')
        pos += strlen(target + pos) + 1;
    if (pos + need + 1 > sizeof(f->persistent)) { errno = ENOSPC; return -1; }
    memcpy(target + pos, name, nl);
    target[pos + nl] = '=';
    memcpy(target + pos + nl + 1, value, vl);
    target[pos + need] = '\0';
    target[pos + need + 1] = '\0';
    return 0;
}
static int fake_should_apply(void *context, const char *name)
{
    (void)context;
    return strcmp(name, "lan_ipaddr") == 0 || strcmp(name, "wan_proto") == 0;
}
static void fake_init(struct fake_nvram *f)
{
    static const char p[] = "lan_ipaddr=192.168.1.1\0router_name=old\0\0";
    static const char t[] = "wanx_ipaddr=10.0.0.2\0wanx_gateway=10.0.0.1\0\0";
    memset(f, 0, sizeof(*f));
    memcpy(f->persistent, p, sizeof(p));
    memcpy(f->temporary, t, sizeof(t));
}
static int run(struct fake_nvram *f, char *profile, int *rollback_failed)
{
    static const char p[] = "lan_ipaddr=192.168.1.1\0router_name=old\0\0";
    static const char t[] = "wanx_ipaddr=10.0.0.2\0wanx_gateway=10.0.0.1\0\0";
    char p_copy[sizeof(p)], t_copy[sizeof(t)];
    const struct nvram_restore_transaction_ops ops = {
        fake_clear, fake_set, fake_should_apply, f
    };
    memcpy(p_copy, p, sizeof(p));
    memcpy(t_copy, t, sizeof(t));
    return nvram_restore_transaction_run(
        profile,
        sizeof("lan_ipaddr=192.168.1.99\0wan_proto=dhcp\0ignored_key=new\0\0"),
        p_copy, sizeof(p_copy), t_copy, sizeof(t_copy), 63, 4095, &ops,
        rollback_failed);
}
static int success_test(void)
{
    static char profile[] = "lan_ipaddr=192.168.1.99\0wan_proto=dhcp\0ignored_key=new\0\0";
    static const char expected[] = "lan_ipaddr=192.168.1.99\0wan_proto=dhcp\0\0";
    struct fake_nvram f; int rollback = -1;
    fake_init(&f);
    if (run(&f, profile, &rollback) || rollback || f.clear_calls != 1 ||
        f.set_calls != 2 || memcmp(f.persistent, expected, sizeof(expected)) || f.temporary[0]) {
        fprintf(stderr, "FAIL: successful profile application semantics\n"); return 0;
    }
    return 1;
}
static int rollback_test(void)
{
    static char profile[] = "lan_ipaddr=192.168.1.99\0wan_proto=dhcp\0ignored_key=new\0\0";
    static const char old_p[] = "lan_ipaddr=192.168.1.1\0router_name=old\0\0";
    static const char old_t[] = "wanx_ipaddr=10.0.0.2\0wanx_gateway=10.0.0.1\0\0";
    struct fake_nvram f; int rollback = -1;
    fake_init(&f); f.fail_set_call_a = 2; errno = 0;
    if (!run(&f, profile, &rollback) || errno != EIO || rollback || f.clear_calls != 2 ||
        f.set_calls != 6 || memcmp(f.persistent, old_p, sizeof(old_p)) ||
        memcmp(f.temporary, old_t, sizeof(old_t))) {
        fprintf(stderr, "FAIL: setter failure did not restore persistent and temp tuples\n"); return 0;
    }
    return 1;
}
static int continue_test(void)
{
    static char profile[] = "lan_ipaddr=192.168.1.99\0wan_proto=dhcp\0ignored_key=new\0\0";
    struct fake_nvram f; int rollback = -1;
    fake_init(&f); f.fail_set_call_a = 2; f.fail_set_call_b = 4; errno = 0;
    if (!run(&f, profile, &rollback) || errno != EIO || !rollback || f.set_calls != 6 ||
        strcmp(f.attempted[4], "wanx_ipaddr") || strcmp(f.attempted[5], "wanx_gateway")) {
        fprintf(stderr, "FAIL: rollback did not continue after a setter error\n"); return 0;
    }
    return 1;
}
static int clear_failure_test(void)
{
    static char profile[] = "lan_ipaddr=192.168.1.99\0wan_proto=dhcp\0ignored_key=new\0\0";
    static const char old_p[] = "lan_ipaddr=192.168.1.1\0router_name=old\0\0";
    static const char old_t[] = "wanx_ipaddr=10.0.0.2\0wanx_gateway=10.0.0.1\0\0";
    struct fake_nvram f; int rollback = -1;
    fake_init(&f); f.fail_clear_call = 1; errno = 0;
    if (!run(&f, profile, &rollback) || errno != EBUSY || rollback || f.clear_calls != 2 ||
        memcmp(f.persistent, old_p, sizeof(old_p)) || memcmp(f.temporary, old_t, sizeof(old_t))) {
        fprintf(stderr, "FAIL: failed initial clear skipped recovery\n"); return 0;
    }
    return 1;
}
static int invalid_snapshot_test(void)
{
    static char profile[] = "lan_ipaddr=192.168.1.99\0wan_proto=dhcp\0ignored_key=new\0\0";
    static char persistent[] = "lan_ipaddr=old\0\0";
    static char temporary[] = { 'x', '=', '1', '\0', '\0', 'z', '\0' };
    struct fake_nvram f; int rollback = -1;
    const struct nvram_restore_transaction_ops ops = {
        fake_clear, fake_set, fake_should_apply, &f
    };
    fake_init(&f); errno = 0;
    if (!nvram_restore_transaction_run(profile, sizeof(profile), persistent, sizeof(persistent),
            temporary, sizeof(temporary), 63, 4095, &ops, &rollback) ||
        errno != EINVAL || f.clear_calls) {
        fprintf(stderr, "FAIL: malformed snapshot mutated NVRAM\n"); return 0;
    }
    return 1;
}
static int profile_codec_test(void)
{
    static const unsigned char legacy_header[] = {
        'H', 'D', 'R', '1', 2, 0, 0, 0
    };
    unsigned char payload[] = { 0x10, 0xfd };
    unsigned long file_length = 0;
    unsigned char random_value = 0;
    int is_current_format = -1;

    if (!nvram_profile_parse_header(legacy_header, 16, 16, &file_length,
                                    &random_value, &is_current_format) ||
        file_length != 2 || random_value != 0 || is_current_format != 0) {
        fprintf(stderr, "FAIL: profile header parser regression\\n");
        return 0;
    }
    nvram_profile_decode_payload(payload, sizeof(payload), 1);
    if (payload[0] != 0xf0 || payload[1] != '\\0') {
        fprintf(stderr, "FAIL: profile payload decoder regression\\n");
        return 0;
    }
    return 1;
}

int main(void)
{
    if (!profile_codec_test() || !success_test() || !rollback_test() ||
        !continue_test() || !clear_failure_test() || !invalid_snapshot_test())
        return 1;
    puts("NVRAM restore transaction and profile codec regression: PASS");
    return 0;
}
