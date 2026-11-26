#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include "dnsmasq_static_host_policy.h"



static void
test_static_arp_header_parser(void)
{
	assert(dnsmasq_static_arp_header_is_valid(
		"IP address HW type Flags HW address Mask Device"));
	assert(dnsmasq_static_arp_header_is_valid(
		"IP address       HW type     Flags       HW address            Mask     Device   "));
	assert(!dnsmasq_static_arp_header_is_valid(""));
	assert(!dnsmasq_static_arp_header_is_valid(
		"IP address HW type Flags HW address Mask"));
	assert(!dnsmasq_static_arp_header_is_valid(
		"IP address HW type Flags HW address Mask Device extra"));
	assert(!dnsmasq_static_arp_header_is_valid(
		"IP address HW type Flags HW address Device Mask"));
	assert(!dnsmasq_static_arp_header_is_valid(NULL));
}

static void
test_static_arp_ipv4_text(void)
{
	assert(dnsmasq_static_arp_ipv4_is_valid("0.0.0.0"));
	assert(dnsmasq_static_arp_ipv4_is_valid("192.0.2.1"));
	assert(dnsmasq_static_arp_ipv4_is_valid("255.255.255.255"));
	assert(!dnsmasq_static_arp_ipv4_is_valid("010.0.0.1"));
	assert(!dnsmasq_static_arp_ipv4_is_valid("192.168.001.1"));
	assert(!dnsmasq_static_arp_ipv4_is_valid("192.168.1.01"));
	assert(!dnsmasq_static_arp_ipv4_is_valid("000.0.0.1"));
	assert(!dnsmasq_static_arp_ipv4_is_valid("192.0.2.256"));
	assert(!dnsmasq_static_arp_ipv4_is_valid("192.0.2"));
	assert(!dnsmasq_static_arp_ipv4_is_valid("192.0.2.1/24"));
	assert(!dnsmasq_static_arp_ipv4_is_valid(NULL));
	assert(!dnsmasq_static_arp_ipv4_is_valid(""));
}

static void
test_static_arp_record_parser(void)
{
	char ip[16], ifname[32];
	unsigned int flags = 0;
	int status;

	status = dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname));
	assert(status == 0);
	assert(strcmp(ip, "192.0.2.1") == 0);
	assert(flags == 0x04);
	assert(strcmp(ifname, "br0") == 0);

	status = dnsmasq_static_arp_record_parse(
		"10.0.0.254 0x1 0x2 aa:bb:cc:dd:ee:ff 255.255.255.0 eth2.1",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname));
	assert(status == 0);
	assert(flags == 0x02);
	assert(strcmp(ip, "10.0.0.254") == 0);
	assert(strcmp(ifname, "eth2.1") == 0);

	/* The cleanup caller issues Ethernet ARP ioctls on br0 only. */
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x0 0x4 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x2 0x4 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0xFFFF 0x4 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4 00:11:22:33:44:55 * br0 extra",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"999.0.2.1 0x1 0x4 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0xGG 0x4 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4junk 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x-1 0x4 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4 00:11:22:33:44 * br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4 00:11:22:33:44:55 999.0.0.0 br0",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4 00:11:22:33:44:55 *",
		ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EIO);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4 00:11:22:33:44:55 * br0",
		ip, 8, &flags, ifname, sizeof(ifname)) == ENOSPC);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4 00:11:22:33:44:55 * br0",
		ip, sizeof(ip), &flags, ifname, 3) == ENOSPC);
	assert(dnsmasq_static_arp_record_parse(
		NULL, ip, sizeof(ip), &flags, ifname, sizeof(ifname)) == EINVAL);
	assert(dnsmasq_static_arp_record_parse(
		"192.0.2.1 0x1 0x4 00:11:22:33:44:55 * br0",
		NULL, sizeof(ip), &flags, ifname, sizeof(ifname)) == EINVAL);
}

static void
test_static_arp_ownership_snapshot(void)
{
	struct dnsmasq_static_arp_snapshot snapshot;

	memset(&snapshot, 0, sizeof(snapshot));
	assert(dnsmasq_static_arp_snapshot_add_ethers_record(
		&snapshot, "00:11:22:33:44:5a 192.0.2.10") == 0);
	assert(snapshot.count == 1);
	assert(dnsmasq_static_arp_snapshot_contains(
		&snapshot, "192.0.2.10", "00:11:22:33:44:5A"));
	assert(!dnsmasq_static_arp_snapshot_contains(
		&snapshot, "192.0.2.10", "02:11:22:33:44:55"));
	assert(!dnsmasq_static_arp_snapshot_contains(
		&snapshot, "192.0.2.11", "00:11:22:33:44:5A"));

	assert(dnsmasq_static_arp_snapshot_contains_record(
		&snapshot,
		"192.0.2.10 0x1 0x6 00:11:22:33:44:5A * br0"));
	/* Same bridge and permanent flag are insufficient without the old MAC/IP pair. */
	assert(!dnsmasq_static_arp_snapshot_contains_record(
		&snapshot,
		"192.0.2.10 0x1 0x6 02:11:22:33:44:55 * br0"));
	assert(!dnsmasq_static_arp_snapshot_contains_record(
		&snapshot,
		"192.0.2.11 0x1 0x6 00:11:22:33:44:5A * br0"));

	assert(dnsmasq_static_arp_snapshot_add_ethers_record(
		&snapshot, "02:11:22:33:44:55 192.0.2.10") == EEXIST);
	assert(dnsmasq_static_arp_snapshot_add_ethers_record(
		&snapshot, "invalid ethers record") == EIO);
	assert(snapshot.count == 1);
}

