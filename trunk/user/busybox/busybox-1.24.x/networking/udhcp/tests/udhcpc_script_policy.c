#include <stdio.h>

#include "udhcpc_script_policy.h"
#include "option_parser_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)


static int test_subnet_mask_requires_contiguous_network_bits(void)
{
	unsigned int prefix = 99;

	CHECK(udhcpc_subnet_mask_prefix_length(0x00000000U, &prefix) == 0);
	CHECK(prefix == 0U);
	CHECK(udhcpc_subnet_mask_prefix_length(0x80000000U, &prefix) == 0);
	CHECK(prefix == 1U);
	CHECK(udhcpc_subnet_mask_prefix_length(0xffffff00U, &prefix) == 0);
	CHECK(prefix == 24U);
	CHECK(udhcpc_subnet_mask_prefix_length(0xffffffffU, &prefix) == 0);
	CHECK(prefix == 32U);

	prefix = 99;
	CHECK(udhcpc_subnet_mask_prefix_length(0xff00ff00U, &prefix) == -1);
	CHECK(prefix == 0U);
	CHECK(udhcpc_subnet_mask_prefix_length(0xffff0001U, &prefix) == -1);
	CHECK(udhcpc_subnet_mask_prefix_length(0x7fffffffU, &prefix) == -1);
	CHECK(udhcpc_subnet_mask_prefix_length(0xffffff00U, NULL) == -1);
	return 0;
}

static int test_option_bitmap_mask_uses_unsigned_high_bit(void)
{
	CHECK(udhcpc_option_bitmap_mask(0U) == 1U);
	CHECK(udhcpc_option_bitmap_mask(30U) == 0x40000000U);
	CHECK(udhcpc_option_bitmap_mask(31U) == 0x80000000U);
	CHECK(udhcpc_option_bitmap_mask(32U) == 1U);
	CHECK(udhcpc_option_bitmap_mask(63U) == 0x80000000U);
	CHECK(udhcpc_option_bitmap_mask(255U) == 0x80000000U);
	return 0;
}

static int test_fixed_route_options_reject_partial_tuples(void)
{
	static const uint8_t router_list[] = {
		192, 0, 2, 1, 198, 51, 100, 1
	};
	static const uint8_t legacy_route_list[] = {
		192, 0, 2, 0, 198, 51, 100, 1,
		198, 51, 100, 0, 192, 0, 2, 1
	};
	static const uint8_t partial_router[] = {
		192, 0, 2, 1, 198
	};
	static const uint8_t partial_legacy_route[] = {
		192, 0, 2, 0, 198, 51, 100, 1, 192
	};

	CHECK(udhcpc_fixed_route_list_option_is_well_formed(NULL, 0U, 4U));
	CHECK(udhcpc_fixed_route_list_option_is_well_formed(router_list,
							 sizeof(router_list), 4U));
	CHECK(!udhcpc_fixed_route_list_option_is_well_formed(partial_router,
							  sizeof(partial_router), 4U));
	CHECK(udhcpc_fixed_route_list_option_is_well_formed(NULL, 0U, 8U));
	CHECK(udhcpc_fixed_route_list_option_is_well_formed(legacy_route_list,
							 sizeof(legacy_route_list), 8U));
	CHECK(!udhcpc_fixed_route_list_option_is_well_formed(
			partial_legacy_route, sizeof(partial_legacy_route), 8U));
	CHECK(!udhcpc_fixed_route_list_option_is_well_formed(router_list,
							 sizeof(router_list), 0U));
	CHECK(!udhcpc_fixed_route_list_option_is_well_formed(NULL, 1U, 4U));
	return 0;
}


