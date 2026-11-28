#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "usb_printer_executable_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
test_dedicated_printer_executables_resolve(void)
{
    int index;
    char name[sizeof("p9109d")];

    CHECK(strcmp(usb_printer_executable_path("u2ec"),
                 "/usr/sbin/u2ec") == 0);
    CHECK(strcmp(usb_printer_executable_path("lpd"),
                 "/usr/sbin/lpd") == 0);
    CHECK(strcmp(usb_printer_executable_path("p910nd"),
                 "/usr/sbin/p910nd") == 0);
    for (index = 0; index <= 9; ++index) {
        snprintf(name, sizeof(name), "p910%dd", index);
        CHECK(strcmp(usb_printer_executable_path(name),
                     "/usr/sbin/p910nd") == 0);
    }
    return 0;
}

static int
test_unknown_or_malformed_process_names_are_rejected(void)
{
    CHECK(usb_printer_executable_path(NULL) == NULL);
    CHECK(errno == EINVAL);
    CHECK(usb_printer_executable_path("") == NULL);
    CHECK(errno == EINVAL);
    CHECK(usb_printer_executable_path("p91010d") == NULL);
    CHECK(errno == ENOENT);
    CHECK(usb_printer_executable_path("p910ad") == NULL);
    CHECK(errno == ENOENT);
    CHECK(usb_printer_executable_path("p910") == NULL);
    CHECK(errno == ENOENT);
    CHECK(usb_printer_executable_path("sleep") == NULL);
    CHECK(errno == ENOENT);
    return 0;
}

static int
test_executable_identity_requires_exact_installed_binary(void)
{
    const char *expected = "/usr/sbin/lpd";

    CHECK(usb_printer_executable_path_matches(
              expected, "/usr/sbin/lpd") == 1);
    CHECK(usb_printer_executable_path_matches(
              expected, "/usr/sbin/lpd (deleted)") == 1);
    CHECK(usb_printer_executable_path_matches(
              expected, "/usr/bin/sleep") == 0);
    CHECK(usb_printer_executable_path_matches(
              expected, "/tmp/lpd") == 0);
    CHECK(usb_printer_executable_path_matches(
              expected, "/usr/sbin/lpd-helper") == 0);
    CHECK(usb_printer_executable_path_matches(
              expected, "/usr/sbin/lpd (deleted)extra") == 0);
    return 0;
}

static int
test_executable_match_rejects_invalid_arguments(void)
{
    CHECK(usb_printer_executable_path_matches(NULL, "/usr/sbin/lpd") == -1);
    CHECK(errno == EINVAL);
    CHECK(usb_printer_executable_path_matches("/usr/sbin/lpd", NULL) == -1);
    CHECK(errno == EINVAL);
    return 0;
}

int
main(void)
{
    CHECK(test_dedicated_printer_executables_resolve() == 0);
    CHECK(test_unknown_or_malformed_process_names_are_rejected() == 0);
    CHECK(test_executable_identity_requires_exact_installed_binary() == 0);
    CHECK(test_executable_match_rejects_invalid_arguments() == 0);
    puts("USB printer executable identity policy: PASS");
    return 0;
}
