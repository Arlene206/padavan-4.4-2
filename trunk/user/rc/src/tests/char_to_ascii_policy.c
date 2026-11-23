#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "char_to_ascii_policy.h"

int
main(void)
{
	char output[RC_CHAR_TO_ASCII_MAX_OUTPUT_SIZE];
	char saved[sizeof(output)];
	char short_output[5] = "keep";
	char short_saved[sizeof(short_output)];
	unsigned char utf8[] = { 0xc3, 0xa9, 0 };
	unsigned char maximum[RC_WLAN_SSID_MAX_LEN + 1];
	unsigned char oversized[RC_WLAN_SSID_MAX_LEN + 2];
	unsigned int index;

	memset(output, 0xa5, sizeof(output));
	assert(rc_char_to_ascii_encode(output, sizeof(output), "AZaz09!*()_-'." ) == 0);
	assert(strcmp(output, "AZaz09!*()_-'.") == 0);
	assert(rc_char_to_ascii_encode(output, sizeof(output), "ssid with%25") == 0);
	assert(strcmp(output, "ssid%20with%2525") == 0);
	assert(rc_char_to_ascii_encode(output, sizeof(output), (const char *)utf8) == 0);
	assert(strcmp(output, "%C3%A9") == 0);

	memset(maximum, 0x80, sizeof(maximum) - 1);
	maximum[sizeof(maximum) - 1] = 0;
	memset(output, 0xa5, sizeof(output));
	memcpy(saved, output, sizeof(saved));
	errno = 0;
	assert(rc_char_to_ascii_encode(output, sizeof(output) - 1, (const char *)maximum) == -1);
	assert(errno == ENOSPC);
	assert(memcmp(output, saved, sizeof(saved)) == 0);

	assert(rc_char_to_ascii_encode(output, sizeof(output), (const char *)maximum) == 0);
	assert(strlen(output) == RC_WLAN_SSID_MAX_LEN * 3U);
	for (index = 0; index < RC_WLAN_SSID_MAX_LEN; index++)
		assert(memcmp(output + index * 3U, "%80", 3) == 0);
	assert(output[sizeof(output) - 1] == '\0');

	memset(oversized, 0x80, sizeof(oversized) - 1);
	oversized[sizeof(oversized) - 1] = 0;
	strcpy(output, "unchanged");
	memcpy(saved, output, sizeof(saved));
	errno = 0;
	assert(rc_char_to_ascii_encode(output, sizeof(output), (const char *)oversized) == -1);
	assert(errno == EMSGSIZE);
	assert(memcmp(output, saved, sizeof(saved)) == 0);

	memcpy(short_saved, short_output, sizeof(short_saved));
	errno = 0;
	assert(rc_char_to_ascii_encode(short_output, sizeof(short_output), "A B") == -1);
	assert(errno == ENOSPC);
	assert(memcmp(short_output, short_saved, sizeof(short_saved)) == 0);

	strcpy(output, "unchanged");
	memcpy(saved, output, sizeof(saved));
	errno = 0;
	assert(rc_char_to_ascii_encode(output, sizeof(output), NULL) == -1);
	assert(errno == EINVAL);
	assert(memcmp(output, saved, sizeof(saved)) == 0);
	errno = 0;
	assert(rc_char_to_ascii_encode(NULL, sizeof(output), "x") == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(rc_char_to_ascii_encode(output, 0, "x") == -1);
	assert(errno == EINVAL);

	assert(rc_char_to_ascii_encode(output, sizeof(output), "") == 0);
	assert(output[0] == '\0');

	puts("Bounded WLAN SSID percent-encoding regression: PASS");
	return 0;
}