static int test_static_routes_reject_malformed_trailing_tuples(void)
{
	static const uint8_t default_route[] = {
		0, 198, 51, 100, 1
	};
	static const uint8_t one_bit_route[] = {
		1, 128, 198, 51, 100, 1
	};
	static const uint8_t twenty_four_bit_route[] = {
		24, 192, 0, 2, 198, 51, 100, 1
	};
	static const uint8_t thirty_two_bit_route[] = {
		32, 192, 0, 2, 1, 198, 51, 100, 1
	};
	static const uint8_t multiple_routes[] = {
		0, 198, 51, 100, 1,
		24, 192, 0, 2, 198, 51, 100, 2
	};
	static const uint8_t invalid_mask[] = {
		33, 198, 51, 100, 1
	};
	static const uint8_t truncated_prefix[] = {
		25, 192, 0, 2, 198, 51, 100, 1
	};
	static const uint8_t truncated_gateway[] = {
		24, 192, 0, 2, 198, 51, 100
	};
	static const uint8_t valid_then_partial[] = {
		0, 198, 51, 100, 1, 24
	};
	static const uint8_t valid_then_invalid[] = {
		0, 198, 51, 100, 1, 33, 198, 51, 100, 2
	};

	CHECK(udhcpc_static_routes_option_is_well_formed(NULL, 0U));
	CHECK(!udhcpc_static_routes_option_is_well_formed(NULL, 1U));
	CHECK(udhcpc_static_routes_option_is_well_formed(default_route,
							 sizeof(default_route)));
	CHECK(udhcpc_static_routes_option_is_well_formed(one_bit_route,
							 sizeof(one_bit_route)));
	CHECK(udhcpc_static_routes_option_is_well_formed(twenty_four_bit_route,
							 sizeof(twenty_four_bit_route)));
	CHECK(udhcpc_static_routes_option_is_well_formed(thirty_two_bit_route,
							 sizeof(thirty_two_bit_route)));
	CHECK(udhcpc_static_routes_option_is_well_formed(multiple_routes,
							 sizeof(multiple_routes)));
	CHECK(!udhcpc_static_routes_option_is_well_formed(invalid_mask,
						  sizeof(invalid_mask)));
	CHECK(!udhcpc_static_routes_option_is_well_formed(truncated_prefix,
						  sizeof(truncated_prefix)));
	CHECK(!udhcpc_static_routes_option_is_well_formed(truncated_gateway,
						  sizeof(truncated_gateway)));
	CHECK(!udhcpc_static_routes_option_is_well_formed(valid_then_partial,
						  sizeof(valid_then_partial)));
	CHECK(!udhcpc_static_routes_option_is_well_formed(valid_then_invalid,
						  sizeof(valid_then_invalid)));
	return 0;
}


static int test_6rd_option_rejects_invalid_prefixes_and_partial_br_addresses(void)
{
	static const uint8_t valid_one_relay[] = {
		8, 32,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		192, 0, 2, 1
	};
	static const uint8_t valid_two_relays[] = {
		8, 32,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		192, 0, 2, 1, 198, 51, 100, 1
	};
	static const uint8_t too_short[21] = { 8, 32 };
	static const uint8_t partial_one[23] = { 8, 32 };
	static const uint8_t partial_two[24] = { 8, 32 };
	static const uint8_t partial_three[25] = { 8, 32 };
	static const uint8_t invalid_ipv4_mask[22] = { 33, 32 };
	static const uint8_t invalid_prefix_length[22] = { 32, 129 };
	static const uint8_t invalid_combined_prefix[22] = { 0, 97 };

	CHECK(udhcpc_6rd_option_is_well_formed(valid_one_relay,
					      sizeof(valid_one_relay)));
	CHECK(udhcpc_6rd_option_is_well_formed(valid_two_relays,
					      sizeof(valid_two_relays)));
	CHECK(!udhcpc_6rd_option_is_well_formed(NULL, 22U));
	CHECK(!udhcpc_6rd_option_is_well_formed(too_short,
					       sizeof(too_short)));
	CHECK(!udhcpc_6rd_option_is_well_formed(partial_one,
					       sizeof(partial_one)));
	CHECK(!udhcpc_6rd_option_is_well_formed(partial_two,
					       sizeof(partial_two)));
	CHECK(!udhcpc_6rd_option_is_well_formed(partial_three,
					       sizeof(partial_three)));
	CHECK(!udhcpc_6rd_option_is_well_formed(invalid_ipv4_mask,
					       sizeof(invalid_ipv4_mask)));
	CHECK(!udhcpc_6rd_option_is_well_formed(invalid_prefix_length,
					       sizeof(invalid_prefix_length)));
	CHECK(!udhcpc_6rd_option_is_well_formed(invalid_combined_prefix,
					       sizeof(invalid_combined_prefix)));
	return 0;
}