static void
test_valid_unicast_mac_text(void)
{
	assert(dnsmasq_static_mac_is_valid("001122334455"));
	assert(dnsmasq_static_mac_is_valid("021122334455"));
	assert(dnsmasq_static_mac_is_valid("aAbBcCdDeEfF"));
	assert(dnsmasq_static_mac_is_valid("fe1122334455"));
}

static void
test_rejects_non_unicast_addresses(void)
{
	assert(!dnsmasq_static_mac_is_valid("000000000000"));
	assert(!dnsmasq_static_mac_is_valid("011122334455"));
	assert(!dnsmasq_static_mac_is_valid("ff1122334455"));
}

static void
test_rejects_injection_characters(void)
{
	assert(!dnsmasq_static_mac_is_valid("00112233,455"));
	assert(!dnsmasq_static_mac_is_valid("00112233\n455"));
	assert(!dnsmasq_static_mac_is_valid("00112233 455"));
	assert(!dnsmasq_static_mac_is_valid("00112233/455"));
	assert(!dnsmasq_static_mac_is_valid("00112233:455"));
	assert(!dnsmasq_static_mac_is_valid("00112233;455"));
	assert(!dnsmasq_static_mac_is_valid("00112233$455"));
}

static void
test_rejects_wrong_lengths(void)
{
	char empty[1] = { '\0' };
	char one_digit[2] = { '0', '\0' };
	char five_digits[6] = { '0', '0', '1', '1', '2', '\0' };
	char eleven_digits[12] = "00112233445";

	assert(!dnsmasq_static_mac_is_valid(NULL));
	assert(!dnsmasq_static_mac_is_valid(empty));
	assert(!dnsmasq_static_mac_is_valid(one_digit));
	assert(!dnsmasq_static_mac_is_valid(five_digits));
	assert(!dnsmasq_static_mac_is_valid(eleven_digits));
	assert(!dnsmasq_static_mac_is_valid("00112233445"));
	assert(!dnsmasq_static_mac_is_valid("0011223344556"));
	assert(!dnsmasq_static_mac_is_valid("00112233445g"));
}


static void
test_static_ethers_record_parser(void)
{
	char ip[16], mac[13];

	assert(dnsmasq_static_ethers_record_parse(
		"00:11:22:33:44:55 192.0.2.10", ip, sizeof(ip),
		mac, sizeof(mac)) == 0);
	assert(strcmp(ip, "192.0.2.10") == 0);
	assert(strcmp(mac, "001122334455") == 0);

	assert(dnsmasq_static_ethers_record_parse(
		"AA:BB:CC:DD:EE:FE 10.0.0.254   ", ip, sizeof(ip),
		mac, sizeof(mac)) == 0);
	assert(strcmp(ip, "10.0.0.254") == 0);
	assert(strcmp(mac, "AABBCCDDEEFE") == 0);
	assert(dnsmasq_static_ethers_record_parse(
		"01:11:22:33:44:55 192.0.2.10", ip, sizeof(ip),
		mac, sizeof(mac)) == EIO);
	assert(dnsmasq_static_ethers_record_parse(
		"00:00:00:00:00:00 192.0.2.10", ip, sizeof(ip),
		mac, sizeof(mac)) == EIO);
	assert(dnsmasq_static_ethers_record_parse(
		"00:11:22:33:44:55 999.0.2.10", ip, sizeof(ip),
		mac, sizeof(mac)) == EIO);
	assert(dnsmasq_static_ethers_record_parse(
		"00:11:22:33:44:5 192.0.2.10", ip, sizeof(ip),
		mac, sizeof(mac)) == EIO);
	assert(dnsmasq_static_ethers_record_parse(
		"00:11:22:33:44:55 192.0.2.10 unexpected", ip,
		sizeof(ip), mac, sizeof(mac)) == EIO);
	assert(dnsmasq_static_ethers_record_parse(
		"00:11:22:33:44:55", ip, sizeof(ip),
		mac, sizeof(mac)) == EIO);
	assert(dnsmasq_static_ethers_record_parse(
		NULL, ip, sizeof(ip), mac, sizeof(mac)) == EINVAL);
	assert(dnsmasq_static_ethers_record_parse(
		"00:11:22:33:44:55 192.0.2.10", ip, 8,
		mac, sizeof(mac)) == ENOSPC);
	assert(dnsmasq_static_ethers_record_parse(
		"00:11:22:33:44:55 192.0.2.10", ip, sizeof(ip),
		mac, 8) == ENOSPC);
}


