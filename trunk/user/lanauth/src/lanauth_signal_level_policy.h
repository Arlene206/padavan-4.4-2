#ifndef LANAUTH_SIGNAL_LEVEL_POLICY_H
#define LANAUTH_SIGNAL_LEVEL_POLICY_H

#include <signal.h>

/* Keep signal-shared state limited to sig_atomic_t; encode each protocol
 * version from a local snapshot instead of temporarily mutating that state. */
static inline char
lanauth_auth1_level_character(sig_atomic_t current_level)
{
    return (char)('0' + current_level);
}

static inline unsigned char
lanauth_auth2_level_byte(sig_atomic_t current_level)
{
    return (unsigned char)(current_level - 1);
}

#endif /* LANAUTH_SIGNAL_LEVEL_POLICY_H */