static int test_effective_lease_time_bounds_match_client_timer(void)
{
	CHECK(udhcpc_lease_time_effective(0U, 0) == 3600U);
	CHECK(udhcpc_lease_time_effective(0U, 1) == 16U);
	CHECK(udhcpc_lease_time_effective(15U, 1) == 16U);
	CHECK(udhcpc_lease_time_effective(16U, 1) == 16U);
	CHECK(udhcpc_lease_time_effective(3600U, 1) == 3600U);
	CHECK(udhcpc_lease_time_effective(0x0fffffffU, 1) == 0x0fffffffU);
	CHECK(udhcpc_lease_time_effective(0x10000000U, 1) == 0x0fffffffU);
	CHECK(udhcpc_lease_time_effective(0xffffffffU, 1) == 0x0fffffffU);
	return 0;
}

static int test_nak_only_deconfigures_an_active_lease(void)
{
	CHECK(!udhcpc_nak_requires_deconfig(1));
	CHECK(udhcpc_nak_requires_deconfig(0));
	return 0;
}


static int test_ack_server_id_filter_respects_selected_server(void)
{
	uint32_t selected_server = 0x01020304U;
	uint32_t alternate_server = 0x05060708U;

	/* REQUESTING and RENEWING stay bound to the selected server. */
	CHECK(!udhcpc_server_id_rejected(0, selected_server, 1, selected_server));
	CHECK(udhcpc_server_id_rejected(0, selected_server, 0, 0));
	CHECK(udhcpc_server_id_rejected(0, selected_server, 1, alternate_server));

	/* Preserve address-less DHCP servers when the offer had no server ID. */
	CHECK(!udhcpc_server_id_rejected(0, 0, 0, 0));
	CHECK(!udhcpc_server_id_rejected(0, 0, 1, alternate_server));

	/* REBINDING is broadcast: accept another identified server's ACK only. */
	CHECK(!udhcpc_server_id_rejected(1, selected_server, 1, alternate_server));
	CHECK(udhcpc_server_id_rejected(1, selected_server, 0, 0));
	return 0;
}

static int test_nak_server_id_filter_respects_rebinding_broadcast(void)
{
	uint32_t original_server = 0x01020304U;
	uint32_t alternate_server = 0x05060708U;

	/* Preserve address-less server compatibility before a server is known. */
	CHECK(!udhcpc_nak_server_id_rejected(0, 0, 0, 0));
	CHECK(!udhcpc_nak_server_id_rejected(0, 0, 1, alternate_server));

	/* SELECTING/RENEWING replies stay pinned to the selected server. */
	CHECK(!udhcpc_nak_server_id_rejected(0, original_server, 1, original_server));
	CHECK(udhcpc_nak_server_id_rejected(0, original_server, 0, 0));
	CHECK(udhcpc_nak_server_id_rejected(0, original_server, 1, alternate_server));

	/* REBINDING broadcasts to every server: accept an alternate server's NAK. */
	CHECK(!udhcpc_nak_server_id_rejected(1, original_server, 1, alternate_server));
	CHECK(!udhcpc_nak_server_id_rejected(1, original_server, 1, original_server));
	CHECK(udhcpc_nak_server_id_rejected(1, original_server, 0, 0));
	return 0;
}

static int test_arp_probe_errors_do_not_publish_a_dhcp_lease(void)
{
	CHECK(udhcpc_arp_probe_failed(-1));
	CHECK(!udhcpc_arp_probe_failed(0));
	CHECK(!udhcpc_arp_probe_failed(1));
	return 0;
}

static int test_failed_deconfig_blocks_state_transition(void)
{
	CHECK(!udhcpc_deconfig_retry_required(0));
	CHECK(udhcpc_deconfig_retry_required(1));
	CHECK(udhcpc_deconfig_retry_required(-1));
	CHECK(udhcpc_deconfig_retry_required(0x100));
	return 0;
}



