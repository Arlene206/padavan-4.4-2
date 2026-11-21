#include <assert.h>
#include <string.h>

#include "../xt_webstr_parser_policy.h"

int
main(void)
{
	const unsigned char unterminated[] = { 'H', 'o', 's', 't', ':', ' ', 'x' };
	const unsigned char terminated[] = { 'a', 'b', '\r' };
	const char words[] = "alpha<&nbsp;>beta<&nbsp;>gamma";
	const char empty_word[] = "<&nbsp;>one";
	const char separator[] = "<&nbsp;>";
	unsigned int value_len = 99, offset, token_len;

	assert(xt_webstr_find_terminator(unterminated,
					 sizeof(unterminated), 0, '\r',
					 &value_len) == -1);
	assert(xt_webstr_find_terminator(terminated,
					 sizeof(terminated), 0, '\r',
					 &value_len) == 0);
	assert(value_len == 2);
	assert(xt_webstr_find_terminator(terminated,
					 sizeof(terminated), 3, '\r',
					 &value_len) == -1);

	assert(xt_webstr_copy_length(256, 256) == 255);
	assert(xt_webstr_copy_length(12, 256) == 12);
	assert(xt_webstr_copy_length(12, 0) == 0);
	assert(xt_webstr_url_path_length(255, 900, 1024) == 768);
	assert(xt_webstr_url_path_length(10, 10, 1024) == 10);
	assert(xt_webstr_url_path_length(1023, 1, 1024) == 0);
	assert(xt_webstr_url_path_length(1024, 1, 1024) == 0);

	offset = 0;
	assert(xt_webstr_next_token(words, sizeof(words) - 1, &offset,
				    separator, sizeof(separator) - 1,
				    &token_len) == 1);
	assert(token_len == 5 && offset == 13);
	assert(xt_webstr_next_token(words, sizeof(words) - 1, &offset,
				    separator, sizeof(separator) - 1,
				    &token_len) == 1);
	assert(token_len == 4);
	assert(xt_webstr_next_token(words, sizeof(words) - 1, &offset,
				    separator, sizeof(separator) - 1,
				    &token_len) == 1);
	assert(token_len == 5);
	assert(xt_webstr_next_token(words, sizeof(words) - 1, &offset,
				    separator, sizeof(separator) - 1,
				    &token_len) == 0);
	offset = 0;
	assert(xt_webstr_next_token(empty_word, sizeof(empty_word) - 1,
				    &offset, separator, sizeof(separator) - 1,
				    &token_len) == 1);
	assert(token_len == 0 && offset == sizeof(separator) - 1);
	offset = sizeof(words);
	assert(xt_webstr_next_token(words, sizeof(words) - 1,
				    &offset, separator, sizeof(separator) - 1,
				    &token_len) == -1);

	assert(xt_webstr_ipv6_fragment_is_initial(0));
	assert(xt_webstr_ipv6_fragment_is_initial(1)); /* More fragments bit. */
	assert(xt_webstr_ipv6_fragment_is_initial(7));
	assert(!xt_webstr_ipv6_fragment_is_initial(8));
	assert(!xt_webstr_ipv6_fragment_is_initial(0xfff8));

	assert(xt_webstr_span_is_valid(80, 40, 40));
	assert(!xt_webstr_span_is_valid(79, 40, 40));
	assert(!xt_webstr_span_is_valid(80, 81, 0));
	assert(xt_webstr_tcp_header_is_valid(80, 40, 20));
	assert(!xt_webstr_tcp_header_is_valid(80, 40, 19));
	assert(!xt_webstr_tcp_header_is_valid(59, 40, 20));
	assert(!xt_webstr_tcp_header_is_valid(80, 70, 20));

	return 0;
}
