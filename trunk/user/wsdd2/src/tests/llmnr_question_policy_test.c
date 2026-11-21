#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "llmnr_question_policy.h"

static size_t
make_question(uint8_t *packet, const uint8_t *labels, size_t labels_len,
              uint16_t qtype, uint16_t qclass)
{
	size_t len = 12;

	memset(packet, 0, 300);
	memcpy(packet + len, labels, labels_len);
	len += labels_len;
	packet[len++] = (uint8_t)(qtype >> 8);
	packet[len++] = (uint8_t)qtype;
	packet[len++] = (uint8_t)(qclass >> 8);
	packet[len++] = (uint8_t)qclass;
	return len;
}

static void
test_valid_single_and_multi_label(void)
{
	uint8_t packet[300];
	const uint8_t single[] = {6, 'r','o','u','t','e','r', 0};
	const uint8_t multi[] = {3,'f','o','o', 3,'b','a','r', 0};
	char name[256];
	uint16_t qtype, qclass;
	size_t len;

	len = make_question(packet, single, sizeof(single), 1, 1);
	assert(llmnr_parse_question(packet, len, name, sizeof(name),
	                            &qtype, &qclass) == 0);
	assert(strcmp(name, "router") == 0 && qtype == 1 && qclass == 1);

	len = make_question(packet, multi, sizeof(multi), 28, 1);
	assert(llmnr_parse_question(packet, len, name, sizeof(name),
	                            &qtype, &qclass) == 0);
	assert(strcmp(name, "foo.bar") == 0 && qtype == 28 && qclass == 1);
}

static void
test_truncated_label_and_question(void)
{
	uint8_t packet[32] = {0};
	char name[256];
	uint16_t qtype, qclass;
	size_t len;

	packet[12] = 5;
	memcpy(packet + 13, "abc", 3);
	assert(llmnr_parse_question(packet, 16, name, sizeof(name),
	                            &qtype, &qclass) == -1);

	packet[12] = 1;
	packet[13] = 'x';
	packet[14] = 0;
	for (len = 15; len < 19; ++len)
		assert(llmnr_parse_question(packet, len, name, sizeof(name),
		                            &qtype, &qclass) == -1);
}

static void
test_bad_label_types_and_missing_root(void)
{
	uint8_t packet[300] = {0};
	char name[256];
	uint16_t qtype, qclass;
	size_t i;

	packet[12] = 0xC0;
	assert(llmnr_parse_question(packet, 17, name, sizeof(name),
	                            &qtype, &qclass) == -1);
	packet[12] = 0x40;
	assert(llmnr_parse_question(packet, 17, name, sizeof(name),
	                            &qtype, &qclass) == -1);
	packet[12] = 64;
	assert(llmnr_parse_question(packet, 80, name, sizeof(name),
	                            &qtype, &qclass) == -1);

	packet[12] = 1;
	packet[13] = 'x';
	packet[14] = 1;
	packet[15] = 'y';
	packet[16] = 1;
	packet[17] = 'z';
	assert(llmnr_parse_question(packet, 20, name, sizeof(name),
	                            &qtype, &qclass) == -1);

	for (i = 0; i < 256; ++i) {
		packet[12] = (uint8_t)i;
		(void)llmnr_parse_question(packet, 17, name, sizeof(name),
		                           &qtype, &qclass);
	}
}

static void
test_dns_name_and_output_bounds(void)
{
	uint8_t packet[300] = {0};
	uint8_t labels[260];
	char name[256];
	char short_name[3];
	uint16_t qtype, qclass;
	size_t i, len;

	for (i = 0; i < 4; ++i) {
		labels[i * 64] = 63;
		memset(labels + i * 64 + 1, 'a' + (int)i, 63);
	}
	labels[256] = 0;
	len = make_question(packet, labels, 257, 1, 1);
	assert(llmnr_parse_question(packet, len, name, sizeof(name),
	                            &qtype, &qclass) == -1);

	{
		const uint8_t valid[] = {3,'f','o','o',0};
		len = make_question(packet, valid, sizeof(valid), 1, 1);
		assert(llmnr_parse_question(packet, len, short_name,
		                            sizeof(short_name), &qtype, &qclass) == -1);
	}
}

int
main(void)
{
	uint8_t packet[300] = {0};
	char name[256];
	uint16_t qtype, qclass;
	size_t len;

	assert(llmnr_parse_question(NULL, 0, name, sizeof(name),
	                            &qtype, &qclass) == -1);
	for (len = 0; len < 17; ++len)
		assert(llmnr_parse_question(packet, len, name, sizeof(name),
		                            &qtype, &qclass) == -1);
	test_valid_single_and_multi_label();
	test_truncated_label_and_question();
	test_bad_label_types_and_missing_root();
	test_dns_name_and_output_bounds();
	return 0;
}
