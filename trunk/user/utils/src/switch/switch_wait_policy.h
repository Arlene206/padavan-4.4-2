#ifndef SWITCH_WAIT_POLICY_H
#define SWITCH_WAIT_POLICY_H
#include <errno.h>

typedef int (*switch_wait_read_fn)(void *, unsigned int, unsigned int *);
typedef void (*switch_wait_delay_fn)(void *, unsigned int);

static int
switch_wait_for_conditions(switch_wait_read_fn read_register,
                           switch_wait_delay_fn delay,
                           void *context, unsigned int reg,
                           unsigned int clear_mask,
                           unsigned int match_mask,
                           int match_any_set, unsigned int attempts,
                           unsigned int delay_us,
                           unsigned int *last_value)
{
	unsigned int attempt, value = 0;
	int status;

	if (read_register == 0 || delay == 0 ||
	    (clear_mask == 0 && match_mask == 0) || attempts == 0 ||
	    last_value == 0)
		return EINVAL;
	*last_value = 0;
	for (attempt = 0; attempt < attempts; attempt++) {
		status = read_register(context, reg, &value);
		if (status != 0)
			return status;
		*last_value = value;
		if ((value & clear_mask) == 0 &&
		    (match_mask == 0 ||
		     (match_any_set ? ((value & match_mask) != 0) :
		                      ((value & match_mask) == match_mask))))
			return 0;
		delay(context, delay_us);
	}
	return ETIMEDOUT;
}

static int
switch_wait_until_clear(switch_wait_read_fn read_register,
                        switch_wait_delay_fn delay, void *context,
                        unsigned int reg, unsigned int busy_mask,
                        unsigned int attempts, unsigned int delay_us,
                        unsigned int *last_value)
{
	return switch_wait_for_conditions(read_register, delay, context, reg,
	                                  busy_mask, 0, 0, attempts, delay_us,
	                                  last_value);
}

static int
switch_wait_for_table_result(switch_wait_read_fn read_register,
                             switch_wait_delay_fn delay, void *context,
                             unsigned int reg, unsigned int busy_mask,
                             unsigned int result_mask, unsigned int attempts,
                             unsigned int delay_us, unsigned int *last_value)
{
	return switch_wait_for_conditions(read_register, delay, context, reg,
	                                  busy_mask, result_mask, 1, attempts,
	                                  delay_us, last_value);
}
#endif /* SWITCH_WAIT_POLICY_H */
