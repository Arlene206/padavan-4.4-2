#ifndef NVRAM_CLI_POLICY_H
#define NVRAM_CLI_POLICY_H

#include <stddef.h>
#include <errno.h>
#include <string.h>

/*
 * NVRAM profile and show buffers can contain Wi-Fi, VPN and administrator
 * credentials. Volatile stores prevent the compiler from removing the wipe.
 */
static void
nvram_cli_secure_clear(void *buffer, size_t length)
{
    volatile unsigned char *cursor = (volatile unsigned char *)buffer;

    if (!buffer)
        return;
    while (length > 0) {
        *cursor++ = 0;
        --length;
    }
}

/* Copy and split a command-line name=value pair without truncation. */
static int
nvram_cli_parse_assignment(char *storage, size_t storage_size,
                            const char *argument,
                            size_t max_name_length,
                            size_t max_value_length,
                            char **name, char **value)
{
    size_t argument_length;
    size_t name_length;
    size_t value_length;
    const char *separator;

    if (name)
        *name = NULL;
    if (value)
        *value = NULL;

    if (!storage || storage_size == 0 || !argument || !name || !value)
        return -1;

    argument_length = strlen(argument);
    if (argument_length == 0 || argument_length >= storage_size)
        return -1;

    separator = (const char *)memchr(argument, '=', argument_length);
    if (!separator || separator == argument)
        return -1;

    name_length = (size_t)(separator - argument);
    value_length = argument_length - name_length - 1;
    if (name_length > max_name_length || value_length > max_value_length)
        return -1;

    memcpy(storage, argument, argument_length + 1);
    storage[name_length] = '\0';
    *name = storage;
    *value = storage + name_length + 1;
    return 0;
}

/* Map checked-get results without confusing backend errors with absence. */
static int
nvram_cli_get_error_status(int api_status, int saved_errno)
{
    if (api_status == 0 || api_status == 1)
        return 0;
    if (api_status < 0 && saved_errno > 0)
        return saved_errno;
    return EIO;
}

#endif /* NVRAM_CLI_POLICY_H */
