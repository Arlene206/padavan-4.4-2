#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/params.h>

#define HMAC_KEY_MAX 4096
#define INPUT_CHUNK 4096

static int
write_all(int fd, const unsigned char *buffer, size_t length)
{
    size_t offset = 0;

    while (offset < length) {
        ssize_t written = write(fd, buffer + offset, length - offset);

        if (written < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (written == 0)
            return -1;
        offset += (size_t)written;
    }
    return 0;
}

static void
report_failure(const char *message)
{
    (void)fprintf(stderr, "aliddns-hmac-sha1: %s\n", message);
}

int
main(void)
{
    unsigned char key[HMAC_KEY_MAX];
    unsigned char input[INPUT_CHUNK];
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned char encoded[EVP_MAX_MD_SIZE * 2 + 2];
    size_t key_length = 0;
    size_t digest_length = 0;
    int key_complete = 0;
    int encoded_length;
    ssize_t input_length;
    EVP_MAC *mac = NULL;
    EVP_MAC_CTX *mac_context = NULL;
    char digest_name[] = "SHA1";
    OSSL_PARAM parameters[] = {
        OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
                                         digest_name, 0),
        OSSL_PARAM_END
    };
    int result = 1;

    (void)signal(SIGPIPE, SIG_IGN);
    memset(key, 0, sizeof(key));
    memset(input, 0, sizeof(input));
    memset(digest, 0, sizeof(digest));
    memset(encoded, 0, sizeof(encoded));

    mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
    if (mac == NULL) {
        report_failure("cannot fetch HMAC implementation");
        goto out;
    }
    mac_context = EVP_MAC_CTX_new(mac);
    if (mac_context == NULL) {
        report_failure("cannot allocate HMAC context");
        goto out;
    }

    for (;;) {
        size_t offset = 0;

        input_length = read(STDIN_FILENO, input, sizeof(input));
        if (input_length < 0) {
            if (errno == EINTR)
                continue;
            report_failure("cannot read signing input");
            goto out;
        }
        if (input_length == 0)
            break;

        if (!key_complete) {
            while (offset < (size_t)input_length && input[offset] != '\0') {
                if (key_length >= sizeof(key)) {
                    report_failure("HMAC key exceeds the input limit");
                    goto out;
                }
                key[key_length++] = input[offset++];
            }

            if (offset < (size_t)input_length && input[offset] == '\0') {
                key_complete = 1;
                ++offset;
                if (EVP_MAC_init(mac_context, key, key_length,
                                 parameters) != 1) {
                    report_failure("cannot initialize HMAC-SHA1");
                    goto out;
                }
            }
        }

        if (key_complete && offset < (size_t)input_length &&
            EVP_MAC_update(mac_context, input + offset,
                           (size_t)input_length - offset) != 1) {
            report_failure("cannot update HMAC-SHA1");
            goto out;
        }
    }

    if (!key_complete) {
        report_failure("missing HMAC key delimiter");
        goto out;
    }
    if (EVP_MAC_final(mac_context, digest, &digest_length,
                      sizeof(digest)) != 1 || digest_length != 20) {
        report_failure("cannot finalize HMAC-SHA1");
        goto out;
    }

    encoded_length = EVP_EncodeBlock(encoded, digest, (int)digest_length);
    if (encoded_length <= 0 ||
        write_all(STDOUT_FILENO, encoded, (size_t)encoded_length) != 0 ||
        write_all(STDOUT_FILENO, (const unsigned char *)"\n", 1) != 0) {
        report_failure("cannot write HMAC result");
        goto out;
    }

    result = 0;

out:
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(input, sizeof(input));
    OPENSSL_cleanse(digest, sizeof(digest));
    OPENSSL_cleanse(encoded, sizeof(encoded));
    EVP_MAC_CTX_free(mac_context);
    EVP_MAC_free(mac);
    return result;
}
