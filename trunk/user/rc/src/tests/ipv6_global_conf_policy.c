#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "ipv6_global_conf_policy.h"
static const char *names[4], *params[4];
static int values[4], statuses[4], calls, mismatch;
static void reset_writer(void) { calls=0; mismatch=0; }
static void expect(int i,const char *n,const char *p,int v,int s)
{ names[i]=n; params[i]=p; values[i]=v; statuses[i]=s; }
static int fake_write(void *ctx,const char *n,const char *p,int v)
{
 int i=calls++; (void)ctx;
 if(i>=4||!names[i]||!params[i]||!n||!p||strcmp(names[i],n)||strcmp(params[i],p)||values[i]!=v){mismatch=1;return EINVAL;}
 return statuses[i];
}
static void test_all_attempted_first_error(void)
{
 const struct rc_ipv6_conf_operation ops[]={{"all","forwarding",1},{"default","disable_ipv6",0},{"br0","disable_ipv6",0},{"all","forwarding",0}};
 reset_writer(); expect(0,"all","forwarding",1,0); expect(1,"default","disable_ipv6",0,EIO); expect(2,"br0","disable_ipv6",0,EPERM); expect(3,"all","forwarding",0,0);
 assert(rc_ipv6_conf_apply(ops,4,fake_write,NULL)==EIO); assert(calls==4&&!mismatch);
}
static void test_success(void)
{
 const struct rc_ipv6_conf_operation ops[]={{"default","accept_ra",0},{"lo","accept_ra_pinfo",0}};
 reset_writer(); expect(0,"default","accept_ra",0,0); expect(1,"lo","accept_ra_pinfo",0,0);
 assert(rc_ipv6_conf_apply(ops,2,fake_write,NULL)==0); assert(calls==2&&!mismatch);
}
static void test_zero_operations(void){reset_writer();assert(rc_ipv6_conf_apply(NULL,0,NULL,NULL)==0);assert(calls==0);}
static void test_invalid_inputs(void)
{
 const struct rc_ipv6_conf_operation op={"default","disable_ipv6",1};
 reset_writer(); assert(rc_ipv6_conf_apply(NULL,1,fake_write,NULL)==EINVAL); assert(rc_ipv6_conf_apply(&op,1,NULL,NULL)==EINVAL); assert(calls==0);
}
int main(void){test_all_attempted_first_error();test_success();test_zero_operations();test_invalid_inputs();puts("IPv6 global interface configuration result propagation: PASS");return 0;}
