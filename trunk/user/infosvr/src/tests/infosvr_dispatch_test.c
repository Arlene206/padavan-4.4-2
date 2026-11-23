#include <assert.h>
#include <stdio.h>
#include <string.h>

#define main infosvr_program_main
#include "../infosvr.c"
#undef main

static const unsigned char local_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

unsigned char *
get_hwaddr(char *ifname, void *buffer, int size)
{
	(void)ifname;
	if (!buffer || size < (int)sizeof(local_mac))
		return NULL;
	memcpy(buffer, local_mac, sizeof(local_mac));
	return (unsigned char *)buffer;
}

char *
get_printer(void *buffer, int size)
{
	if (size > 0)
		snprintf((char *)buffer, (size_t)size, "printer");
	return (char *)buffer;
}

char *
get_productid(void *buffer, int size)
{
	if (size > 0)
		snprintf((char *)buffer, (size_t)size, "product");
	return (char *)buffer;
}

char *
get_firmware(void *buffer, int size)
{
	if (size > 0)
		snprintf((char *)buffer, (size_t)size, "firmware");
	return (char *)buffer;
}

char *
get_ssid(char *ifname, void *buffer, int size)
{
	(void)ifname;
	if (size > 0)
		snprintf((char *)buffer, (size_t)size, "ssid");
	return (char *)buffer;
}

char *
get_netmask(char *ifname, void *buffer, int size)
{
	(void)ifname;
	if (size > 0)
		snprintf((char *)buffer, (size_t)size, "255.255.255.0");
	return (char *)buffer;
}

int
get_opmode(char *ifname)
{
	(void)ifname;
	return 1;
}

int
get_regulation(char *ifname)
{
	(void)ifname;
	return 0xff;
}

union packet_frame {
	IBOX_COMM_PKT_HDR_EX header;
	unsigned char bytes[512];
};

static void
prepare_request(union packet_frame *frame, unsigned short opcode,
                unsigned int info, const unsigned char mac[6])
{
	memset(frame, 0, sizeof(*frame));
	frame->header.ServiceID = NET_SERVICE_ID_IBOX_INFO;
	frame->header.PacketType = NET_PACKET_TYPE_CMD;
	frame->header.OpCode = opcode;
	frame->header.Info = info;
	if (mac)
		memcpy(frame->header.MacAddress, mac, 6);
}

int
main(void)
{
	static const unsigned char remote_mac_a[6] = {0x02, 0xaa, 0xbb, 0xcc, 0xdd, 0x01};
	static const unsigned char remote_mac_b[6] = {0x02, 0xaa, 0xbb, 0xcc, 0xdd, 0x02};
	union packet_frame frame;
	char unaligned_storage[sizeof(frame.bytes) + sizeof(DWORD)];
	char *unaligned_packet = unaligned_storage;
	IBOX_COMM_PKT_RES_EX *response;
	PKT_GET_INFO *info;

	prepare_request(&frame, NET_CMD_ID_SETINFO, 0x13572468U, remote_mac_a);
	assert(process_packet(-1, frame.bytes, sizeof(frame.header), "br0") == NULL);

	/* A duplicate unsupported command with a different peer must not replay a stale reply. */
	memcpy(frame.header.MacAddress, remote_mac_b, sizeof(remote_mac_b));
	assert(process_packet(-1, frame.bytes, sizeof(frame.header), "br0") == NULL);

	prepare_request(&frame, NET_CMD_ID_GETINFO, 0x24681357U, NULL);
	while ((unsigned long)unaligned_packet % sizeof(DWORD) == 0)
		++unaligned_packet;
	memcpy(unaligned_packet, frame.bytes, sizeof(frame.bytes));
	assert((unsigned long)unaligned_packet % sizeof(DWORD) != 0);
	response = (IBOX_COMM_PKT_RES_EX *)process_packet(
		-1, unaligned_packet, sizeof(IBOX_COMM_PKT_HDR), "br0");
	assert(response != NULL);
	assert(response->ServiceID == NET_SERVICE_ID_IBOX_INFO);
	assert(response->PacketType == NET_PACKET_TYPE_RES);
	assert(response->OpCode == NET_CMD_ID_GETINFO);

	/* The regular aligned-frame path must retain the same response contract. */
	response = (IBOX_COMM_PKT_RES_EX *)process_packet(
		-1, frame.bytes, sizeof(IBOX_COMM_PKT_HDR), "br0");
	assert(response != NULL);
	assert(response->ServiceID == NET_SERVICE_ID_IBOX_INFO);
	assert(response->PacketType == NET_PACKET_TYPE_RES);
	assert(response->OpCode == NET_CMD_ID_GETINFO);
	assert(response->Info == 0);
	/* GETINFO has an 8-byte base header; bytes after it are payload, not an extended MAC. */

	info = (PKT_GET_INFO *)((unsigned char *)response + sizeof(IBOX_COMM_PKT_RES));
	assert(memcmp(response->MacAddress, info->PrinterInfo, sizeof(response->MacAddress)) == 0);
	assert(memcmp(info->MacAddress, local_mac, sizeof(local_mac)) == 0);
	assert(strcmp((char *)info->SSID, "ssid") == 0);

	puts("infosvr accepts unaligned input and clears cached replies: PASS");
	return 0;
}
