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
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <net/ethernet.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <syslog.h>
#include "router_account_policy.h"
#include "router_account_nvram_policy.h"
#include <utime.h>

#include "nvram_linux.h"
#include "shutils.h"
#include "shutils_pidfile_policy.h"
#include "file_write_status.h"
#include "passwd_temp_file_policy.h"

/* Activate 64-bit file support on Linux/32bit plus others */
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1

/*
 * Reads file and returns contents
 * @param	fd	file descriptor
 * @return	contents of file or NULL if an error occurred
 */
static char *
fd2str(int fd, size_t chunk_size)
{
	char *buf = NULL;
	size_t count = 0;
	ssize_t n;

	if (chunk_size == 0) {
		close(fd);
		errno = EINVAL;
		return NULL;
	}

	for (;;) {
		char *new_buf;

		if (count == (size_t)-1 ||
		    chunk_size > (size_t)-1 - count - 1) {
			free(buf);
			close(fd);
			errno = EOVERFLOW;
			return NULL;
		}

		new_buf = realloc(buf, count + chunk_size + 1);
		if (!new_buf) {
			free(buf);
			close(fd);
			return NULL;
		}
		buf = new_buf;

		do {
			n = read(fd, buf + count, chunk_size);
		} while (n < 0 && errno == EINTR);

		if (n < 0) {
			int saved_errno = errno;
			free(buf);
			close(fd);
			errno = saved_errno;
			return NULL;
		}

		count += (size_t)n;
		if ((size_t)n < chunk_size)
			break;
	}

	buf[count] = '\0';
	close(fd);
	return buf;
}

/*
 * Reads file and returns contents
 * @param	path	path to file
 * @return	contents of file or NULL if an error occurred
 */
char *
file2str(const char *path, size_t chunk_size)
{
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1) {
		perror(path);
		return NULL;
	}

	return fd2str(fd, chunk_size);
}

/* 
 * Waits for a file descriptor to change status or unblocked signal
 * @param	fd	file descriptor
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @return	1 if descriptor changed status or 0 if timed out or -1 on error
 */
int
waitfor (int fd, int timeout)
{
	fd_set rfds;
	struct timeval tv = { timeout, 0 };

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	return select(fd + 1, &rfds, NULL, NULL, (timeout > 0) ? &tv : NULL);
}

void time_zone_x_mapping()
{
	char tmpstr[32];
	char *ptr;

	snprintf(tmpstr, sizeof(tmpstr), "%s", nvram_safe_get("time_zone"));

	/* replace . with : */
	if ((ptr=strchr(tmpstr, '.'))!=NULL) *ptr = ':';
	/* remove *_? */
	if ((ptr=strchr(tmpstr, '_'))!=NULL) *ptr = 0x0;

	/* special mapping */
	if (strcmp(tmpstr, "JST") == 0)
		nvram_set("time_zone_x", "UCT-9");
	else if (strcmp(tmpstr, "EST-10EDT") == 0)
		nvram_set("time_zone_x", "EST-10EDT,M10.5.0,M3.5.0/3");
	else if (strcmp(tmpstr, "TST-10TDT") == 0)
		nvram_set("time_zone_x", "UCT-10");
	else if (strcmp(tmpstr, "CST-9:30CDT") == 0)
		nvram_set("time_zone_x", "UCT-9:30");
	else if (strcmp(tmpstr, "EET-2EETDST") == 0)
		nvram_set("time_zone_x", "EET-2EETDST,M3.5.0/3,M10.5.0/4");
	else if (strcmp(tmpstr, "MET-1METDST") == 0 ||
	         strcmp(tmpstr, "MEZ-1MESZ") == 0 ||
	         strcmp(tmpstr, "CET-1CEST") == 0)
		nvram_set("time_zone_x", "CET-1CEST,M3.5.0,M10.5.0/3");
	else if (strcmp(tmpstr, "GMT0BST") == 0)
		nvram_set("time_zone_x", "GMT0BST,M3.5.0/1,M10.5.0");
	else if (strcmp(tmpstr, "EUT1EUTDST") == 0)
		nvram_set("time_zone_x", "AZOT1AZOST,M3.5.0/0,M10.5.0/1");
	else if (strcmp(tmpstr, "BRT3BRST") == 0)
		nvram_set("time_zone_x", "BRT3BRST,M10.3.0/0,M2.3.0/0");
	else if (strcmp(tmpstr, "AST4ADT") == 0)
		nvram_set("time_zone_x", "AST4ADT,M3.2.0,M11.1.0");
	else if (strcmp(tmpstr, "EST5EDT") == 0)
		nvram_set("time_zone_x", "EST5EDT,M3.2.0,M11.1.0");
	else if (strcmp(tmpstr, "CST6CDT") == 0)
		nvram_set("time_zone_x", "CST6CDT,M3.2.0,M11.1.0");
	else if (strcmp(tmpstr, "MST7MDT") == 0)
		nvram_set("time_zone_x", "MST7MDT,M3.2.0,M11.1.0");
	else if (strcmp(tmpstr, "PST8PDT") == 0)
		nvram_set("time_zone_x", "PST8PDT,M3.2.0,M11.1.0");
	else if (strcmp(tmpstr, "NAST9NADT") == 0)
		nvram_set("time_zone_x", "AKST9AKDT,M3.2.0,M11.1.0");
	else
		nvram_set("time_zone_x", tmpstr);

	{
		FILE *fp = fopen("/etc/TZ", "w");
		if (fp) {
			fputs(nvram_safe_get("time_zone_x"), fp);
			fputc('\n', fp);
			fclose(fp);
		}
	}
}

