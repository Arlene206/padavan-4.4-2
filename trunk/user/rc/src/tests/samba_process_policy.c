#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "samba_process_policy.h"
struct fake { const enum process_presence *states; size_t count,next; int validation_error,signal_error,signal_count; pid_t pids[4]; int sigs[4]; };
static enum process_presence validate(void *ctx,const struct process_identity *id)
{
	struct fake *x=ctx;enum process_presence s;assert(id!=NULL);
	s=x->next<x->count?x->states[x->next++]:PROCESS_PRESENCE_ABSENT;
	if(s==PROCESS_PRESENCE_UNKNOWN)errno=x->validation_error?x->validation_error:EIO;
	return s;
}
static int send_signal(void *ctx,pid_t pid,int sig)
{
	struct fake *x=ctx;if(x->signal_error){errno=x->signal_error;return -1;}
	assert(x->signal_count<4);x->pids[x->signal_count]=pid;x->sigs[x->signal_count++]=sig;return 0;
}
int main(void)
{
	struct process_identity ids[2];struct process_snapshot snap;
	memset(ids,0,sizeof(ids));ids[0].pid=101;ids[1].pid=202;
	memset(&snap,0,sizeof(snap));snap.state=PROCESS_PRESENCE_RUNNING;snap.items=ids;snap.count=2;
	assert(samba_daemon_action_for_presence(PROCESS_PRESENCE_ABSENT)==SAMBA_DAEMON_ACTION_START);
	assert(samba_daemon_action_for_presence(PROCESS_PRESENCE_RUNNING)==SAMBA_DAEMON_ACTION_RELOAD);
	assert(samba_daemon_action_for_presence(PROCESS_PRESENCE_UNKNOWN)==SAMBA_DAEMON_ACTION_ABORT);
	assert(samba_daemon_action_for_presence((enum process_presence)9)==SAMBA_DAEMON_ACTION_ABORT);
	assert(samba_process_require_absent(PROCESS_PRESENCE_ABSENT,0)==0);
	errno=0;assert(samba_process_require_absent(PROCESS_PRESENCE_RUNNING,0)==-1&&errno==EBUSY);
	errno=0;assert(samba_process_require_absent(PROCESS_PRESENCE_UNKNOWN,EACCES)==-1&&errno==EACCES);
	errno=0;assert(samba_process_require_absent(PROCESS_PRESENCE_UNKNOWN,0)==-1&&errno==EIO);
	errno=0;assert(samba_process_require_absent((enum process_presence)9,0)==-1&&errno==EIO);
	{
		const enum process_presence s[]={PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_RUNNING};
		struct fake x={s,4,0,0,0,0,{0},{0}};
		assert(samba_daemon_signal_snapshot(&snap,SIGHUP,validate,send_signal,&x)==0);
		assert(x.next==4&&x.signal_count==2&&x.pids[0]==101&&x.pids[1]==202&&x.sigs[1]==SIGHUP);
	}
	{
		const enum process_presence s[]={PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_UNKNOWN};
		struct fake x={s,2,0,EACCES,0,0,{0},{0}};
		errno=0;assert(samba_daemon_signal_snapshot(&snap,SIGHUP,validate,send_signal,&x)==-1);
		assert(errno==EACCES&&x.next==2&&x.signal_count==0);
	}
	{
		const enum process_presence s[]={PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_UNKNOWN};
		struct fake x={s,4,0,EACCES,0,0,{0},{0}};
		errno=0;assert(samba_daemon_signal_snapshot(&snap,SIGHUP,validate,send_signal,&x)==-1);
		assert(errno==EACCES&&x.signal_count==1&&x.pids[0]==101);
	}
	{
		const enum process_presence s[]={PROCESS_PRESENCE_ABSENT,PROCESS_PRESENCE_ABSENT,PROCESS_PRESENCE_ABSENT,PROCESS_PRESENCE_ABSENT};
		struct fake x={s,4,0,0,0,0,{0},{0}};
		errno=0;assert(samba_daemon_signal_snapshot(&snap,SIGHUP,validate,send_signal,&x)==-1);
		assert(errno==ESRCH&&x.signal_count==0);
	}
	{
		const enum process_presence s[]={
			PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_RUNNING,
			PROCESS_PRESENCE_RUNNING,PROCESS_PRESENCE_RUNNING
		};
		struct fake x={s,4,0,0,EPERM,0,{0},{0}};
		errno=0;assert(samba_daemon_signal_snapshot(&snap,SIGHUP,validate,send_signal,&x)==-1);
		assert(errno==EPERM&&x.signal_count==0);
	}
	{
		struct fake x;memset(&x,0,sizeof(x));
		errno=0;assert(samba_daemon_signal_snapshot(NULL,SIGHUP,validate,send_signal,&x)==-1&&errno==EINVAL);
		snap.state=PROCESS_PRESENCE_ABSENT;errno=0;
		assert(samba_daemon_signal_snapshot(&snap,SIGHUP,validate,send_signal,&x)==-1&&errno==ESRCH);
		snap.state=PROCESS_PRESENCE_RUNNING;snap.count=0;errno=0;
		assert(samba_daemon_signal_snapshot(&snap,SIGHUP,validate,send_signal,&x)==-1&&errno==EIO);
	}
	assert(samba_storage_restart_action_for(1, 1) ==
	       SAMBA_STORAGE_RESTART_ACTION_START_SAMBA);
	assert(samba_storage_restart_action_for(1, 0) ==
	       SAMBA_STORAGE_RESTART_ACTION_START_WINS);
	assert(samba_storage_restart_action_for(0, 1) ==
	       SAMBA_STORAGE_RESTART_ACTION_START_WINS);
	assert(samba_storage_restart_action_for(0, 0) ==
	       SAMBA_STORAGE_RESTART_ACTION_START_WINS);
	assert(samba_storage_restart_action_for(1, -1) ==
	       SAMBA_STORAGE_RESTART_ACTION_ABORT);
	assert(samba_storage_restart_action_for(0, -1) ==
	       SAMBA_STORAGE_RESTART_ACTION_ABORT);
	assert(samba_storage_restart_action_for(1, 2) ==
	       SAMBA_STORAGE_RESTART_ACTION_ABORT);
	assert(samba_storage_restart_action_for(2, 1) ==
	       SAMBA_STORAGE_RESTART_ACTION_ABORT);
	puts("Samba process action and identity-bound signal policy: PASS");return 0;
}
