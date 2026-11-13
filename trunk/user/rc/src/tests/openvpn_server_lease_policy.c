#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "openvpn_server_lease_policy.h"

static void
expect_read(FILE *fp, const char *expected)
{
	char actual[512];
	size_t length;

	assert(fflush(fp) == 0);
	rewind(fp);
	length = fread(actual, 1, sizeof(actual) - 1, fp);
	actual[length] = '\0';
	assert(strcmp(actual, expected) == 0);
}

int
main(void)
{
	FILE *source, *destination;
	int trailing_newline = 0;
	char long_line[320];

	assert(openvpn_server_lease_pid_matches("4321", 4, (pid_t)4321) == 1);
	assert(openvpn_server_lease_pid_matches("4321\n", 5, (pid_t)4321) == 1);
	assert(openvpn_server_lease_pid_matches("4321", 4, (pid_t)4322) == 0);
	errno = 0;
	assert(openvpn_server_lease_pid_matches("1", 1, (pid_t)1) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(openvpn_server_lease_pid_matches("43x1", 4, (pid_t)4321) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(openvpn_server_lease_pid_matches("999999999999999999999999", 24,
		(pid_t)4321) == -1);
	assert(errno == ERANGE);

	assert(openvpn_server_lease_line_matches(
		"tun11 10.8.0.2 198.51.100.7 laptop\n",
		"10.8.0.2", "198.51.100.7") == 1);
	assert(openvpn_server_lease_line_matches(
		"tun11 10.8.0.2 198.51.100.7 laptop\n",
		"10.8.0.3", "198.51.100.7") == 0);
	assert(openvpn_server_lease_line_matches(
		"tun11 10.8.0.2 198.51.100.7 laptop\n",
		"10.8.0.2", "198.51.100.8") == 0);
	errno = 0;
	assert(openvpn_server_lease_line_matches("malformed\n", "10.8.0.2", "198.51.100.7") == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(openvpn_server_lease_line_matches(NULL, "10.8.0.2", "198.51.100.7") == -1);
	assert(errno == EINVAL);

	source = tmpfile();
	destination = tmpfile();
	assert(source != NULL && destination != NULL);
	assert(fputs("tun11 10.8.0.2 198.51.100.7 laptop\n", source) >= 0);
	assert(fputs("tun11 10.8.0.3 198.51.100.8 phone\n", source) >= 0);
	rewind(source);
	assert(openvpn_server_lease_copy(source, destination,
		"10.8.0.2", "198.51.100.7", 1, &trailing_newline) == 0);
	assert(trailing_newline == 1);
	expect_read(destination, "tun11 10.8.0.3 198.51.100.8 phone\n");
	fclose(source);
	fclose(destination);

	source = tmpfile();
	destination = tmpfile();
	assert(source != NULL && destination != NULL);
	assert(fputs("tun11 10.8.0.2 198.51.100.7 laptop", source) >= 0);
	rewind(source);
	assert(openvpn_server_lease_copy(source, destination, NULL, NULL, 0,
		&trailing_newline) == 0);
	assert(trailing_newline == 0);
	expect_read(destination, "tun11 10.8.0.2 198.51.100.7 laptop");
	fclose(source);
	fclose(destination);

	memset(long_line, 'x', sizeof(long_line));
	long_line[sizeof(long_line) - 1] = '\0';
	source = tmpfile();
	destination = tmpfile();
	assert(source != NULL && destination != NULL);
	assert(fputs(long_line, source) >= 0);
	rewind(source);
	errno = 0;
	assert(openvpn_server_lease_copy(source, destination, NULL, NULL, 0,
		&trailing_newline) == -1);
	assert(errno == EOVERFLOW);
	fclose(source);
	fclose(destination);

	source = tmpfile();
	destination = tmpfile();
	assert(source != NULL && destination != NULL);
	assert(fwrite("tun11 10.8.0.2 198.51.100.7 laptop\0ignored\n", 1,
		sizeof("tun11 10.8.0.2 198.51.100.7 laptop\0ignored\n") - 1,
		source) == sizeof("tun11 10.8.0.2 198.51.100.7 laptop\0ignored\n") - 1);
	rewind(source);
	errno = 0;
	assert(openvpn_server_lease_copy(source, destination, NULL, NULL, 0,
		&trailing_newline) == -1);
	assert(errno == EINVAL);
	expect_read(destination, "");
	fclose(source);
	fclose(destination);

	puts("OpenVPN server lease snapshot parsing and filtering policy: PASS");
	return 0;
}