static int
passwd_temp_stream_finish(FILE *stream)
{
	int status = 0;
	if (!stream) { errno = EINVAL; return EINVAL; }
	if (fflush(stream) != 0) status = errno ? errno : EIO;
	else if (ferror(stream)) status = errno ? errno : EIO;
	if (status == 0 && fsync(fileno(stream)) != 0) status = errno ? errno : EIO;
	if (fclose(stream) != 0 && status == 0) status = errno ? errno : EIO;
	if (status != 0) errno = status;
	return status;
}

int
change_passwd_unix(char *user, char *pass)
{
	FILE *fp = NULL;
	char tmpfile[] = "/tmp/.tmpchpw.XXXXXX";
	char cmdbuf[128];
	int fd = -1, status = 0, write_status = 0;
	int command_status, command_length;

	if (!user || !*user) user = SYS_USER_ROOT;
	if (!pass) pass = DEF_ROOT_PASSWORD;
	if (!router_account_name_is_valid(user) ||
	    strchr(pass, '\n') || strchr(pass, '\r') || strchr(pass, ':')) {
		errno = EINVAL;
		return EINVAL;
	}

	fd = passwd_temp_file_open(tmpfile, 0600);
	if (fd < 0) return errno ? errno : EIO;
	fp = fdopen(fd, "w");
	if (!fp) {
		status = errno ? errno : EIO;
		(void)close(fd);
		fd = -1;
		goto cleanup;
	}
	fd = -1;
	if (fprintf(fp, "%s:%s\n", user, pass) < 0)
		write_status = errno ? errno : EIO;
	status = passwd_temp_stream_finish(fp);
	fp = NULL;
	if (write_status != 0) status = write_status;
	if (status != 0) goto cleanup;

	command_length = snprintf(cmdbuf, sizeof(cmdbuf),
				  "/usr/sbin/chpasswd -m < '%s'", tmpfile);
	if (command_length < 0 || (size_t)command_length >= sizeof(cmdbuf)) {
		status = ENAMETOOLONG;
		goto cleanup;
	}
	errno = 0;
	command_status = system(cmdbuf);
	if (command_status == -1) status = errno ? errno : EIO;
	else if (command_status != 0) status = command_status;

cleanup:
	if (fp) (void)fclose(fp);
	if (fd >= 0) (void)close(fd);
	if (unlink(tmpfile) != 0 && status == 0) status = errno ? errno : EIO;
	if (chmod("/etc/shadow", 0640) != 0 && status == 0) status = errno ? errno : EIO;
	if (status != 0) errno = status;
	return status;
}

