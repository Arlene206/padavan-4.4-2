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
#define MAX_TEXT_READS 10
struct fake_text_reader {
	int fail_at, failure_status, calls;
	const char *names[MAX_TEXT_READS];
	const char *values[MAX_TEXT_READS];
};

static int
fake_read_text(void *context, const char *name, char *value, size_t value_size)
{
	struct fake_text_reader *fake = (struct fake_text_reader *)context;
	int index;
	size_t length;

	assert(fake && name && value && value_size > 0);
	index = fake->calls++;
	assert(index < MAX_TEXT_READS);
	fake->names[index] = name;
	if (index == fake->fail_at)
		return fake->failure_status;
	if (!fake->values[index])
		fake->values[index] = "";
	length = strlen(fake->values[index]);
	if (length >= value_size)
		return ENAMETOOLONG;
	memcpy(value, fake->values[index], length + 1);
	return 0;
}

static void
test_pppoe_string_snapshot_reads_all_fields(void)
{
	static const char *const names[] = {
		"pppoe_username", "pppoe_passwd", "ppp_peer",
		"pppoe_ac", "pppoe_service", "ppp_pppd"
	};
	static const char *const inputs[] = {
		"alice", "secret", "peer.example", "access-node", "fiber", "debug"
	};
	struct fake_text_reader fake;
	struct ppp_wan_option_text_values values;
	int i;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	for (i = 0; i < 6; ++i)
		fake.values[i] = inputs[i];
	assert(ppp_wan_option_text_values_read(1, fake_read_text, &fake, &values) == 0);
	assert(fake.calls == 6);
	for (i = 0; i < 6; ++i)
		assert(strcmp(fake.names[i], names[i]) == 0);
	assert(strcmp(values.username, "alice") == 0);
	assert(strcmp(values.password, "secret") == 0);
	assert(strcmp(values.peer, "peer.example") == 0);
	assert(strcmp(values.access_concentrator, "access-node") == 0);
	assert(strcmp(values.service, "fiber") == 0);
	assert(strcmp(values.user_options, "debug") == 0);
}

static void
test_empty_peer_uses_checked_management_gateway_fallback(void)
{
	static const char *const names[] = {
		"pppoe_username", "pppoe_passwd", "ppp_peer", "man_gateway", "ppp_pppd"
	};
	struct fake_text_reader fake;
	struct ppp_wan_option_text_values values;
	int i;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.values[0] = "alice";
	fake.values[1] = "secret";
	fake.values[2] = "";
	fake.values[3] = "gateway.example";
	fake.values[4] = "";
	assert(ppp_wan_option_text_values_read(0, fake_read_text, &fake, &values) == 0);
	assert(fake.calls == 5);
	for (i = 0; i < 5; ++i)
		assert(strcmp(fake.names[i], names[i]) == 0);
	assert(strcmp(values.peer, "gateway.example") == 0);
	assert(values.access_concentrator[0] == '\0');
	assert(values.service[0] == '\0');
}

static void
test_text_backend_failure_discards_partial_values(void)
{
	struct fake_text_reader fake;
	struct ppp_wan_option_text_values values;
	int failure;

	for (failure = 0; failure < 6; ++failure) {
		memset(&fake, 0, sizeof(fake));
		fake.fail_at = failure;
		fake.failure_status = EACCES;
		fake.values[0] = "alice";
		fake.values[1] = "secret";
		fake.values[2] = "peer";
		fake.values[3] = "ac";
		fake.values[4] = "service";
		fake.values[5] = "extra";
		memset(&values, 0x5a, sizeof(values));
		assert(ppp_wan_option_text_values_read(1, fake_read_text, &fake, &values) == EACCES);
		assert(fake.calls == failure + 1);
		assert(values.username[0] == '\0' && values.password[0] == '\0');
		assert(values.peer[0] == '\0' && values.access_concentrator[0] == '\0');
		assert(values.service[0] == '\0' && values.user_options[0] == '\0');
	}
}

static void
test_empty_text_defaults_and_oversize_failure(void)
{
	struct fake_text_reader fake;
	struct ppp_wan_option_text_values values;
	char oversized[300];

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	assert(ppp_wan_option_text_values_read(1, fake_read_text, &fake, &values) == 0);
	assert(fake.calls == 7);
	assert(values.username[0] == '\0' && values.password[0] == '\0');
	assert(values.peer[0] == '\0' && values.access_concentrator[0] == '\0');
	assert(values.service[0] == '\0' && values.user_options[0] == '\0');

	memset(oversized, 'x', sizeof(oversized) - 1);
	oversized[sizeof(oversized) - 1] = '\0';
	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.values[0] = oversized;
	assert(ppp_wan_option_text_values_read(0, fake_read_text, &fake, &values) == ENAMETOOLONG);
	assert(fake.calls == 1 && values.username[0] == '\0');
}

int main(void)
{
	test_pppoe_options();
	test_pptp_and_l2tp_options();
	test_missing_values_keep_zero_defaults();
	test_failures_stop_and_clear_snapshot();
	test_invalid_arguments_fail_closed();
	test_pppoe_string_snapshot_reads_all_fields();
	test_empty_peer_uses_checked_management_gateway_fallback();
	test_text_backend_failure_discards_partial_values();
	test_empty_text_defaults_and_oversize_failure();
	puts("PPP WAN option checked-read preflight regression: PASS");
	return 0;
}
