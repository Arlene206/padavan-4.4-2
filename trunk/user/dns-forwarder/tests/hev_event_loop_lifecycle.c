#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "../dns-forwarder-1.2.1/hev-event-loop.h"
#include "../dns-forwarder-1.2.1/hev-event-source-fds.h"
#include "../dns-forwarder-1.2.1/hev-event-source-signal.h"

static void
test_default_event_source_functions_are_initialized(void)
{
	HevEventSource *source;

	source = hev_event_source_new (NULL, sizeof (HevEventSource));
	assert(source);
	assert(source->funcs.prepare);
	assert(source->funcs.check);
	assert(source->funcs.dispatch);
	assert(source->funcs.finalize);
	hev_event_source_unref (source);
}

static void
test_event_source_name_storage_is_safe(void)
{
	HevEventSource *source = hev_event_source_fds_new();
	const char *name;

	assert(source);
	hev_event_source_set_name(source, "dns-forwarder");
	assert(0 == strcmp(hev_event_source_get_name(source), "dns-forwarder"));

	/* The getter's borrowed pointer may be passed back without a use-after-free. */
	name = hev_event_source_get_name(source);
	hev_event_source_set_name(source, name);
	assert(0 == strcmp(hev_event_source_get_name(source), "dns-forwarder"));

	/* Empty names still require one byte for the terminating NUL. */
	hev_event_source_set_name(source, "");
	assert(0 == strcmp(hev_event_source_get_name(source), ""));
	hev_event_source_unref(source);
}


struct prepare_failure_source
{
	HevEventSource parent;
	unsigned int prepare_calls;
};

static bool
prepare_succeeds_once_then_fails (HevEventSource *source)
{
	struct prepare_failure_source *self =
		(struct prepare_failure_source *) source;

	self->prepare_calls++;
	return self->prepare_calls == 1;
}

static HevEventSourceFuncs prepare_failure_funcs =
{
	.prepare = prepare_succeeds_once_then_fails,
};

static bool
quit_loop_callback (void *data)
{
	hev_event_loop_quit ((HevEventLoop *) data);
	return true;
}

static void
test_source_removed_when_post_dispatch_prepare_fails (void)
{
	HevEventLoop *loop;
	struct prepare_failure_source *source;
	int fds[2];

	assert (0 == pipe (fds));
	loop = hev_event_loop_new ();
	assert (loop);

	source = (struct prepare_failure_source *)
		hev_event_source_new (&prepare_failure_funcs, sizeof (*source));
	assert (source);
	source->prepare_calls = 0;

	assert (hev_event_source_add_fd ((HevEventSource *) source,
					fds[0], EPOLLIN));
	hev_event_source_set_callback ((HevEventSource *) source,
					quit_loop_callback, loop, NULL);
	assert (hev_event_loop_add_source (loop, (HevEventSource *) source));
	assert (source->prepare_calls == 1);

	assert (1 == write (fds[1], "x", 1));
	hev_event_loop_run (loop);

	/* The second prepare occurs after dispatch and deliberately fails. */
	assert (source->prepare_calls == 2);
	assert (NULL == hev_event_source_get_loop ((HevEventSource *) source));

	hev_event_source_unref ((HevEventSource *) source);
	hev_event_loop_unref (loop);
	close (fds[0]);
	close (fds[1]);
}

static unsigned int signal_callback_count;

static bool
quit_on_signal (void *data)
{
	HevEventLoop *loop = data;
	signal_callback_count++;
	hev_event_loop_quit (loop);
	return false;
}

static void
test_sigterm_is_consumed_by_event_source (void)
{
	HevEventLoop *loop = hev_event_loop_new ();
	HevEventSource *source;

	assert (loop);
	signal_callback_count = 0;
	source = hev_event_source_signal_new (SIGTERM);
	assert (source);
	hev_event_source_set_callback (source, quit_on_signal, loop, NULL);
	assert (hev_event_loop_add_source (loop, source));
	hev_event_source_unref (source);

	/* SIGTERM is blocked and delivered through signalfd, not default-killed. */
	assert (0 == kill (getpid (), SIGTERM));
	hev_event_loop_run (loop);
	assert (signal_callback_count == 1);
	hev_event_loop_unref (loop);
}

int
main (void)
{
	HevEventLoop *first = NULL, *second = NULL;
	HevEventSource *source = NULL;
	int first_pipe[2], second_pipe[2];

	test_default_event_source_functions_are_initialized();
	test_event_source_name_storage_is_safe();
	test_source_removed_when_post_dispatch_prepare_fails();
	test_sigterm_is_consumed_by_event_source();

	assert(0 == pipe(first_pipe));
	assert(0 == pipe(second_pipe));
	first = hev_event_loop_new ();
	second = hev_event_loop_new ();
	assert(first && second);

	source = hev_event_source_fds_new ();
	assert(source);
	assert(hev_event_source_add_fd (source, first_pipe[0], EPOLLIN));

	/* The first fd registers, the second fails; rollback must remove the first. */
	assert(hev_event_source_add_fd (source, -1, EPOLLIN));
	assert(!hev_event_loop_add_source (first, source));
	assert(NULL == hev_event_source_get_loop (source));
	(void) hev_event_source_del_fd (source, -1);
	assert(hev_event_loop_add_source (first, source));
	assert(first == hev_event_source_get_loop (source));

	/* Failed attached-fd registration must not leave a dangling list entry. */
	assert(!hev_event_source_add_fd (source, -1, EPOLLIN));
	assert(hev_event_loop_del_source (first, source));
	assert(NULL == hev_event_source_get_loop (source));

	/* A detached source can be rebuilt and attached to another event loop. */
	(void) hev_event_source_del_fd (source, first_pipe[0]);
	assert(hev_event_source_add_fd (source, second_pipe[0], EPOLLIN));
	assert(hev_event_loop_add_source (second, source));
	assert(second == hev_event_source_get_loop (source));
	assert(hev_event_loop_del_source (second, source));
	assert(NULL == hev_event_source_get_loop (source));
	hev_event_source_unref (source);

	close (first_pipe[0]);
	close (first_pipe[1]);
	close (second_pipe[0]);
	close (second_pipe[1]);
	hev_event_loop_unref (first);
	hev_event_loop_unref (second);
	puts ("DNS Forwarder event-loop registration rollback and source reattach: PASS");
	return 0;
}
