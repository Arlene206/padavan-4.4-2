#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "firewall_vts_lport_policy.h"

static void
assert_error_preserves_output(int read_status, int present,
                              const char *value, int expected_status)
{
    int has_translation = 7;

    assert(firewall_vts_lport_read_status(read_status, present, value,
                                          &has_translation) == expected_status);
    assert(has_translation == 7);
}

static void
test_missing_optional_translation_port_keeps_external_port_fallback(void)
{
    int has_translation = 9;

    assert(firewall_vts_lport_read_status(0, 0, NULL,
                                          &has_translation) == 0);
    assert(has_translation == 0);
    assert(firewall_vts_lport_read_status(0, 0, "",
                                          &has_translation) == 0);
    assert(has_translation == 0);
}

static void
test_empty_and_configured_translation_values(void)
{
    int has_translation = 9;

    assert(firewall_vts_lport_read_status(0, 1, "",
                                          &has_translation) == 0);
    assert(has_translation == 0);
    assert(firewall_vts_lport_read_status(0, 1, "80",
                                          &has_translation) == 0);
    assert(has_translation == 1);
    assert(firewall_vts_lport_read_status(0, 1, "65535",
                                          &has_translation) == 0);
    assert(has_translation == 1);
}

static void
test_nvram_read_errors_never_become_missing_values(void)
{
    assert_error_preserves_output(EACCES, 0, NULL, EACCES);
    assert_error_preserves_output(-1, 0, NULL, EIO);
    assert_error_preserves_output(ERANGE, 1, "80", ERANGE);
}

static void
test_malformed_reader_results_are_rejected(void)
{
    assert_error_preserves_output(0, 2, "80", EINVAL);
    assert_error_preserves_output(0, 1, NULL, EIO);

    assert(firewall_vts_lport_read_status(0, 0, NULL, NULL) == EINVAL);
}

int
main(void)
{
    test_missing_optional_translation_port_keeps_external_port_fallback();
    test_empty_and_configured_translation_values();
    test_nvram_read_errors_never_become_missing_values();
    test_malformed_reader_results_are_rejected();

    puts("Firewall virtual-server optional translation-port read policy: PASS");
    return 0;
}
