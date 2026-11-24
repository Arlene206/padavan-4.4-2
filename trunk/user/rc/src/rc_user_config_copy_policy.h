#ifndef RC_USER_CONFIG_COPY_POLICY_H
#define RC_USER_CONFIG_COPY_POLICY_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef int (*rc_user_config_forbidden_fn)(const char *line,
                                            const char **forbid_list);

static int
rc_user_config_emit_line(FILE *output, char *line, size_t length,
                         const char **forbid_list,
                         rc_user_config_forbidden_fn is_forbidden)
{
    if (length == 0 || line[0] == '\r' ||
        line[0] == '#' || line[0] == ';')
        return 0;

    line[length++] = '\n';
    line[length] = '\0';

    if (forbid_list && is_forbidden(line, forbid_list))
        return 0;

    if (fputs(line, output) == EOF)
        return errno ? errno : EIO;
    return 0;
}

/*
 * Read complete physical records. Records over 255 bytes or containing NUL
 * are drained and rejected, so no suffix can be reinterpreted as a new line.
 */
static int
rc_user_config_copy_stream(FILE *output, FILE *input,
                           const char **forbid_list,
                           rc_user_config_forbidden_fn is_forbidden)
{
    char line[257];
    size_t length = 0;
    int overlong = 0, invalid = 0, ch, status;

    if (!output || !input || (forbid_list && !is_forbidden))
        return EINVAL;

    while ((ch = fgetc(input)) != EOF) {
        if (ch == '\n') {
            if (!overlong && !invalid) {
                status = rc_user_config_emit_line(
                    output, line, length, forbid_list, is_forbidden);
                if (status != 0)
                    return status;
            }
            length = 0;
            overlong = 0;
            invalid = 0;
            continue;
        }

        if (ch == '\0')
            invalid = 1;

        if (!overlong && length < 255)
            line[length++] = (char)ch;
        else
            overlong = 1;
    }

    if (ferror(input))
        return errno ? errno : EIO;

    if ((length != 0 || overlong || invalid) && !overlong && !invalid) {
        status = rc_user_config_emit_line(
            output, line, length, forbid_list, is_forbidden);
        if (status != 0)
            return status;
    }

    if (fflush(output) != 0)
        return errno ? errno : EIO;

    return 0;
}

#endif
