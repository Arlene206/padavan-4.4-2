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

int
main(void)
{
	test_static_arp_header_parser();
	test_static_arp_record_parser();
	test_valid_unicast_mac_text();
	test_rejects_non_unicast_addresses();
	test_rejects_injection_characters();
	test_rejects_wrong_lengths();
	test_static_arp_command_status();
	test_static_arp_transaction_success();
	test_static_arp_transaction_rolls_back_prior_additions();
	test_static_arp_transaction_continues_after_delete_failure();
	test_static_arp_transaction_prevalidates_all_entries();
	puts("DNSMasq static-host MAC and ARP transaction policies: PASS");
	return 0;
}
