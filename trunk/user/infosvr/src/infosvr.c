/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/stat.h>

#include <ibox.h>
#include "infosvr.h"
#include "infosvr_packet_policy.h"
#include "infosvr_signal_policy.h"
#include "infosvr_io_policy.h"
#include "infosvr_interface_policy.h"
#include "infosvr_pidfile_policy.h"

#define SRV_ADDR INADDR_ANY
#define CLN_ADDR INADDR_BROADCAST

static const char* g_pidfile = "/var/run/infosvr.pid";
static int g_pidfile_fd = -1;
static int g_fd4 = -1;
static volatile sig_atomic_t g_stop_requested = 0;

int open_socket4(char *ifname)
{
	const int int_1 = 1;
	struct sockaddr_in addr;
	struct ifreq ifr;
	struct timeval receive_timeout;
	int fd = -1;

	memset(&ifr, 0, sizeof(ifr));
	if (infosvr_interface_name_copy(
		    ifr.ifr_name, sizeof(ifr.ifr_name), ifname) < 0)
		return -1;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto error;

	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0)
		goto error;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&int_1, sizeof(int_1)) < 0)
		goto error;

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *)&int_1, sizeof(int_1)) < 0)
		goto error;

	/* Bound recvfrom() too: a stop signal can arrive just before the call. */
	receive_timeout.tv_sec = INFOSVR_SOCKET_TIMEOUT_SECONDS;
	receive_timeout.tv_usec = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&receive_timeout,
		       sizeof(receive_timeout)) < 0)
		goto error;
	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&receive_timeout,
		       sizeof(receive_timeout)) < 0)
		goto error;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(SRV_ADDR);
	addr.sin_port = htons(IBOX_SRV_PORT);
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		goto error;

	return fd;

error:
	if (fd >= 0)
		close(fd);
	return -1;
}

int read_packet(int fd, void *packet, int size)
{
	int res;

	if (size <= 0) {
		errno = EINVAL;
		return -1;
	}

	if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested)) {
		errno = EINTR;
		return -1;
	}

	if (infosvr_packet_buffer_prepare(packet, (size_t)size) < 0)
		return -1;

	do {
		res = recvfrom(fd, packet, (size_t)size, MSG_TRUNC, NULL, NULL);
	} while (res < 0 && errno == EINTR &&
		 !INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested));

	return res;
}

int send_packet(int fd, void *packet, int size)
{
	struct sockaddr_in addr;
	int res = 0;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(CLN_ADDR);
	addr.sin_port = htons(IBOX_CLI_PORT);

	if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested)) {
		errno = EINTR;
		return -1;
	}

	while ((res = sendto(fd, packet, size, 0, (struct sockaddr *)&addr, sizeof(addr))) < 0 &&
	       errno == EINTR && !INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))
		continue;

	return res;
}

void *process_packet(int fd, void *packet, int packet_len, char *ifname)
{
	static char packet_res[512] = {0};
	IBOX_COMM_PKT_HDR_EX *hdr = packet;
	IBOX_COMM_PKT_RES_EX *hdr_res = (IBOX_COMM_PKT_RES_EX *) &packet_res;
	PKT_GET_INFO *info;

	if (packet_len < 0 ||
	    !infosvr_packet_length_valid((size_t)packet_len, sizeof(IBOX_COMM_PKT_HDR)))
		return NULL;

	if (hdr->ServiceID != NET_SERVICE_ID_IBOX_INFO ||
	    hdr->PacketType != NET_PACKET_TYPE_CMD)
		return NULL;

	if (hdr->OpCode != NET_CMD_ID_GETINFO && hdr->OpCode != NET_CMD_ID_GETINFO_MANU &&
	    !infosvr_packet_length_valid((size_t)packet_len,
					 offsetof(IBOX_COMM_PKT_HDR_EX, Password)))
		return NULL;

	/* Only the implemented read-only opcodes are allowed to produce replies. */
	if (hdr->OpCode != NET_CMD_ID_GETINFO && hdr->OpCode != NET_CMD_ID_GETINFO_MANU)
		return NULL;

	/* This response buffer is reused; clear every field before building a reply. */
	memset(packet_res, 0, sizeof(packet_res));
	hdr_res->ServiceID = NET_SERVICE_ID_IBOX_INFO;
	hdr_res->PacketType = NET_PACKET_TYPE_RES;
	hdr_res->OpCode = hdr->OpCode;

	switch (hdr->OpCode) {
	case NET_CMD_ID_GETINFO:
	case NET_CMD_ID_GETINFO_MANU:
		info = (PKT_GET_INFO *)(packet_res + sizeof(IBOX_COMM_PKT_RES));
		memset(info, 0, sizeof(*info));
		get_printer(info->PrinterInfo, sizeof(info->PrinterInfo));
		get_productid(info->ProductID, sizeof(info->ProductID));
		get_firmware(info->FirmwareVersion, sizeof(info->FirmwareVersion));
		get_ssid(ifname, info->SSID, sizeof(info->SSID));
		get_netmask(ifname, info->NetMask, sizeof(info->NetMask));
		get_hwaddr(ifname, info->MacAddress, sizeof(info->MacAddress));
		info->OperationMode = get_opmode(ifname);
		info->Regulation = get_regulation(ifname);
		break;
	default:
		return NULL;
	}

	return hdr_res;
}

