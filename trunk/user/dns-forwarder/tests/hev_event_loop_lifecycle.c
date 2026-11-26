#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "../dns-forwarder-1.2.1/hev-event-loop.h"
#include "../dns-forwarder-1.2.1/hev-event-source-fds.h"

int
main (void)
{
	HevEventLoop *first = NULL, *second = NULL;
	HevEventSource *source = NULL;
	int first_pipe[2], second_pipe[2];

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
