#ifndef VPN_CLIENT_NVRAM_POLICY_H
#define VPN_CLIENT_NVRAM_POLICY_H

#include <errno.h>

static inline int
vpn_client_nvram_write_status(int api_result, int saved_errno)
{
    if (api_result == 0)
        return 0;
    return saved_errno > 0 ? saved_errno : EIO;
}

static inline int
vpn_client_nvram_read_status(int api_result, int saved_errno,
                             int value_present)
{
    if ((api_result == 0 && !value_present) ||
        (api_result == 1 && value_present))
        return 0;
    if (api_result < 0)
        return saved_errno > 0 ? saved_errno : EIO;
    return EIO;
}

typedef int (*vpn_client_nvram_set_fn)(void *context, const char *name,
                                       const char *value);

struct vpn_client_nvram_value {
    const char *name;
    const char *value;
};

static inline int
vpn_client_nvram_set_required_values(
    const struct vpn_client_nvram_value *values, unsigned int count,
    vpn_client_nvram_set_fn set_value, void *context)
{
    unsigned int i;
    int status;

    if ((!values && count != 0) || !set_value)
        return EINVAL;
    for (i = 0; i < count; ++i) {
        if (!values[i].name || !values[i].name[0] || !values[i].value)
            return EINVAL;
    }
    for (i = 0; i < count; ++i) {
        status = set_value(context, values[i].name, values[i].value);
        if (status != 0)
            return status;
    }
    return 0;
}

static inline int
vpn_client_nvram_clear_runtime_state(vpn_client_nvram_set_fn set_value,
                                     void *context)
{
    static const struct vpn_client_nvram_value values[] = {
        { "vpnc_dns_t", "" },
        { "vpnc_dom_t", "" },
        { "vpnc_state_t", "0" }
    };

    return vpn_client_nvram_set_required_values(
        values, (unsigned int)(sizeof(values) / sizeof(values[0])),
        set_value, context);
}

#endif /* VPN_CLIENT_NVRAM_POLICY_H */