static int
router_account_nvram_get_checked(void *context, const char *name, char **value)
{
	(void)context;
	return nvram_get_checked(name, value);
}

int
recreate_passwd_unix(int force_create)
{
	FILE *fp1 = NULL, *fp2 = NULL;
	int passwd_fd = -1, group_fd = -1, shadow_fd = -1;
	int i, j, uid, duplicate, status = 0, file_status, password_status;
	int update_count, rollback_failed = 0, backup_cleanup_failed = 0;
	int password_rollback_status;
	int passwd_had_original = 0, group_had_original = 0, shadow_had_original = 0;
	char account_key[32], *rootnm, *usernm;
	char root_name[ROUTER_ACCOUNT_NV_NAME_SIZE];
	char root_password[NVRAM_MAX_VALUE_LEN + 1];
	char account_names[ROUTER_ACCOUNT_NV_MAX_ACCOUNTS][ROUTER_ACCOUNT_NV_NAME_SIZE];
	int account_count;
	char passwd_temp[] = "/etc/.passwd.XXXXXX";
	char group_temp[] = "/etc/.group.XXXXXX";
	char shadow_temp[] = "/etc/.shadow.XXXXXX";
	char passwd_backup[] = "/etc/.passwd-backup.XXXXXX";
	char group_backup[] = "/etc/.group-backup.XXXXXX";
	char shadow_backup[] = "/etc/.shadow-backup.XXXXXX";
	passwd_file_update_t file_updates[3];

	/* Snapshot all NVRAM values before staged account files exist.
	 * The checked getter returns a shared-cache pointer; copy before next read. */
	root_password[0] = '\0';
	status = router_account_nvram_read_copy(router_account_nvram_get_checked,
		NULL, "http_username", "", root_name, sizeof(root_name));
	if (status != 0)
		goto snapshot_failed;
	if (!root_name[0])
		snprintf(root_name, sizeof(root_name), "%s", SYS_USER_ROOT);
	rootnm = root_name;
	if (!router_account_name_is_valid(rootnm)) {
		status = EINVAL;
		goto snapshot_failed;
	}
	if (force_create) {
		status = router_account_nvram_read_copy(router_account_nvram_get_checked,
			NULL, "http_passwd", "", root_password, sizeof(root_password));
		if (status != 0)
			goto snapshot_failed;
		if (strchr(root_password, '\n') || strchr(root_password, '\r') ||
		    strchr(root_password, ':')) {
			status = EINVAL;
			goto snapshot_failed;
		}
	}
	status = router_account_nvram_read_count(router_account_nvram_get_checked,
		NULL, "acc_num", ROUTER_ACCOUNT_NV_MAX_ACCOUNTS, &account_count);
	if (status != 0)
		goto snapshot_failed;
	for (i = 0; i < account_count; ++i) {
		int key_length;
		key_length = snprintf(account_key, sizeof(account_key), "acc_username%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(account_key)) {
			status = ENAMETOOLONG;
			goto snapshot_failed;
		}
		status = router_account_nvram_read_copy(router_account_nvram_get_checked,
			NULL, account_key, "", account_names[i], sizeof(account_names[i]));
		if (status != 0)
			goto snapshot_failed;
	}

	passwd_fd = passwd_temp_file_open(passwd_temp, 0644);
	if (passwd_fd < 0) { status = errno ? errno : EIO; goto cleanup; }
	group_fd = passwd_temp_file_open(group_temp, 0644);
	if (group_fd < 0) { status = errno ? errno : EIO; goto cleanup; }
	fp1 = fdopen(passwd_fd, "w");
	if (!fp1) { status = errno ? errno : EIO; goto cleanup; }
	passwd_fd = -1;
	fp2 = fdopen(group_fd, "w");
	if (!fp2) { status = errno ? errno : EIO; goto cleanup; }
	group_fd = -1;

	if (fprintf(fp1, "%s:x:%d:%d::%s:%s\n", rootnm, 0, 0,
		    SYS_HOME_PATH_ROOT, SYS_SHELL) < 0 ||
	    fprintf(fp1, "%s:x:%d:%d::%s:%s\n", SYS_USER_NOBODY, 99, 99,
		    "/media", "/bin/false") < 0 ||
	    fprintf(fp1, "%s:x:%d:%d::%s:%s\n", "sshd", 100, 99,
		    "/var/empty", "/bin/false") < 0 ||
	    fprintf(fp2, "%s:x:%d:%s\n", SYS_GROUP_ROOT, 0, rootnm) < 0 ||
	    fprintf(fp2, "%s:x:%d:\n", SYS_GROUP_NOGROUP, 99) < 0) {
		status = errno ? errno : EIO;
		goto cleanup;
	}

	uid = 1000;
	for (i = 0; i < account_count; ++i) {
		usernm = account_names[i];
		if (!router_account_name_is_valid(usernm) ||
		    router_account_name_is_reserved(usernm, rootnm) ||
		    !strcmp(usernm, "anonymous"))
			continue;
		duplicate = 0;
		for (j = 0; j < i; ++j) {
			if (!strcmp(usernm, account_names[j])) {
				duplicate = 1;
				break;
			}
		}
		if (duplicate) continue;
		if (fprintf(fp1, "%s:x:%d:%d:::\n", usernm, uid, uid) < 0 ||
		    fprintf(fp2, "%s:x:%d:\n", usernm, uid) < 0) {
			status = errno ? errno : EIO;
			goto cleanup;
		}
		uid++;
	}

	file_status = passwd_temp_stream_finish(fp1);
	fp1 = NULL;
	if (file_status != 0) { status = file_status; goto cleanup; }
	file_status = passwd_temp_stream_finish(fp2);
	fp2 = NULL;
	if (file_status != 0) { status = file_status; goto cleanup; }

	if (force_create) {
		shadow_fd = passwd_temp_file_open(shadow_temp, 0640);
		if (shadow_fd < 0) { status = errno ? errno : EIO; goto cleanup; }
		fp1 = fdopen(shadow_fd, "w");
		if (!fp1) { status = errno ? errno : EIO; goto cleanup; }
		shadow_fd = -1;
		if (fprintf(fp1, "%s:!:%d:0:99999:7:::\n", rootnm, 16000) < 0 ||
		    fprintf(fp1, "%s:%s:%d:0:99999:7:::\n",
			    SYS_USER_NOBODY, "*", 16000) < 0 ||
		    fprintf(fp1, "%s:%s:%d:0:99999:7:::\n",
			    "sshd", "*", 16000) < 0) {
			status = errno ? errno : EIO;
			goto cleanup;
		}
		file_status = passwd_temp_stream_finish(fp1);
		fp1 = NULL;
		if (file_status != 0) { status = file_status; goto cleanup; }
	}

	/*
	 * Hold hard-link recovery points for every live file before the first
	 * replacement. This allows a failed second/third rename to restore the
	 * already-published files without losing the previous account database.
	 */
	if (passwd_temp_file_backup_create("/etc/passwd", passwd_backup,
					  &passwd_had_original) != 0 ||
	    passwd_temp_file_backup_create("/etc/group", group_backup,
					  &group_had_original) != 0 ||
	    (force_create &&
	     passwd_temp_file_backup_create("/etc/shadow", shadow_backup,
					    &shadow_had_original) != 0)) {
		status = errno ? errno : EIO;
		goto cleanup;
	}

	file_updates[0].staged_path = passwd_temp;
	file_updates[0].destination_path = "/etc/passwd";
	file_updates[0].backup_path = passwd_backup;
	file_updates[0].had_original = passwd_had_original;
	file_updates[1].staged_path = group_temp;
	file_updates[1].destination_path = "/etc/group";
	file_updates[1].backup_path = group_backup;
	file_updates[1].had_original = group_had_original;
	update_count = 2;
	if (force_create) {
		file_updates[2].staged_path = shadow_temp;
		file_updates[2].destination_path = "/etc/shadow";
		file_updates[2].backup_path = shadow_backup;
		file_updates[2].had_original = shadow_had_original;
		update_count = 3;
	}
	if (passwd_file_publish_transaction_retain(file_updates, update_count,
						    rename, unlink,
						    &rollback_failed) != 0) {
		status = errno ? errno : EIO;
		if (rollback_failed) {
			syslog(LOG_ERR,
			       "account database rollback incomplete; recovery links retained: passwd=%s group=%s shadow=%s",
			       passwd_backup, group_backup, shadow_backup);
		}
		goto cleanup;
	}

	passwd_temp[0] = '\0';
	group_temp[0] = '\0';
	if (force_create) {
		shadow_temp[0] = '\0';
		password_status = change_passwd_unix(rootnm, root_password);
		if (password_status != 0) {
			status = password_status;
			if (passwd_file_rollback_transaction(
				file_updates, update_count, rename, unlink,
				&rollback_failed) != 0) {
				password_rollback_status = errno ? errno : EIO;
				syslog(LOG_ERR,
				       "root password installation failed (status=%d) and account database rollback was incomplete (status=%d); recovery links retained: passwd=%s group=%s shadow=%s",
				       password_status, password_rollback_status,
				       passwd_backup, group_backup, shadow_backup);
			}
		} else if (passwd_file_finalize_transaction(
				   file_updates, update_count, unlink,
				   &backup_cleanup_failed) != 0) {
			status = errno ? errno : EIO;
			syslog(LOG_ERR,
			       "root password installed but account database recovery-link cleanup failed (status=%d); links retained where present: passwd=%s group=%s shadow=%s",
			       status, passwd_backup, group_backup, shadow_backup);
		}
	} else if (passwd_file_finalize_transaction(
			   file_updates, update_count, unlink,
			   &backup_cleanup_failed) != 0) {
		status = errno ? errno : EIO;
		syslog(LOG_ERR,
		       "account database recovery-link cleanup failed (status=%d); links retained where present: passwd=%s group=%s",
		       status, passwd_backup, group_backup);
	}

cleanup:
	if (fp1) (void)fclose(fp1);
	if (fp2) (void)fclose(fp2);
	if (passwd_fd >= 0) (void)close(passwd_fd);
	if (group_fd >= 0) (void)close(group_fd);
	if (shadow_fd >= 0) (void)close(shadow_fd);
	if (passwd_temp[0]) (void)unlink(passwd_temp);
	if (group_temp[0]) (void)unlink(group_temp);
	if (shadow_temp[0]) (void)unlink(shadow_temp);
	if (!rollback_failed && !backup_cleanup_failed) {
		if (passwd_backup[0]) (void)unlink(passwd_backup);
		if (group_backup[0]) (void)unlink(group_backup);
		if (force_create && shadow_backup[0]) (void)unlink(shadow_backup);
	}
	if (status != 0) errno = status;
	return status;

snapshot_failed:
	errno = status;
	return status;
}

