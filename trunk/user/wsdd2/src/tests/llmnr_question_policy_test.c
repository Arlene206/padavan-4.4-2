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

	memset(packet, 0, 12 + labels_len + 4);
	memcpy(packet + len, labels, labels_len);
	len += labels_len;
	packet[len++] = (uint8_t)(qtype >> 8);
	packet[len++] = (uint8_t)qtype;
	packet[len++] = (uint8_t)(qclass >> 8);
	packet[len++] = (uint8_t)qclass;
	return len;
}

static void
append_opt(uint8_t *packet, size_t *length, uint8_t version,
           const uint8_t *options, size_t options_length)
{
	size_t pos = *length;
	assert(options_length <= 0xffffU);
	packet[pos++] = 0;
	packet[pos++] = 0;
	packet[pos++] = 41;
	packet[pos++] = 0x04;
	packet[pos++] = 0xd0;
	packet[pos++] = 0;
	packet[pos++] = version;
	packet[pos++] = 0;
	packet[pos++] = 0;
	packet[pos++] = (uint8_t)(options_length >> 8);
	packet[pos++] = (uint8_t)options_length;
	if (options_length) {
		memcpy(packet + pos, options, options_length);
		pos += options_length;
	}
	*length = pos;
}

static void
test_edns0_response_layout_keeps_answer_before_opt(void)
{
	uint8_t packet[512] = {0}, response[512] = {0};
	const uint8_t labels[] = {6, 'r','o','u','t','e','r', 0};
	const uint8_t options[] = {0xfd, 0xe9, 0, 0};
	const uint8_t answer[] = {0xc0, 0x0c, 0, 1, 0, 1, 0, 0, 0, 0, 0, 4, 192, 0, 2, 1};
	struct llmnr_additional_info info;
	char name[256];
	uint16_t qtype, qclass;
	size_t question_end, packet_len, response_len, opt_offset;

	packet_len = make_question(packet, labels, sizeof(labels), 1, 1);
	packet[11] = 1;
	append_opt(packet, &packet_len, 0, options, sizeof(options));
	assert(llmnr_parse_question_ex(packet, packet_len, name, sizeof(name),
	                               &qtype, &qclass, &question_end) == 0);
	assert(llmnr_parse_additional_section(packet, packet_len, question_end, 1, &info) == 0);
	assert(strcmp(name, "router") == 0 && info.has_opt && !info.duplicate_opt && !info.malformed_opt);
	assert(info.version == 0);
	assert(llmnr_build_response_packet(packet, packet_len, question_end, answer, sizeof(answer),
	                                   1, 0, 0, response, sizeof(response), &response_len) == 0);
	assert(response[2] == 0x80 && response[3] == 0);
	assert(response[6] == 0 && response[7] == 1 && response[8] == 0 && response[9] == 0);
	assert(response[10] == 0 && response[11] == 1);
	assert(memcmp(response + question_end, answer, sizeof(answer)) == 0);
	opt_offset = question_end + sizeof(answer);
	assert(response[opt_offset] == 0 && response[opt_offset + 1] == 0 && response[opt_offset + 2] == 41);
	assert(response[opt_offset + 3] == 2 && response[opt_offset + 4] == 0);
	assert(response[opt_offset + 5] == 0 && response[opt_offset + 6] == 0);
	assert(response[opt_offset + 9] == 0 && response[opt_offset + 10] == 0);
	assert(response_len == opt_offset + 11);
	assert(llmnr_parse_additional_section(response, response_len, opt_offset, 1, &info) == 0);
	assert(info.has_opt && !info.malformed_opt && !info.duplicate_opt);
}

