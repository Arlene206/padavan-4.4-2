#ifndef _RA_SWITCH_MDIO_POLICY_H
#define _RA_SWITCH_MDIO_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>

#endif

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

/* Page-select register 31 gates every local/global PHY register access. */
typedef int (*ra_switch_mdio_read_fn)(void *, unsigned int, unsigned int, unsigned int *);
typedef int (*ra_switch_mdio_read_cl45_fn)(void *, unsigned int, unsigned int,
					   unsigned int, unsigned int *);
typedef int (*ra_switch_mdio_write_fn)(void *, unsigned int, unsigned int, unsigned int);
typedef int (*ra_switch_mdio_write_cl45_fn)(void *, unsigned int, unsigned int,
					    unsigned int, unsigned int);

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
