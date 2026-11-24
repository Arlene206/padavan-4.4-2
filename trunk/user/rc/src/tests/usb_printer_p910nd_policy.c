#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "usb_printer_p910nd_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
test_each_supported_device_maps_to_its_port(void)
{
    char device[sizeof("/dev/usb/lp0")];
    char port[2];

    memcpy(device, "/dev/usb/lp0", sizeof(device));
    {
        int index;
        for (index = 0; index <= 9; ++index) {
            device[sizeof(device) - 2] = (char)('0' + index);
            CHECK(usb_printer_p910nd_port_from_device(
                      device, port, sizeof(port)) == 0);
            CHECK(port[0] == (char)('0' + index));
            CHECK(port[1] == '\0');
        }
    }
    return 0;
}

static int
test_invalid_device_paths_fail_without_mutation(void)
{
    char port[2] = "x";

    CHECK(usb_printer_p910nd_port_from_device(NULL, port, sizeof(port)) == EINVAL);
    CHECK(strcmp(port, "x") == 0);
    CHECK(usb_printer_p910nd_port_from_device("/dev/usb/lp", port,
                                               sizeof(port)) == EINVAL);
    CHECK(strcmp(port, "x") == 0);
    CHECK(usb_printer_p910nd_port_from_device("/dev/usb/lp10", port,
                                               sizeof(port)) == EINVAL);
    CHECK(strcmp(port, "x") == 0);
    CHECK(usb_printer_p910nd_port_from_device("/dev/usb/lp2x", port,
                                               sizeof(port)) == EINVAL);
    CHECK(strcmp(port, "x") == 0);
    CHECK(usb_printer_p910nd_port_from_device("/dev/lp2", port,
                                               sizeof(port)) == EINVAL);
    CHECK(strcmp(port, "x") == 0);
    CHECK(usb_printer_p910nd_port_from_device("/dev/usb/lp2", port, 1) ==
          ENAMETOOLONG);
    CHECK(strcmp(port, "x") == 0);
    return 0;
}

static int
test_process_name_tracks_port_number(void)
{
    char process_name[sizeof("p9100d")];

    CHECK(usb_printer_p910nd_process_name('0', process_name,
                                           sizeof(process_name)) == 0);
    CHECK(strcmp(process_name, "p9100d") == 0);
    CHECK(usb_printer_p910nd_process_name('9', process_name,
                                           sizeof(process_name)) == 0);
    CHECK(strcmp(process_name, "p9109d") == 0);
    return 0;
}

static int
test_invalid_process_names_fail_without_mutation(void)
{
    char process_name[sizeof("p9100d")] = "xxxxxx";

    CHECK(usb_printer_p910nd_process_name('x', process_name,
                                           sizeof(process_name)) == EINVAL);
    CHECK(strcmp(process_name, "xxxxxx") == 0);
    CHECK(usb_printer_p910nd_process_name('0', process_name,
                                           sizeof(process_name) - 1) ==
          ENAMETOOLONG);
    CHECK(strcmp(process_name, "xxxxxx") == 0);
    CHECK(usb_printer_p910nd_process_name('0', NULL,
                                           sizeof(process_name)) == EINVAL);
    CHECK(strcmp(process_name, "xxxxxx") == 0);
    return 0;
}

int
main(void)
{
    CHECK(test_each_supported_device_maps_to_its_port() == 0);
    CHECK(test_invalid_device_paths_fail_without_mutation() == 0);
    CHECK(test_process_name_tracks_port_number() == 0);
    CHECK(test_invalid_process_names_fail_without_mutation() == 0);
    puts("USB p910nd per-device port identity policy: PASS");
    return 0;
}
