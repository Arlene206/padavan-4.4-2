#ifndef HTTPD_BASIC_AUTH_H
#define HTTPD_BASIC_AUTH_H

#include <string.h>
#include <strings.h>

/* auth_basic_data is bounded by MAX_AUTH_LEN (128 bytes) in httpd.c. */
#define HTTPD_BASIC_AUTH_MAX_ENCODED_LEN 172

extern int b64_decode_strict(const char *str, unsigned char *space, int size);

/*
 * A request may carry at most one Authorization field. Repeated fields are
 * ambiguous across HTTP parsers, so invalidate the credential rather than
 * letting the last header silently replace the first.
 */
struct httpd_basic_auth_header {
	const char *value;
	int seen;
	int duplicate;
};

static int
httpd_basic_auth_header_record(struct httpd_basic_auth_header *header,
			       const char *value)
{
	if (!header || !value)
		return 0;
	if (header->seen) {
		header->duplicate = 1;
		header->value = NULL;
		return 0;
	}
	header->seen = 1;
	header->value = value;
	return 1;
}


/*
 * Decode one HTTP Basic Authorization field. The caller's header parser
 * removes leading field whitespace; this helper accepts the RFC case-
 * insensitive scheme, one or more SP separators, and normal trailing OWS
 * plus the header line ending. The Base64 token itself must be canonical.
 */
static int
httpd_decode_basic_authorization(const char *authorization, char *decoded,
				 int decoded_size)
{
	char encoded[HTTPD_BASIC_AUTH_MAX_ENCODED_LEN + 1];
	const char *value, *end;
	size_t encoded_len;
	int decoded_len;

	if (!decoded || decoded_size < 1)
		return -1;
	decoded[0] = '\0';
	if (!authorization || decoded_size < 2)
		return -1;

	if (strncasecmp(authorization, "Basic", 5) != 0 ||
	    authorization[5] != ' ')
		return -1;

	value = authorization + 5;
	while (*value == ' ')
		value++;
	end = value + strlen(value);

	/* Strip only the terminating HTTP line ending, then optional OWS. */
	if (end > value && end[-1] == '\n') {
		end--;
		if (end > value && end[-1] == '\r')
			end--;
	} else if (end > value && end[-1] == '\r') {
		return -1;
	}
	while (end > value && (end[-1] == ' ' || end[-1] == '\t'))
		end--;

	encoded_len = (size_t)(end - value);
	if (encoded_len == 0 ||
	    encoded_len > HTTPD_BASIC_AUTH_MAX_ENCODED_LEN)
		return -1;

	memcpy(encoded, value, encoded_len);
	encoded[encoded_len] = '\0';

	/* Reserve one byte for NUL and reject binary-NUL credential ambiguity. */
	decoded_len = b64_decode_strict(encoded, (unsigned char *)decoded,
					decoded_size - 1);
	if (decoded_len <= 0 || decoded_len >= decoded_size ||
	    memchr(decoded, '\0', (size_t)decoded_len) != NULL ||
	    memchr(decoded, ':', (size_t)decoded_len) == NULL) {
		decoded[0] = '\0';
		return -1;
	}

	decoded[decoded_len] = '\0';
	return decoded_len;
}

#endif /* HTTPD_BASIC_AUTH_H */