static int test_deconfig_retry_plan_resets_state_and_timer(void)
{
	struct udhcpc_deconfig_retry_plan plan;

	udhcpc_deconfig_retry_plan_init(&plan, 4, 1, 30);
	CHECK(plan.resume_state == 4);
	CHECK(plan.release_on_usr2 == 1);
	CHECK(plan.timeout == 30);
	CHECK(plan.packet_num == 0);
	CHECK(plan.already_waited_sec == 0);

	udhcpc_deconfig_retry_plan_init(&plan, 5, 0, 0);
	CHECK(plan.resume_state == 5);
	CHECK(plan.release_on_usr2 == 0);
	CHECK(plan.timeout == 1);
	CHECK(plan.packet_num == 0);
	CHECK(plan.already_waited_sec == 0);

	udhcpc_deconfig_retry_plan_init(&plan, 6, -1, -3);
	CHECK(plan.resume_state == 6);
	CHECK(plan.release_on_usr2 == 1);
	CHECK(plan.timeout == 1);
	CHECK(plan.packet_num == 0);
	CHECK(plan.already_waited_sec == 0);
	return 0;
}

static int test_cleanup_retry_delay_has_one_second_floor(void)
{
	CHECK(udhcpc_retry_delay(5) == 5);
	CHECK(udhcpc_retry_delay(1) == 1);
	CHECK(udhcpc_retry_delay(0) == 1);
	CHECK(udhcpc_retry_delay(-3) == 1);
	CHECK(udhcpc_decline_retry_delay(5) == 5);
	CHECK(udhcpc_decline_retry_delay(0) == 1);
	CHECK(udhcpc_decline_retry_delay(-3) == 1);
	return 0;
}


static int test_leasefail_and_decline_backoff_have_one_second_floor(void)
{
	CHECK(udhcpc_retry_delay(20) == 20);
	CHECK(udhcpc_retry_delay(0) == 1);
	CHECK(udhcpc_retry_delay(-1) == 1);
	CHECK(udhcpc_decline_retry_delay(20) == 20);
	CHECK(udhcpc_decline_retry_delay(0) == 1);
	CHECK(udhcpc_decline_retry_delay(-1) == 1);
	return 0;
}

static int test_failed_packet_send_blocks_dhcp_reselection_until_retry(void)
{
	CHECK(!udhcpc_packet_send_failed(0));
	CHECK(!udhcpc_packet_send_failed(548));
	CHECK(udhcpc_packet_send_failed(-1));
	CHECK(udhcpc_decline_retry_delay(5) == 5);
	CHECK(udhcpc_decline_retry_delay(0) == 1);
	CHECK(udhcpc_decline_retry_delay(-3) == 1);
	return 0;
}

static int test_pending_deconfig_manual_stop_releases_existing_lease(void)
{
	CHECK(udhcpc_deconfig_pending_release_required(1, 1));
	CHECK(!udhcpc_deconfig_pending_release_required(1, 0));
	CHECK(!udhcpc_deconfig_pending_release_required(0, 1));
	CHECK(!udhcpc_deconfig_pending_release_required(0, 0));
	return 0;
}

static int test_initial_acquisition_retries_selected_request(void)
{
	int timeout = 90;
	int packet_num = 3;
	unsigned already_waited_sec = 12;

	udhcpc_script_failure_prepare_retry(1, 4, 20, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 4);
	CHECK(already_waited_sec == 0);
	CHECK(packet_num == 0);
	return 0;
}

static int test_nonpositive_retry_interval_is_throttled(void)
{
	int timeout = 90;
	int packet_num = 3;
	unsigned already_waited_sec = 12;

	udhcpc_script_failure_prepare_retry(1, 0, 0, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 1);
	CHECK(already_waited_sec == 0);
	CHECK(packet_num == 0);

	timeout = 90;
	packet_num = 3;
	already_waited_sec = 12;
	udhcpc_script_failure_prepare_retry(1, -5, 0, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 1);
	CHECK(already_waited_sec == 0);
	CHECK(packet_num == 0);
	return 0;
}

static int test_failed_renewal_preserves_existing_lease_deadline(void)
{
	int timeout = 1800;
	int packet_num = 2;
	unsigned already_waited_sec = 300;

	udhcpc_script_failure_prepare_retry(0, 20, 25, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 1800);
	CHECK(already_waited_sec == 325);
	CHECK(packet_num == 2);
	return 0;
}

