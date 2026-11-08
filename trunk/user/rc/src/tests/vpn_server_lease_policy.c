#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "vpn_server_lease_policy.h"

static int
read_one(const unsigned char *bytes, size_t length, char *line, size_t line_size,
         size_t *line_length)
{
	FILE *fp = tmpfile();
	int result;

	assert(fp != NULL);
	assert(fwrite(bytes, 1, length, fp) == length);
	rewind(fp);
	result = vpn_server_lease_read_line(fp, line, line_size, line_length);
	fclose(fp);
	return result;
}

int
main(void)
{
	static const char valid[] = "ppp10 10.0.0.1 10.0.0.2 Alice Smith\n";
	static const char empty_peer[] = "ppp10 10.0.0.1 10.0.0.2 \n";
	static const char no_newline[] = "ppp10 10.0.0.1 10.0.0.2 Alice";
	static const char too_few[] = "ppp10 10.0.0.1\n";
	static const char long_ifname[] = "ppp1234567890123 10.0.0.1 10.0.0.2 Alice\n";
	static const char long_peer[] = "ppp10 10.0.0.1 10.0.0.2 1234567890123456789012345678901234567890123456789012345678901234\n";
	static const char crlf[] = "ppp10 10.0.0.1 10.0.0.2 Alice\r\n";
	static const unsigned char embedded_nul[] = {
		'p','p','p','1',' ','1','0','.','0','.','0','.','1',' ',
		'1','0','.','0','.','0','.','2',' ','A','l','i','c','e',0,'\n'
	};
	char line[VPN_SERVER_LEASE_LINE_SIZE];
	char ifname[VPN_SERVER_LEASE_IFNAME_SIZE];
	size_t line_length;

	assert(read_one((const unsigned char *)valid, sizeof(valid) - 1,
			line, sizeof(line), &line_length) == 1);
	assert(line_length == sizeof(valid) - 1);
	assert(vpn_server_lease_parse_line(line, line_length, ifname, sizeof(ifname)) == 0);
	assert(strcmp(ifname, "ppp10") == 0);
	assert(vpn_server_lease_should_keep(ifname, "ppp2"));
	assert(!vpn_server_lease_should_keep(ifname, "ppp10"));

	assert(read_one((const unsigned char *)no_newline, sizeof(no_newline) - 1,
			line, sizeof(line), &line_length) == -1);
	assert(read_one((const unsigned char *)too_few, sizeof(too_few) - 1,
			line, sizeof(line), &line_length) == 1);
	assert(vpn_server_lease_parse_line(line, line_length, ifname, sizeof(ifname)) == -1);
	assert(vpn_server_lease_parse_line(empty_peer, sizeof(empty_peer) - 1,
					  ifname, sizeof(ifname)) == -1);
	assert(vpn_server_lease_parse_line(long_ifname, sizeof(long_ifname) - 1,
					  ifname, sizeof(ifname)) == -1);
	assert(vpn_server_lease_parse_line(long_peer, sizeof(long_peer) - 1,
					  ifname, sizeof(ifname)) == -1);
	assert(vpn_server_lease_parse_line(crlf, sizeof(crlf) - 1,
					  ifname, sizeof(ifname)) == -1);
	assert(read_one(embedded_nul, sizeof(embedded_nul), line, sizeof(line),
			&line_length) == -1);

	{
		unsigned char oversized[VPN_SERVER_LEASE_LINE_SIZE + 20];
		memset(oversized, 'x', sizeof(oversized));
		oversized[sizeof(oversized) - 1] = '\n';
		assert(read_one(oversized, sizeof(oversized), line, sizeof(line),
				&line_length) == -1);
	}

	puts("VPN server lease inventory policy: PASS");
	return 0;
}
