/*
 * lanauth client
 * (c) visir
 * (c) 2009 theMIROn
 * "THE BEER-WARE LICENSE" (Revision 43):
 * <visir@telenet.ru> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.                    
 * compile: gcc -O2 -Wall -s -o lanauth lanauth.c
 * run: ./lanauth -p yourpassword
 */

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<syslog.h>

#include "lanauth_credential_file_policy.h"
#include "lanauth_privilege_drop_policy.h"
#include "lanauth_socket_io_policy.h"

#define LTC_NO_CIPHERS
#define LTC_NO_MODES
#define LTC_NO_HASHES
#define LTC_NO_MACS
#define LTC_NO_PRNGS
#define LTC_NO_PK
#define LTC_NO_PKCS
#define LTC_NO_TEST
#define MD5
#define RIPEMD160
#include<tomcrypt.h>

static int nodaemon = 0;		/* don't become daemon */
static int nobind = 1;			/* don't really bind, just assume we binded */
static int nolip = 0;			/* don't touch localip */
static int ver = 2;			/* protocol version */
static char level = 2;			/* access level */
static int sock = -1;			/* socket */
static char localip[17] = "";		/* local ip adress */
static char gateip[17] = "10.0.0.1";	/* gateway ip adress */
static char pass[21] = "";		/* password */
static unsigned char challenge[256], digest[256];

static void opensock();		/* connect to server */
static void auth1();			/* generate response for v1 protocol */
static void auth2();			/* generate response for v2 protocol */
static void sigusr(int sig);		/* change access level */
static int tmread(char *buf, int size, int timeout);	/* read with timeout */

static void usage() {
	printf("Usage: lanauth [-i] [-v 1|2] [-b localip] [-n] [-g gid] [-u uid] [-s gateip] [-l accesslevel] (-f password-file | -p password)\n");
	exit(0);
}

static void fatal(char *s, ...) {
	va_list ap;
	va_start(ap, s);
	vsyslog(LOG_ERR, s, ap);
	va_end(ap);
	exit(1);
}

static int
lanauth_set_group(void *context, gid_t gid)
{
    (void)context;
    return setgid(gid);
}
static int
lanauth_set_user(void *context, uid_t uid)
{
    (void)context;
    return setuid(uid);
}
static int
lanauth_drop_privileges(int change_gid, gid_t gid,
                        int change_uid, uid_t uid,
                        enum lanauth_privilege_drop_stage *failure_stage)
{
    return lanauth_apply_privilege_drop_with_ops(
        change_gid, gid, change_uid, uid,
        lanauth_set_group, lanauth_set_user, NULL, failure_stage);
}

