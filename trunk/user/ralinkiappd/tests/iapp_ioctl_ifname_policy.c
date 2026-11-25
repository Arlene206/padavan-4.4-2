#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/iapp_ioctl_ifname_policy.h"
#include "../src/iapp_rx_length_policy.h"

typedef struct _FT_CLIENT_INFO {
	struct _FT_CLIENT_INFO *next;
	int used;
} FT_CLIENT_INFO;

#include "../src/ft_client_hash_policy.h"

static void
test_ft_client_hash_slot_reuse_severs_stale_links(void)
{
	FT_CLIENT_INFO entries[4] = { { 0 } };
	FT_CLIENT_INFO *first_bucket = NULL;
	FT_CLIENT_INFO *second_bucket = NULL;
	FT_CLIENT_INFO *slot;

	entries[0].used = 1;
	entries[1].used = 1;
	entries[2].used = 1;
	entries[0].next = &entries[1];
	entries[1].next = &entries[2];
	first_bucket = &entries[0];

	/* Removing a middle node must preserve the old bucket and clear next. */
	assert(ft_client_hash_unlink(&first_bucket, &entries[1], 4));
	assert(first_bucket == &entries[0]);
	assert(entries[0].next == &entries[2]);
	assert(entries[1].next == NULL);

	/* Reusing the slot in another bucket must not import the old tail. */
	entries[1].used = 0;
	entries[1].next = &entries[2];
	slot = ft_client_hash_find_free_slot(entries, 4);
	assert(slot == &entries[1]);
	assert(slot->next == NULL);
	assert(ft_client_hash_append(&second_bucket, slot, 4));
	assert(second_bucket == &entries[1]);
	assert(second_bucket->next == NULL);

	/* Head and tail removal must leave detached nodes unlinked. */
	assert(ft_client_hash_unlink(&second_bucket, &entries[1], 4));
	assert(second_bucket == NULL);
	assert(entries[1].next == NULL);
	assert(ft_client_hash_unlink(&first_bucket, &entries[0], 4));
	assert(first_bucket == &entries[2]);
	assert(entries[0].next == NULL);
	assert(ft_client_hash_unlink(&first_bucket, &entries[2], 4));
	assert(first_bucket == NULL);
	assert(entries[2].next == NULL);
}

static void
test_ft_client_hash_full_and_corrupt_chains_fail_closed(void)
{
	FT_CLIENT_INFO entries[2] = { { 0 } };
	FT_CLIENT_INFO *head;

	entries[0].used = 1;
	entries[1].used = 1;
	head = &entries[0];
	entries[0].next = &entries[1];
	assert(ft_client_hash_find_free_slot(entries, 2) == NULL);
	assert(!ft_client_hash_append(&head, &entries[0], 2));

	/* A pre-existing loop must be bounded rather than traversed forever. */
	entries[1].next = &entries[0];
	assert(!ft_client_hash_append(&head, &entries[1], 2));
	assert(!ft_client_hash_unlink(&head, NULL, 2));
	assert(!ft_client_hash_unlink(&head, &entries[1], 0));
}

static void
test_valid_two_interface_slots(void)
{
	char names[2][16] = { "rai0", "ra0" };
	const char *selected;

	selected = iapp_ioctl_ifname_select((const char *)names, 2, sizeof(names[0]),
						 2, 0, 12, "ra0");
	assert(strcmp(selected, "rai0") == 0);
	selected = iapp_ioctl_ifname_select((const char *)names, 2, sizeof(names[0]),
						 2, 1, 12, "fallback0");
	assert(strcmp(selected, "ra0") == 0);
}

static void
test_out_of_range_and_unconfigured_indices_fall_back(void)
{
	char names[2][16] = { "rai0", "ra0" };
	const char *fallback = "fallback0";

	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 2, 2, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 2, 255, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 1, 1, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 0, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, -1, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 2, -1, 12, fallback), fallback) == 0);
}

static void
test_malformed_slots_and_names_fall_back(void)
{
	char unterminated[2][16];
	char empty[2][16] = { "", "ra0" };
	char too_long[2][32] = { "interface-name-too-long", "ra0" };
	const char *fallback = "fallback0";

	memset(unterminated, 'x', sizeof(unterminated));
	assert(strcmp(iapp_ioctl_ifname_select((const char *)unterminated, 2, 16, 2, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)empty, 2, 16, 2, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)too_long, 2, sizeof(too_long[0]), 2, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select(NULL, 2, 16, 2, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)empty, 2, 0, 2, 0, 12, fallback), fallback) == 0);
}

