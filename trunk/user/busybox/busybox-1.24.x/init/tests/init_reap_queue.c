#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>

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

int main(void)
{
	test_empty_queue();
	test_rejects_invalid_pids();
	test_fifo_drain_and_reuse();
	test_full_queue_and_short_destination();
	test_rejects_corrupt_queue_count();
	return 0;
}
