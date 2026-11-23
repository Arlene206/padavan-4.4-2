#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

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
	failures += check(infosvr_packet_length_fits(512, 512),
			  "datagram filling the receive buffer is accepted");
	failures += check(!infosvr_packet_length_fits(513, 512),
			  "oversized datagram is rejected");
	failures += check(!infosvr_packet_length_fits(1, 0),
			  "zero receive capacity is rejected");


static int
check_msg_trunc_reports_full_udp_length(void)
{
	int receiver = -1, sender = -1, failures = 0;
	struct sockaddr_in address;
	socklen_t address_length = sizeof(address);
	char payload[700], receive_buffer[512];
	ssize_t sent, received;

	receiver = socket(AF_INET, SOCK_DGRAM, 0);
	sender = socket(AF_INET, SOCK_DGRAM, 0);
	failures += check(receiver >= 0 && sender >= 0,
			  "loopback UDP sockets are created");
	if (receiver < 0 || sender < 0)
		goto cleanup;

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = 0;
	failures += check(bind(receiver, (struct sockaddr *)&address,
			      sizeof(address)) == 0,
			  "loopback UDP receiver binds");
	if (failures)
		goto cleanup;

	failures += check(getsockname(receiver, (struct sockaddr *)&address,
				      &address_length) == 0,
			  "bound UDP port is retrieved");
	if (failures)
		goto cleanup;

	memset(payload, 0x5a, sizeof(payload));
	sent = sendto(sender, payload, sizeof(payload), 0,
		      (struct sockaddr *)&address, sizeof(address));
	failures += check(sent == (ssize_t)sizeof(payload),
			  "oversized UDP datagram is queued");
	if (sent != (ssize_t)sizeof(payload))
		goto cleanup;

	received = recvfrom(receiver, receive_buffer, sizeof(receive_buffer),
			    MSG_TRUNC, NULL, NULL);
	failures += check(received == (ssize_t)sizeof(payload),
			  "MSG_TRUNC reports original datagram length");

cleanup:
	if (sender >= 0)
		close(sender);
	if (receiver >= 0)
		close(receiver);
	return failures;
}

	failures += check_msg_trunc_reports_full_udp_length();
	if (failures)
		return 1;

	puts("infosvr packet initialization, length and UDP truncation policy: PASS");
	return 0;
}
