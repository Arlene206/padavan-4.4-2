#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "service_lifecycle_policy.h"

struct fake_service {
	const char *name;
	int present;
	int stop_on_term;
	int stop_on_kill;
	int initial_unknown;
	int unknown_after_term;
	int validate_unknown_after_term;
	int change_identity_after_term;
	int disappear_on_validate;
	int validate_unknown_initial;
	unsigned int sleeps;
	struct process_identity identity;
};

struct fake_context {
	struct fake_service *services;
	size_t service_count;
	unsigned int term_signals;
	unsigned int kill_signals;
	unsigned int total_sleeps;
	unsigned int scans;
	unsigned int validations;
	unsigned int scans_before_first_signal;
};

static struct fake_service *
fake_find(struct fake_context *context, const char *name)
{
	size_t i;
	for (i = 0; i < context->service_count; i++) {
		if (strcmp(context->services[i].name, name) == 0)
			return &context->services[i];
	}
	return NULL;
}

static enum process_presence
fake_scan(void *opaque, const char *name, struct process_snapshot *snapshot)
{
	struct fake_context *context = (struct fake_context *)opaque;
	struct fake_service *service = fake_find(context, name);

	context->scans++;
	memset(snapshot, 0, sizeof(*snapshot));
	if (!service) {
		snapshot->state = PROCESS_PRESENCE_UNKNOWN;
		return snapshot->state;
	}
	if (service->initial_unknown && context->term_signals == 0) {
		snapshot->state = PROCESS_PRESENCE_UNKNOWN;
		return snapshot->state;
	}
	if (service->unknown_after_term && context->term_signals > 0) {
		snapshot->state = PROCESS_PRESENCE_UNKNOWN;
		return snapshot->state;
	}
	if (!service->present) {
		snapshot->state = PROCESS_PRESENCE_ABSENT;
		return snapshot->state;
	}
	snapshot->items = (struct process_identity *)
		malloc(sizeof(*snapshot->items));
	assert(snapshot->items != NULL);
	snapshot->items[0] = service->identity;
	if (service->change_identity_after_term && context->term_signals > 0)
		snapshot->items[0].start_time++;
	snapshot->count = 1;
	snapshot->state = PROCESS_PRESENCE_RUNNING;
	return snapshot->state;
}

static enum process_presence
fake_validate(void *opaque, const char *name,
		  const struct process_identity *identity)
{
	struct fake_context *context = (struct fake_context *)opaque;
	struct fake_service *service = fake_find(context, name);

	context->validations++;
	if (!service)
		return PROCESS_PRESENCE_UNKNOWN;
	if (service->validate_unknown_initial && context->term_signals == 0)
		return PROCESS_PRESENCE_UNKNOWN;
	if (service->validate_unknown_after_term && context->term_signals > 0)
		return PROCESS_PRESENCE_UNKNOWN;
	if (service->disappear_on_validate) {
		service->present = 0;
		return PROCESS_PRESENCE_ABSENT;
	}
	if (!service->present)
		return PROCESS_PRESENCE_ABSENT;
	if (service->change_identity_after_term && context->term_signals > 0)
		return PROCESS_PRESENCE_ABSENT;
	if (identity->pid != service->identity.pid ||
	    identity->start_time != service->identity.start_time)
		return PROCESS_PRESENCE_ABSENT;
	return PROCESS_PRESENCE_RUNNING;
}

static int
fake_signal(void *opaque, const struct process_identity *identity,
		int signal_number)
{
	struct fake_context *context = (struct fake_context *)opaque;
	size_t i;

	if (context->term_signals == 0)
		context->scans_before_first_signal = context->scans;
	for (i = 0; i < context->service_count; i++) {
		struct fake_service *service = &context->services[i];
		if (service->identity.pid != identity->pid)
			continue;
		if (signal_number == SIGTERM) {
			context->term_signals++;
			if (service->stop_on_term == 1)
				service->present = 0;
		} else if (signal_number == SIGKILL) {
			context->kill_signals++;
			if (service->stop_on_kill)
				service->present = 0;
		}
		return 0;
	}
	return -1;
}

