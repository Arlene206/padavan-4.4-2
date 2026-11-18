#ifndef __MODEM_SG_MODULE_POLICY_H__
#define __MODEM_SG_MODULE_POLICY_H__

/* The module must be positively present after the load/already-loaded path. */
static inline int
modem_sg_module_ready_status(int module_presence)
{
	return (module_presence == 1) ? 0 : -1;
}

#endif