static int test_script_runtime_cannot_extend_expired_lease(void)
{
	int timeout = 1800;
	int packet_num = 1;
	unsigned already_waited_sec = 1790;

	udhcpc_script_failure_prepare_retry(0, 20, 15, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 0);
	CHECK(already_waited_sec == 0);
	CHECK(packet_num == 1);
	return 0;
}

static int test_exact_lease_deadline_is_expired(void)
{
	int timeout = 30;
	int packet_num = 0;
	unsigned already_waited_sec = 20;

	udhcpc_script_failure_prepare_retry(0, 20, 10, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 0);
	CHECK(already_waited_sec == 0);
	return 0;
}

static int test_already_expired_lease_is_not_resurrected(void)
{
	int timeout = 5;
	int packet_num = 0;
	unsigned already_waited_sec = 6;

	udhcpc_script_failure_prepare_retry(0, 20, 0, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 0);
	CHECK(already_waited_sec == 0);
	return 0;
}


static int test_dhcp_option_tlv_bounds(void)
{
	static const uint8_t one_byte_header[] = { 0x35U };
	static const uint8_t truncated_payload[] = { 0x35U, 2U, 1U };
	static const uint8_t exact_payload[] = { 0x35U, 2U, 1U, 2U };
	static const uint8_t maximum_payload[257] = { 0x35U, 255U };
	unsigned int total_size;

	total_size = 77U;
	CHECK(!udhcp_option_tlv_size(NULL, 2U, &total_size));
	CHECK(total_size == 77U);
	CHECK(!udhcp_option_tlv_size(one_byte_header, 1U, &total_size));
	CHECK(total_size == 77U);
	CHECK(!udhcp_option_tlv_size(truncated_payload, 3U, &total_size));
	CHECK(total_size == 77U);
	CHECK(udhcp_option_tlv_size(exact_payload, 4U, &total_size));
	CHECK(total_size == 4U);
	CHECK(udhcp_option_tlv_size(maximum_payload, 257U, &total_size));
	CHECK(total_size == 257U);
	CHECK(!udhcp_option_tlv_size(maximum_payload, 256U, &total_size));
	CHECK(total_size == 257U);
	CHECK(!udhcp_option_tlv_size(exact_payload, 2U, &total_size));
	return 0;
}

static int test_dhcp_option_overload_is_a_validated_single_byte(void)
{
	static const uint8_t zero_value[] = { 0x34U, 1U, 0U };
	static const uint8_t valid_file[] = { 0x34U, 1U, 1U };
	static const uint8_t valid_sname[] = { 0x34U, 1U, 2U };
	static const uint8_t valid_both[] = { 0x34U, 1U, 3U };
	static const uint8_t reserved_bits[] = { 0x34U, 1U, 0x81U };
	static const uint8_t empty[] = { 0x34U, 0U, 0U };
	static const uint8_t overlong[] = { 0x34U, 2U, 3U, 0U };
	static const uint8_t wrong_code[] = { 0x35U, 1U, 1U };
	static const uint8_t short_header[] = { 0x34U };
	uint8_t fields = 0xa5U;

	CHECK(udhcp_option_overload_fields(valid_file, 3U, &fields));
	CHECK(fields == 1U);
	CHECK(udhcp_option_overload_fields(valid_sname, 3U, &fields));
	CHECK(fields == 2U);
	CHECK(udhcp_option_overload_fields(valid_both, 3U, &fields));
	CHECK(fields == 3U);

	fields = 0xa5U;
	CHECK(!udhcp_option_overload_fields(NULL, 3U, &fields));
	CHECK(fields == 0xa5U);
	CHECK(!udhcp_option_overload_fields(short_header, 1U, &fields));
	CHECK(fields == 0xa5U);
	CHECK(!udhcp_option_overload_fields(empty, 3U, &fields));
	CHECK(fields == 0xa5U);
	CHECK(!udhcp_option_overload_fields(overlong, 4U, &fields));
	CHECK(fields == 0xa5U);
	CHECK(!udhcp_option_overload_fields(wrong_code, 3U, &fields));
	CHECK(fields == 0xa5U);
	CHECK(!udhcp_option_overload_fields(reserved_bits, 3U, &fields));
	CHECK(fields == 0xa5U);
	CHECK(!udhcp_option_overload_fields(zero_value, 3U, &fields));
	CHECK(fields == 0xa5U);
	CHECK(!udhcp_option_overload_fields(valid_both, 2U, &fields));
	CHECK(fields == 0xa5U);
	return 0;
}

