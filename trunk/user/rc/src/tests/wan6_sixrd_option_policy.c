#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../wan6_sixrd_option_policy.h"

static int
is_ipv4_literal(const char *address)
{
	struct in_addr parsed;

	return address && inet_pton(AF_INET, address, &parsed) == 1;
}

static void
test_single_relay(void)
{
	char first[WAN6_SIXRD_IPV4_TEXT_SIZE];

	assert(wan6_sixrd_relay_list_first_valid(
		"192.0.2.1", first, sizeof(first), is_ipv4_literal));
	assert(strcmp(first, "192.0.2.1") == 0);
}

static void
test_multiple_relays_select_first_and_validate_all(void)
{
	char first[WAN6_SIXRD_IPV4_TEXT_SIZE];

	assert(wan6_sixrd_relay_list_first_valid(
		"  192.0.2.1\t198.51.100.2 203.0.113.3  ",
		first, sizeof(first), is_ipv4_literal));
	assert(strcmp(first, "192.0.2.1") == 0);

	strcpy(first, "stale");
	assert(!wan6_sixrd_relay_list_first_valid(
		"192.0.2.1 999.1.1.1", first, sizeof(first),
		is_ipv4_literal));
	assert(first[0] == '\0');
}

static void
test_reject_empty_and_truncated_tokens(void)
{
	char first[WAN6_SIXRD_IPV4_TEXT_SIZE];
	char too_small[8];

	assert(!wan6_sixrd_relay_list_first_valid(
		"", first, sizeof(first), is_ipv4_literal));
	assert(!wan6_sixrd_relay_list_first_valid(
		" \t\r\n", first, sizeof(first), is_ipv4_literal));
	assert(!wan6_sixrd_relay_list_first_valid(
		"192.0.2.1", too_small, sizeof(too_small), is_ipv4_literal));
	assert(too_small[0] == '\0');
	assert(!wan6_sixrd_relay_list_first_valid(
		"192.0.2.1 198.51.100.256", first, sizeof(first),
		is_ipv4_literal));
	assert(!wan6_sixrd_relay_list_first_valid(
		"192.0.2.1 1234567890123456", first, sizeof(first),
		is_ipv4_literal));
}

static void
test_reject_legacy_ipv4_aliases(void)
{
	char first[WAN6_SIXRD_IPV4_TEXT_SIZE];

	assert(!wan6_sixrd_relay_list_first_valid(
		"127.1", first, sizeof(first), is_ipv4_literal));
	assert(first[0] == '\0');
	assert(!wan6_sixrd_relay_list_first_valid(
		"0xC0000201", first, sizeof(first), is_ipv4_literal));
	assert(first[0] == '\0');
	assert(!wan6_sixrd_relay_list_first_valid(
		"192.0.2.1 127.1", first, sizeof(first), is_ipv4_literal));
	assert(first[0] == '\0');
}

static void
test_accept_maximum_relay_text(void)
{
	char relay_list[WAN6_SIXRD_OPTION_VALUE_MAX];
	char first[WAN6_SIXRD_IPV4_TEXT_SIZE];
	size_t used = 0;
	unsigned int i;

	relay_list[0] = '\0';
	for (i = 0; i < 59U; ++i) {
		int written = snprintf(relay_list + used,
				       sizeof(relay_list) - used,
				       "%s198.198.198.198", i ? " " : "");

		assert(written > 0);
		assert((size_t)written < sizeof(relay_list) - used);
		used += (size_t)written;
	}
	assert(used < sizeof(relay_list));
	assert(wan6_sixrd_relay_list_first_valid(
		relay_list, first, sizeof(first), is_ipv4_literal));
	assert(strcmp(first, "198.198.198.198") == 0);
}


static void
test_reject_relay_count_above_dhcp_limit(void)
{
	char relay_list[WAN6_SIXRD_OPTION_VALUE_MAX];
	char first[WAN6_SIXRD_IPV4_TEXT_SIZE];
	size_t used = 0;
	unsigned int i;

	relay_list[0] = '\0';
	for (i = 0; i < WAN6_SIXRD_MAX_RELAY_COUNT + 1U; ++i) {
		int written = snprintf(relay_list + used,
				       sizeof(relay_list) - used,
				       "%s198.198.198.198", i ? " " : "");

		assert(written > 0);
		assert((size_t)written < sizeof(relay_list) - used);
		used += (size_t)written;
	}
	strcpy(first, "stale");
	assert(!wan6_sixrd_relay_list_first_valid(
		relay_list, first, sizeof(first), is_ipv4_literal));
	assert(first[0] == '\0');
}

int
main(void)
{
	test_single_relay();
	test_multiple_relays_select_first_and_validate_all();
	test_reject_empty_and_truncated_tokens();
	test_reject_legacy_ipv4_aliases();
	test_accept_maximum_relay_text();
	test_reject_relay_count_above_dhcp_limit();
	puts("WAN 6rd multi-border-relay option regression: PASS");
	return 0;
}
