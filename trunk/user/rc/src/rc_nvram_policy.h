#ifndef RC_NVRAM_POLICY_H
#define RC_NVRAM_POLICY_H

#include <stdio.h>
#include <string.h>
#include <nvram_linux.h>

struct rc_nvram_policy_ops {
    int (*clear)(void *context);
    int (*get)(void *context, const char *name, char **value);
    int (*set)(void *context, const char *name, const char *value);
    int (*commit)(void *context);
};

static int
rc_nvram_policy_read(const struct rc_nvram_policy_ops *ops, void *context,
                     const char *name, char **value)
{
    int result;

    if (!ops || !ops->get || !name || !value)
        return -1;

    *value = NULL;
    result = ops->get(context, name, value);
    if (result < 0 || result > 1 || (result == 1 && !*value))
        return -1;

    return result;
}

static int
rc_nvram_should_restore_defaults(const struct rc_nvram_policy_ops *ops,
                                 void *context, int *restore_defaults)
{
    static const char *const incompatible_keys[] = {
        "buildno", "buildinfo", "extendno", NULL
    };
    char *value;
    int result;
    int i;

    if (!restore_defaults)
        return -1;

    result = rc_nvram_policy_read(ops, context, "restore_defaults", &value);
    if (result < 0)
        return -1;

    *restore_defaults = (result == 0 || strcmp(value, "0") != 0);
    if (*restore_defaults)
        return 0;

    for (i = 0; incompatible_keys[i]; i++) {
        result = rc_nvram_policy_read(ops, context, incompatible_keys[i],
                                      &value);
        if (result < 0)
            return -1;
        if (result == 0)
            return 0;
    }

    *restore_defaults = 1;
    return 0;
}

static int
rc_nvram_apply_defaults(int restore_defaults,
                        const struct nvram_pair *defaults,
                        const char *lan_mac,
                        const struct rc_nvram_policy_ops *ops,
                        void *context)
{
    const struct nvram_pair *np;
    char *existing;
    char formatted[32];
    const char *value;
    int result;
    int need_commit;
    int length;

    if (!defaults || !lan_mac || !ops || !ops->set ||
        (restore_defaults && !ops->clear))
        return -1;

    if (restore_defaults && ops->clear(context) < 0)
        return -1;

    need_commit = restore_defaults ? 1 : 0;
    for (np = defaults; np->name; np++) {
        if (!np->value)
            return -1;

        if (!restore_defaults) {
            result = rc_nvram_policy_read(ops, context, np->name, &existing);
            if (result < 0)
                return -1;
            if (result == 1)
                continue;
        }

        value = np->value;
        if (strstr(np->name, "wl_ssid") || strstr(np->name, "rt_ssid") ||
            !strcmp(np->name, "wl_guest_ssid") ||
            !strcmp(np->name, "rt_guest_ssid")) {
            length = snprintf(formatted, sizeof(formatted), np->value, lan_mac);
            if (length < 0 || (size_t)length >= sizeof(formatted))
                return -1;
            value = formatted;
        }

        if (ops->set(context, np->name, value) < 0)
            return -1;
        need_commit = 1;
    }

    return need_commit;
}

static int
rc_nvram_commit_if_needed(int need_commit,
                          const struct rc_nvram_policy_ops *ops,
                          void *context)
{
    if (!need_commit)
        return 0;
    if (!ops || !ops->commit)
        return -1;

    return (ops->commit(context) == 0) ? 0 : -1;
}

static int
rc_nvram_erase_and_commit(const struct rc_nvram_policy_ops *ops, void *context)
{
    if (!ops || !ops->set || !ops->commit)
        return -1;
    if (ops->set(context, "restore_defaults", "1") < 0)
        return -1;

    return (ops->commit(context) == 0) ? 0 : -1;
}

#endif
