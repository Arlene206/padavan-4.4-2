#ifndef RC_NETWORK_SYSCTL_H
#define RC_NETWORK_SYSCTL_H

/* Status-returning network sysctl operations shared by WAN startup/reload. */
int set_nf_conntrack(void);
int set_tcp_syncookies(int enabled, int *syncookies_status);
int set_igmp_mld_version(void);

#endif /* RC_NETWORK_SYSCTL_H */
