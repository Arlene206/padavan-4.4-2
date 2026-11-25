#ifndef PROCESS_PIDFILE_H
#define PROCESS_PIDFILE_H

#include <stddef.h>
#include <sys/types.h>
#include "process_inventory.h"

/* Strict record format: decimal PID, one space, decimal /proc start_time. */
int process_pidfile_parse_record(const char *text, size_t length,
				 pid_t *pid, unsigned long long *start_time);
int process_pidfile_parse_link_record(const char *text, size_t length,
				      pid_t *pid, unsigned long long *start_time);
enum process_presence process_pidfile_read_identity(const char *path,
						    const char *process_name,
						    struct process_identity *identity);
int process_pidfile_write(const char *path, const char *process_name);
int process_pidfile_signal(const char *path, const char *process_name, int sig);

#endif