static void
test_static_arp_command_status(void)
{
	assert(dnsmasq_static_arp_command_errno(0, 0) == 0);
	assert(dnsmasq_static_arp_command_errno(9 << 8, 0) == EIO);
	assert(dnsmasq_static_arp_command_errno(-1, ENOENT) == ENOENT);
	assert(dnsmasq_static_arp_command_errno(-1, 0) == EIO);
	assert(dnsmasq_static_arp_command_errno(SIGTERM, 0) == EIO);
}


#define ARP_TRANSACTION_TEST_MAX 8

struct arp_transaction_test_context {
	const char *ips[ARP_TRANSACTION_TEST_MAX];
	char macs[ARP_TRANSACTION_TEST_MAX][13];
	int adds[ARP_TRANSACTION_TEST_MAX];
	size_t calls;
	size_t fail_add_call;
	const char *fail_delete_ip;
	int add_error;
	int delete_error;
};

static int
mock_static_arp_command(const char *ip, const char *mac, int add, void *opaque)
{
	struct arp_transaction_test_context *context =
		(struct arp_transaction_test_context *)opaque;
	size_t call = context->calls++;

	assert(call < ARP_TRANSACTION_TEST_MAX);
	assert(mac != NULL && mac[0] != '\0');
	context->ips[call] = ip;
	strncpy(context->macs[call], mac, sizeof(context->macs[call]) - 1);
	context->macs[call][sizeof(context->macs[call]) - 1] = '\0';
	context->adds[call] = add;
	if (add && context->fail_add_call == call + 1)
		return context->add_error;
	if (!add && context->fail_delete_ip &&
	    strcmp(ip, context->fail_delete_ip) == 0)
		return context->delete_error;
	return 0;
}

static void
test_static_arp_transaction_success(void)
{
	const char *ips[] = { "192.0.2.10", "192.0.2.11" };
	const char *macs[] = { "001122334410", "001122334411" };
	struct arp_transaction_test_context context;

	memset(&context, 0, sizeof(context));
	assert(dnsmasq_static_arp_apply_transaction(ips, macs, 2,
						    mock_static_arp_command, &context) == 0);
	assert(context.calls == 2);
	assert(context.adds[0] == 1 && strcmp(context.ips[0], ips[0]) == 0);
	assert(context.adds[1] == 1 && strcmp(context.ips[1], ips[1]) == 0);
}

static void
test_static_arp_transaction_rolls_back_prior_additions(void)
{
	const char *ips[] = { "192.0.2.10", "192.0.2.11" };
	const char *macs[] = { "001122334410", "001122334411" };
	struct arp_transaction_test_context context;

	memset(&context, 0, sizeof(context));
	context.fail_add_call = 2;
	context.add_error = EIO;
	assert(dnsmasq_static_arp_apply_transaction(ips, macs, 2,
						    mock_static_arp_command, &context) == EIO);
	assert(context.calls == 3);
	assert(context.adds[0] == 1 && strcmp(context.ips[0], ips[0]) == 0);
	assert(context.adds[1] == 1 && strcmp(context.ips[1], ips[1]) == 0);
	assert(context.adds[2] == 0 && strcmp(context.ips[2], ips[0]) == 0);
}

static void
test_static_arp_transaction_continues_after_delete_failure(void)
{
	const char *ips[] = {
		"192.0.2.10", "192.0.2.11", "192.0.2.12"
	};
	const char *macs[] = {
		"001122334410", "001122334411", "001122334412"
	};
	struct arp_transaction_test_context context;

	memset(&context, 0, sizeof(context));
	context.fail_add_call = 3;
	context.add_error = EIO;
	context.fail_delete_ip = ips[0];
	context.delete_error = EACCES;
	assert(dnsmasq_static_arp_apply_transaction(ips, macs, 3,
						    mock_static_arp_command, &context) == EACCES);
	assert(context.calls == 5);
	assert(context.adds[3] == 0 && strcmp(context.ips[3], ips[1]) == 0);
	assert(context.adds[4] == 0 && strcmp(context.ips[4], ips[0]) == 0);
}

