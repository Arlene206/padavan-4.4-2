#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "ppp_wan_option_policy.h"

#define MAX_READS 12
struct fake_reader {
	int fail_at, failure_status, calls;
	const char *names[MAX_READS];
	int values[MAX_READS];
};
static int fake_read(void *context,const char *name,int *value)
{
	struct fake_reader *f=(struct fake_reader *)context;
	int i;
	assert(f && name && value);
	i=f->calls++;
	assert(i<MAX_READS);
	f->names[i]=name;
	if(i==f->fail_at) return f->failure_status;
	*value=f->values[i];
	return 0;
}
static void test_pppoe_options(void)
{
	static const char *const names[]={"ppp_auth","pppoe_mtu","pppoe_mru","pppoe_idletime","pppoe_demand","pppoe_txonly_x","dnsenable_x","ppp_lcp","ppp_alcp"};
	struct fake_reader f; struct ppp_wan_option_values v; unsigned int i;
	memset(&f,0,sizeof(f)); f.fail_at=-1;
	for(i=0;i<9;i++) f.values[i]=(int)i+1;
	assert(ppp_wan_option_values_read(1,0,0,fake_read,&f,&v)==0);
	assert(f.calls==9);
	for(i=0;i<9;i++) assert(strcmp(f.names[i],names[i])==0);
	assert(v.auth_type==1 && v.mtu==2 && v.mru==3);
	assert(v.pppoe_idletime==4 && v.pppoe_demand==5 && v.pppoe_txonly_x==6);
	assert(v.dnsenable_x==7 && v.lcp_echo==8 && v.adaptive_lcp==9);
	assert(v.mppe_mode==0);
}
static void test_pptp_and_l2tp_options(void)
{
	static const char *const pptp[]={"ppp_auth","pptp_mtu","pptp_mru","dnsenable_x","ppp_mppe","ppp_lcp","ppp_alcp"};
	static const char *const l2tp[]={"ppp_auth","l2tp_mtu","l2tp_mru","dnsenable_x","ppp_mppe","ppp_lcp","ppp_alcp"};
	struct fake_reader f; struct ppp_wan_option_values v; unsigned int i;
	memset(&f,0,sizeof(f)); f.fail_at=-1;
	for(i=0;i<7;i++) f.values[i]=(int)i+10;
	assert(ppp_wan_option_values_read(0,1,0,fake_read,&f,&v)==0 && f.calls==7);
	for(i=0;i<7;i++) assert(strcmp(f.names[i],pptp[i])==0);
	assert(v.auth_type==10 && v.mtu==11 && v.mru==12);
	assert(v.dnsenable_x==13 && v.mppe_mode==14 && v.lcp_echo==15 && v.adaptive_lcp==16);
	memset(&f,0,sizeof(f)); f.fail_at=-1;
	for(i=0;i<7;i++) f.values[i]=(int)i+20;
	assert(ppp_wan_option_values_read(0,0,1,fake_read,&f,&v)==0 && f.calls==7);
	for(i=0;i<7;i++) assert(strcmp(f.names[i],l2tp[i])==0);
	assert(v.auth_type==20 && v.mtu==21 && v.mru==22);
	assert(v.dnsenable_x==23 && v.mppe_mode==24 && v.lcp_echo==25 && v.adaptive_lcp==26);
}
static void test_missing_values_keep_zero_defaults(void)
{
	struct fake_reader f; struct ppp_wan_option_values v;
	memset(&f,0,sizeof(f)); f.fail_at=-1;
	assert(ppp_wan_option_values_read(1,0,0,fake_read,&f,&v)==0);
	assert(f.calls==9);
	assert(v.auth_type==0 && v.mtu==0 && v.mru==0);
	assert(v.pppoe_idletime==0 && v.pppoe_demand==0 && v.pppoe_txonly_x==0);
	assert(v.dnsenable_x==0 && v.mppe_mode==0 && v.lcp_echo==0 && v.adaptive_lcp==0);
}
static void test_failures_stop_and_clear_snapshot(void)
{
	struct fake_reader f; struct ppp_wan_option_values v; int fail_at;
	for(fail_at=0;fail_at<9;fail_at++) {
		memset(&f,0,sizeof(f)); f.fail_at=fail_at; f.failure_status=EACCES;
		memset(&v,0x5a,sizeof(v));
		assert(ppp_wan_option_values_read(1,0,0,fake_read,&f,&v)==EACCES);
		assert(f.calls==fail_at+1);
		assert(v.auth_type==0 && v.mtu==0 && v.mru==0);
		assert(v.pppoe_idletime==0 && v.pppoe_demand==0 && v.pppoe_txonly_x==0);
		assert(v.dnsenable_x==0 && v.mppe_mode==0 && v.lcp_echo==0 && v.adaptive_lcp==0);
	}
}
static void test_invalid_arguments_fail_closed(void)
{
	struct fake_reader f; struct ppp_wan_option_values v;
	memset(&f,0,sizeof(f));
	assert(ppp_wan_option_values_read(0,0,0,fake_read,&f,&v)==EINVAL);
	assert(ppp_wan_option_values_read(1,1,0,fake_read,&f,&v)==EINVAL);
	assert(ppp_wan_option_values_read(1,0,0,NULL,&f,&v)==EINVAL);
	assert(ppp_wan_option_values_read(1,0,0,fake_read,&f,NULL)==EINVAL);
	assert(f.calls==0);
}
int main(void)
{
	test_pppoe_options();
	test_pptp_and_l2tp_options();
	test_missing_values_keep_zero_defaults();
	test_failures_stop_and_clear_snapshot();
	test_invalid_arguments_fail_closed();
	puts("PPP WAN option checked-read preflight regression: PASS");
	return 0;
}
