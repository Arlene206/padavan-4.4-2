#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "init_reap_queue.h"

static void test_empty_queue(void)
{
	struct init_reap_queue queue = {0};
	pid_t pids[INIT_REAP_QUEUE_CAPACITY];

	assert(queue.count == 0);
	assert(init_reap_queue_has_capacity(&queue));
	assert(init_reap_queue_drain(&queue, pids, INIT_REAP_QUEUE_CAPACITY) == 0);
	assert(queue.count == 0);
}

static void test_rejects_invalid_pids(void)
{
	struct init_reap_queue queue = {0};

	assert(init_reap_queue_push(&queue, 0) == -1);
	assert(init_reap_queue_push(&queue, (pid_t)-1) == -1);
	assert(queue.count == 0);
}

static void test_fifo_drain_and_reuse(void)
{
	struct init_reap_queue queue = {0};
	pid_t pids[INIT_REAP_QUEUE_CAPACITY];

	assert(init_reap_queue_push(&queue, 101) == 1);
	assert(init_reap_queue_push(&queue, 7) == 1);
	assert(init_reap_queue_push(&queue, 4096) == 1);
	assert(queue.count == 3);
	assert(init_reap_queue_drain(&queue, pids, 3) == 3);
	assert(pids[0] == 101);
	assert(pids[1] == 7);
	assert(pids[2] == 4096);
	assert(queue.count == 0);
	assert(init_reap_queue_push(&queue, 55) == 1);
	assert(init_reap_queue_drain(&queue, pids, INIT_REAP_QUEUE_CAPACITY) == 1);
	assert(pids[0] == 55);
}

static void test_full_queue_and_short_destination(void)
{
	struct init_reap_queue queue = {0};
	pid_t pids[INIT_REAP_QUEUE_CAPACITY];
	int i;

	for (i = 0; i < INIT_REAP_QUEUE_CAPACITY; i++)
		assert(init_reap_queue_push(&queue, (pid_t)(1000 + i)) == 1);
	assert(!init_reap_queue_has_capacity(&queue));
	assert(init_reap_queue_push(&queue, 9999) == 0);
	assert(init_reap_queue_drain(&queue, pids,
				     INIT_REAP_QUEUE_CAPACITY - 1) == -1);
	assert(queue.count == INIT_REAP_QUEUE_CAPACITY);
	assert(init_reap_queue_drain(&queue, pids, INIT_REAP_QUEUE_CAPACITY)
	       == INIT_REAP_QUEUE_CAPACITY);
	for (i = 0; i < INIT_REAP_QUEUE_CAPACITY; i++)
		assert(pids[i] == (pid_t)(1000 + i));
	assert(queue.count == 0);
}

static void test_rejects_corrupt_queue_count(void)
{
	struct init_reap_queue queue = {0};
	pid_t pids[INIT_REAP_QUEUE_CAPACITY];

	queue.count = INIT_REAP_QUEUE_CAPACITY + 1;
	assert(!init_reap_queue_has_capacity(&queue));
	assert(init_reap_queue_drain(&queue, pids, INIT_REAP_QUEUE_CAPACITY) == -1);
	assert(queue.count == INIT_REAP_QUEUE_CAPACITY + 1);
}

struct fake_waitpid_step {
	pid_t expected_pid;
	pid_t result;
	int expected_options;
	int error_number;
};

static struct fake_waitpid_step fake_waitpid_steps[3];
static int fake_waitpid_step_count;
static int fake_waitpid_step_index;

static pid_t
fake_init_waitpid(pid_t pid, int *status, int options)
{
	struct fake_waitpid_step step;

	assert(fake_waitpid_step_index < fake_waitpid_step_count);
	step = fake_waitpid_steps[fake_waitpid_step_index++];
	assert(pid == step.expected_pid);
	assert(status == NULL);
	assert(options == step.expected_options);
	errno = step.error_number;
	return step.result;
}

static void test_wait_for_exact_child_only(void)
{
	int child_reaped = 0;

	fake_waitpid_steps[0].expected_pid = 4321;
	fake_waitpid_steps[0].result = 4321;
	fake_waitpid_steps[0].expected_options = 0;
	fake_waitpid_steps[0].error_number = 0;
	fake_waitpid_step_count = 1;
	fake_waitpid_step_index = 0;

	assert(init_wait_for_child(4321, fake_init_waitpid, &child_reaped) == 0);
	assert(child_reaped == 1);
	assert(fake_waitpid_step_index == 1);
}