int
_eval(char *const argv[], char *path, int timeout, int *ppid)
{
	sigset_t set;
	pid_t pid, ret;
	int status;
	int fd;
	int flags;
	int sig, i;

	switch (pid = fork()) {
	case -1:	/* error */
		perror("fork");
		return errno;
	case 0:	 /* child */
		/* Reset signal handlers set for parent process */
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		/* Unblock signals if called from signal handler */
		sigemptyset(&set);
		sigprocmask(SIG_SETMASK, &set, NULL);

		/* Clean up */
		for (i=3; i<256; i++)    // close un-needed fd
			close(i);
		ioctl(0, TIOCNOTTY, 0);	// detach from current process
		setsid();
		
		/* Redirect stdout to <path> */
		if (path) {
			flags = O_WRONLY | O_CREAT;
			if (!strncmp(path, ">>", 2)) {
				/* append to <path> */
				flags |= O_APPEND;
				path += 2;
			} else if (!strncmp(path, ">", 1)) {
				/* overwrite <path> */
				flags |= O_TRUNC;
				path += 1;
			}
			if ((fd = open(path, flags, 0644)) < 0)
				perror(path);
			else {
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
			}
		}
		
		/* execute command */
		setenv("PATH", SYS_EXEC_PATH, 1);
		alarm(timeout);
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(errno);
	default:	/* parent */
		if (ppid) {
			*ppid = pid;
			return 0;
		} else {
			do
				ret = waitpid(pid, &status, 0);
			while ((ret == -1) && (errno == EINTR));
			
			if (ret != pid) {
				perror("waitpid");
				return errno;
			}
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}

/*
 * fread() with automatic retry on syscall interrupt
 * @param	ptr	location to store to
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully read
 */
int
safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fread((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

/*
 * fwrite() with automatic retry on syscall interrupt
 * @param	ptr	location to read from
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully written
 */
int
safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fwrite((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

/*
 * Convert Ethernet address string representation to binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @param	e	binary data
 * @return	TRUE if conversion was successful and FALSE otherwise
 */
int
ether_atoe(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, ETHER_ADDR_LEN);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
	}
	return (i == ETHER_ADDR_LEN);
}

/*
 * Convert Ethernet address binary data to string representation
 * @param	e	binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @return	a
 */
char *
ether_etoa(const unsigned char *e, char *a)
{
	char *c = a;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

/*
 * Convert Ethernet address binary data to string representation
 * @param       e       binary data
 * @param       a       string in xxxxxxxxxxxx notation
 * @return      a
 */
char *
ether_etoa2(const unsigned char *e, char *a)
{
	char *c = a;
	int i;
	
	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		c += sprintf(c, "%02x", e[i] & 0xff);
	}
	return a;
}

char *
ether_etoa3(const unsigned char *e, char *a)
{
	char *c = a;
	int i;
	
	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

/*
 *  * description: parse va and do system
 *   */
int doSystem(const char *fmt, ...)
{
	char cmd_buf[512];
	va_list pargv;
	int len;

	if (!fmt)
		return -1;

	va_start(pargv, fmt);
	len = vsnprintf(cmd_buf, sizeof(cmd_buf), fmt, pargv);
	va_end(pargv);

	/* Never execute a silently truncated shell command. */
	if (len < 0 || len >= (int)sizeof(cmd_buf)) {
		errno = EOVERFLOW;
		return -1;
	}

	return system(cmd_buf);
}

unsigned long
get_swap_size(void)
{
	struct sysinfo info;

	sysinfo(&info);
	if (info.totalswap > 0)
		return info.totalswap;

	return 0;
}

unsigned int
get_mtd_size(const char *mtd)
{
	FILE *fp;
	char line[128], bnm[64];
	int idx;
	unsigned int mtd_size = 0;

	if ((fp = fopen("/proc/mtd", "r"))) {
		fgets(line, sizeof(line), fp); //skip the 1st line
		while (fgets(line, sizeof(line), fp)) {
			unsigned int bsz = 0;
			if (sscanf(line, "mtd%d: %x %*s \"%63s\"", &idx, &bsz, bnm) > 2) {
				/* strip tailed " character, if present. */
				char *p = strchr(bnm, '"');
				if (p)
					*p = '\0';
				if (!strcmp(bnm, mtd) && bsz > 0) {
					mtd_size = bsz;
					break;
				}
			}
		}
		fclose(fp);
	}

	return mtd_size;
}

/* 
 *  * Kills process whose PID is stored in plaintext in pidfile
 *   * @param       pidfile PID file, signal
 *    * @return      0 on success and errno on failure
 *     */
int
kill_pidfile_s(char *pidfile, int sig)
{
	FILE *fp;
	pid_t pid;
	struct stat st;
	char buf[64];
	size_t length;
	int fd, saved_errno = 0;

	if (!pidfile || !*pidfile) {
		errno = EINVAL;
		return -1;
	}

	/* O_NONBLOCK prevents a stale FIFO path from hanging callers. */
	fd = shutils_pidfile_open(pidfile);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		saved_errno = errno ? errno : EIO;
	else if (!shutils_pidfile_metadata_is_safe(&st, geteuid()))
		saved_errno = EPERM;
	else if (st.st_size >= (off_t)sizeof(buf))
		saved_errno = EOVERFLOW;
	if (saved_errno != 0) {
		(void)close(fd);
		errno = saved_errno;
		return -1;
	}

	fp = fdopen(fd, "r");
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		(void)close(fd);
		errno = saved_errno;
		return -1;
	}

	errno = 0;
	length = fread(buf, 1, sizeof(buf), fp);
	if (ferror(fp))
		saved_errno = errno ? errno : EIO;
	else if (length == sizeof(buf) || !feof(fp))
		saved_errno = EOVERFLOW;
	else if (shutils_pidfile_parse_record(buf, length, &pid) != 0)
		saved_errno = errno ? errno : EINVAL;

	if (saved_errno == 0)
		errno = 0;
	if (fclose(fp) != 0 && saved_errno == 0)
		saved_errno = errno ? errno : EIO;
	if (saved_errno != 0) {
		errno = saved_errno;
		return -1;
	}

	return kill(pid, sig);
}

/* 
 * Kills process whose PID is stored in plaintext in pidfile
 * @param	pidfile	PID file
 * @return	0 on success and errno on failure
 */
int
kill_pidfile(char *pidfile)
{
	return kill_pidfile_s(pidfile, SIGTERM);
}

/* remove space in the end of string */
char *
trim_r(char *str)
{
	int i = strlen(str);

	while (i>=1)
	{
		if (*(str+i-1) == ' ' || *(str+i-1) == 0x0a || *(str+i-1) == 0x0d)
			*(str+i-1)=0x0;
		else
			break;
		i--;
	}

	return (str);
}

int
get_param_int(char *line, const char *param, int base, int defval)
{
	char *ptr = strstr(line, param);
	if (!ptr)
		return defval;
	ptr += strlen(param);
	return strtol(ptr, NULL, base);
}

char *
get_param_str(char *line, const char *param, int dups)
{
	char *ptr = strstr(line, param);
	if (!ptr)
		return NULL;
	ptr += strlen(param);
	return (dups) ? strdup(ptr) : ptr;
}

int
fput_string(const char *name, const char *value)
{
	FILE *fp;
	int saved_errno;

	fp = fopen(name, "w");
	if (!fp) {
		saved_errno = errno ? errno : EIO;
		perror(name);
		errno = saved_errno;
		return saved_errno;
	}
	return fput_string_stream(fp, value);
}

int
fput_int(const char *name, int value)
{
	char svalue[32];
	sprintf(svalue, "%d", value);
	return fput_string(name, svalue);
}

int 
compare_text_files(const char* file1, const char* file2)
{
	FILE *fp1, *fp2;
	int ret = 0;
	char *v1, *v2;
	char buf1[MAX_FILE_LINE_SIZE];
	char buf2[MAX_FILE_LINE_SIZE];

	fp1 = fopen(file1, "r");
	if (!fp1)
		return -1;
	fp2 = fopen(file2, "r");
	if (!fp2) {
		fclose(fp1);
		return -1;
	}

	for (;;) {
		v1 = fgets(buf1, sizeof(buf1), fp1);
		v2 = fgets(buf2, sizeof(buf2), fp2);
		if (!v1 || !v2) {
			if (v1 != v2)
				ret = 1;
			break;
		}
		
		if (strcmp(buf1, buf2) != 0) {
			ret = 1;
			break;
		}
	}

	fclose(fp2);
	fclose(fp1);

	return ret;
}

void
logmessage(char *logheader, char *fmt, ...)
{
	va_list args;
	char buf[512];

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	openlog(logheader, 0, 0);
	syslog(0, buf);
	closelog();
	va_end(args);
}