static void
test_non_opt_additional_records_are_ignored_in_reply(void)
{
	uint8_t packet[128] = {0}, response[128] = {0};
	const uint8_t labels[] = {6, 'r','o','u','t','e','r', 0};
	const uint8_t answer[] = {0xc0, 0x0c, 0, 1, 0, 1, 0, 0, 0, 0, 0, 4, 192, 0, 2, 1};
	struct llmnr_additional_info info;
	char name[256];
	uint16_t qtype, qclass;
	size_t question_end, packet_len, response_len;

	packet_len = make_question(packet, labels, sizeof(labels), 1, 1);
	packet[11] = 1;
	packet[packet_len++] = 0;
	packet[packet_len++] = 0;
	packet[packet_len++] = 16; /* TXT */
	packet[packet_len++] = 0;
	packet[packet_len++] = 1;
	memset(packet + packet_len, 0, 4);
	packet_len += 4;
	packet[packet_len++] = 0;
	packet[packet_len++] = 3;
	memcpy(packet + packet_len, "abc", 3);
	packet_len += 3;
	assert(llmnr_parse_question_ex(packet, packet_len, name, sizeof(name), &qtype, &qclass, &question_end) == 0);
	assert(llmnr_parse_additional_section(packet, packet_len, question_end, 1, &info) == 0);
	assert(!info.has_opt);
	assert(llmnr_build_response_packet(packet, packet_len, question_end, answer, sizeof(answer),
	                                   0, 0, 0, response, sizeof(response), &response_len) == 0);
	assert(response[7] == 1 && response[11] == 0);
	assert(response_len == question_end + sizeof(answer));
	assert(memcmp(response + question_end, answer, sizeof(answer)) == 0);
}

static void
test_duplicate_malformed_and_future_version_opt_errors(void)
{
	uint8_t packet[256] = {0}, response[256] = {0};
	const uint8_t labels[] = {6, 'r','o','u','t','e','r', 0};
	const uint8_t malformed_option[] = {0, 10, 0, 1};
	struct llmnr_additional_info info;
	char name[256];
	uint16_t qtype, qclass;
	size_t question_end, packet_len, response_len;

	packet_len = make_question(packet, labels, sizeof(labels), 1, 1);
	packet[11] = 1;
	append_opt(packet, &packet_len, 0, malformed_option, sizeof(malformed_option));
	assert(llmnr_parse_question_ex(packet, packet_len, name, sizeof(name), &qtype, &qclass, &question_end) == 0);
	assert(llmnr_parse_additional_section(packet, packet_len, question_end, 1, &info) == 0);
	assert(info.has_opt && info.malformed_opt);
	assert(llmnr_build_response_packet(packet, packet_len, question_end, NULL, 0, 1, 1, 0,
	                                   response, sizeof(response), &response_len) == 0);
	assert(response[7] == 0 && response[11] == 1 && response[3] == 1);

	memset(packet, 0, sizeof(packet));
	packet_len = make_question(packet, labels, sizeof(labels), 1, 1);
	packet[11] = 2;
	append_opt(packet, &packet_len, 0, NULL, 0);
	append_opt(packet, &packet_len, 0, NULL, 0);
	assert(llmnr_parse_question_ex(packet, packet_len, name, sizeof(name), &qtype, &qclass, &question_end) == 0);
	assert(llmnr_parse_additional_section(packet, packet_len, question_end, 2, &info) == 0);
	assert(info.has_opt && info.duplicate_opt);
	assert(llmnr_build_response_packet(packet, packet_len, question_end, NULL, 0, 1, 1, 0,
	                                   response, sizeof(response), &response_len) == 0);
	assert(response[3] == 1 && response[7] == 0 && response[11] == 1);

	memset(packet, 0, sizeof(packet));
	packet_len = make_question(packet, labels, sizeof(labels), 1, 1);
	packet[11] = 1;
	append_opt(packet, &packet_len, 1, NULL, 0);
	assert(llmnr_parse_question_ex(packet, packet_len, name, sizeof(name), &qtype, &qclass, &question_end) == 0);
	assert(llmnr_parse_additional_section(packet, packet_len, question_end, 1, &info) == 0);
	assert(info.has_opt && info.version == 1 && !info.malformed_opt);
	assert(llmnr_build_response_packet(packet, packet_len, question_end, NULL, 0, 1, 0, 1,
	                                   response, sizeof(response), &response_len) == 0);
	assert(response[3] == 0 && response[7] == 0 && response[11] == 1);
	assert(response[question_end + 5] == 1 && response[question_end + 6] == 0);
}

