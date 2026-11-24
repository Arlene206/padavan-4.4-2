#ifndef PADAVAN_LLDT_CONFIG_PARSE_POLICY_H
#define PADAVAN_LLDT_CONFIG_PARSE_POLICY_H
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

/* Parse one length-delimited "name = value" assignment. 1=parsed, 0=not an
 * assignment, -1=malformed/overlong; output tokens remain empty on failure. */
static inline int
lldt_config_parse_assignment(const char *line, size_t length,
                             char *name, size_t name_capacity,
                             char *value, size_t value_capacity)
{
    size_t position = 0;
    size_t name_start;
    size_t name_length;
    size_t value_start;
    size_t value_length;

    if (!line || !name || name_capacity == 0 ||
        !value || value_capacity == 0) {
        errno = EINVAL;
        return -1;
    }
    name[0] = '\0';
    value[0] = '\0';
    if (length != 0 && memchr(line, '\0', length) != NULL) {
        errno = EINVAL;
        return -1;
    }
    while (position < length && isspace((unsigned char)line[position]))
        position++;
    if (position == length || line[position] == '#')
        return 0;
    name_start = position;
    while (position < length && !isspace((unsigned char)line[position]) &&
           line[position] != '=')
        position++;
    name_length = position - name_start;
    if (name_length == 0)
        return 0;
    while (position < length && isspace((unsigned char)line[position]))
        position++;
    if (position == length || line[position] != '=')
        return 0;
    position++;
    while (position < length && isspace((unsigned char)line[position]))
        position++;
    value_start = position;
    while (position < length && !isspace((unsigned char)line[position]))
        position++;
    value_length = position - value_start;
    if (value_length == 0)
        return 0;
    if (name_length >= name_capacity || value_length >= value_capacity) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(name, line + name_start, name_length);
    name[name_length] = '\0';
    memcpy(value, line + value_start, value_length);
    value[value_length] = '\0';
    return 1;
}

/* Parse a bounded unsigned decimal; reject signs, junk, and values > maximum. */
static inline int
lldt_config_parse_unsigned(const char *text, unsigned long maximum,
                           unsigned long *value_out)
{
    const unsigned char *cursor;
    unsigned long value = 0;

    if (!text || !value_out) {
        errno = EINVAL;
        return -1;
    }
    *value_out = 0;
    if (text[0] == '\0') {
        errno = EINVAL;
        return -1;
    }
    for (cursor = (const unsigned char *)text; *cursor != '\0'; cursor++) {
        unsigned int digit;
        if (*cursor < '0' || *cursor > '9') {
            errno = EINVAL;
            return -1;
        }
        digit = (unsigned int)(*cursor - '0');
        if ((unsigned long)digit > maximum ||
            value > (maximum - (unsigned long)digit) / 10UL) {
            errno = ERANGE;
            return -1;
        }
        value = value * 10UL + (unsigned long)digit;
    }
    *value_out = value;
    return 0;
}
#endif
