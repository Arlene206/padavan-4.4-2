#include <stdio.h>

#include "udhcpc_script_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)


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

int main(void)
{
	if (test_option_bitmap_mask_uses_unsigned_high_bit() != 0 ||
	    test_effective_lease_time_bounds_match_client_timer() != 0 ||
	    test_nak_only_deconfigures_an_active_lease() != 0 ||
	    test_nak_server_id_filter_respects_rebinding_broadcast() != 0 ||
	    test_arp_probe_errors_do_not_publish_a_dhcp_lease() != 0 ||
	    test_failed_deconfig_blocks_state_transition() != 0 ||
	    test_deconfig_retry_plan_resets_state_and_timer() != 0 ||
	    test_cleanup_retry_delay_has_one_second_floor() != 0 ||
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