static void
fake_sleep(void *opaque, unsigned int seconds)
{
	struct fake_context *context = (struct fake_context *)opaque;
	size_t i;

	context->total_sleeps += seconds;
	for (i = 0; i < context->service_count; i++) {
		struct fake_service *service = &context->services[i];
		if (service->stop_on_term == 2 &&
		    context->total_sleeps >= 2)
			service->present = 0;
	}
}

static const struct service_lifecycle_ops fake_ops = {
	fake_scan, fake_validate, fake_signal, fake_sleep
};

static void
fake_setup(struct fake_context *context, struct fake_service *services,
	   size_t count)
{
	size_t i;

	memset(context, 0, sizeof(*context));
	context->services = services;
	context->service_count = count;
	for (i = 0; i < count; i++) {
		services[i].sleeps = 0;
		services[i].identity.pid = (pid_t)(100 + i);
		services[i].identity.start_time = 1000 + i;
		services[i].identity.argv0_hash = 11;
		services[i].identity.argv1_hash = 0;
		services[i].identity.comm_truncated = 0;
		strncpy(services[i].identity.comm, services[i].name,
			sizeof(services[i].identity.comm) - 1);
	}
}

static void
test_unknown_first_middle_last_sends_zero_signals(void)
{
	int unknown_index;

	for (unknown_index = 0; unknown_index < 3; unknown_index++) {
		struct fake_service services[3];
		struct fake_context context;
		char *names[] = { "svc-a", "svc-b", "svc-c", NULL };

		memset(services, 0, sizeof(services));
		services[0].name = "svc-a";
		services[1].name = "svc-b";
		services[2].name = "svc-c";
		services[0].present = services[1].present = services[2].present = 1;
		fake_setup(&context, services, 3);
		services[unknown_index].initial_unknown = 1;
		assert(service_lifecycle_stop(names, 2, 1, &fake_ops, &context) ==
		       SERVICE_LIFECYCLE_UNKNOWN);
		assert(context.term_signals == 0);
		assert(context.kill_signals == 0);
	}
}

