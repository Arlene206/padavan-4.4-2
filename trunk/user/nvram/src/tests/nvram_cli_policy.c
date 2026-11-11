#include <stdio.h>
#include <string.h>

#include "nvram_cli_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL:%d: %s\\n", __LINE__, #condition); \
        return 0; \
    } \
} while (0)

static int test_valid_assignments(void)
{
    char storage[1024];
    char *name = NULL;
    char *value = NULL;

    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage),
                                     "wl_ssid=Guest Network", 63, 4095,
                                     &name, &value) == 0);
    CHECK(name == storage);
    CHECK(strcmp(name, "wl_ssid") == 0);
    CHECK(strcmp(value, "Guest Network") == 0);

    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage),
                                     "empty=", 63, 4095,
                                     &name, &value) == 0);
    CHECK(strcmp(name, "empty") == 0);
    CHECK(strcmp(value, "") == 0);
    return 1;
}

static int test_rejects_invalid_assignments(void)
{
    char storage[1024];
    char *name;
    char *value;
    char long_name[68];
    char long_value[16];
    char oversized[1025];

    name = (char *)1;
    value = (char *)1;
    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage),
                                     "missing-separator", 63, 4095,
                                     &name, &value) < 0);
    CHECK(name == NULL && value == NULL);
    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage),
                                     "=value", 63, 4095,
                                     &name, &value) < 0);
    CHECK(name == NULL && value == NULL);
    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage),
                                     "", 63, 4095, &name, &value) < 0);
    CHECK(name == NULL && value == NULL);

    memset(long_name, 'n', sizeof(long_name));
    long_name[64] = '=';
    long_name[65] = 'v';
    long_name[66] = '\0';
    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage),
                                     long_name, 63, 4095,
                                     &name, &value) < 0);

    memcpy(long_value, "key=12345", 10);
    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage),
                                     long_value, 63, 4,
                                     &name, &value) < 0);

    memset(oversized, 'x', sizeof(oversized));
    oversized[0] = 'k';
    oversized[1] = '=';
    oversized[sizeof(oversized) - 1] = '\0';
    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage),
                                     oversized, 63, 4095,
                                     &name, &value) < 0);

    CHECK(nvram_cli_parse_assignment(NULL, sizeof(storage), "key=value",
                                     63, 4095, &name, &value) < 0);
    CHECK(nvram_cli_parse_assignment(storage, 0, "key=value", 63, 4095,
                                     &name, &value) < 0);
    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage), NULL,
                                     63, 4095, &name, &value) < 0);
    return 1;
}

static int test_exact_buffer_boundary(void)
{
    char storage[1024];
    char assignment[1024];
    char *name = NULL;
    char *value = NULL;

    memset(assignment, 'v', sizeof(assignment));
    assignment[0] = 'k';
    assignment[1] = '=';
    assignment[sizeof(assignment) - 1] = '\0';
    CHECK(nvram_cli_parse_assignment(storage, sizeof(storage), assignment,
                                     63, 4095, &name, &value) == 0);
    CHECK(strcmp(name, "k") == 0);
    CHECK(strlen(value) == sizeof(assignment) - 3);
    return 1;
}

int main(void)
{
    if (!test_valid_assignments() || !test_rejects_invalid_assignments() ||
        !test_exact_buffer_boundary())
        return 1;
    puts("NVRAM CLI assignment policy: PASS");
    return 0;
}