static void
test_static_arp_transaction_prevalidates_all_entries(void)
{
	const char *ips[] = { "192.0.2.10", NULL };
	const char *macs[] = { "001122334410", "001122334411" };
	struct arp_transaction_test_context context;

	memset(&context, 0, sizeof(context));
	assert(dnsmasq_static_arp_apply_transaction(ips, macs, 2,
						    mock_static_arp_command, &context) == EINVAL);
	assert(context.calls == 0);
}

static void
test_static_arp_snapshot_apply_replays_saved_ethers_state(void)
{
	struct dnsmasq_static_arp_snapshot snapshot;
	struct arp_transaction_test_context context;

	memset(&snapshot, 0, sizeof(snapshot));
	assert(dnsmasq_static_arp_snapshot_add_ethers_record(
		&snapshot, "00:11:22:33:44:10 192.0.2.10") == 0);
	assert(dnsmasq_static_arp_snapshot_add_ethers_record(
		&snapshot, "00:11:22:33:44:11 192.0.2.11") == 0);

	memset(&context, 0, sizeof(context));
	assert(dnsmasq_static_arp_snapshot_apply(
		&snapshot, mock_static_arp_command, &context) == 0);
	assert(context.calls == 2);
	assert(context.adds[0] == 1 && strcmp(context.ips[0], "192.0.2.10") == 0);
	assert(strcmp(context.macs[0], "001122334410") == 0);
	assert(context.adds[1] == 1 && strcmp(context.ips[1], "192.0.2.11") == 0);
	assert(strcmp(context.macs[1], "001122334411") == 0);

	memset(&context, 0, sizeof(context));
	context.fail_add_call = 2;
	context.add_error = EIO;
	assert(dnsmasq_static_arp_snapshot_apply(
		&snapshot, mock_static_arp_command, &context) == EIO);
	assert(context.calls == 3);
	assert(context.adds[0] == 1 && strcmp(context.ips[0], "192.0.2.10") == 0);
	assert(context.adds[1] == 1 && strcmp(context.ips[1], "192.0.2.11") == 0);
	assert(context.adds[2] == 0 && strcmp(context.ips[2], "192.0.2.10") == 0);

	memset(&context, 0, sizeof(context));
	snapshot.entries[1].mac[0] = 'z';
	assert(dnsmasq_static_arp_snapshot_apply(
		&snapshot, mock_static_arp_command, &context) == EINVAL);
	assert(context.calls == 0);
	snapshot.entries[1].mac[0] = '0';
	memset(&context, 0, sizeof(context));
	snapshot.count = DNSMASQ_STATIC_ARP_MAX + 1;
	assert(dnsmasq_static_arp_snapshot_apply(
		&snapshot, mock_static_arp_command, &context) == EINVAL);
	assert(context.calls == 0);
}


static void
test_static_arp_transaction_rejects_invalid_values_before_commands(void)
{
	const char *valid_ips[] = { "192.0.2.10", "192.0.2.11" };
	const char *injected_ips[] = {
		"192.0.2.10", "192.0.2.11;touch /tmp/arp-policy-injection"
	};
	const char *valid_macs[] = { "001122334410", "001122334411" };
	const char *injected_macs[] = {
		"001122334410", "001122334411;touch /tmp/arp-policy-injection"
	};
	const char *multicast_macs[] = { "001122334410", "011122334411" };
	struct arp_transaction_test_context context;

	memset(&context, 0, sizeof(context));
	assert(dnsmasq_static_arp_apply_transaction(injected_ips, valid_macs, 2,
						    mock_static_arp_command, &context) == EINVAL);
	assert(context.calls == 0);

	memset(&context, 0, sizeof(context));
	assert(dnsmasq_static_arp_apply_transaction(valid_ips, injected_macs, 2,
						    mock_static_arp_command, &context) == EINVAL);
	assert(context.calls == 0);

	memset(&context, 0, sizeof(context));
	assert(dnsmasq_static_arp_apply_transaction(valid_ips, multicast_macs, 2,
						    mock_static_arp_command, &context) == EINVAL);
	assert(context.calls == 0);
}


int
main(void)
{
	test_static_arp_header_parser();
	test_static_arp_ipv4_text();
	test_static_arp_record_parser();
	test_static_arp_ownership_snapshot();
	test_valid_unicast_mac_text();
	test_rejects_non_unicast_addresses();
	test_rejects_injection_characters();
	test_rejects_wrong_lengths();
	test_static_ethers_record_parser();
	test_static_arp_command_status();
	test_static_arp_transaction_success();
	test_static_arp_transaction_rolls_back_prior_additions();
	test_static_arp_transaction_continues_after_delete_failure();
	test_static_arp_transaction_prevalidates_all_entries();
	test_static_arp_snapshot_apply_replays_saved_ethers_state();
	test_static_arp_transaction_rejects_invalid_values_before_commands();
	puts("DNSMasq static-host MAC and ARP transaction policies: PASS");
	return 0;
}
