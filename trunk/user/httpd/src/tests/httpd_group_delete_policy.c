#include <stdio.h>
#include <string.h>

#include "httpd_group_delete_policy.h"

static int
expect_result(const char *text, size_t length, int maximum,
	      int expected_status, int expected_index)
{
	int index = -99;
	int status = httpd_group_delete_index_parse(text, length, maximum,
						    &index);

	if (status != expected_status ||
	    (status == 0 && index != expected_index)) {
		fprintf(stderr, "group delete parse mismatch (status=%d index=%d)\n",
			status, index);
		return 0;
	}
	return 1;
}

int
main(void)
{
	char max_length[255];
	char truncated_prefix[256];

	if (!expect_result("0", 1, 128, 0, 0) ||
	    !expect_result("127", 3, 128, 0, 127) ||
	    !expect_result("  +3", 4, 128, 0, 3) ||
	    !expect_result("-1", 2, 128, 1, -99) ||
	    !expect_result("", 0, 128, -1, -99) ||
	    !expect_result("3junk", 5, 128, -1, -99) ||
	    !expect_result("-2", 2, 128, -1, -99) ||
	    !expect_result("128", 3, 128, -1, -99) ||
	    !expect_result("999999999999999999999999999999", 30,
			   128, -1, -99))
		return 1;

	/* The full 255-byte buffer is accepted only when it parses completely. */
	memset(max_length, ' ', sizeof(max_length));
	max_length[sizeof(max_length) - 1] = '1';
	if (!expect_result(max_length, sizeof(max_length), 128, 0, 1))
		return 1;

	/*
	 * Old websScan() silently retained only 255 bytes. A valid leading
	 * index plus 254 spaces therefore hid any attacker-controlled suffix.
	 * The bounded parser must reject the entire oversized value.
	 */
	truncated_prefix[0] = '1';
	memset(truncated_prefix + 1, ' ', sizeof(truncated_prefix) - 1);
	if (!expect_result(truncated_prefix, sizeof(truncated_prefix),
			   128, -1, -99))
		return 1;

	puts("HTTPD group-delete index parser: PASS");
	return 0;
}
