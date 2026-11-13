#ifndef XT_WEBSTR_PARSER_POLICY_H
#define XT_WEBSTR_PARSER_POLICY_H

/* Shared allocation-free bounds checks; keep GNU89 kernel compatibility. */
static inline int
xt_webstr_span_is_valid(unsigned int total, unsigned int offset,
			unsigned int length)
{
	return offset <= total && length <= total - offset;
}

static inline int
xt_webstr_find_terminator(const unsigned char *data, unsigned int data_len,
			  unsigned int start, unsigned char terminator,
			  unsigned int *value_len)
{
	unsigned int i;

	if (!data || !value_len || start > data_len)
		return -1;
	for (i = start; i < data_len; ++i) {
		if (data[i] == terminator) {
			*value_len = i - start;
			return 0;
		}
	}
	return -1;
}

static inline unsigned int
xt_webstr_copy_length(unsigned int requested, unsigned int destination_size)
{
	if (!destination_size)
		return 0;
	return requested < destination_size ? requested : destination_size - 1;
}

static inline unsigned int
xt_webstr_url_path_length(unsigned int host_len, unsigned int path_len,
			  unsigned int url_size)
{
	unsigned int available;

	if (!url_size || host_len >= url_size)
		return 0;
	available = url_size - host_len - 1;
	return path_len < available ? path_len : available;
}

/* Return 1 and advance offset for one token; return 0 at end, -1 on bad input. */
static inline int
xt_webstr_next_token(const char *list, unsigned int list_len,
		     unsigned int *offset, const char *separator,
		     unsigned int separator_len, unsigned int *token_len)
{
	unsigned int start, end;

	if (!list || !offset || !separator || !separator_len || !token_len ||
	    *offset > list_len)
		return -1;
	if (*offset == list_len)
		return 0;

	start = *offset;
	end = start;
	while (end < list_len) {
		if (separator_len <= list_len - end &&
		    memcmp(list + end, separator, separator_len) == 0)
			break;
		++end;
	}
	*token_len = end - start;
	*offset = end < list_len ? end + separator_len : list_len;
	return 1;
}

static inline int
xt_webstr_ipv6_fragment_is_initial(unsigned int fragment_offset)
{
	/* The low three bits are reserved/M; bits 3..15 carry fragment offset. */
	return (fragment_offset & ~0x7U) == 0;
}

static inline int
xt_webstr_tcp_header_is_valid(unsigned int packet_len, unsigned int tcp_offset,
			      unsigned int tcp_header_len)
{
	return tcp_header_len >= 20 &&
	       xt_webstr_span_is_valid(packet_len, tcp_offset, tcp_header_len);
}

#endif