static int
lanauth_socket_wait_readable(void *context, int fd, int timeout_seconds)
{
    fd_set read_fds;
    struct timeval timeout;
    int status;

    (void)context;
    if (fd < 0 || fd >= FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;
    status = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    if (status > 0 && !FD_ISSET(fd, &read_fds)) {
        errno = EIO;
        return -1;
    }
    return status;
}

static ssize_t
lanauth_socket_read_data(void *context, int fd, void *buffer, size_t size)
{
    (void)context;
    return read(fd, buffer, size);
}

static ssize_t
lanauth_socket_write_data(void *context, int fd, const void *buffer,
                          size_t size)
{
    (void)context;
    return write(fd, buffer, size);
}


int main(int argc, char **argv) {
	char *s;
	char *password_file = NULL;
	int password_argument = 0;
	uid_t requested_uid = 0;
	gid_t requested_gid = 0;
	int change_uid = 0;
	int change_gid = 0;
	int privilege_status;
	enum lanauth_privilege_drop_stage privilege_failure_stage;
	int op;
	int socket_io_status;
	unsigned char ch;
	unsigned char curlevel = 0xff;
	char redirected_gateip[sizeof(gateip)];
	size_t sent_size;
	/* process command line arguments */
	while(1) {
		op = getopt(argc, argv, "iv:b:ns:p:f:l:u:g:");
		if (op == -1) break;
		switch(op) {
			case 'i':
				nodaemon = 1;
				break;
			case 'v':
				ver = atoi(optarg);
				if (ver != 1 && ver != 2) usage();
				break;
			case 'b':
				strncpy(localip, optarg, sizeof(localip)-1);
				localip[sizeof(localip)-1] = 0;
				nobind = 0;
				nolip = 1;
				break;
			case 'n':
				nobind = 1;
				if (!nolip) usage();
				break;
			case 's':
				strncpy(gateip, optarg, sizeof(gateip)-1);
				gateip[sizeof(gateip)-1] = 0;
				break;
			case 'p':
				if (password_file || password_argument) usage();
				strncpy(pass, optarg, sizeof(pass)-1);
				pass[sizeof(pass)-1] = 0;
				/* Preserve the legacy CLI; RC uses the private -f transport. */
				for (s = optarg; *s; s++) *s = '*';
				password_argument = 1;
				break;
			case 'f':
				if (password_file || password_argument) usage();
				password_file = optarg;
				break;
			case 'l':
				level = atoi(optarg);
				break;
			case 'u':
				requested_uid = (uid_t)atoi(optarg);
				change_uid = 1;
				break;
			case 'g':
				requested_gid = (gid_t)atoi(optarg);
				change_gid = 1;
				break;
			default:
				usage();
		}
	}

	if (password_file) {
		int password_status = lanauth_read_private_password_file(
			password_file, pass, sizeof(pass));
		if (password_status != 0) {
			fprintf(stderr, "lanauth: cannot read private password file (status=%d)\n",
				password_status);
			return 1;
		}
	} else if (!pass || !*pass) {
		usage();
	}

	openlog("lanauth", nodaemon ? LOG_PERROR|LOG_PID : LOG_PID, LOG_DAEMON);

	privilege_status = lanauth_drop_privileges(
		change_gid, requested_gid, change_uid, requested_uid,
		&privilege_failure_stage);
	if (privilege_status != 0) {
		if (privilege_failure_stage == LANAUTH_PRIVILEGE_DROP_STAGE_GID)
			fatal("setgid(%lu) failed (status=%d)",
			      (unsigned long)requested_gid, privilege_status);
		if (privilege_failure_stage == LANAUTH_PRIVILEGE_DROP_STAGE_UID)
			fatal("setuid(%lu) failed (status=%d)",
			      (unsigned long)requested_uid, privilege_status);
		fatal("cannot apply requested privilege drop (status=%d)",
		      privilege_status);
	}

	signal(SIGUSR1, sigusr);
	signal(SIGUSR2, sigusr);
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		fatal("signal(SIGPIPE): %m");
	if (!nodaemon) {
		signal(SIGHUP, SIG_IGN);
		daemon(0, 0);
	}

	/* main loop */
	while(1) {
		curlevel = 0xff;
		ch = 0;
		/* connect to server */
		opensock();
		/* start conversation */
		if (!tmread(&ch, 1, 10)) {
			close(sock);
			sleep(5);
			continue;
		}
		switch(ch) {
			case 0:	/* access closed */
				syslog(LOG_NOTICE, "access closed for us");
				close(sock);
				sleep(60);
			case 1: /* continue authorization */
				break;
			case 2: /* redirect to real server */
				if (!tmread((char *)&ch, 1, 10)) {
					close(sock);
					sleep(5);
					continue;
				}
				if(ch < 7 || ch > 15)
					fatal("redirect: invalid gateway length %d", ch);
				if (!tmread(redirected_gateip, (int)ch, 10)) {
					close(sock);
					sleep(5);
					continue;
				}
				redirected_gateip[ch] = '\0';
				memcpy(gateip, redirected_gateip, (size_t)ch + 1);
				syslog(LOG_NOTICE, "gate changed to %s", gateip);
				close(sock);
				break;
			default:
				close(sock);
				syslog(LOG_NOTICE, "unknown protocol %d", ch);
				sleep(60);
				break;
		}
		if (ch != 1) continue;
		/* main loop of challenge-response authorization */
		while(1) {
			if (!tmread(challenge, sizeof(challenge), 240)) {
				close(sock);
				if (curlevel == 0xff) sleep(5);
				break;
			}
			switch(ver) {
				case 1: auth1(); break;
				case 2: auth2(); break;
			}
			socket_io_status = lanauth_write_all_with_ops(
				sock, digest, (ver == 1) ? 16U : 256U,
				lanauth_socket_write_data, NULL, &sent_size);
			if (socket_io_status != 0) {
				syslog(LOG_NOTICE,
				       "auth response write incomplete (%lu/%u bytes, status=%d); reconnecting",
				       (unsigned long)sent_size,
				       (unsigned)((ver == 1) ? 16 : 256), socket_io_status);
				close(sock);
				sleep((curlevel == 0xff) ? 60 : 5);
				break;
			}
			if (!tmread(&ch, 1, 10)) {
				close(sock);
				syslog(LOG_NOTICE, "auth unsuccessful");
				sleep((curlevel == 0xff) ? 60 : 5);
				break;
			}
			if (curlevel != ch) {
				curlevel = ch;
				syslog(LOG_NOTICE, "auth successful, access level = %d", curlevel);
			}
		}
	}
}

static void sigusr(int sig) {
	switch(sig) {
		case SIGUSR1: level = 1; break;
		case SIGUSR2: level = 2; break;
	}
}

static void opensock() {
	struct sockaddr_in sin;
	int len;
	while (1) {
		/* create socket */
		if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		    fatal("socket: %m");

		/* bind it to specified ip, if needed */
		if (*localip && !nobind) {
			bzero(&sin, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_addr.s_addr = inet_addr(localip);
			sin.sin_port = 0;
			if(sin.sin_addr.s_addr == INADDR_NONE)
				fatal("%s: invalid ip address", localip);
			if(bind(sock, (struct sockaddr*)&sin, sizeof(sin)) == -1)
				fatal("bind: %m");
		}
	
		/* connect to server */
		bzero(&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(gateip);
		sin.sin_port = htons((ver == 1) ? 8899 : 8314);
		if (sin.sin_addr.s_addr == INADDR_NONE)
			fatal("%s: invalid ip address", gateip);

		if (connect(sock, (struct sockaddr*)&sin, sizeof(sin)) == -1) {
			syslog(LOG_NOTICE, "connect: %m");
			sleep(30);
			close(sock);
			continue;
		}
		break;
	}

	/* save localip, if needed */
	len = sizeof(sin);
	getsockname(sock, (struct sockaddr*)&sin, &len);
	if (!nolip)
		strcpy(localip, inet_ntoa(sin.sin_addr));
	syslog(LOG_NOTICE, "connected, gateway %s, local ip %s", gateip, localip);
}

static int tmread(char *buf, int size, int timeout) {
	int status, failure_stage;
	size_t completed;

	if (size <= 0 || timeout < 0) {
		errno = EINVAL;
		return 0;
	}
	status = lanauth_read_exact_with_ops(
		sock, buf, (size_t)size, timeout, 3,
		lanauth_socket_wait_readable, lanauth_socket_read_data, NULL,
		&failure_stage, &completed);
	if (status == 0)
		return size;
	if (failure_stage == LANAUTH_SOCKET_IO_STAGE_WAIT && status != EINTR) {
		errno = status;
		fatal("select: %m");
	}
	if (status == EINTR) {
		syslog(LOG_DEBUG, "reconnecting by request");
		return 0;
	}
	if (status == ETIMEDOUT) {
		if (completed == 0)
			syslog(LOG_DEBUG, "no response from server, reconnecting");
		else
			syslog(LOG_DEBUG,
			       "incomplete response from server (%lu/%d bytes), reconnecting",
			       (unsigned long)completed, size);
		return 0;
	}
	if (status == ECONNRESET)
		syslog(LOG_DEBUG, "server closed connection");
	else {
		errno = status;
		syslog(LOG_DEBUG, "read: %m");
	}
	return 0;
}

static void auth1() {
	hash_state ctx;
	md5_init(&ctx);
	md5_process(&ctx, challenge+1, challenge[0]);
	md5_process(&ctx, localip, strlen(localip));
	level += '0';
	md5_process(&ctx, &level, 1);
	level -= '0';
	md5_process(&ctx, pass, strlen(pass));
	md5_done(&ctx, digest);
}

static void auth2() {
	hash_state ctx;
	int i;
	for(i = 0; i < 255; i++)digest[i] = rand() % 256;
	digest[0] = level - 1;
	digest[1] = 2 + rand() % 230;
	rmd160_init(&ctx);
	rmd160_process(&ctx, challenge+1, challenge[0]);
	rmd160_process(&ctx, pass, strlen(pass));
	rmd160_done(&ctx, digest+digest[1]);
}
