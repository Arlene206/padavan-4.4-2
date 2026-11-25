#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "eapol_config_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
test_quoted_credentials_escape_parser_metacharacters(void)
{
    FILE *stream;
    char output[128];
    size_t length;

    stream = tmpfile();
    CHECK(stream != NULL);
    CHECK(wpa_eapol_config_write_quoted(
        stream, "alice \"admin\"\\lab") == 0);
    CHECK(fflush(stream) == 0);
    rewind(stream);
    length = fread(output, 1, sizeof(output) - 1, stream);
    output[length] = '\0';
    CHECK(strcmp(output, "\"alice \\\"admin\\\"\\\\lab\"") == 0);
    CHECK(fclose(stream) == 0);
    return 0;
}

static int
test_control_characters_are_rejected(void)
{
    FILE *stream;
    int result;

    stream = tmpfile();
    CHECK(stream != NULL);
    errno = 0;
    result = wpa_eapol_config_write_quoted(stream, "alice\npassword");
    CHECK(result == -1);
    CHECK(errno == EINVAL);
    CHECK(fclose(stream) == 0);
    return 0;
}

static int
test_null_arguments_are_rejected(void)
{
    FILE *stream = tmpfile();

    CHECK(stream != NULL);
    CHECK(wpa_eapol_config_write_quoted(NULL, "value") == -1);
    CHECK(errno == EINVAL);
    CHECK(wpa_eapol_config_write_quoted(stream, NULL) == -1);
    CHECK(errno == EINVAL);
    CHECK(fclose(stream) == 0);
    return 0;
}

int
main(void)
{
    CHECK(test_quoted_credentials_escape_parser_metacharacters() == 0);
    CHECK(test_control_characters_are_rejected() == 0);
    CHECK(test_null_arguments_are_rejected() == 0);
    puts("EAPoL credential configuration escaping policy: PASS");
    return 0;
}