static int test_dhcp_option_overload_is_unique_to_main_field(void)
{
	CHECK(udhcp_option_overload_location_valid(0U, 0U));
	CHECK(!udhcp_option_overload_location_valid(1U, 0U));
	CHECK(!udhcp_option_overload_location_valid(0U, 1U));
	CHECK(!udhcp_option_overload_location_valid(1U, 1U));
	CHECK(!udhcp_option_overload_location_valid(2U, 0U));
	return 0;
}

static int
test_select_wait_clamps_elapsed_timeout_before_unsigned_subtraction(void)
{
	CHECK(udhcpc_wait_remaining_seconds(30, 0U) == 30U);
	CHECK(udhcpc_wait_remaining_seconds(30, 29U) == 1U);
	CHECK(udhcpc_wait_remaining_seconds(30, 30U) == 0U);
	CHECK(udhcpc_wait_remaining_seconds(30, 31U) == 0U);
	CHECK(udhcpc_wait_remaining_seconds(30, 0xffffffffU) == 0U);
	CHECK(udhcpc_wait_remaining_seconds(0, 0U) == 0U);
	CHECK(udhcpc_wait_remaining_seconds(-1, 0U) == 0U);
	CHECK(udhcpc_wait_remaining_seconds(2147483647, 2147483646U) == 1U);
	CHECK(udhcpc_wait_remaining_seconds(2147483647, 2147483647U) == 0U);
	return 0;
}

int main(void)
{
	if (test_select_wait_clamps_elapsed_timeout_before_unsigned_subtraction() != 0 ||
	    test_dhcp_option_tlv_bounds() != 0 ||
	    test_dhcp_option_overload_is_a_validated_single_byte() != 0 ||
	    test_dhcp_option_overload_is_unique_to_main_field() != 0 ||
	    test_fixed_route_options_reject_partial_tuples() != 0 ||
	    test_static_routes_reject_malformed_trailing_tuples() != 0 ||
	    test_6rd_option_rejects_invalid_prefixes_and_partial_br_addresses() != 0 ||
	    test_subnet_mask_requires_contiguous_network_bits() != 0 ||
	    test_option_bitmap_mask_uses_unsigned_high_bit() != 0 ||
	    test_effective_lease_time_bounds_match_client_timer() != 0 ||
	    test_nak_only_deconfigures_an_active_lease() != 0 ||
	    test_ack_server_id_filter_respects_selected_server() != 0 ||
	    test_nak_server_id_filter_respects_rebinding_broadcast() != 0 ||
	    test_arp_probe_errors_do_not_publish_a_dhcp_lease() != 0 ||
	    test_failed_deconfig_blocks_state_transition() != 0 ||
	    test_deconfig_retry_plan_resets_state_and_timer() != 0 ||
	    test_cleanup_retry_delay_has_one_second_floor() != 0 ||
	    test_leasefail_and_decline_backoff_have_one_second_floor() != 0 ||
	    test_failed_packet_send_blocks_dhcp_reselection_until_retry() != 0 ||
	    test_pending_deconfig_manual_stop_releases_existing_lease() != 0 ||
	    test_initial_acquisition_retries_selected_request() != 0 ||
	    test_nonpositive_retry_interval_is_throttled() != 0 ||
	    test_failed_renewal_preserves_existing_lease_deadline() != 0 ||
	    test_script_runtime_cannot_extend_expired_lease() != 0 ||
	    test_exact_lease_deadline_is_expired() != 0 ||
	    test_already_expired_lease_is_not_resurrected() != 0)
		return 1;

	puts("udhcpc script failure and deconfig retry policy: PASS");
	return 0;
}
