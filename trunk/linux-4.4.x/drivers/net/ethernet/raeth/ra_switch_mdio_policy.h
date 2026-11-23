#ifndef _RA_SWITCH_MDIO_POLICY_H
#define _RA_SWITCH_MDIO_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>

#endif

/* Page-select register 31 gates every local/global PHY register access. */
typedef int (*ra_switch_mdio_read_fn)(void *, unsigned int, unsigned int, unsigned int *);
typedef int (*ra_switch_mdio_read_cl45_fn)(void *, unsigned int, unsigned int,
					   unsigned int, unsigned int *);
typedef int (*ra_switch_mdio_write_fn)(void *, unsigned int, unsigned int, unsigned int);

static inline int ra_switch_mdio_normalize_status(int status)
{
	return status > 0 ? -EIO : status;
}

/* Publish an MDIO sample only after the underlying transaction succeeds. */
static inline int
ra_switch_mdio_read_checked(unsigned int port, unsigned int reg,
			    ra_switch_mdio_read_fn read, void *context,
			    unsigned int *value)
{
	unsigned int result = 0;
	int status;

	if (!read || !value)
		return -EINVAL;
	*value = 0;
	status = ra_switch_mdio_normalize_status(read(context, port, reg, &result));
	if (status != 0)
		return status;
	*value = result;
	return 0;
}

/* Publish a Clause 45 sample only after the complete MDIO transaction succeeds. */
static inline int
ra_switch_mdio_read_cl45_checked(unsigned int port, unsigned int dev,
				 unsigned int reg, ra_switch_mdio_read_cl45_fn read,
				 void *context, unsigned int *value)
{
	unsigned int result = 0;
	int status;

	if (!read || !value)
		return -EINVAL;
	*value = 0;
	status = ra_switch_mdio_normalize_status(
		read(context, port, dev, reg, &result));
	if (status != 0)
		return status;
	*value = result;
	return 0;
}

/* Keep the caller's previous link state untouched when an MDIO sample fails. */
static inline int
ra_switch_mdio_read_link_status(unsigned int port, unsigned int reg,
				unsigned int link_mask,
				ra_switch_mdio_read_fn read,
				void *context, int *link_up)
{
	unsigned int value;
	int status;

	if (!link_up || !link_mask)
		return -EINVAL;
	status = ra_switch_mdio_read_checked(port, reg, read, context, &value);
	if (status != 0)
		return status;
	*link_up = (value & link_mask) != 0;
	return 0;
}

static inline int
ra_switch_mdio_page_read(int local, unsigned int port, unsigned int page,
			 unsigned int reg, ra_switch_mdio_read_fn read,
			 ra_switch_mdio_write_fn write, void *context,
			 unsigned int *value)
{
	unsigned int page_select;
	int status;

	if (!read || !write || !value)
		return -EINVAL;
	*value = 0;
	page_select = ((local ? 1U : 0U) << 15) | ((page & 0x7U) << 12);
	status = ra_switch_mdio_normalize_status(write(context, port, 31U, page_select));
	if (status != 0)
		return status;
	return ra_switch_mdio_read_checked(port, reg, read, context, value);
}

static inline int
ra_switch_mdio_page_write(int local, unsigned int port, unsigned int page,
			  unsigned int reg, unsigned int value,
			  ra_switch_mdio_write_fn write, void *context)
{
	unsigned int page_select;
	int status;

	if (!write)
		return -EINVAL;
	page_select = ((local ? 1U : 0U) << 15) | ((page & 0x7U) << 12);
	status = ra_switch_mdio_normalize_status(write(context, port, 31U, page_select));
	if (status != 0)
		return status;
	return ra_switch_mdio_normalize_status(write(context, port, reg, value));
}

typedef void (*ra_switch_mdio_delay_fn)(void *context);

static inline int
ra_switch_mdio_wait_nonzero(ra_switch_mdio_read_fn read,
			    ra_switch_mdio_delay_fn delay, void *context,
			    unsigned int port, unsigned int reg,
			    unsigned int attempts, unsigned int *value)
{
	unsigned int sample;
	unsigned int i;
	int status;

	if (!read || !delay || !value || attempts == 0U)
		return -EINVAL;
	*value = 0;
	for (i = 0U; i < attempts; ++i) {
		delay(context);
		sample = 0;
		status = ra_switch_mdio_normalize_status(
			read(context, port, reg, &sample));
		if (status != 0)
			return status;
		if (sample != 0U) {
			*value = sample;
			return 0;
		}
	}
	return -ETIMEDOUT;
}

#endif
