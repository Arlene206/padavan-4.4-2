#include <assert.h>
#include <stddef.h>
#include <signal.h>

#include "init_signal_pending.h"

static void test_collects_distinct_delayed_events(void)
{
	struct init_signal_pending pending = {0};
	int shutdown_signal = 0;
	sig_atomic_t events;

	assert(init_signal_pending_record(&pending, SIGHUP) == 1);
	assert(init_signal_pending_record(&pending, SIGINT) == 1);
	assert(init_signal_pending_record(&pending, SIGQUIT) == 1);
	assert(init_signal_pending_record(&pending, SIGTERM) == 1);
	assert(init_signal_pending_record(&pending, SIGUSR2) == 1);

	events = init_signal_pending_take(&pending, &shutdown_signal);
	assert(events == (INIT_SIGNAL_PENDING_HUP | INIT_SIGNAL_PENDING_INT |
			  INIT_SIGNAL_PENDING_QUIT | INIT_SIGNAL_PENDING_SHUTDOWN));
	assert(shutdown_signal == SIGUSR2);
	assert(pending.events == 0);
	assert(pending.shutdown_signal == 0);
}

static void test_coalesces_repeated_events_without_losing_other_events(void)
{
	struct init_signal_pending pending = {0};
	int shutdown_signal = 0;
	sig_atomic_t events;

	assert(init_signal_pending_record(&pending, SIGHUP) == 1);
	assert(init_signal_pending_record(&pending, SIGHUP) == 1);
	assert(init_signal_pending_record(&pending, SIGINT) == 1);
	assert(init_signal_pending_record(&pending, SIGINT) == 1);
	events = init_signal_pending_take(&pending, &shutdown_signal);
	assert(events == (INIT_SIGNAL_PENDING_HUP | INIT_SIGNAL_PENDING_INT));
	assert(shutdown_signal == 0);
	assert(init_signal_pending_take(&pending, &shutdown_signal) == 0);
}

static void test_retains_shutdown_mode_and_clears_each_batch(void)
{
	struct init_signal_pending pending = {0};
	int shutdown_signal = 0;

	assert(init_signal_pending_record(&pending, SIGUSR1) == 1);
	assert(init_signal_pending_take(&pending, &shutdown_signal)
	       == INIT_SIGNAL_PENDING_SHUTDOWN);
	assert(shutdown_signal == SIGUSR1);

	assert(init_signal_pending_record(&pending, SIGTERM) == 1);
	assert(init_signal_pending_take(&pending, &shutdown_signal)
	       == INIT_SIGNAL_PENDING_SHUTDOWN);
	assert(shutdown_signal == SIGTERM);
}

static void test_restores_a_batch_without_overwriting_newer_shutdown(void)
{
	struct init_signal_pending pending = {0};
	int shutdown_signal = 0;
	sig_atomic_t old_events;

	assert(init_signal_pending_record(&pending, SIGHUP) == 1);
	assert(init_signal_pending_record(&pending, SIGTERM) == 1);
	old_events = init_signal_pending_take(&pending, &shutdown_signal);
	assert(old_events == (INIT_SIGNAL_PENDING_HUP | INIT_SIGNAL_PENDING_SHUTDOWN));
	assert(shutdown_signal == SIGTERM);

	/* Model a newly pending shutdown event before restoring the snapshot. */
	assert(init_signal_pending_record(&pending, SIGUSR2) == 1);
	assert(init_signal_pending_restore(&pending, old_events, shutdown_signal) == 0);
	assert(init_signal_pending_take(&pending, &shutdown_signal)
	       == (INIT_SIGNAL_PENDING_HUP | INIT_SIGNAL_PENDING_SHUTDOWN));
	assert(shutdown_signal == SIGUSR2);
}

static void test_take_urgent_keeps_reload_and_ctrl_alt_del_pending(void)
{
	struct init_signal_pending pending = {0};
	int shutdown_signal = 0;
	sig_atomic_t events;

	assert(init_signal_pending_record(&pending, SIGHUP) == 1);
	assert(init_signal_pending_record(&pending, SIGINT) == 1);
	assert(init_signal_pending_record(&pending, SIGQUIT) == 1);
	assert(init_signal_pending_record(&pending, SIGTERM) == 1);

	events = init_signal_pending_take_urgent(&pending, &shutdown_signal);
	assert(events == (INIT_SIGNAL_PENDING_QUIT | INIT_SIGNAL_PENDING_SHUTDOWN));
	assert(shutdown_signal == SIGTERM);
	assert(pending.events == (INIT_SIGNAL_PENDING_HUP | INIT_SIGNAL_PENDING_INT));
	assert(pending.shutdown_signal == 0);

	events = init_signal_pending_take(&pending, &shutdown_signal);
	assert(events == (INIT_SIGNAL_PENDING_HUP | INIT_SIGNAL_PENDING_INT));
	assert(shutdown_signal == 0);
}

static void test_take_urgent_handles_restart_without_clearing_deferred_events(void)
{
	struct init_signal_pending pending = {0};
	int shutdown_signal = 0;

	assert(init_signal_pending_record(&pending, SIGHUP) == 1);
	assert(init_signal_pending_record(&pending, SIGQUIT) == 1);
	assert(init_signal_pending_take_urgent(&pending, &shutdown_signal)
	       == INIT_SIGNAL_PENDING_QUIT);
	assert(shutdown_signal == 0);
	assert(pending.events == INIT_SIGNAL_PENDING_HUP);
}

static void test_ignores_unhandled_signals_and_rejects_invalid_state(void)
{
	struct init_signal_pending pending = {0};

	assert(init_signal_pending_record(&pending, SIGCHLD) == 0);
	assert(pending.events == 0);
	assert(init_signal_pending_record(NULL, SIGTERM) == -1);
	assert(init_signal_pending_take(NULL, NULL) == (sig_atomic_t)-1);
	assert(init_signal_pending_restore(NULL, 0, 0) == -1);
	assert(init_signal_pending_restore(&pending, (sig_atomic_t)-1, 0) == -1);
}

int main(void)
{
	test_collects_distinct_delayed_events();
	test_coalesces_repeated_events_without_losing_other_events();
	test_retains_shutdown_mode_and_clears_each_batch();
	test_restores_a_batch_without_overwriting_newer_shutdown();
	test_take_urgent_keeps_reload_and_ctrl_alt_del_pending();
test_take_urgent_handles_restart_without_clearing_deferred_events();
test_ignores_unhandled_signals_and_rejects_invalid_state();
	return 0;
}
