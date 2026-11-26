#include <assert.h>
#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t commit_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gate_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gate_changed = PTHREAD_COND_INITIALIZER;

static int current_revision;
static int flash_revision;

struct commit_task {
	int lock_before_snapshot;
	int pause_after_snapshot;
	int captured;
	int resume;
	int snapshot;
};

static void wait_until_captured(struct commit_task *task)
{
	pthread_mutex_lock(&gate_lock);
	while (!task->captured)
		pthread_cond_wait(&gate_changed, &gate_lock);
	pthread_mutex_unlock(&gate_lock);
}

static void resume_after_snapshot(struct commit_task *task)
{
	pthread_mutex_lock(&gate_lock);
	task->resume = 1;
	pthread_cond_broadcast(&gate_changed);
	pthread_mutex_unlock(&gate_lock);
}

static void *commit_worker(void *argument)
{
	struct commit_task *task = argument;

	/* Correct commits hold the flash mutex before reading in-memory state. */
	if (task->lock_before_snapshot)
		pthread_mutex_lock(&commit_lock);

	task->snapshot = current_revision;

	if (task->pause_after_snapshot) {
		pthread_mutex_lock(&gate_lock);
		task->captured = 1;
		pthread_cond_broadcast(&gate_changed);
		while (!task->resume)
			pthread_cond_wait(&gate_changed, &gate_lock);
		pthread_mutex_unlock(&gate_lock);
	}

	if (!task->lock_before_snapshot)
		pthread_mutex_lock(&commit_lock);

	flash_revision = task->snapshot;
	pthread_mutex_unlock(&commit_lock);
	return NULL;
}

static void test_snapshot_before_mutex_can_regress_flash(void)
{
	pthread_t older_thread, newer_thread;
	struct commit_task older = { 0, 1, 0, 0, 0 };
	struct commit_task newer = { 0, 0, 0, 0, 0 };

	current_revision = 1;
	flash_revision = 0;
	assert(pthread_create(&older_thread, NULL, commit_worker, &older) == 0);
	wait_until_captured(&older);

	/* A later writer snapshots and commits revision 2 first. */
	current_revision = 2;
	assert(pthread_create(&newer_thread, NULL, commit_worker, &newer) == 0);
	assert(pthread_join(newer_thread, NULL) == 0);
	assert(flash_revision == 2);

	/* The old snapshot then acquires the writer mutex and overwrites revision 2. */
	resume_after_snapshot(&older);
	assert(pthread_join(older_thread, NULL) == 0);
	assert(flash_revision == 1);
}

static void test_mutex_before_snapshot_preserves_latest_flash(void)
{
	pthread_t older_thread, newer_thread;
	struct commit_task older = { 1, 1, 0, 0, 0 };
	struct commit_task newer = { 1, 0, 0, 0, 0 };

	current_revision = 1;
	flash_revision = 0;
	assert(pthread_create(&older_thread, NULL, commit_worker, &older) == 0);
	wait_until_captured(&older);

	current_revision = 2;
	assert(pthread_create(&newer_thread, NULL, commit_worker, &newer) == 0);

	/* The newer commit cannot snapshot until the earlier flash write completes. */
	resume_after_snapshot(&older);
	assert(pthread_join(older_thread, NULL) == 0);
	assert(pthread_join(newer_thread, NULL) == 0);
	assert(flash_revision == 2);
}

int main(void)
{
	test_snapshot_before_mutex_can_regress_flash();
	test_mutex_before_snapshot_preserves_latest_flash();
	puts("NVRAM concurrent commit snapshot ordering: PASS");
	return 0;
}
