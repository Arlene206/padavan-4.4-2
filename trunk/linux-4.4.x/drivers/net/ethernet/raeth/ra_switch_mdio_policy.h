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
typedef int (*ra_switch_mdio_write_cl45_fn)(void *, unsigned int, unsigned int,
					    unsigned int, unsigned int);
typedef void (*ra_switch_mdio_delay_fn)(void *);

static inline int ra_switch_mdio_normalize_status(int status)
{
	return status > 0 ? -EIO : status;
}

/* Do not let legacy PHY write wrappers silently discard MDIO failures. */
static inline int
ra_switch_mdio_write_checked(unsigned int port, unsigned int reg,
			     unsigned int value,
			     ra_switch_mdio_write_fn write, void *context)
{
	if (!write)
		return -EINVAL;
	return ra_switch_mdio_normalize_status(write(context, port, reg, value));
}

static inline int
ra_switch_mdio_write_cl45_checked(unsigned int port, unsigned int dev,
				  unsigned int reg, unsigned int value,
				  ra_switch_mdio_write_cl45_fn write,
				  void *context)
{
	if (!write)
		return -EINVAL;
	return ra_switch_mdio_normalize_status(
		write(context, port, dev, reg, value));
}

/*
 * Restore a saved calibration register when valid, then always try to
 * disable the calibration engine. Preserve the first I/O error.
 */