static void
test_additional_section_bounds_and_compressed_names(void)
{
	uint8_t packet[128] = {0};
	const uint8_t labels[] = {6, 'r','o','u','t','e','r', 0};
	struct llmnr_additional_info info;
	char name[256];
	uint16_t qtype, qclass;
	size_t question_end, packet_len;

	packet_len = make_question(packet, labels, sizeof(labels), 1, 1);
	packet[11] = 1;
	packet[packet_len++] = 0xc0;
	packet[packet_len++] = 0x0c;
	packet[packet_len++] = 0;
	packet[packet_len++] = 16;
	packet[packet_len++] = 0;
	packet[packet_len++] = 1;
	memset(packet + packet_len, 0, 4);
	packet_len += 4;
	packet[packet_len++] = 0;
	packet[packet_len++] = 0;
	assert(llmnr_parse_question_ex(packet, packet_len, name, sizeof(name), &qtype, &qclass, &question_end) == 0);
	assert(llmnr_parse_additional_section(packet, packet_len, question_end, 1, &info) == 0);
	assert(!info.has_opt);

	packet[11] = 0;
	packet[packet_len++] = 0xaa;
	assert(llmnr_parse_additional_section(packet, packet_len, question_end, 0, &info) == -1);

	memset(packet, 0, sizeof(packet));
	packet_len = make_question(packet, labels, sizeof(labels), 1, 1);
	packet[11] = 1;
	packet[packet_len++] = 0;
	packet[packet_len++] = 0;
	packet[packet_len++] = 41;
	assert(llmnr_parse_question_ex(packet, packet_len, name, sizeof(name), &qtype, &qclass, &question_end) == 0);
	assert(llmnr_parse_additional_section(packet, packet_len, question_end, 1, &info) == -1);
}

static void
test_hostname_is_always_terminated(void)
{
	char hostname[256];

	memset(hostname, 'x', sizeof(hostname));
	assert(llmnr_read_hostname(hostname, sizeof(hostname)) == 0);
	assert(hostname[sizeof(hostname) - 1] == '\0');
	assert(memchr(hostname, '\0', sizeof(hostname)) != NULL);
	assert(llmnr_read_hostname(NULL, sizeof(hostname)) == -1);
	assert(llmnr_read_hostname(hostname, 0) == -1);
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
test_ambiguous_text_labels_are_rejected(void)
{
	uint8_t packet[64];
	const uint8_t embedded_nul[] = {
		8, 'r','o','u','t','e','r','\0','x', 0
	};
	const uint8_t embedded_dot[] = {
		8, 'r','o','u','t','e','r','.','x', 0
	};
	char name[256];
	uint16_t qtype, qclass;
	size_t len;

	len = make_question(packet, embedded_nul, sizeof(embedded_nul), 1, 1);
	assert(llmnr_parse_question(packet, len, name, sizeof(name),
	                            &qtype, &qclass) == -1);

	len = make_question(packet, embedded_dot, sizeof(embedded_dot), 1, 1);
	assert(llmnr_parse_question(packet, len, name, sizeof(name),
	                            &qtype, &qclass) == -1);
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
	test_hostname_is_always_terminated();
	test_edns0_response_layout_keeps_answer_before_opt();
	test_non_opt_additional_records_are_ignored_in_reply();
	test_duplicate_malformed_and_future_version_opt_errors();
	test_additional_section_bounds_and_compressed_names();
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
	test_ambiguous_text_labels_are_rejected();
	test_dns_name_and_output_bounds();
	return 0;
}
