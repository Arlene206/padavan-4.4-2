#ifndef LLMNR_QUESTION_POLICY_H
#define LLMNR_QUESTION_POLICY_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

/* Parse one uncompressed DNS question without reading outside the datagram. */
static inline int
llmnr_parse_question_ex(const uint8_t *packet, size_t packet_len,
                        char *name, size_t name_capacity,
                        uint16_t *qtype, uint16_t *qclass,
                        size_t *question_end)
{
	const uint8_t *cursor;
	const uint8_t *end;
	size_t name_len = 0;
	size_t wire_len = 0;

	if (!packet || !name || name_capacity == 0 || !qtype || !qclass ||
	    packet_len < 17)
		return -1;
	cursor = packet + 12;
	end = packet + packet_len;
	name[0] = '\0';
	for (;;) {
		size_t label_len, separator_len;
		if (cursor >= end)
			return -1;
		label_len = *cursor++;
		if (label_len == 0)
			break;
		if (label_len > 63 || (size_t)(end - cursor) < label_len)
			return -1;
		if (wire_len > 255 - (label_len + 1))
			return -1;
		{
			size_t i;
			for (i = 0; i < label_len; ++i)
				if (cursor[i] == '\0' || cursor[i] == '.')
					return -1;
		}
		wire_len += label_len + 1;
		separator_len = name_len ? 1 : 0;
		if (separator_len + label_len >= name_capacity - name_len)
			return -1;
		if (separator_len)
			name[name_len++] = '.';
		memcpy(name + name_len, cursor, label_len);
		name_len += label_len;
		name[name_len] = '\0';
		cursor += label_len;
	}
	if (wire_len >= 255 || (size_t)(end - cursor) < 4)
		return -1;
	*qtype = (uint16_t)(((uint16_t)cursor[0] << 8) | cursor[1]);
	*qclass = (uint16_t)(((uint16_t)cursor[2] << 8) | cursor[3]);
	if (question_end)
		*question_end = (size_t)(cursor + 4 - packet);
	return 0;
}

static inline int
llmnr_parse_question(const uint8_t *packet, size_t packet_len,
                     char *name, size_t name_capacity,
                     uint16_t *qtype, uint16_t *qclass)
{
	return llmnr_parse_question_ex(packet, packet_len, name, name_capacity,
	                               qtype, qclass, NULL);
}

struct llmnr_additional_info {
	int has_opt;
	int duplicate_opt;
	int malformed_opt;
	uint8_t version;
};

