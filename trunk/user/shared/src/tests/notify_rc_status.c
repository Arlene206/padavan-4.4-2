#define _GNU_SOURCE
static char test_notify_dir[512];
static char test_incomplete_dir[512];
#define DIR_RC_NOTIFY test_notify_dir
#define DIR_RC_INCOMPLETE test_incomplete_dir
#define RC_NOTIFY_OWNER geteuid()
#define kill notify_rc_status_test_kill
#include "../notify_rc.c"
#undef kill

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int test_kill_calls;
static int test_kill_failure;

int
notify_rc_status_test_kill(pid_t pid, int signal_number)
{
	assert(pid == 1);
	assert(signal_number == SIGUSR1);
	++test_kill_calls;
	if (test_kill_failure) {
		errno = EPERM;
		return -1;
	}
	return 0;
}

static void
remove_marker(const char *directory, const char *name)
{
	char path[1024];
	int length = snprintf(path, sizeof(path), "%s/%s", directory, name);
	assert(length > 0 && length < (int)sizeof(path));
	(void)unlink(path);
}

static void
assert_marker(const char *directory, const char *name)
{
	char path[1024];
	struct stat st;
	int length = snprintf(path, sizeof(path), "%s/%s", directory, name);
	assert(length > 0 && length < (int)sizeof(path));
	assert(lstat(path, &st) == 0);
	assert(S_ISREG(st.st_mode));
	assert(st.st_uid == geteuid());
	assert(st.st_nlink == 1);
	assert((st.st_mode & 0777) == 0600);
}

static void
test_queue_failure_is_reported(void)
{
	assert(mkdir(DIR_RC_NOTIFY, 0700) == 0);
	errno = 0;
	assert(notify_rc_status("restart_firewall") == -1);
	assert(errno == ENOENT);
	assert(test_kill_calls == 0);
}

static void
test_success_and_duplicate_queue_are_reported(void)
{
	assert(mkdir(DIR_RC_INCOMPLETE, 0700) == 0);
	assert(notify_rc_status("restart_firewall") == 0);
	assert_marker(DIR_RC_NOTIFY, "restart_firewall");
	assert_marker(DIR_RC_INCOMPLETE, "restart_firewall");
	assert(test_kill_calls == 1);

	assert(notify_rc_status("restart_firewall") == 0);
	assert(test_kill_calls == 2);
}

static void
test_signal_failure_keeps_queued_status_successful(void)
{
	test_kill_failure = 1;
	assert(notify_rc_status("restart_dns") == 0);
	assert_marker(DIR_RC_NOTIFY, "restart_dns");
	assert_marker(DIR_RC_INCOMPLETE, "restart_dns");
	assert(test_kill_calls == 3);
	test_kill_failure = 0;
}

static void
test_invalid_or_unsafe_queue_is_rejected(void)
{
	errno = 0;
	assert(notify_rc_status("../restart_firewall") == -1);
	assert(errno == EINVAL);

	remove_marker(DIR_RC_INCOMPLETE, "restart_firewall");
	remove_marker(DIR_RC_INCOMPLETE, "restart_dns");
	assert(rmdir(DIR_RC_INCOMPLETE) == 0);
	assert(mkdir(DIR_RC_INCOMPLETE, 0777) == 0);
	assert(chmod(DIR_RC_INCOMPLETE, 0777) == 0);
	errno = 0;
	assert(notify_rc_status("restart_upnp") == -1);
	assert(errno == EPERM);
	assert(test_kill_calls == 3);

	remove_marker(DIR_RC_NOTIFY, "restart_firewall");
	remove_marker(DIR_RC_NOTIFY, "restart_dns");
	assert(rmdir(DIR_RC_NOTIFY) == 0);
	assert(rmdir(DIR_RC_INCOMPLETE) == 0);
}

int
main(void)
{
	char root[] = "/tmp/rc-notify-status.XXXXXX";
	int length;

	assert(mkdtemp(root) != NULL);
	length = snprintf(test_notify_dir, sizeof(test_notify_dir), "%s/queue", root);
	assert(length > 0 && length < (int)sizeof(test_notify_dir));
	length = snprintf(test_incomplete_dir, sizeof(test_incomplete_dir), "%s/incomplete", root);
	assert(length > 0 && length < (int)sizeof(test_incomplete_dir));

	test_queue_failure_is_reported();
	test_success_and_duplicate_queue_are_reported();
	test_signal_failure_keeps_queued_status_successful();
	test_invalid_or_unsafe_queue_is_rejected();
	assert(rmdir(root) == 0);
	puts("RC notification enqueue status regression: PASS");
	return 0;
}