static inline int
ra_switch_mdio_restore_cl45_and_disable(
		unsigned int port,
		unsigned int restore_dev, unsigned int restore_reg,
		unsigned int restore_value, int restore_valid,
		unsigned int disable_dev, unsigned int disable_reg,
		ra_switch_mdio_write_cl45_fn write, void *context)
{
	int status = 0;
	int cleanup_status;

	if (!write)
		return -EINVAL;
	if (restore_valid)
		status = ra_switch_mdio_write_cl45_checked(
			port, restore_dev, restore_reg, restore_value,
			write, context);
	cleanup_status = ra_switch_mdio_write_cl45_checked(
		port, disable_dev, disable_reg, 0U, write, context);
	if (status == 0)
		status = cleanup_status;
	return status;
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

/*
 * Update selected register bits only after a checked read. The new value is
 * published only after the write succeeds.
 */
static inline int
ra_switch_mdio_update_bits_checked(unsigned int port, unsigned int reg,
				   unsigned int clear_mask,
				   unsigned int set_mask,
				   ra_switch_mdio_read_fn read,
				   ra_switch_mdio_write_fn write,
				   void *context,
				   unsigned int *updated_value)
{
	unsigned int value = 0;
	int status;

	if (!read || !write || !updated_value)
		return -EINVAL;
	*updated_value = 0;
	status = ra_switch_mdio_read_checked(port, reg, read, context, &value);
	if (status != 0)
		return status;
	value = (value & ~clear_mask) | set_mask;
	status = ra_switch_mdio_write_checked(port, reg, value, write, context);
	if (status != 0)
		return status;
	*updated_value = value;
	return 0;
}

/* Attempt both cleanup writes, preserving the first error. */
static inline int
ra_switch_mdio_cleanup_two_writes(unsigned int port_a, unsigned int reg_a,
				  unsigned int value_a,
				  unsigned int port_b, unsigned int reg_b,
				  unsigned int value_b,
				  ra_switch_mdio_write_fn write,
				  void *context)
{
	int first_status;
	int second_status;

	if (!write)
		return -EINVAL;
	first_status = ra_switch_mdio_write_checked(port_a, reg_a, value_a,
						    write, context);
	second_status = ra_switch_mdio_write_checked(port_b, reg_b, value_b,
						     write, context);
	return first_status != 0 ? first_status : second_status;
}

/*
 * Start an analog calibration, poll its ready bit, and always attempt to
 * clear the trigger bit. A cleanup failure also invalidates a ready sample.
 */
static inline int
ra_switch_mdio_calibration_wait(unsigned int port, unsigned int reg,
				unsigned int trigger_mask,
				unsigned int ready_mask,
				unsigned int max_polls,
				ra_switch_mdio_read_fn read,
				 ra_switch_mdio_write_fn write,
				 void *context,
				 ra_switch_mdio_delay_fn delay,
				 void *delay_context,
				 unsigned int *ready)
{
	unsigned int value = 0;
	unsigned int i;
	int status;
	int cleanup_status;

	if (!read || !write || !ready || max_polls == 0U)
		return -EINVAL;
	*ready = 0;

	status = ra_switch_mdio_read_checked(port, reg, read, context, &value);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_write_checked(port, reg,
					      value & ~trigger_mask,
					      write, context);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_read_checked(port, reg, read, context, &value);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_write_checked(port, reg,
					      value | trigger_mask,
					      write, context);
	if (status != 0)
		goto cleanup;

	for (i = 0U; i < max_polls; ++i) {
		if (delay)
			delay(delay_context);
		status = ra_switch_mdio_read_checked(port, reg, read,
						     context, &value);
		if (status != 0)
			goto cleanup;
		if (value & ready_mask) {
			*ready = 1U;
			break;
		}
	}

cleanup:
	/* Re-read before modifying the register; never restore stale control bits. */
	cleanup_status = ra_switch_mdio_read_checked(port, reg, read,
						     context, &value);
	if (cleanup_status == 0)
		cleanup_status = ra_switch_mdio_write_checked(
			port, reg, value & ~trigger_mask, write, context);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	if (status != 0)
		*ready = 0U;
	return status;
}

/* One coherent snapshot for the GE2 link polling transaction. */
struct ra_switch_mdio_phy_link_snapshot {
	unsigned int link_up;
	unsigned int fc_phy;
	unsigned int fc_lp;
	unsigned int link_speed;
	unsigned int local_eee;
	unsigned int lp_eee;
};

/* Do not publish a partial link sample or turn an MDIO error into link-down. */
static inline int
ra_switch_mdio_read_phy_link_snapshot(unsigned int port,
				      ra_switch_mdio_read_fn read,
				      ra_switch_mdio_read_cl45_fn read_cl45,
				      void *context,
				      struct ra_switch_mdio_phy_link_snapshot *snapshot)
{
	struct ra_switch_mdio_phy_link_snapshot sample;
	unsigned int value;
	int status;

	if (!read || !read_cl45 || !snapshot)
		return -EINVAL;

	snapshot->link_up = 0;
	snapshot->fc_phy = 0;
	snapshot->fc_lp = 0;
	snapshot->link_speed = 0;
	snapshot->local_eee = 0;
	snapshot->lp_eee = 0;
	sample.link_up = 0;
	sample.fc_phy = 0;
	sample.fc_lp = 0;
	sample.link_speed = 0;
	sample.local_eee = 0;
	sample.lp_eee = 0;

	status = ra_switch_mdio_read_checked(port, 0x1U, read, context, &value);
	if (status != 0)
		return status;
	sample.link_up = (value >> 2) & 0x1U;
	if (!sample.link_up) {
		*snapshot = sample;
		return 0;
	}

	status = ra_switch_mdio_read_checked(port, 0x4U, read, context,
					     &sample.fc_phy);
	if (status != 0)
		return status;
	status = ra_switch_mdio_read_checked(port, 0x5U, read, context,
					     &sample.fc_lp);
	if (status != 0)
		return status;
	status = ra_switch_mdio_read_cl45_checked(port, 0x1eU, 0xa2U,
						  read_cl45, context,
						  &sample.link_speed);
	if (status != 0)
		return status;
	status = ra_switch_mdio_read_cl45_checked(port, 0x7U, 0x3cU,
						  read_cl45, context,
						  &sample.local_eee);
	if (status != 0)
		return status;
	status = ra_switch_mdio_read_cl45_checked(port, 0x7U, 0x3dU,
						  read_cl45, context,
						  &sample.lp_eee);
	if (status != 0)
		return status;

	*snapshot = sample;
	return 0;
}

/* Rebuild volatile capability bits so renegotiation can clear stale EEE. */
static inline unsigned int
ra_switch_mdio_update_ge2_mac_value(unsigned int previous,
				    unsigned int fc_phy,
				    unsigned int fc_lp,
				    unsigned int link_speed,
				    unsigned int local_eee,
				    unsigned int lp_eee)
{
	unsigned int value = previous;

	if ((fc_phy & 0xc00U) == (fc_lp & 0xc00U))
		value |= 0x30U;
	else
		value &= ~0x30U;

	if (link_speed & 0x20U)
		value |= 0x2U;
	else
		value &= ~0x2U;

	value &= ~0x0cU;
	if ((link_speed & 0x0eU) == 0x04U)
		value |= 0x04U;
	else if ((link_speed & 0x0eU) == 0x08U)
		value |= 0x08U;

	value &= ~(0x80U | 0x40U);
	if ((local_eee & 0x4U) && (lp_eee & 0x4U))
		value |= 0x80U;
	if ((local_eee & 0x2U) && (lp_eee & 0x2U))
		value |= 0x40U;

	return value & 0xffU;
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
