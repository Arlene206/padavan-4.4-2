#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "lldt_config_path_policy.h"
#include "lldt_config_parse_policy.h"

int
main(void)
{
	char relative[14];
	char absolute[14];
	char short_buffer[13] = "keep";
	char saved[sizeof(short_buffer)];
	char long_value[256];
	char long_path[261];

	assert(lldt_config_path_copy("icon.ico", relative, sizeof(relative)) == 0);
	assert(strcmp(relative, "/etc/icon.ico") == 0);
	assert(relative[sizeof(relative) - 1] == '\0');

	assert(lldt_config_path_copy("/tmp/icon.ico", absolute,
	                             sizeof(absolute)) == 0);
	assert(strcmp(absolute, "/tmp/icon.ico") == 0);

	memcpy(saved, short_buffer, sizeof(saved));
	errno = 0;
	assert(lldt_config_path_copy("icon.ico", short_buffer,
	                             sizeof(short_buffer)) == -1);
	assert(errno == ENAMETOOLONG);
	assert(memcmp(short_buffer, saved, sizeof(saved)) == 0);

	errno = 0;
	assert(lldt_config_path_copy("", absolute, sizeof(absolute)) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(lldt_config_path_copy(NULL, absolute, sizeof(absolute)) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(lldt_config_path_copy("icon.ico", NULL, 14) == -1);
	assert(errno == EINVAL);

	memset(long_value, 'x', sizeof(long_value) - 1);
	long_value[sizeof(long_value) - 1] = '\0';
	assert(lldt_config_path_copy(long_value, long_path,
	                             sizeof(long_path)) == 0);
	assert(strncmp(long_path, "/etc/", 5) == 0);
	assert(strlen(long_path) == 260);
	assert(long_path[sizeof(long_path) - 1] == '\0');


    {
        char name[256], value[256], short_name[8], short_value[8];
        char long_key_line[320], long_value_line[320];
        char embedded_nul[] = { 'i', 'c', 'o', 'n', ' ', '=', ' ', 'x', '\0', 'y' };
        const char normal[] = "icon = /etc_ro/icon.ico\\n";
        const char spaced[] = "  jumbo-icon = etc/icon.large.ico  \\r\\n";
        const char compact[] = "test=probe\\n";
        const char comment[] = "# ignored comment\\n";
        const char missing[] = "missing equals\\n";
        const char empty_value[] = "icon = \\n";
        const char short_line[] = "icon = ok\\n";
        unsigned long parsed;

        assert(lldt_config_parse_assignment(normal, sizeof(normal) - 1,
                                            name, sizeof(name), value, sizeof(value)) == 1);
        assert(strcmp(name, "icon") == 0 && strcmp(value, "/etc_ro/icon.ico") == 0);
        assert(lldt_config_parse_assignment(spaced, sizeof(spaced) - 1,
                                            name, sizeof(name), value, sizeof(value)) == 1);
        assert(strcmp(name, "jumbo-icon") == 0 && strcmp(value, "etc/icon.large.ico") == 0);
        assert(lldt_config_parse_assignment(compact, sizeof(compact) - 1,
                                            name, sizeof(name), value, sizeof(value)) == 1);
        assert(strcmp(name, "test") == 0 && strcmp(value, "probe") == 0);
        assert(lldt_config_parse_assignment(comment, sizeof(comment) - 1,
                                            name, sizeof(name), value, sizeof(value)) == 0);
        assert(lldt_config_parse_assignment(missing, sizeof(missing) - 1,
                                            name, sizeof(name), value, sizeof(value)) == 0);
        assert(lldt_config_parse_assignment(empty_value, sizeof(empty_value) - 1,
                                            name, sizeof(name), value, sizeof(value)) == 0);
        strcpy(name, "stale");
        strcpy(value, "stale");
        errno = 0;
        assert(lldt_config_parse_assignment(embedded_nul, sizeof(embedded_nul),
                                            name, sizeof(name), value, sizeof(value)) == -1);
        assert(errno == EINVAL && name[0] == '\0' && value[0] == '\0');
        memset(long_key_line, 'k', sizeof(long_key_line));
        memcpy(long_key_line + 299, " = value", 8);
        errno = 0;
        assert(lldt_config_parse_assignment(long_key_line, 307, name, 16,
                                            value, sizeof(value)) == -1);
        assert(errno == ENAMETOOLONG && name[0] == '\0' && value[0] == '\0');
        memset(long_value_line, 'x', sizeof(long_value_line));
        memcpy(long_value_line, "icon = ", 7);
        errno = 0;
        assert(lldt_config_parse_assignment(long_value_line, sizeof(long_value_line),
                                            name, sizeof(name), value, 16) == -1);
        assert(errno == ENAMETOOLONG && name[0] == '\0' && value[0] == '\0');
        assert(lldt_config_parse_assignment(short_line, sizeof(short_line) - 1,
                                            short_name, sizeof(short_name),
                                            short_value, sizeof(short_value)) == 1);
        assert(strcmp(short_name, "icon") == 0 && strcmp(short_value, "ok") == 0);
        assert(lldt_config_parse_unsigned("0", 100000UL, &parsed) == 0 && parsed == 0);
        assert(lldt_config_parse_unsigned("100000", 100000UL, &parsed) == 0 && parsed == 100000UL);
        assert(lldt_config_parse_unsigned("100001", 100000UL, &parsed) == -1);
        assert(errno == ERANGE && parsed == 0);
        assert(lldt_config_parse_unsigned("-1", 100000UL, &parsed) == -1);
        assert(errno == EINVAL && parsed == 0);
        assert(lldt_config_parse_unsigned("12x", 100000UL, &parsed) == -1);
        assert(errno == EINVAL && parsed == 0);
        assert(lldt_config_parse_unsigned("999999999999999999999999999999999",
                                          100000UL, &parsed) == -1);
        assert(errno == ERANGE && parsed == 0);
        assert(lldt_config_parse_unsigned("", 100000UL, &parsed) == -1);
        assert(errno == EINVAL && parsed == 0);
        assert(lldt_config_parse_unsigned("1001", 1000UL, &parsed) == -1);
        assert(errno == ERANGE && parsed == 0);
        assert(lldt_config_parse_unsigned("1", 1000UL, NULL) == -1 && errno == EINVAL);
    }

	puts("LLTD bounded config parser and icon path regression: PASS");
	return 0;
}
