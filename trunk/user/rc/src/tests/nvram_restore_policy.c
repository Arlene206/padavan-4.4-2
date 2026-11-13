#include <stdio.h>
#include <string.h>
#include <nvram_linux.h>
#include "rc_nvram_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
                fprintf(stderr, "FAIL:%d: %s\\n", __LINE__, #condition); \
        return 1; \
    } \
} while (0)

struct mock_entry {
    char name[64];
    char value[128];
};

struct mock_nvram {
    struct mock_entry entries[16];
    int count;
    int clear_result;
    const char *set_fail_name;
    const char *get_fail_name;
    int commit_result;
    int clear_calls;
    int set_calls;
    int commit_calls;
};

static int
mock_find(struct mock_nvram *mock, const char *name)
{
    int i;
    for (i = 0; i < mock->count; i++) {
        if (!strcmp(mock->entries[i].name, name))
            return i;
    }
    return -1;
}

static int
mock_clear(void *context)
{
    struct mock_nvram *mock = (struct mock_nvram *)context;
    mock->clear_calls++;
    if (mock->clear_result < 0)
        return mock->clear_result;
    mock->count = 0;
    return 0;
}

static int
mock_get(void *context, const char *name, char **value)
{
    struct mock_nvram *mock = (struct mock_nvram *)context;
    int index;

    *value = NULL;
    if (mock->get_fail_name && !strcmp(name, mock->get_fail_name))
        return -1;
    index = mock_find(mock, name);
    if (index < 0)
        return 0;
    *value = mock->entries[index].value;
    return 1;
}

static int
mock_set(void *context, const char *name, const char *value)
{
    struct mock_nvram *mock = (struct mock_nvram *)context;
    int index;

    mock->set_calls++;
    if (mock->set_fail_name && !strcmp(name, mock->set_fail_name))
        return -1;
    index = mock_find(mock, name);
    if (index < 0) {
        if (mock->count >= (int)(sizeof(mock->entries) / sizeof(mock->entries[0])))
            return -1;
        index = mock->count++;
    }
    snprintf(mock->entries[index].name, sizeof(mock->entries[index].name),
             "%s", name);
    snprintf(mock->entries[index].value, sizeof(mock->entries[index].value),
             "%s", value);
    return 0;
}

static int
mock_commit(void *context)
{
    struct mock_nvram *mock = (struct mock_nvram *)context;
    mock->commit_calls++;
    return mock->commit_result;
}

static const struct rc_nvram_policy_ops mock_ops = {
    mock_clear, mock_get, mock_set, mock_commit
};

static void
mock_reset(struct mock_nvram *mock)
{
    memset(mock, 0, sizeof(*mock));
}

static int
mock_put(struct mock_nvram *mock, const char *name, const char *value)
{
    return mock_set(mock, name, value);
}

int
main(void)
{
    static const struct nvram_pair defaults[] = {
        { "restore_defaults", "0" },
        { "wl_ssid", "Padavan-%s" },
        { "lan_ipaddr", "192.168.1.1" },
        { NULL, NULL }
    };
    struct mock_nvram mock;
    int restore;
    int result;

    mock_reset(&mock);
    CHECK(mock_put(&mock, "restore_defaults", "0") == 0);
    CHECK(rc_nvram_should_restore_defaults(&mock_ops, &mock, &restore) == 0);
    CHECK(restore == 0);
    CHECK(mock_put(&mock, "buildno", "1") == 0);
    CHECK(mock_put(&mock, "buildinfo", "x") == 0);
    CHECK(mock_put(&mock, "extendno", "y") == 0);
    CHECK(rc_nvram_should_restore_defaults(&mock_ops, &mock, &restore) == 0);
    CHECK(restore == 1);

    mock_reset(&mock);
    CHECK(mock_put(&mock, "restore_defaults", "0") == 0);
    mock.get_fail_name = "restore_defaults";
    CHECK(rc_nvram_should_restore_defaults(&mock_ops, &mock, &restore) == -1);

    mock_reset(&mock);
    mock.clear_result = -1;
    CHECK(rc_nvram_apply_defaults(1, defaults, "A1B2", &mock_ops, &mock) == -1);
    CHECK(mock.clear_calls == 1 && mock.set_calls == 0);

    mock_reset(&mock);
    mock.set_fail_name = "wl_ssid";
    CHECK(rc_nvram_apply_defaults(1, defaults, "A1B2", &mock_ops, &mock) == -1);
    CHECK(mock.clear_calls == 1 && mock.set_calls == 2);

    mock_reset(&mock);
    mock.get_fail_name = "wl_ssid";
    CHECK(rc_nvram_apply_defaults(0, defaults, "A1B2", &mock_ops, &mock) == -1);
    CHECK(mock.set_calls == 1);

    mock_reset(&mock);
    CHECK(mock_put(&mock, "lan_ipaddr", "10.0.0.1") == 0);
    result = rc_nvram_apply_defaults(0, defaults, "A1B2", &mock_ops, &mock);
    CHECK(result == 1);
    CHECK(mock_find(&mock, "wl_ssid") >= 0);
    CHECK(!strcmp(mock.entries[mock_find(&mock, "wl_ssid")].value,
                  "Padavan-A1B2"));
    CHECK(!strcmp(mock.entries[mock_find(&mock, "lan_ipaddr")].value,
                  "10.0.0.1"));

    mock_reset(&mock);
    CHECK(rc_nvram_apply_defaults(0, defaults, "A1B2", &mock_ops, &mock) == 1);
    CHECK(mock.set_calls == 3);

    mock_reset(&mock);
    CHECK(rc_nvram_commit_if_needed(0, &mock_ops, &mock) == 0);
    CHECK(mock.commit_calls == 0);
    mock.commit_result = -1;
    CHECK(rc_nvram_commit_if_needed(1, &mock_ops, &mock) == -1);
    CHECK(mock.commit_calls == 1);
    mock.commit_result = 0;
    CHECK(rc_nvram_commit_if_needed(1, &mock_ops, &mock) == 0);
    CHECK(mock.commit_calls == 2);

    mock_reset(&mock);
    mock.set_fail_name = "restore_defaults";
    CHECK(rc_nvram_erase_and_commit(&mock_ops, &mock) == -1);
    CHECK(mock.commit_calls == 0);
    mock_reset(&mock);
    mock.commit_result = -1;
    CHECK(rc_nvram_erase_and_commit(&mock_ops, &mock) == -1);
    CHECK(mock.commit_calls == 1);
    mock_reset(&mock);
    CHECK(rc_nvram_erase_and_commit(&mock_ops, &mock) == 0);
    CHECK(mock.commit_calls == 1);

    puts("RC NVRAM persistence policy: PASS");
    return 0;
}
