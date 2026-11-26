#include <stdio.h>
#include <string.h>

#include "ralink_country_region_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static const ralink_country_region_entry_t countries[] = {
    { "GB", 1, 1, 0 },
    { "CA", 0, 0, 5 },
    { "CN", 1, 4, 3 }
};

static int
test_missing_and_unknown_country_codes_use_default(void)
{
    CHECK(ralink_country_region_lookup(NULL, 0, countries, 3) == 1);
    CHECK(ralink_country_region_lookup(NULL, 1, countries, 3) == 1);
    CHECK(ralink_country_region_lookup("", 0, countries, 3) == 1);
    CHECK(ralink_country_region_lookup("ZZ", 1, countries, 3) == 1);
    CHECK(ralink_country_region_lookup("GB", 0, NULL, 0) == 1);
    CHECK(ralink_country_region_lookup("GB", 1, countries, 0) == 1);
    return 0;
}

static int
test_known_country_codes_are_case_insensitive_and_band_specific(void)
{
    CHECK(ralink_country_region_lookup("GB", 0, countries, 3) == 1);
    CHECK(ralink_country_region_lookup("ca", 0, countries, 3) == 0);
    CHECK(ralink_country_region_lookup("CA", 1, countries, 3) == 0);
    CHECK(ralink_country_region_lookup("cn", 0, countries, 3) == 1);
    CHECK(ralink_country_region_lookup("CN", 1, countries, 3) == 4);
    return 0;
}

static int
test_country_code_normalization_is_null_safe(void)
{
    CHECK(strcmp(ralink_country_code_normalize(NULL), "GB") == 0);
    CHECK(strcmp(ralink_country_code_normalize(""), "GB") == 0);
    CHECK(strcmp(ralink_country_code_normalize("U"), "GB") == 0);
    CHECK(strcmp(ralink_country_code_normalize("USA"), "GB") == 0);
    CHECK(strcmp(ralink_country_code_normalize("ca"), "ca") == 0);
    return 0;
}

int
main(void)
{
    CHECK(test_missing_and_unknown_country_codes_use_default() == 0);
    CHECK(test_known_country_codes_are_case_insensitive_and_band_specific() == 0);
    CHECK(test_country_code_normalization_is_null_safe() == 0);
    puts("Ralink missing country-code fallback and band region policy: PASS");
    return 0;
}
