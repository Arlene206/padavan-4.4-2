#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static ssize_t aliddns_hmac_test_read(int fd, void *buffer, size_t length);
static ssize_t aliddns_hmac_test_write(int fd, const void *buffer, size_t length);
#define ALIDDNS_HMAC_READ aliddns_hmac_test_read
#define ALIDDNS_HMAC_WRITE aliddns_hmac_test_write
#define main aliddns_hmac_helper_main
#include "../src/aliddns-hmac-sha1.c"
#undef main

static unsigned char input_data[256];
static size_t input_length;
static size_t input_offset;
static unsigned char output_data[256];
static size_t output_length;
static unsigned int read_calls;
static unsigned int write_calls;
static unsigned int read_eintr_remaining;
static unsigned int write_eintr_remaining;
static int read_always_eintr;
static int write_always_eintr;
static int write_zero;
static size_t read_chunk_limit;
static size_t write_chunk_limit;

static ssize_t
aliddns_hmac_test_read(int fd, void *buffer, size_t length)
{
    size_t amount;

    assert(fd == STDIN_FILENO);
    ++read_calls;
    if (read_always_eintr || read_eintr_remaining > 0U) {
        if (read_eintr_remaining > 0U)
            --read_eintr_remaining;
        errno = EINTR;
        return -1;
    }
    if (input_offset == input_length)
        return 0;
    amount = input_length - input_offset;
    if (amount > length)
        amount = length;
    if (read_chunk_limit > 0U && amount > read_chunk_limit)
        amount = read_chunk_limit;
    memcpy(buffer, input_data + input_offset, amount);
    input_offset += amount;
    return (ssize_t)amount;
}

static ssize_t
aliddns_hmac_test_write(int fd, const void *buffer, size_t length)
{
    size_t amount = length;

    assert(fd == STDOUT_FILENO);
    ++write_calls;
    if (write_always_eintr || write_eintr_remaining > 0U) {
        if (write_eintr_remaining > 0U)
            --write_eintr_remaining;
        errno = EINTR;
        return -1;
    }
    if (write_zero)
        return 0;
    if (write_chunk_limit > 0U && amount > write_chunk_limit)
        amount = write_chunk_limit;
    assert(output_length + amount <= sizeof(output_data));
    memcpy(output_data + output_length, buffer, amount);
    output_length += amount;
    return (ssize_t)amount;
}

static void
reset_io(const unsigned char *key, size_t key_length,
         const unsigned char *message, size_t message_length)
{
    assert(key_length + 1U + message_length <= sizeof(input_data));
    memcpy(input_data, key, key_length);
    input_data[key_length] = '\0';
    memcpy(input_data + key_length + 1U, message, message_length);
    input_length = key_length + 1U + message_length;
    input_offset = 0U;
    output_length = 0U;
    read_calls = 0U;
    write_calls = 0U;
    read_eintr_remaining = 0U;
    write_eintr_remaining = 0U;
    read_always_eintr = 0;
    write_always_eintr = 0;
    write_zero = 0;
    read_chunk_limit = 0U;
    write_chunk_limit = 0U;
}

int
main(void)
{
    unsigned char key[20];
    const unsigned char message[] = "Hi There";
    static const char expected[] = "thcxhlUFcmTii8C2+zeMjvFGvgA=\n";
    int status;

    memset(key, 11, sizeof(key));

    /* Intermittent EINTR, fragmented reads and partial writes still succeed. */
    reset_io(key, sizeof(key), message, sizeof(message) - 1U);
    read_eintr_remaining = 3U;
    write_eintr_remaining = 2U;
    read_chunk_limit = 5U;
    write_chunk_limit = 4U;
    status = aliddns_hmac_helper_main();
    assert(status == 0);
    assert(read_eintr_remaining == 0U && write_eintr_remaining == 0U);
    assert(read_calls > 5U && write_calls > 5U);
    assert(output_length == sizeof(expected) - 1U);
    assert(memcmp(output_data, expected, sizeof(expected) - 1U) == 0);

    /* Persistent read interruption must stop after the initial try + 8 retries. */
    reset_io(key, sizeof(key), message, sizeof(message) - 1U);
    read_always_eintr = 1;
    status = aliddns_hmac_helper_main();
    assert(status != 0);
    assert(read_calls == ALIDDNS_HMAC_MAX_EINTR_RETRIES + 1U);
    assert(write_calls == 0U && output_length == 0U);

    /* Persistent output interruption is bounded and never emits partial data. */
    reset_io(key, sizeof(key), message, sizeof(message) - 1U);
    write_always_eintr = 1;
    status = aliddns_hmac_helper_main();
    assert(status != 0);
    assert(write_calls == ALIDDNS_HMAC_MAX_EINTR_RETRIES + 1U);
    assert(output_length == 0U);

    /* A zero-length write is a hard I/O error rather than a no-progress loop. */
    reset_io(key, sizeof(key), message, sizeof(message) - 1U);
    write_zero = 1;
    status = aliddns_hmac_helper_main();
    assert(status != 0);
    assert(write_calls == 1U && output_length == 0U);

    puts("AliDDNS HMAC read/write EINTR budget and partial-I/O regression: PASS");
    return 0;
}
