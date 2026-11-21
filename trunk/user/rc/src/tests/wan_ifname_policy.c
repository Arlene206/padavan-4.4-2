#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan_ifname_policy.h"

static void
test_valid_interface_names_are_copied(void)
{
        char name[16];

        assert(wan_ifname_copy(name, sizeof(name), "eth3.2") == 0);
        assert(strcmp(name, "eth3.2") == 0);
        assert(wan_ifname_copy(name, sizeof(name), "ppp0") == 0);
        assert(strcmp(name, "ppp0") == 0);
        assert(wan_ifname_copy(name, sizeof(name), "usb_net-1") == 0);
        assert(strcmp(name, "usb_net-1") == 0);
}

static void
test_kernel_sized_boundary_is_enforced(void)
{
        char name[16];
        const char maximum_name[] = "abcdefghijklmn0";

        assert(strlen(maximum_name) == sizeof(name) - 1);
        assert(wan_ifname_copy(name, sizeof(name), maximum_name) == 0);
        assert(strcmp(name, maximum_name) == 0);

        strcpy(name, "stale");
        assert(wan_ifname_copy(name, sizeof(name), "abcdefghijklmnop") ==
               ENAMETOOLONG);
        assert(name[0] == '\0');

        strcpy(name, "stale");
        assert(wan_ifname_copy(name, 5, "eth0") == 0);
        assert(strcmp(name, "eth0") == 0);
        strcpy(name, "x");
        assert(wan_ifname_copy(name, 5, "eth00") == ENAMETOOLONG);
        assert(name[0] == '\0');
}

static void
test_empty_and_invalid_tokens_are_rejected(void)
{
        char name[16];
        const char *invalid[] = {
                "", ".", "..", "eth 0", "eth0\nrule", "eth0\t",
                "eth/0", "eth:0", "eth#0", "eth'0", "eth=0",
                "eth@0", NULL
        };
        size_t i;

        strcpy(name, "stale");
        assert(wan_ifname_copy(name, sizeof(name), NULL) == EINVAL);
        assert(name[0] == '\0');
        for (i = 0; invalid[i] != NULL; ++i) {
                strcpy(name, "stale");
                assert(wan_ifname_copy(name, sizeof(name), invalid[i]) == EINVAL);
                assert(name[0] == '\0');
        }
}

static void
test_invalid_destination_is_rejected(void)
{
        char name[4] = "old";

        assert(wan_ifname_copy(NULL, sizeof(name), "eth0") == EINVAL);
        assert(wan_ifname_copy(name, 0, "eth0") == EINVAL);
        assert(strcmp(name, "old") == 0);
}

int
main(void)
{
        test_valid_interface_names_are_copied();
        test_kernel_sized_boundary_is_enforced();
        test_empty_and_invalid_tokens_are_rejected();
        test_invalid_destination_is_rejected();
        puts("WAN interface-name bounds regression: PASS");
        return 0;
}