static void
test_full_preflight_finishes_before_first_term(void)
{
	struct fake_service services[3];
	struct fake_context context;
	char *names[] = { "svc-a", "svc-b", "svc-c", NULL };

	memset(services, 0, sizeof(services));
	services[0].name = "svc-a";
	services[1].name = "svc-b";
	services[2].name = "svc-c";
	services[0].present = services[1].present = services[2].present = 1;
	services[0].stop_on_term = services[1].stop_on_term =
		services[2].stop_on_term = 1;
	fake_setup(&context, services, 3);
	assert(service_lifecycle_stop(names, 2, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_STOPPED);
	assert(context.scans_before_first_signal >= 6);
	assert(context.term_signals == 3);
	assert(context.kill_signals == 0);
}

static void
test_unknown_during_term_wait_blocks_kill_and_followup(void)
{
	struct fake_service service;
	struct fake_context context;
	char *names[] = { "pppd", NULL };

	memset(&service, 0, sizeof(service));
	service.name = "pppd";
	service.present = 1;
	service.unknown_after_term = 1;
	fake_setup(&context, &service, 1);
	assert(service_lifecycle_stop(names, 3, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_UNKNOWN);
	assert(context.term_signals == 1);
	assert(context.kill_signals == 0);
	assert(!service_lifecycle_allows_followup(SERVICE_LIFECYCLE_UNKNOWN));
}

static void
test_unknown_before_kill_blocks_kill(void)
{
	struct fake_service service;
	struct fake_context context;
	char *names[] = { "xl2tpd", NULL };

	memset(&service, 0, sizeof(service));
	service.name = "xl2tpd";
	service.present = 1;
	service.validate_unknown_after_term = 1;
	fake_setup(&context, &service, 1);
	assert(service_lifecycle_stop(names, 1, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_UNKNOWN);
	assert(context.term_signals == 1);
	assert(context.kill_signals == 0);
}

static void
test_pid_start_time_change_blocks_escalation(void)
{
	struct fake_service service;
	struct fake_context context;
	char *names[] = { "pptpd", NULL };

	memset(&service, 0, sizeof(service));
	service.name = "pptpd";
	service.present = 1;
	service.change_identity_after_term = 1;
	fake_setup(&context, &service, 1);
	assert(service_lifecycle_stop(names, 1, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_CHANGED);
	assert(context.term_signals == 1);
	assert(context.kill_signals == 0);
}

static void
test_disappearance_before_signal_is_not_signalled(void)
{
	struct fake_service service;
	struct fake_context context;
	char *names[] = { "ntpd", NULL };

	memset(&service, 0, sizeof(service));
	service.name = "ntpd";
	service.present = 1;
	service.disappear_on_validate = 1;
	fake_setup(&context, &service, 1);
	assert(service_lifecycle_stop(names, 2, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_STOPPED);
	assert(context.term_signals == 0 && context.kill_signals == 0);
}

static void
test_identity_unknown_in_preflight_sends_zero(void)
{
	struct fake_service service;
	struct fake_context context;
	char *names[] = { "dnsmasq", NULL };

	memset(&service, 0, sizeof(service));
	service.name = "dnsmasq";
	service.present = 1;
	service.validate_unknown_initial = 1;
	fake_setup(&context, &service, 1);
	assert(service_lifecycle_stop(names, 2, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_UNKNOWN);
	assert(context.term_signals == 0 && context.kill_signals == 0);
}

static void
test_force_kill_revalidates_and_waits_for_exit(void)
{
	struct fake_service service;
	struct fake_context context;
	char *names[] = { "pppd", NULL };

	memset(&service, 0, sizeof(service));
	service.name = "pppd";
	service.present = 1;
	service.stop_on_kill = 1;
	fake_setup(&context, &service, 1);
	assert(service_lifecycle_stop(names, 2, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_STOPPED);
	assert(context.term_signals == 1 && context.kill_signals == 1);
	assert(context.validations >= 4);
}

static void
test_nonforced_timeout_blocks_followup(void)
{
	struct fake_service service;
	struct fake_context context;
	char *names[] = { "ntpd", NULL };

	memset(&service, 0, sizeof(service));
	service.name = "ntpd";
	service.present = 1;
	fake_setup(&context, &service, 1);
	assert(service_lifecycle_stop(names, 1, 0, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_TIMEOUT);
	assert(context.kill_signals == 0);
	assert(!service_lifecycle_allows_followup(SERVICE_LIFECYCLE_TIMEOUT));
	assert(service_lifecycle_allows_followup(SERVICE_LIFECYCLE_STOPPED));
}

static void
test_delayed_exit_and_empty_list(void)
{
	struct fake_service service;
	struct fake_context context;
	char *names[] = { "delayed", NULL };
	char *empty[] = { NULL };

	memset(&service, 0, sizeof(service));
	service.name = "delayed";
	service.present = 1;
	service.stop_on_term = 2;
	fake_setup(&context, &service, 1);
	assert(service_lifecycle_stop(names, 3, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_STOPPED);
	assert(context.kill_signals == 0 && context.total_sleeps == 2);
	assert(service_lifecycle_stop(empty, 1, 1, &fake_ops, &context) ==
	       SERVICE_LIFECYCLE_STOPPED);
}

int
main(void)
{
	test_unknown_first_middle_last_sends_zero_signals();
	test_full_preflight_finishes_before_first_term();
	test_unknown_during_term_wait_blocks_kill_and_followup();
	test_unknown_before_kill_blocks_kill();
	test_pid_start_time_change_blocks_escalation();
	test_disappearance_before_signal_is_not_signalled();
	test_identity_unknown_in_preflight_sends_zero();
	test_force_kill_revalidates_and_waits_for_exit();
	test_nonforced_timeout_blocks_followup();
	test_delayed_exit_and_empty_list();
	return 0;
}
