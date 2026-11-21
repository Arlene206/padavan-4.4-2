#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "wsd_http_header_policy.h"

static void
test_find_crlf_bounds(void)
{
	char raw[] = {'a', 'b', '\r', '\n', 'x'};

	assert(wsd_http_find_crlf(NULL, sizeof(raw)) == NULL);
	assert(wsd_http_find_crlf(raw, 0) == NULL);
	assert(wsd_http_find_crlf(raw, 3) == NULL);
	assert(wsd_http_find_crlf(raw, 4) == raw + 2);
	assert(wsd_http_find_crlf(raw, sizeof(raw)) == raw + 2);
}

static void
test_content_length(void)
{
	size_t length = 77;

	assert(wsd_http_parse_content_length("12", &length) == 0);
	assert(length == 12);
	assert(wsd_http_parse_content_length("  4096\t", &length) == 0);
	assert(length == 4096);
	length = 77;
	assert(wsd_http_parse_content_length("0", &length) == -1);
	assert(errno == EINVAL && length == 77);
	assert(wsd_http_parse_content_length("-1", &length) == -1);
	assert(wsd_http_parse_content_length("+1", &length) == -1);
	assert(wsd_http_parse_content_length("12x", &length) == -1);
	assert(wsd_http_parse_content_length("12 3", &length) == -1);
	assert(wsd_http_parse_content_length(
		"999999999999999999999999999999999", &length) == -1);
	assert(errno == ERANGE);
	assert(wsd_http_parse_content_length("1", NULL) == -1);
}

static void
test_fragmented_line_and_body(void)
{
	int sockets[2];
	pid_t child;
	char buffer[64];
	size_t length = 0;
	const char expected[] = "POST /device HTTP/1.1\r\n";
	const char body[] = "<soap/>";
	size_t body_length = sizeof(body) - 1;

	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
	child = fork();
	assert(child >= 0);
	if (child == 0) {
		close(sockets[0]);
		assert(write(sockets[1], "POST /de", 8) == 8);
		usleep(20000);
		assert(write(sockets[1], "vice HTTP/1.1\r", 14) == 14);
		usleep(20000);
		assert(write(sockets[1], "\n", 1) == 1);
		assert(write(sockets[1], body, body_length) == (ssize_t)body_length);
		close(sockets[1]);
		_exit(0);
	}
	close(sockets[1]);
	assert(wsd_http_read_until_crlf(sockets[0], buffer, &length,
	                                sizeof(buffer)) == 0);
	{
		char *line_end = wsd_http_find_crlf(buffer, length);
		size_t consumed;
		assert(line_end != NULL);
		assert((size_t)(line_end - buffer) == strlen(expected) - 2);
		consumed = (size_t)(line_end + 2 - buffer);
		length -= consumed;
		memmove(buffer, buffer + consumed, length);
	}
	assert(wsd_http_read_exact(sockets[0], buffer, &length,
	                           body_length) == 0);
	assert(length == body_length);
	assert(memcmp(buffer, body, body_length) == 0);
	close(sockets[0]);
	{
		int status;
		assert(waitpid(child, &status, 0) == child);
		assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	}
}

static void
test_unterminated_line_is_bounded(void)
{
	int sockets[2];
	char buffer[5] = "abcd";
	size_t length = 4;

	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
	assert(wsd_http_read_until_crlf(sockets[0], buffer, &length,
	                                sizeof(buffer)) == -1);
	assert(errno == EMSGSIZE);
	close(sockets[0]);
	close(sockets[1]);
}

static void
test_eof_is_reported(void)
{
	int sockets[2];
	char buffer[16];
	size_t length = 0;

	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
	close(sockets[1]);
	assert(wsd_http_read_until_crlf(sockets[0], buffer, &length,
	                                sizeof(buffer)) == -1);
	assert(errno == ECONNRESET);
	close(sockets[0]);
}

int
main(void)
{
	test_find_crlf_bounds();
	test_content_length();
	test_fragmented_line_and_body();
	test_unterminated_line_is_bounded();
	test_eof_is_reported();
	return 0;
}