static void
test_wifi_index_bounds(void)
{
	assert(iapp_wifi_index_valid(0, 2, 1));
	assert(iapp_wifi_index_valid(0, 2, 2));
	assert(iapp_wifi_index_valid(1, 2, 2));
	assert(!iapp_wifi_index_valid(1, 2, 1));
	assert(!iapp_wifi_index_valid(2, 2, 2));
	assert(!iapp_wifi_index_valid(255, 2, 2));
	assert(!iapp_wifi_index_valid(-1, 2, 2));
	assert(!iapp_wifi_index_valid(0, 2, 0));
	assert(!iapp_wifi_index_valid(0, 2, 3));
	assert(!iapp_wifi_index_valid(0, 2, -1));
}

static void
test_received_payload_bounds(void)
{
	assert(iapp_rx_payload_fits(16, 16, 0));
	assert(iapp_rx_payload_fits(32, 16, 16));
	assert(iapp_rx_payload_fits(33, 16, 17));
	assert(!iapp_rx_payload_fits(15, 16, 0));
	assert(!iapp_rx_payload_fits(31, 16, 16));
	assert(!iapp_rx_payload_fits(32, 16, 17));
	assert(!iapp_rx_payload_fits(SIZE_MAX, SIZE_MAX, 1));
	assert(iapp_rx_payload_fits(SIZE_MAX, SIZE_MAX, 0));

	/* A short Ethernet frame is zero-padded to the minimum wire length.
	 * RRB's 24-byte fixed prefix leaves 36 bytes in a 60-byte frame. */
	assert(iapp_rx_payload_fits(60, 24, 36));
	assert(!iapp_rx_payload_fits(60, 24, 37));
	assert(iapp_rx_payload_fits(64, 24, 40));
	assert(!iapp_rx_payload_fits(64, 24, 41));

	/* 32-bit target ABI: packed RT_SIGNAL_STRUC fixed prefix (28 bytes)
	 * plus the 8-byte FT event header, before the declared event body. */
	assert(iapp_rx_payload_fits(36, 36, 0));
	assert(iapp_rx_payload_fits(40, 36, 4));
	assert(!iapp_rx_payload_fits(39, 36, 4));
	assert(!iapp_rx_payload_fits(35, 36, 0));

	/* TCP read() may return a partial prefix; append bounds stay safe. */
	assert(iapp_rx_buffer_append_fits(8, 0, 3));
	assert(iapp_rx_buffer_append_fits(8, 3, 5));
	assert(!iapp_rx_buffer_append_fits(8, 3, 6));
	assert(!iapp_rx_buffer_append_fits(8, 9, 0));
	assert(!iapp_rx_buffer_append_fits(SIZE_MAX, SIZE_MAX, 1));

	assert(iapp_rx_frame_length_matches(6, 6));
	assert(!iapp_rx_frame_length_matches(5, 6));
	assert(!iapp_rx_frame_length_matches(7, 6));

	assert(iapp_rx_length_within_capacity(32, 0));
	assert(iapp_rx_length_within_capacity(32, 32));
	assert(!iapp_rx_length_within_capacity(32, 33));
	assert(!iapp_rx_length_within_capacity(32, -1));
	assert(!iapp_rx_length_within_capacity(0, 1));
}

static void
test_parser_rejects_count_overflow_and_invalid_names(void)
{
	assert(iapp_ioctl_ifname_arg_valid(0, 2, 4, 16));
	assert(iapp_ifname_arg_valid(15, 16));
	assert(!iapp_ifname_arg_valid(0, 16));
	assert(!iapp_ifname_arg_valid(16, 16));
	assert(iapp_ioctl_ifname_arg_valid(1, 2, 3, 16));
	assert(!iapp_ioctl_ifname_arg_valid(2, 2, 4, 16));
	assert(!iapp_ioctl_ifname_arg_valid(-1, 2, 4, 16));
	assert(!iapp_ioctl_ifname_arg_valid(0, 2, 0, 16));
	assert(!iapp_ioctl_ifname_arg_valid(0, 2, 16, 16));
}

int
main(void)
{
	test_ft_client_hash_slot_reuse_severs_stale_links();
	test_ft_client_hash_full_and_corrupt_chains_fail_closed();
	test_valid_two_interface_slots();
	test_out_of_range_and_unconfigured_indices_fall_back();
	test_malformed_slots_and_names_fall_back();
	test_wifi_index_bounds();
	test_received_payload_bounds();
	test_parser_rejects_count_overflow_and_invalid_names();
	puts("IAPP ioctl interface index and parser bounds policy: PASS");
	return 0;
}
