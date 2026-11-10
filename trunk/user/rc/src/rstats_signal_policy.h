#ifndef RSTATS_SIGNAL_POLICY_H
#define RSTATS_SIGNAL_POLICY_H

#include <signal.h>

enum {
  RSTATS_PENDING_TIME_REFRESH = 1,
  RSTATS_PENDING_SPEED_EXPORT = 2,
  RSTATS_PENDING_HISTORY_EXPORT = 4,
  RSTATS_PENDING_TERMINATE = 8
};

static inline void
rstats_record_signal(volatile sig_atomic_t *pending, int signal_number)
{
  if (!pending)
    return;

  switch (signal_number) {
  case SIGHUP:
    *pending |= RSTATS_PENDING_TIME_REFRESH;
    break;
  case SIGUSR1:
    *pending |= RSTATS_PENDING_SPEED_EXPORT;
    break;
  case SIGUSR2:
    *pending |= RSTATS_PENDING_HISTORY_EXPORT;
    break;
  case SIGTERM:
    *pending |= RSTATS_PENDING_TERMINATE;
    break;
  default:
    break;
  }
}

/* Call only while all managed signals are blocked. */
static inline sig_atomic_t
rstats_take_pending_events(volatile sig_atomic_t *pending)
{
  sig_atomic_t events;
  if (!pending)
    return 0;
  events = *pending;
  *pending = 0;
  return events;
}

#endif /* RSTATS_SIGNAL_POLICY_H */
