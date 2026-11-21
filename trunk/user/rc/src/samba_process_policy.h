#ifndef PADAVAN_SAMBA_PROCESS_POLICY_H
#define PADAVAN_SAMBA_PROCESS_POLICY_H
#include <errno.h>
#include "process_inventory.h"
enum samba_daemon_action { SAMBA_DAEMON_ACTION_ABORT=-1, SAMBA_DAEMON_ACTION_START=0, SAMBA_DAEMON_ACTION_RELOAD=1 };
static enum samba_daemon_action samba_daemon_action_for_presence(enum process_presence state)
{
	if(state==PROCESS_PRESENCE_ABSENT)return SAMBA_DAEMON_ACTION_START;
	if(state==PROCESS_PRESENCE_RUNNING)return SAMBA_DAEMON_ACTION_RELOAD;
	return SAMBA_DAEMON_ACTION_ABORT;
}
/* Stop/restart postconditions require a complete ABSENT inventory. */
static int
samba_process_require_absent(enum process_presence state, int scan_error)
{
	if (state == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (state == PROCESS_PRESENCE_RUNNING)
		errno = EBUSY;
	else if (state == PROCESS_PRESENCE_UNKNOWN)
		errno = scan_error > 0 ? scan_error : EIO;
	else
		errno = EIO;
	return -1;
}
typedef enum process_presence (*samba_process_identity_validate_fn)(void *,const struct process_identity *);
typedef int (*samba_process_signal_fn)(void *,pid_t,int);
/* Complete preflight before first signal; revalidate each PID before its targeted signal. */
static int samba_daemon_signal_snapshot(const struct process_snapshot *snapshot,int sig,
				samba_process_identity_validate_fn validate,
				samba_process_signal_fn send_signal,void *context)
{
	size_t i;int signalled=0,saved_errno;enum process_presence state;
	if(!snapshot||!validate||!send_signal||sig<=0){errno=EINVAL;return -1;}
	if(snapshot->state!=PROCESS_PRESENCE_RUNNING||!snapshot->items||!snapshot->count){
		errno=snapshot->state==PROCESS_PRESENCE_ABSENT?ESRCH:EIO;return -1;
	}
	for(i=0;i<snapshot->count;i++){
		errno=0;state=validate(context,&snapshot->items[i]);
		if(state==PROCESS_PRESENCE_UNKNOWN){errno=errno?errno:EIO;return -1;}
		if(state!=PROCESS_PRESENCE_RUNNING&&state!=PROCESS_PRESENCE_ABSENT){errno=EIO;return -1;}
	}
	for(i=0;i<snapshot->count;i++){
		errno=0;state=validate(context,&snapshot->items[i]);
		if(state==PROCESS_PRESENCE_ABSENT)continue;
		if(state!=PROCESS_PRESENCE_RUNNING){errno=state==PROCESS_PRESENCE_UNKNOWN&&errno?errno:EIO;return -1;}
		if(send_signal(context,snapshot->items[i].pid,sig)!=0){
			saved_errno=errno?errno:EIO;if(saved_errno==ESRCH)continue;errno=saved_errno;return -1;
		}
		signalled++;
	}
	if(!signalled){errno=ESRCH;return -1;}return 0;
}
/* Choose the service set after restart_smbd() stops both share daemons.
 * WINS remains an independent nmbd consumer, even when Samba shares are off. */
enum samba_storage_restart_action {
	SAMBA_STORAGE_RESTART_ACTION_ABORT = -1,
	SAMBA_STORAGE_RESTART_ACTION_START_WINS = 0,
	SAMBA_STORAGE_RESTART_ACTION_START_SAMBA = 1
};

static enum samba_storage_restart_action
samba_storage_restart_action_for(int samba_enabled, int mounted_devices)
{
	if ((samba_enabled != 0 && samba_enabled != 1) ||
	    (mounted_devices != 0 && mounted_devices != 1))
		return SAMBA_STORAGE_RESTART_ACTION_ABORT;
	if (!samba_enabled || !mounted_devices)
		return SAMBA_STORAGE_RESTART_ACTION_START_WINS;
	return SAMBA_STORAGE_RESTART_ACTION_START_SAMBA;
}
#endif
