#include <stdio.h>
#include <string.h>

#include "infosvr_packet_policy.h"

static int
check(int condition, const char *message)
{
	if (!condition) {
		fprintf(stderr, "FAIL: %s\n", message);
		return 1;
	}

	return 0;
}

int
main(void)
{
	unsigned char packet[512];
	size_t i;
	int failures = 0;

	memset(packet, 0xa5, sizeof(packet));
	failures += check(infosvr_packet_buffer_prepare(packet, sizeof(packet)) == 0,
			  "full receive buffer is initialized");

	for (i = 0; i < sizeof(packet); ++i)
		failures += check(packet[i] == 0,
				  "all bytes including the unused tail are zero");

	packet[0] = 0x0c;
	packet[1] = 0x15;
	failures += check(packet[0] == 0x0c && packet[1] == 0x15 && packet[2] == 0,
			  "short datagram retains deterministic zero padding");

	errno = 0;
	failures += check(infosvr_packet_buffer_prepare(NULL, sizeof(packet)) == -1 &&
			  errno == EINVAL, "NULL receive buffer is rejected");
	errno = 0;
	failures += check(infosvr_packet_buffer_prepare(packet, 0) == -1 &&
			  errno == EINVAL, "zero receive-buffer size is rejected");

	failures += check(!infosvr_packet_length_valid(7, 8),
			  "truncated base IBOX header is rejected");
	failures += check(infosvr_packet_length_valid(8, 8),
			  "complete base IBOX header is accepted");
	failures += check(!infosvr_packet_length_valid(13, 14),
			  "truncated extended MAC prefix is rejected");
	failures += check(infosvr_packet_length_valid(14, 14),
			  "complete extended MAC prefix is accepted");
	failures += check(!infosvr_packet_length_valid(512, 0),
			  "zero required header length is rejected");

	if (failures)
		return 1;

	puts("infosvr packet initialization and length policy: PASS");
	return 0;
}