void res_cleanup(void)
{
	if (g_fd4 >= 0) {
		close(g_fd4);
		g_fd4 = -1;
	}

	if (g_pidfile_fd >= 0) {
		struct stat opened_status, path_status;

		/*
		 * A stale or replaced PID file is not ours to remove. The lock
		 * remains held until after the owner check and conditional unlink.
		 */
		if (infosvr_pidfile_matches(g_pidfile_fd, getpid()) == 1 &&
		    fstat(g_pidfile_fd, &opened_status) == 0 &&
		    lstat(g_pidfile, &path_status) == 0 &&
		    S_ISREG(path_status.st_mode) &&
		    opened_status.st_dev == path_status.st_dev &&
		    opened_status.st_ino == path_status.st_ino &&
		    opened_status.st_nlink == 1)
			unlink(g_pidfile);

		close(g_pidfile_fd);
		g_pidfile_fd = -1;
	}
}

static int
acquire_pidfile(void)
{
	struct stat status;
	int fd, saved_errno;

	/*
	 * Do not use O_TRUNC here: a competing daemon must acquire the inode
	 * lock before the active PID record can be replaced.
	 */
	fd = open(g_pidfile, O_RDWR | O_CREAT | O_NOFOLLOW, 0644);
	if (fd < 0)
		return -1;

	if (infosvr_pidfile_try_exclusive_lock(fd) != 0)
		goto fail;
	if (fstat(fd, &status) != 0)
		goto fail;
	if (!S_ISREG(status.st_mode) || status.st_nlink != 1) {
		errno = EINVAL;
		goto fail;
	}
	if (infosvr_pidfile_write(fd, getpid()) != 0)
		goto fail;

	g_pidfile_fd = fd;
	return 0;

fail:
	saved_errno = errno;
	close(fd);
	errno = saved_errno;
	return -1;
}

static void catch_sig_infosvr(int sig)
{
	(void)sig;
	INFOSVR_SIGNAL_REQUEST_STOP(g_stop_requested);
}

int main(int argc, char *argv[])
{
	char packet[512], *reply, *ifname;
	int max_fd, res;
	int exit_status = 0, saved_errno = 0;
	fd_set rfds, active_rfds;
	struct timeval select_timeout;
	struct sigaction sigterm_action;

	if (!argv[1]) {
		fprintf(stderr, "usage: %s {ifname}\n", argv[0]);
		return 1;
	}

	ifname = argv[1];

	g_fd4 = open_socket4(ifname);
	if (g_fd4 < 0) {
		perror("open_socket");
		return 1;
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	memset(&sigterm_action, 0, sizeof(sigterm_action));
	sigterm_action.sa_handler = catch_sig_infosvr;
	/* Do not SA_RESTART select()/recvfrom(); the main loop must see SIGTERM. */
	sigterm_action.sa_flags = 0;
	if (sigemptyset(&sigterm_action.sa_mask) != 0 ||
	    sigaction(SIGTERM, &sigterm_action, NULL) != 0) {
		perror("sigaction");
		close(g_fd4);
		g_fd4 = -1;
		return 1;
	}

	if (daemon(0, 0) < 0) {
		close(g_fd4);
		perror("daemon");
		exit(errno);
	}

	/* The daemonized child must own the singleton PID-file lock before serving. */
	if (acquire_pidfile() != 0) {
		int saved_errno = errno;

		if (g_fd4 >= 0) {
			close(g_fd4);
			g_fd4 = -1;
		}
		errno = saved_errno;
		perror("pidfile");
		return 1;
	}

	FD_ZERO(&rfds);
	FD_SET(g_fd4, &rfds);
	max_fd = g_fd4;

	while (!INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested)) {
		active_rfds = rfds;
		/* select() mutates timeval; reset it for each bounded wait. */
		select_timeout.tv_sec = INFOSVR_SELECT_TIMEOUT_SECONDS;
		select_timeout.tv_usec = 0;
		res = select(max_fd + 1, &active_rfds, NULL, NULL, &select_timeout);
		if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))
			break;
		if (res < 0 && infosvr_io_error_retryable(errno))
			continue;
		if (res < 0) {
			saved_errno = errno ? errno : EIO;
			errno = saved_errno;
			perror("select");
			exit_status = 1;
			break;
		}
		if (res == 0 || !FD_ISSET(g_fd4, &active_rfds))
			continue;

		res = read_packet(g_fd4, packet, sizeof(packet));
		if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))
			break;
		if (res < 0) {
			if (infosvr_io_error_retryable(errno))
				continue;
			saved_errno = errno ? errno : EIO;
			errno = saved_errno;
			perror("recvfrom");
			exit_status = 1;
			break;
		}
		/* MSG_TRUNC reports the full UDP datagram length on Linux. */
		if (!infosvr_packet_length_fits((size_t)res, sizeof(packet)))
			continue;
		if (res < (int)sizeof(IBOX_COMM_PKT_HDR))
			continue;

		reply = process_packet(g_fd4, packet, res, ifname);
		if (INFOSVR_SIGNAL_STOP_PENDING(g_stop_requested))
			break;
		if (reply)
			send_packet(g_fd4, reply, sizeof(packet));
	}

	res_cleanup();
	if (exit_status != 0)
		errno = saved_errno ? saved_errno : EIO;

	return exit_status;
}
