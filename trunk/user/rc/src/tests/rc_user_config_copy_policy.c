#include <stdio.h>
#include <string.h>

#include "rc_user_config_copy_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
is_forbidden(const char *line, const char **forbid_list)
{
    const char **item;

    for (item = forbid_list; *item; item++) {
        if (strncmp(line, *item, strlen(*item)) == 0)
            return 1;
    }
    return 0;
}

static int
test_overlong_and_binary_suffixes_are_not_reparsed(void)
{
    FILE *input = tmpfile(), *output = tmpfile();
    const char *forbid_list[] = { "blocked=", NULL };
    char actual[1024], expected[512];
    size_t expected_length, actual_length;
    int i, status;

    CHECK(input != NULL && output != NULL);
    CHECK(fputs("keep=first\n#comment\n;comment\nblocked=secret\n",
                input) >= 0);

    for (i = 0; i < 255; i++)
        CHECK(fputc('X', input) != EOF);
    CHECK(fputs("injected=payload\n", input) >= 0);

    CHECK(fputs("binary-prefix=", input) >= 0);
    CHECK(fputc('\0', input) != EOF);
    CHECK(fputs("binary-injected=payload\n", input) >= 0);

    CHECK(fputs("keep=second\n", input) >= 0);
    for (i = 0; i < 255; i++)
        CHECK(fputc('B', input) != EOF);
    CHECK(fputc('\n', input) != EOF);
    CHECK(fputs("unterminated=value", input) >= 0);
    CHECK(fflush(input) == 0);
    rewind(input);

    status = rc_user_config_copy_stream(output, input, forbid_list,
                                        is_forbidden);
    CHECK(status == 0);
    CHECK(fflush(output) == 0);
    rewind(output);

    memcpy(expected, "keep=first\nkeep=second\n", 23);
    expected_length = 23;
    memset(expected + expected_length, 'B', 255);
    expected_length += 255;
    expected[expected_length++] = '\n';
    memcpy(expected + expected_length, "unterminated=value\n", 19);
    expected_length += 19;

    actual_length = fread(actual, 1, sizeof(actual), output);
    CHECK(!ferror(output));
    CHECK(actual_length == expected_length);
    CHECK(memcmp(actual, expected, expected_length) == 0);

    fclose(input);
    fclose(output);
    return 0;
}

static int
test_forbid_filter_and_normal_records(void)
{
    FILE *input = tmpfile(), *output = tmpfile();
    const char *forbid_list[] = { "secret=", NULL };
    char actual[128];
    size_t length;
    int status;

    CHECK(input != NULL && output != NULL);
    CHECK(fputs("alpha=1\nsecret=hidden\nbeta=2\n", input) >= 0);
    rewind(input);
    status = rc_user_config_copy_stream(output, input, forbid_list,
                                        is_forbidden);
    CHECK(status == 0);
    rewind(output);
    length = fread(actual, 1, sizeof(actual), output);
    CHECK(length == strlen("alpha=1\nbeta=2\n"));
    CHECK(memcmp(actual, "alpha=1\nbeta=2\n", length) == 0);

    fclose(input);
    fclose(output);
    return 0;
}

int
main(void)
{
    CHECK(test_overlong_and_binary_suffixes_are_not_reparsed() == 0);
    CHECK(test_forbid_filter_and_normal_records() == 0);
    puts("RC user-config whole-line truncation policy: PASS");
    return 0;
}