static inline uint16_t
llmnr_policy_read_u16(const uint8_t *data)
{
	return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static inline int
llmnr_skip_wire_name(const uint8_t *packet, size_t packet_len,
                     size_t *offset, int *is_root)
{
	size_t cursor, wire_len = 0;
	int root;

	if (!packet || !offset || !is_root || *offset >= packet_len)
		return -1;
	cursor = *offset;
	root = packet[cursor] == 0;
	for (;;) {
		uint8_t label_len;
		if (cursor >= packet_len)
			return -1;
		label_len = packet[cursor++];
		if (label_len == 0) {
			if (wire_len >= 255)
				return -1;
			*offset = cursor;
			*is_root = root;
			return 0;
		}
		if ((label_len & 0xc0U) == 0xc0U) {
			size_t pointer;
			if (cursor >= packet_len)
				return -1;
			pointer = ((size_t)(label_len & 0x3fU) << 8) | packet[cursor];
			if (pointer >= cursor - 1)
				return -1;
			*offset = cursor + 1;
			*is_root = 0;
			return 0;
		}
		if ((label_len & 0xc0U) != 0 || label_len > 63 ||
		    (size_t)(packet_len - cursor) < label_len)
			return -1;
		wire_len += (size_t)label_len + 1;
		if (wire_len > 254)
			return -1;
		cursor += label_len;
	}
}

/*
 * LLMNR RFC 4795 section 2.9: ordinary Additional RRs are ignored. Parse
 * their bounds but retain only EDNS OPT metadata for the response.
 */
static inline int
llmnr_parse_additional_section(const uint8_t *packet, size_t packet_len,
                               size_t offset, uint16_t record_count,
                               struct llmnr_additional_info *info)
{
	uint16_t i;
	size_t cursor = offset;

	if (!packet || !info || offset > packet_len)
		return -1;
	info->has_opt = 0;
	info->duplicate_opt = 0;
	info->malformed_opt = 0;
	info->version = 0;
	for (i = 0; i < record_count; ++i) {
		size_t name_start = cursor, rdlength, rdata_end;
		uint16_t type;
		int owner_is_root = 0;
		if (llmnr_skip_wire_name(packet, packet_len, &cursor,
		                         &owner_is_root) != 0 ||
		    packet_len - cursor < 10)
			return -1;
		type = llmnr_policy_read_u16(packet + cursor);
		rdlength = llmnr_policy_read_u16(packet + cursor + 8);
		if (rdlength > packet_len - cursor - 10)
			return -1;
		if (type == 41U) {
			size_t option_cursor;
			if (info->has_opt)
				info->duplicate_opt = 1;
			else {
				info->has_opt = 1;
				info->version = packet[cursor + 5];
			}
			if (!owner_is_root || cursor - name_start != 1 ||
			    packet[name_start] != 0)
				info->malformed_opt = 1;
			option_cursor = cursor + 10;
			rdata_end = option_cursor + rdlength;
			while (option_cursor < rdata_end) {
				size_t option_length;
				if (rdata_end - option_cursor < 4) {
					info->malformed_opt = 1;
					break;
				}
				option_length = llmnr_policy_read_u16(packet + option_cursor + 2);
				option_cursor += 4;
				if (option_length > rdata_end - option_cursor) {
					info->malformed_opt = 1;
					break;
				}
				option_cursor += option_length;
			}
		}
		cursor += 10 + rdlength;
	}
	return cursor == packet_len ? 0 : -1;
}

static inline int
llmnr_build_response_packet(const uint8_t *request, size_t request_len,
                            size_t question_end,
                            const uint8_t *answer, size_t answer_len,
                            int include_opt, uint8_t rcode,
                            uint8_t extended_rcode,
                            uint8_t *response, size_t response_capacity,
                            size_t *response_len)
{
	size_t required;
	if (!request || !response || !response_len || question_end < 12 ||
	    question_end > request_len || rcode > 15U ||
	    (answer_len != 0 && !answer) || question_end > response_capacity)
		return -1;
	if (answer_len > response_capacity - question_end)
		return -1;
	required = question_end + answer_len;
	if (include_opt) {
		if (11U > response_capacity - required)
			return -1;
		required += 11U;
	}
	memcpy(response, request, question_end);
	response[2] = 0x80U;
	response[3] = rcode;
	response[4] = 0;
	response[5] = 1;
	response[6] = 0;
	response[7] = answer_len ? 1U : 0U;
	response[8] = 0;
	response[9] = 0;
	response[10] = 0;
	response[11] = include_opt ? 1U : 0U;
	if (answer_len)
		memcpy(response + question_end, answer, answer_len);
	if (include_opt) {
		size_t opt = question_end + answer_len;
		response[opt++] = 0;
		response[opt++] = 0;
		response[opt++] = 41;
		response[opt++] = 2; /* advertise 512-byte UDP payload */
		response[opt++] = 0;
		response[opt++] = extended_rcode;
		response[opt++] = 0; /* EDNS version */
		response[opt++] = 0; /* flags */
		response[opt++] = 0;
		response[opt++] = 0; /* RDLENGTH */
		response[opt++] = 0;
	}
	*response_len = required;
	return 0;
}

/* gethostname() may omit NUL when the supplied buffer is exactly filled. */
static inline int
llmnr_read_hostname(char *name, size_t capacity)
{
	int status;

	if (!name || capacity == 0)
		return -1;
	status = gethostname(name, capacity);
	name[capacity - 1] = '\0';
	return status;
}

#endif