static void test_wait_for_exact_child_reports_eintr_to_main_loop(void)
{
	int child_reaped = 1;

	fake_waitpid_steps[0].expected_pid = 1234;
	fake_waitpid_steps[0].result = (pid_t)-1;
	fake_waitpid_steps[0].expected_options = 0;
	fake_waitpid_steps[0].error_number = EINTR;
	fake_waitpid_step_count = 1;
	fake_waitpid_step_index = 0;

	assert(init_wait_for_child(1234, fake_init_waitpid, &child_reaped) == EINTR);
	assert(child_reaped == 0);
	assert(fake_waitpid_step_index == 1);
}

static void test_wait_for_exact_child_accepts_signal_handler_reap(void)
{
	int child_reaped = 1;

	/* SIGTSTP may have reaped and queued this exact PID already. */
	fake_waitpid_steps[0].expected_pid = 9876;
	fake_waitpid_steps[0].result = (pid_t)-1;
	fake_waitpid_steps[0].expected_options = 0;
	fake_waitpid_steps[0].error_number = ECHILD;
	fake_waitpid_step_count = 1;
	fake_waitpid_step_index = 0;

	assert(init_wait_for_child(9876, fake_init_waitpid, &child_reaped) == 0);
	assert(child_reaped == 0);
	assert(fake_waitpid_step_index == 1);
}

static void test_wait_for_exact_child_propagates_errors(void)
{
	int child_reaped = 1;

	fake_waitpid_steps[0].expected_pid = 7654;
	fake_waitpid_steps[0].result = (pid_t)-1;
	fake_waitpid_steps[0].expected_options = 0;
	fake_waitpid_steps[0].error_number = EACCES;
	fake_waitpid_step_count = 1;
	fake_waitpid_step_index = 0;

	assert(init_wait_for_child(7654, fake_init_waitpid, &child_reaped) == EACCES);
	assert(child_reaped == 0);
	assert(fake_waitpid_step_index == 1);

	fake_waitpid_steps[0].result = 0;
	fake_waitpid_steps[0].error_number = 0;
	fake_waitpid_step_count = 1;
	fake_waitpid_step_index = 0;
	assert(init_wait_for_child(7654, fake_init_waitpid, &child_reaped) == EIO);
	assert(child_reaped == 0);
}

static void test_wait_for_exact_child_rejects_invalid_inputs(void)
{
	int child_reaped = 1;

	assert(init_wait_for_child(0, fake_init_waitpid, &child_reaped) == EINVAL);
	assert(child_reaped == 0);
	child_reaped = 1;
	assert(init_wait_for_child(4321, NULL, &child_reaped) == EINVAL);
	assert(child_reaped == 0);
	assert(init_wait_for_child(4321, fake_init_waitpid, NULL) == EINVAL);
}


static void test_wait_for_exact_child_does_not_reap_peer(void)
{
	pid_t peer_pid, target_pid, reaped_pid;
	siginfo_t info;
	int attempts, child_reaped, status, wait_result;

	peer_pid = fork();
	assert(peer_pid >= 0);
	if (peer_pid == 0)
		_exit(17);

	/* Wait until the peer is exited but remains a waitable child. */
	for (attempts = 0; attempts < 1000; attempts++) {
		memset(&info, 0, sizeof(info));
		wait_result = waitid(P_PID, (id_t)peer_pid, &info,
				     WEXITED | WNOHANG | WNOWAIT);
		if (wait_result == 0 && info.si_pid == peer_pid)
			break;
		assert(wait_result == 0 || errno == EINTR);
		usleep(1000);
	}
	assert(attempts < 1000);

	target_pid = fork();
	assert(target_pid >= 0);
	if (target_pid == 0) {
		usleep(50000);
		_exit(23);
	}

	child_reaped = 0;
	assert(init_wait_for_child(target_pid, waitpid, &child_reaped) == 0);
	assert(child_reaped == 1);

	/* Waiting for the target must not consume this unrelated zombie. */
	reaped_pid = waitpid(peer_pid, &status, WNOHANG);
	assert(reaped_pid == peer_pid);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 17);

	errno = 0;
	assert(waitpid(target_pid, &status, WNOHANG) == -1);
	assert(errno == ECHILD);
}


int main(void)
{
	test_wait_for_exact_child_only();
	test_wait_for_exact_child_reports_eintr_to_main_loop();
	test_wait_for_exact_child_accepts_signal_handler_reap();
	test_wait_for_exact_child_propagates_errors();
	test_wait_for_exact_child_rejects_invalid_inputs();
	test_wait_for_exact_child_does_not_reap_peer();
	test_empty_queue();
	test_rejects_invalid_pids();
	test_fifo_drain_and_reuse();
	test_full_queue_and_short_destination();
	test_rejects_corrupt_queue_count();
	return 0;
}
