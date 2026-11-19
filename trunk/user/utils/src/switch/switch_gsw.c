#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

#include <ra_ioctl.h>
#include "switch_ipv4_parse_policy.h"
#include "switch_wait_policy.h"

#define RT_SWITCH_HELP		1
#define RT_TABLE_MANIPULATE	1

#if defined (CONFIG_RALINK_MT7620) && !defined (CONFIG_MT7530_GSW)
#define MAX_PORT		7
#else
#define MAX_PORT		6
#endif

#define REG_ESW_WT_MAC_MFC	0x10
#define REG_ESW_WT_MAC_ATA1	0x74
#define REG_ESW_WT_MAC_ATA2	0x78
#define REG_ESW_WT_MAC_ATWD	0x7C
#define REG_ESW_WT_MAC_ATC	0x80

#define REG_ESW_TABLE_TSRA1	0x84
#define REG_ESW_TABLE_TSRA2	0x88
#define REG_ESW_TABLE_ATRD	0x8C

#define REG_ESW_VLAN_VTCR	0x90
#define REG_ESW_VLAN_VAWD1	0x94
#define REG_ESW_VLAN_VAWD2	0x98

#if !defined (CONFIG_MT7530_GSW)
#define REG_ESW_VLAN_ID_BASE	0x100
#endif

static int esw_fd = -1;

void switch_init(void)
{
	esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (esw_fd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
}

void switch_fini(void)
{
	close(esw_fd);
	esw_fd = -1;
}

static unsigned int
parse_ipv4_argument(const char *text)
{
	unsigned int address;

	if (switch_parse_ipv4_address(text, &address) != 0) {
		fprintf(stderr, "switch: invalid IPv4 address: %.64s\n",
			text ? text : "(null)");
		switch_fini();
		exit(EXIT_FAILURE);
	}
	return address;
}

static unsigned int
parse_portmap_argument(const char *text)
{
	unsigned int portmap;

	if (switch_parse_portmap(text, &portmap) != 0) {
		fprintf(stderr, "switch: invalid port map: %.64s\n",
			text ? text : "(null)");
		switch_fini();
		exit(EXIT_FAILURE);
	}
	return portmap;
}

void usage(char *cmd)
{
#if RT_SWITCH_HELP
	printf("Usage:\n");
	printf(" %s acl etype add [ethtype] [portmap]    - drop etherytype packets\n", cmd);
	printf(" %s acl dip add [dip] [portmap]          - drop dip packets\n", cmd);
	printf(" %s acl dip meter [dip] [portmap][meter:kbps] - rate limit dip packets\n", cmd);
	printf(" %s acl dip trtcm [dip] [portmap][CIR:kbps][CBS][PIR][PBS] - TrTCM dip packets\n", cmd);
	printf(" %s acl port add [sport] [portmap]       - drop src port packets\n", cmd);
	printf(" %s acl L4 add [2byes] [portmap]         - drop L4 packets with 2bytes payload\n", cmd);
	printf(" %s add [mac] [portmap]                  - add an entry to switch table\n", cmd);
	printf(" %s add [mac] [portmap] [vlan id]        - add an entry to switch table\n", cmd);
	printf(" %s add [mac] [portmap] [vlan id] [age]  - add an entry to switch table\n", cmd);
	printf(" %s clear                                - clear switch table\n", cmd);
	printf(" %s del [mac]                            - delete an entry from switch table\n", cmd);
	printf(" %s del [mac] [fid]                      - delete an entry from switch table\n", cmd);
	printf(" %s dip add [dip] [portmap]              - add a dip entry to switch table\n", cmd);
	printf(" %s dip del [dip]                        - del a dip entry to switch table\n", cmd);
	printf(" %s dip dump                             - dump switch dip table\n", cmd);
	printf(" %s dip clear                            - clear switch dip table\n", cmd);
	printf(" %s dump                                 - dump switch table\n", cmd);
	printf(" %s ingress-rate on [port] [Mbps]        - set ingress rate limit on port 0~4 \n", cmd);
	printf(" %s egress-rate on [port] [Mbps]         - set egress rate limit on port 0~4 \n", cmd);
	printf(" %s ingress-rate off [port]              - del ingress rate limit on port 0~4 \n", cmd);
	printf(" %s egress-rate off [port]               - del egress rate limit on port 0~4\n", cmd);
	printf(" %s filt [mac]                           - add a SA filtering entry (with portmap 1111111) to switch table\n", cmd);
	printf(" %s filt [mac] [portmap]                 - add a SA filtering entry to switch table\n", cmd);
	printf(" %s filt [mac] [portmap] [vlan id]       - add a SA filtering entry to switch table\n", cmd);
	printf(" %s filt [mac] [portmap] [vlan id] [age] - add a SA filtering entry to switch table\n", cmd);
	printf(" %s mymac [mac] [portmap]                - add a mymac entry to switch table\n", cmd);
	printf(" %s mirror monitor [portnumber]          - enable port mirror and indicate monitor port number\n", cmd);
	printf(" %s mirror target [portnumber] [0:off, 1:rx, 2:tx, 3:all] - set port mirror target\n", cmd);
	printf(" %s phy [phy_addr]                       - dump phy register of specific port\n", cmd);
	printf(" %s phy                                  - dump all phy registers\n", cmd);
	printf(" %s reg r [offset]                       - register read from offset\n", cmd);
	printf(" %s reg w [offset] [value]               - register write value to offset\n", cmd);
	printf(" %s reg d [offset]                       - register dump\n", cmd);
	printf(" %s sip add [sip] [dip] [portmap]        - add a sip entry to switch table\n", cmd);
	printf(" %s sip del [sip] [dip]                  - del a sip entry to switch table\n", cmd);
	printf(" %s sip dump                             - dump switch sip table\n", cmd);
	printf(" %s sip clear                            - clear switch sip table\n", cmd);
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_MT7530_GSW)
	printf(" %s vlan dump <max_vid>                  - dump switch vlan table, up to specified vlan id\n", cmd);
#else
	printf(" %s vlan dump                            - dump switch vlan table\n", cmd);
#endif
	printf(" %s tag on [port]                        - egress tag on port 0~6\n", cmd);
	printf(" %s tag off [port]                       - egress untag on port 0~6\n", cmd);
	printf(" %s tag swap [port]                      - egress swap cvid<->stag on port 0~6\n", cmd);
	printf(" %s tag stack [port]                     - egress stack stag on port 0~6\n", cmd);
	printf(" %s pvid [port] [pvid]                   - set pvid on port 0~6\n", cmd);
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_MT7530_GSW)
	printf(" %s vlan set [vid] [portmap] <stag> <eg_con> <eg_tag> - set vlan id and associated member\n", cmd);
#else
	printf(" %s vlan set [idx] [vid] [portmap] <stag> <eg_con> <eg_tag> - set vlan id and associated member\n", cmd);
#endif
#endif
	switch_fini();
	exit(EXIT_FAILURE);
}

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_MT7530_GSW)
int reg_read(int offset, int *value)
{
	struct ifreq ifr;
	struct esw_reg reg;
	struct ra_mii_ioctl_data mii;

	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &mii;

	mii.phy_id = 0x1f;
	mii.reg_num = offset;

	if (-1 == ioctl(esw_fd, RAETH_MII_READ, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(EXIT_FAILURE);
	}

	*value = mii.val_out;
	return 0;
}

int reg_write(int offset, int value)
{
	struct ifreq ifr;
	struct esw_reg reg;
	struct ra_mii_ioctl_data mii;

	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &mii;

	mii.phy_id = 0x1f;
	mii.reg_num = offset;
	mii.val_in = value;

	if (-1 == ioctl(esw_fd, RAETH_MII_WRITE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(EXIT_FAILURE);
	}
	return 0;
}
#else
int reg_read(int offset, int *value)
{
	struct ifreq ifr;
	esw_reg reg;

	if (value == NULL)
		return -1;
	reg.off = offset;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_READ, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(EXIT_FAILURE);
	}
	*value = reg.val;
	return 0;
}

int reg_write(int offset, int value)
{
	struct ifreq ifr;
	esw_reg reg;

	reg.off = offset;
	reg.val = value;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_WRITE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(EXIT_FAILURE);
	}
	return 0;
}
#endif

int phy_dump(int phy_addr)
{
	struct ifreq ifr;
	struct esw_reg reg;

	reg.val = phy_addr;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_PHY_DUMP, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(EXIT_FAILURE);
	}
	return 0;
}

int ingress_rate_set(int on_off, int port, int bw)
{
	struct ifreq ifr;
	struct esw_rate reg;

	reg.on_off = on_off;
	reg.port = port;
	reg.bw = bw;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_INGRESS_RATE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(EXIT_FAILURE);
	}
	return 0;
}

int egress_rate_set(int on_off, int port, int bw)
{
	struct ifreq ifr;
	struct esw_rate reg;

	reg.on_off = on_off;
	reg.port = port;
	reg.bw = bw;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_EGRESS_RATE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(EXIT_FAILURE);
	}
	return 0;
}

static int
switch_wait_read_register(void *context, unsigned int offset,
                          unsigned int *value)
{
	int raw_value = 0;
	int status;
	(void)context;
	status = reg_read((int)offset, &raw_value);
	if (status != 0)
		return status;
	*value = (unsigned int)raw_value;
	return 0;
}

static void
switch_wait_delay(void *context, unsigned int delay_us)
{
	(void)context;
	usleep(delay_us);
}

static void
switch_wait_failure(const char *operation, unsigned int offset,
                    unsigned int last_value, int status)
{
	if (status == ETIMEDOUT) {
		fprintf(stderr,
		        "switch: %s timed out (register=0x%08x, last=0x%08x)\n",
		        operation, (unsigned int)offset, last_value);
	} else {
		fprintf(stderr,
		        "switch: %s wait failed (register=0x%08x, status=%d)\n",
		        operation, (unsigned int)offset, status);
	}
	switch_fini();
	exit(EXIT_FAILURE);
}

static void
wait_vtcr(void)
{
	unsigned int last_value = 0;
	int status;
	status = switch_wait_until_clear(switch_wait_read_register,
	                                 switch_wait_delay, NULL,
	                                 REG_ESW_VLAN_VTCR, 0x80000000U,
	                                 20U, 1000U, &last_value);
	if (status != 0)
		switch_wait_failure("VLAN table engine", REG_ESW_VLAN_VTCR,
		                    last_value, status);
}

#if RT_TABLE_MANIPULATE

static void
wait_mac_atc(void)
{
	unsigned int last_value = 0;
	int status;
	status = switch_wait_until_clear(switch_wait_read_register,
	                                 switch_wait_delay, NULL,
	                                 REG_ESW_WT_MAC_ATC, 0x8000U,
	                                 20U, 1000U, &last_value);
	if (status != 0)
		switch_wait_failure("MAC table engine", REG_ESW_WT_MAC_ATC,
		                    last_value, status);
}

static void
wait_search_ready(int *value, const char *table_name)
{
	unsigned int last_value = 0;
	int status;
	if (value == NULL)
		switch_wait_failure("invalid table search result",
		                    REG_ESW_WT_MAC_ATC, 0, EINVAL);
	status = switch_wait_for_table_result(switch_wait_read_register,
	                                      switch_wait_delay, NULL,
	                                      REG_ESW_WT_MAC_ATC, 0x8000U,
	                                      0x6000U, 20U, 5000U,
	                                      &last_value);
	if (status != 0)
		switch_wait_failure(table_name, REG_ESW_WT_MAC_ATC,
		                    last_value, status);
	*value = (int)last_value;
}

/* convert IP address from number to string */
void
ip_to_str (
	char *		str,
	unsigned int	ip
	)
{
    unsigned char *	ptr = (char *)&ip;
    unsigned char	c[4];

    c[0] = *(ptr);
    c[1] = *(ptr+1);
    c[2] = *(ptr+2);
    c[3] = *(ptr+3);
    /* sprintf(str, "%d.%d.%d.%d", c[0], c[1], c[2], c[3]); */
    sprintf(str, "%d.%d.%d.%d", c[3], c[2], c[1], c[0]);
}

void acl_dip_meter(int argc, char *argv[])
{
	unsigned int i, j, value, ip_value, meter;

	if (argc < 7) {
		printf("insufficient arguments!\n");
		return;
	}

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
			return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}


	ip_value = parse_ipv4_argument(argv[4]);
	//set pattern
	value = (ip_value >> 16);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);
	
	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x8 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 0);  //w_acl entry 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = (ip_value &0xffff);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);
	
	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x9 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 1);  //w_acl entry 1
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = 0x3; //bit0,1
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80009000 + 0);  //w_acl control mask  0
	//value = (0x80009000 + 1);  //w_acl control mask  1
	
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set action
        meter = strtoul(argv[6], NULL, 0);
	value = meter >> 6;//divide 64, rate limit
	value |= 0x1 << 15; //enable rate control

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //reserved
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x8000d000 + 0);  //w_acl rate control 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();
}


void acl_dip_trtcm(int argc, char *argv[])
{
	unsigned int i, j, value, ip_value;
	unsigned int CIR, CBS, PIR, PBS;

	if (argc < 10)
		usage(argv[0]);

	if (argc < 6) {
		printf("insufficient arguments!\n");
		return;
	}

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
			return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}

	ip_value = parse_ipv4_argument(argv[4]);
	//set pattern
	value = (ip_value >> 16);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x8 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 0);  //w_acl entry 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = (ip_value &0xffff);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x9 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 1);  //w_acl entry 1
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set CBS PBS
        CIR = strtoul(argv[6], NULL, 0);
        CBS = strtoul(argv[7], NULL, 0);
        PIR = strtoul(argv[8], NULL, 0);
        PBS = strtoul(argv[9], NULL, 0);

	value = CBS << 16; //bit16~31
	value |= PBS; //bit0~15
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	CIR = CIR >> 6;
	PIR = PIR >> 6;
	
	value = CIR << 16; //bit16~31
	value |= PIR; //bit0~15
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80007000 + 0);  //w_acl trtcm  #0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = 0x3; //bit0,1
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80009000 + 0);  //w_acl control mask  0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set action
	value = 0x0; //No drop
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0x1 << (11+1); //TrTCM green  meter#0 Low drop
	value |= 0x2 << (8+1); //TrTCM yellow  meter#0 Med drop
	value |= 0x3 << (5+1); //TrTCM red  meter#0    Hig drop
	value |= 0x1 << 0; //TrTCM drop pcd select
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x8000b000 + 0);  //w_acl rule control 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();
}

void acl_ethertype(int argc, char *argv[])
{
	unsigned int i, j, value, ethertype;

	if (argc < 6) {
		printf("insufficient arguments!\n");
		return;
	}

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
			return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}

	ethertype = strtoul(argv[4], NULL, 16);
	//set pattern
	value = ethertype;
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x0 << 16; //mac header
	value |= 0x6 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 0);  //w_acl entry 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = 0x1; //bit0
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80009000 + 0);  //w_acl control rule  0
	//value = (0x80009000 + 7);  //w_acl control rule 7
	//value = (0x80009000 + 15);  //w_acl control rule 15
	//value = (0x80009000 + 16);  //w_acl control rule 16
	//value = (0x80009000 + 31);  //w_acl control rule 31
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set action
	value = 0x0; //default. Nodrop
	value |= 1 << 28;//acl intterupt enable
	value |= 1 << 27;//acl hit count
	value |= 6 << 4;//acl UP
	value |= 6 << 16;//eg-tag tagged
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x8000b000 + 0);  //w_acl rule action control 0
	//value = (0x8000b000 + 7);  //w_acl rule action control 7
	//value = (0x8000b000 + 15);  //w_acl rule action control 15
	//value = (0x8000b000 + 16);  //w_acl rule action control 16
	//value = (0x8000b000 + 31);  //w_acl rule action control 31
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();
}

void acl_dip_modify(int argc, char *argv[])
{
	unsigned int i, j, value, ip_value;

	if (argc < 6) {
		printf("insufficient arguments!\n");
		return;
	}

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
			return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}


	ip_value = parse_ipv4_argument(argv[4]);
	//set pattern
	value = (ip_value >> 16);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x8 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 0);  //w_acl entry 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = (ip_value &0xffff);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x9 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 1);  //w_acl entry 1
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = 0x3; //bit0,1
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80009000 + 0);  //w_acl control rule  0
	//value = (0x80009000 + 7);  //w_acl control rule 7
	//value = (0x80009000 + 15);  //w_acl control rule 15
	//value = (0x80009000 + 16);  //w_acl control rule 16
	//value = (0x80009000 + 31);  //w_acl control rule 31
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set action
	value = 0x0; //default. Nodrop
	value |= 1 << 28;//acl intterupt enable
	value |= 1 << 27;//acl hit count
	value |= 6 << 4;//acl UP
	value |= 6 << 16;//eg-tag tagged
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x8000b000 + 0);  //w_acl rule action control 0
	//value = (0x8000b000 + 7);  //w_acl rule action control 7
	//value = (0x8000b000 + 15);  //w_acl rule action control 15
	//value = (0x8000b000 + 16);  //w_acl rule action control 16
	//value = (0x8000b000 + 31);  //w_acl rule action control 31
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();
}

void acl_dip_pppoe(int argc, char *argv[])
{
	unsigned int i, j, value, ip_value;

	if (argc < 6) {
		printf("insufficient arguments!\n");
		return;
	}

		
	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
			return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}


	ip_value = parse_ipv4_argument(argv[4]);
	//set pattern
	value = (ip_value >> 16);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x8 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 0);  //w_acl entry 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = (ip_value &0xffff);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x9 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 1);  //w_acl entry 1
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = 0x3; //bit0,1
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	//value = (0x80009000 + 0);  //w_acl control rule  0
	value = (0x80009000 + 7);  //w_acl control rule 7
	//value = (0x80009000 + 15);  //w_acl control rule 15
	//value = (0x80009000 + 16);  //w_acl control rule 16
	//value = (0x80009000 + 31);  //w_acl control rule 31
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set action
	value = 0x0; //default. Nodrop
	value |= 1 << 28;//acl intterupt enable
	value |= 1 << 27;//acl hit count
	value |= 1 << 20;//pppoe header remove
	value |= 1 << 21;//SA MAC SWAP
	value |= 1 << 22;//DA MAC SWAP
	
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	//value = (0x8000b000 + 0);  //w_acl rule action control 0
	value = (0x8000b000 + 7);  //w_acl rule action control 7
	//value = (0x8000b000 + 15);  //w_acl rule action control 15
	//value = (0x8000b000 + 16);  //w_acl rule action control 16
	//value = (0x8000b000 + 31);  //w_acl rule action control 31
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();
}

void acl_dip_add(int argc, char *argv[])
{
	unsigned int i, j, value, ip_value;

	if (argc < 6) {
		printf("insufficient arguments!\n");
		return;
	}

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
			return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}

	ip_value = parse_ipv4_argument(argv[4]);
	//set pattern
	value = (ip_value >> 16);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x8 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 0);  //w_acl entry 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = (ip_value &0xffff);
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x2 << 16; //ip header
	value |= 0x9 << 1; //word offset
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 1);  //w_acl entry 1
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set pattern
	value = 0x3; //bit0,1
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80009000 + 0);  //w_acl control rule  0
	//value = (0x80009000 + 2);  //w_acl control rule 2
	//value = (0x80009000 + 15);  //w_acl control rule 15
	//value = (0x80009000 + 16);  //w_acl control rule 16
	//value = (0x80009000 + 31);  //w_acl control rule 31
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set action
	//value = 0x0; //default
	value = 0x7; //drop
	value |= 1 << 28;//acl intterupt enable
	value |= 1 << 27;//acl hit count
	value |= 2 << 24;//acl hit count group index (0~3)
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x8000b000 + 0);  //w_acl rule action control 0
	//value = (0x8000b000 + 2);  //w_acl rule action control 2
	//value = (0x8000b000 + 15);  //w_acl rule action control 15
	//value = (0x8000b000 + 16);  //w_acl rule action control 16
	//value = (0x8000b000 + 31);  //w_acl rule action control 31
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

}

void acl_l4_add(int argc, char *argv[])
{
	unsigned int i, j, value;

	if (argc < 6) {
		printf("insufficient arguments!\n");
		return;
	}

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
			return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}

        value = strtoul(argv[4], NULL, 16);
	//set pattern
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x5 << 16; //L4 payload
	value |= 0x0 << 1; //word offset 0 = tcp src port
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 0);  //w_acl entry 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set rue mask
	value = 0x1; //bit0
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80009000 + 0);  //w_acl control mask  0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set action
	value = 0x7; //drop
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x8000b000 + 0);  //w_acl rule control 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();
}


void acl_sp_add(int argc, char *argv[])
{
	unsigned int i, j, value;

	if (argc < 6) {
		printf("insufficient arguments!\n");
		return;
	}

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
			return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}

        value = strtoul(argv[4], NULL, 0);
	//set pattern
	value |= 0xffff0000;//compare mask

	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = j << 8; //w_port_map
	value |= 0x1 << 19; //enable
	value |= 0x4 << 16; //L4 header
	value |= 0x0 << 1; //word offset 0 = tcp src port
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80005000 + 0);  //w_acl entry 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set rue mask
	value = 0x1; //bit0
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x80009000 + 0);  //w_acl control mask  0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();

	//set action
	value = 0x7; //drop
	//value |= 1;//valid
	reg_write(REG_ESW_VLAN_VAWD1, value);
//	printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

	value = 0; //bit32~63
	reg_write(REG_ESW_VLAN_VAWD2, value);
//	printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

	value = (0x8000b000 + 0);  //w_acl rule control 0
	reg_write(REG_ESW_VLAN_VTCR, value);
//	printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

	wait_vtcr();
}


void dip_dump(void)
{
	int i, j, value, mac, mac2, value2;
	char tmpstr[16];
	reg_write(REG_ESW_WT_MAC_ATC, 0x8104);//dip search command
	printf("hash   port(0:6)   rsp_cnt  flag  timer    dip-address      ATRD\n");
	for (i = 0; i < 0x800; i++) {
		wait_search_ready(&value, "DIP table search");
		if (value & (0x1 << 13)) {
			reg_read(REG_ESW_TABLE_ATRD, &value2);
			printf("%03x:   ", (value >> 16) & 0xfff);
			j = (value2 >> 4) & 0xff;
			printf("%c", (j & 0x01)? '1':'-');
			printf("%c", (j & 0x02)? '1':'-');
			printf("%c", (j & 0x04)? '1':'-');
			printf("%c ", (j & 0x08)? '1':'-');
			printf("%c", (j & 0x10)? '1':'-');
			printf("%c", (j & 0x20)? '1':'-');
			printf("%c", (j & 0x40)? '1':'-');
			reg_read(REG_ESW_TABLE_TSRA2, &mac2);
			printf("     0x%04x", (mac2 & 0xffff));
			printf("  0x%02x", ((mac2 >> 16)& 0xff));
			printf("  %5d", ((mac2 >> 24)& 0xff));
			reg_read(REG_ESW_TABLE_TSRA1, &mac);
			ip_to_str(tmpstr, mac);
			printf("    %-15s", tmpstr);
			printf("  0x%08x\n", value2);
			if (value & 0x4000) {
				printf("end of table %d\n", i);
				return;
			}
		} else if (value & 0x4000) {
			printf("found the last entry %d (not ready)\n", i);
			return;
		}
		reg_write(REG_ESW_WT_MAC_ATC, 0x8105);
		usleep(5000);
	}
}




void dip_add(int argc, char *argv[])
{
	unsigned int value, portmap;

	if (argc < 5)
		usage(argv[0]);

	value = parse_ipv4_argument(argv[3]);
	portmap = parse_portmap_argument(argv[4]);

	/* Validate the full command before the first switch register write. */
	reg_write(REG_ESW_WT_MAC_ATA1, value);
	value = (portmap << 4) | (0x3U << 2);
	reg_write(REG_ESW_WT_MAC_ATWD, value);

	usleep(5000);
	reg_read(REG_ESW_WT_MAC_ATWD, &value);
	printf("REG_ESW_WT_MAC_ATWD is 0x%x\n\r", value);

	value = 0x8011;  /* single w_dip_cmd */
	reg_write(REG_ESW_WT_MAC_ATC, value);
	usleep(1000);
	wait_mac_atc();
}


void dip_del(int argc, char *argv[])
{
	unsigned int value;

	if (argc < 4)
		usage(argv[0]);

	value = parse_ipv4_argument(argv[3]);

	reg_write(REG_ESW_WT_MAC_ATA1, value);

	value = 0;
	reg_write(REG_ESW_WT_MAC_ATA2, value);

	value = 0; //STATUS=0, delete dip
	reg_write(REG_ESW_WT_MAC_ATWD, value);

        value = 0x8011;  //w_dip_cmd
	reg_write(REG_ESW_WT_MAC_ATC, value);

	wait_mac_atc();
}


void dip_clear(void)
{
	int value;

	reg_write(REG_ESW_WT_MAC_ATC, 0x8102); /* clear all dip */
	wait_mac_atc();
	reg_read(REG_ESW_WT_MAC_ATC, &value);
	printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r", value);
}


void sip_dump(void)
{
	int i, j, value, mac, mac2, value2;
	char tmpstr[16];
	reg_write(REG_ESW_WT_MAC_ATC, 0x8204);//sip search command
	printf("hash  port(0:6)   dip-address    sip-address      ATRD\n");
	for (i = 0; i < 0x800; i++) {
		wait_search_ready(&value, "SIP table search");
		if (value & (0x1 << 13)) {
			reg_read(REG_ESW_TABLE_ATRD, &value2);
			printf("%03x:  ", (value >> 16) & 0xfff);
			j = (value2 >> 4) & 0xff;
			printf("%c", (j & 0x01)? '1':'-');
			printf("%c", (j & 0x02)? '1':'-');
			printf("%c", (j & 0x04)? '1':'-');
			printf("%c", (j & 0x08)? '1':'-');
			printf(" %c", (j & 0x10)? '1':'-');
			printf("%c", (j & 0x20)? '1':'-');
			printf("%c", (j & 0x40)? '1':'-');
			reg_read(REG_ESW_TABLE_TSRA2, &mac2);
			ip_to_str(tmpstr, mac2);
			printf("   %-15s", tmpstr);
			reg_read(REG_ESW_TABLE_TSRA1, &mac);
			ip_to_str(tmpstr, mac);
			printf("    %-15s", tmpstr);
			printf("      0x%x\n", value2);
			if (value & 0x4000) {
				printf("end of table %d\n", i);
				return;
			}
		} else if (value & 0x4000) {
			printf("found the last entry %d (not ready)\n", i);
			return;
		}
		reg_write(REG_ESW_WT_MAC_ATC, 0x8205);
		usleep(5000);
	}
}


void sip_add(int argc, char *argv[])
{
	unsigned int value, sip_value, dip_value, portmap;

	if (argc < 6)
		usage(argv[0]);

	/* Validate the full command before the first switch register write. */
	sip_value = parse_ipv4_argument(argv[3]);
	dip_value = parse_ipv4_argument(argv[4]);
	portmap = parse_portmap_argument(argv[5]);

	value = sip_value;
	reg_write(REG_ESW_WT_MAC_ATA2, value);
	value = dip_value;
	reg_write(REG_ESW_WT_MAC_ATA1, value);
	value = (portmap << 4) | (0x3U << 2); /* static port map */

	reg_write(REG_ESW_WT_MAC_ATWD, value);
	usleep(5000);
	reg_read(REG_ESW_WT_MAC_ATWD, &value);
	printf("REG_ESW_WT_MAC_ATWD is 0x%x\n\r", value);

	value = 0x8021; /* single w_sip_cmd */
	reg_write(REG_ESW_WT_MAC_ATC, value);
	usleep(1000);
	wait_mac_atc();
}


void sip_del(int argc, char *argv[])
{
	unsigned int value, sip_value, dip_value;

	if (argc < 5)
		usage(argv[0]);

	/* Validate both addresses before mutating the switch table. */
	sip_value = parse_ipv4_argument(argv[3]);
	dip_value = parse_ipv4_argument(argv[4]);

	value = sip_value;
	reg_write(REG_ESW_WT_MAC_ATA2, value); /* SIP */
	value = dip_value;
	reg_write(REG_ESW_WT_MAC_ATA1, value); /* DIP */

	value = 0; /* STATUS=0, delete SIP */
	reg_write(REG_ESW_WT_MAC_ATWD, value);
	value = 0x8021; /* w_sip_cmd */
	reg_write(REG_ESW_WT_MAC_ATC, value);
	wait_mac_atc();
}

void sip_clear(void)
{
	int value;

	reg_write(REG_ESW_WT_MAC_ATC, 0x8202); /* clear all sip */
	wait_mac_atc();
	reg_read(REG_ESW_WT_MAC_ATC, &value);
	printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r", value);
}


void table_dump(void)
{
	int i, j, value, mac, mac2, value2;
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	printf("hash  port(0:6)   fid   vid  age   mac-address     filter my_mac\n");
#else
	printf("hash  port(0:6)   fid   vid  age   mac-address     filter\n");
#endif
	reg_write(REG_ESW_WT_MAC_ATC, 0x8004);
	for (i = 0; i < 0x800; i++) {
		usleep(5000);
		wait_search_ready(&value, "MAC table search");
		if (value & (0x1 << 13)) {
			printf("%03x:   ", (value >> 16) & 0xfff);
			reg_read(REG_ESW_TABLE_ATRD, &value2);
			j = (value2 >> 4) & 0xff;
			printf("%c", (j & 0x01)? '1':'-');
			printf("%c", (j & 0x02)? '1':'-');
			printf("%c", (j & 0x04)? '1':'-');
			printf("%c ", (j & 0x08)? '1':'-');
			printf("%c", (j & 0x10)? '1':'-');
			printf("%c", (j & 0x20)? '1':'-');
			printf("%c", (j & 0x40)? '1':'-');
			printf("%c", (j & 0x80)? '1':'-');
			reg_read(REG_ESW_TABLE_TSRA2, &mac2);
			printf("   %2d", (mac2 >> 12) & 0x7);
			printf("  %4d", (mac2 & 0xfff));
			printf(" %4d", (value2 >> 24) & 0xff);
			reg_read(REG_ESW_TABLE_TSRA1, &mac);
			printf("  %08x", mac);
			printf("%04x", ((mac2 >> 16) & 0xffff));
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
			printf("     %c", (((value2 >> 20) & 0x03)== 0x03)? 'y':'-');
			printf("     %c\n", (((value2 >> 23) & 0x01)== 0x01)? 'y':'-');
#else
			printf("     %c\n", (((value2 >> 20) & 0x03)== 0x03)? 'y':'-');
#endif
			if (value & 0x4000) {
				printf("end of table %d\n", i);
				return;
			}
		} else if (value & 0x4000) {
			printf("found the last entry %d (not ready)\n", i);
			return;
		}
		reg_write(REG_ESW_WT_MAC_ATC, 0x8005);
	}
}

void table_add(int argc, char *argv[])
{
    unsigned int mac_high, mac_low, portmap, vid = 0;
    unsigned int age = 255, eg_tag = 0, value;
    unsigned int is_filter, is_mymac;

    if (argc < 3 || argc > 7)
        usage(argv[0]);
    is_filter = (argv[1][0] == 'f') ? 1U : 0U;
    is_mymac = (argv[1][0] == 'm') ? 1U : 0U;

    if (switch_parse_mac_address(argv[2], &mac_high, &mac_low) != 0)
        usage(argv[0]);
    if (argc > 3) {
        if (switch_parse_table_portmap(argv[3], &portmap) != 0)
            usage(argv[0]);
    } else if (is_filter) {
        portmap = 0x7fU;
    } else {
        usage(argv[0]);
    }
    if (argc > 4 &&
        switch_parse_uint_range(argv[4], 0U, 4095U, &vid) != 0)
        usage(argv[0]);
    if (argc > 5 &&
        switch_parse_uint_range(argv[5], 1U, 255U, &age) != 0)
        usage(argv[0]);
    if (argc > 6 &&
        switch_parse_uint_range(argv[6], 0U, 7U, &eg_tag) != 0)
        usage(argv[0]);

    /* Validate every argument before staging any table register. */
    reg_write(REG_ESW_WT_MAC_ATA1, mac_high);
    value = (mac_low << 16) | (1U << 15) | vid;
    reg_write(REG_ESW_WT_MAC_ATA2, value);
    value = portmap << 4;
    if (argc > 5)
        value |= (age << 24) | (0x1U << 2);
    else
        value |= (0xffU << 24) | (0x3U << 2);
    if (argc > 6)
        value |= eg_tag << 13;
    if (is_filter)
        value |= 7U << 20;
    if (is_mymac)
        value |= 1U << 23;

    reg_write(REG_ESW_WT_MAC_ATWD, value);
    usleep(5000);
    reg_read(REG_ESW_WT_MAC_ATWD, &value);
    printf("REG_ESW_WT_MAC_ATWD is 0x%x\n\r", value);
    value = 0x8001;
    reg_write(REG_ESW_WT_MAC_ATC, value);
    usleep(1000);
    wait_mac_atc();
}

void table_del(int argc, char *argv[])
{
    unsigned int mac_high, mac_low, fid = 0, value;

    if (argc < 3 || argc > 4)
        usage(argv[0]);
    if (switch_parse_mac_address(argv[2], &mac_high, &mac_low) != 0)
        usage(argv[0]);
    if (argc > 3 &&
        switch_parse_uint_range(argv[3], 0U, 7U, &fid) != 0)
        usage(argv[0]);

    /* Validate all arguments before staging the deletion. */
    reg_write(REG_ESW_WT_MAC_ATA1, mac_high);
    value = (mac_low << 16) | (fid << 12);
    reg_write(REG_ESW_WT_MAC_ATA2, value);
    reg_write(REG_ESW_WT_MAC_ATWD, 0U);
    reg_write(REG_ESW_WT_MAC_ATC, 0x8001U);
    wait_mac_atc();
}


void table_clear(void)
{
	int value;

	reg_write(REG_ESW_WT_MAC_ATC, 0x8002); /* clear all mac */
	wait_mac_atc();
	reg_read(REG_ESW_WT_MAC_ATC, &value);
	printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r", value);
}


void set_mirror_to(int argc, char *argv[])
{
	unsigned int value;
        unsigned int idx;

	if (argc != 4 ||
	    switch_parse_uint_range(argv[3], 0U, MAX_PORT, &idx) != 0)
		usage(argv[0]);

	reg_read(REG_ESW_WT_MAC_MFC, &value);
	value |= 0x1 << 3;
	value &= 0xfffffff8;
	value |= idx << 0;

	reg_write(REG_ESW_WT_MAC_MFC, value);
}

void set_mirror_from(int argc, char *argv[])
{
	unsigned int offset, value;
        unsigned int idx, mirror;

	if (argc != 5 ||
	    switch_parse_uint_range(argv[3], 0U, MAX_PORT, &idx) != 0 ||
	    switch_parse_uint_range(argv[4], 0U, 3U, &mirror) != 0)
		usage(argv[0]);

	offset = (0x2004 | (idx << 8));
	reg_read(offset, &value);

	value &= 0xfffffcff;
	value |= mirror << 8;

	reg_write(offset, value);
}

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_MT7530_GSW)
void vlan_dump(int max_vid)
{
	int i, j, vid, mask, mask2, value, value2;

	if (max_vid < 1)
		max_vid = 1;
	else
	if (max_vid > 4095)
		max_vid = 4095;

	printf("  vid  portmap  eg-tag  eg-con  stag  ivl  fid\n");
//                  1  -------  -------      1     1    1    7

	for (i = 1; i <= max_vid; i++) {
		value = (0x80000000 + i);  //r_vid_cmd
		reg_write(REG_ESW_VLAN_VTCR, value);
		wait_vtcr();
		reg_read(REG_ESW_VLAN_VAWD1, &value);
		reg_read(REG_ESW_VLAN_VAWD2, &value2);
		
		if (value & 0x01) {
			printf(" %4d  ", i);
			for (j = 0; j < 7; j++) {
				mask = ((1u << j) << 16);
				printf("%c", (value & mask)? '1':'-');
			}
			
			if (value & (1u << 28)) {
				printf("%s", "  ");
				for (j = 0; j < 7; j++) {
					mask = ((1u << j) << 16);
					mask2 = (value2 >> (j*2)) & 0x3;
					if (!(value & mask))
						printf("%c", '-');
					else if (mask2 == 0)
						printf("%c", 'u');
					else if (mask2 == 1)
						printf("%c", 'w');
					else if (mask2 == 2)
						printf("%c", 't');
					else
						printf("%c", 's');
				}
			} else {
				printf("%s", "  -------");
			}
			
			printf("  %5d",  (value>>29)&0x1);
			printf("  %4d", ((value & 0xfff0)>>4)) ;
			printf("  %3d",  (value>>30)&0x1);
			if (value & (1u << 30))
				printf("    %c\n", '-');
			else
				printf("  %3d\n", ((value & 0xe)>>1));
		}
	}

	printf("\nPVID:\n");
	printf("port  pvid  prio  matrix\n");
//                 0     1     3  ----1--
	for (i = 0; i < 7; i++) {
		reg_read(0x2014 + i*0x100, &value);
		reg_read(0x2004 + i*0x100, &value2);
		printf("%4d  %4d  %4d  ", i, value & 0xfff, (value >> 13) & 0x7);
		value2 >>= 16;
		value2  &= 0xff;
		for (j = 0; j < 7; j++) {
			mask = (1u << j);
			printf("%c", (value2 & mask)? '1':'-');
		}
		printf("\n");
	}
}
#else
void vlan_dump(void)
{
	int i, j, k, vid, mask, mask2, value, value2;

	printf("idx  vid  portmap   eg-tag  eg-con  stag  ivl  fid\n");
//                0    1  --------  --------     1     1    1    7

	for (i = 0; i < 8; i++) {
		reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &vid);
		
		for (k = 0; k < 2; k++) {
			value = (0x80000000 + 2*i+k);  //r_vid_cmd
			reg_write(REG_ESW_VLAN_VTCR, value);
			wait_vtcr();
			reg_read(REG_ESW_VLAN_VAWD1, &value);
			reg_read(REG_ESW_VLAN_VAWD2, &value2);
			
			if (k == 1)
				vid >>= 12;
			
			printf(" %2d %4d  ", 2*i+k, vid & 0xfff);
			
			if (value & 0x01) {
				for (j = 0; j < 8; j++) {
					mask = ((1u << j) << 16);
					printf("%c", (value & mask)? '1':'-');
				}
				
				if (value & (1u << 28)) {
					printf("%s", "  ");
					for (j = 0; j < 8; j++) {
						mask = ((1u << j) << 16);
						mask2 = (value2 >> (j*2)) & 0x3;
						if (!(value & mask))
							printf("%c", '-');
						else if (mask2 == 0)
							printf("%c", 'u');
						else if (mask2 == 1)
							printf("%c", 'w');
						else if (mask2 == 2)
							printf("%c", 't');
						else
							printf("%c", 's');
					}
				} else {
					printf("%s", "  --------");
				}
				
				printf("  %4d",  (value>>29)&0x1);
				printf("  %4d", ((value & 0xfff0)>>4));
				printf("  %3d",  (value>>30)&0x1);
				if (value & (1u << 30))
					printf("    %c\n", '-');
				else
					printf("  %3d\n", ((value & 0xe)>>1));
			} else {
				printf(" invalid\n");
			}
		}
	}

	printf("\nPVID:\n");
	printf("port  pvid  prio\n");
//                 0     1     3
	for (i = 0; i < 8; i++) {
		reg_read(0x2014 + i*0x100, &value);
		printf("%4d  %4d  %4d\n", i, value & 0xfff, (value >> 13) & 0x7);
	}
}
#endif

#endif /* RT_TABLE_MANIPULATE */

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_MT7530_GSW)
void vlan_set(int argc, char *argv[])
{
	int i;
	unsigned int port_mask, cvid, stag, eg_con, egtag_mask, value, value2;

	if (argc < 5 || argc > 8) {
		printf("invalid argument count!\n");
		return;
	}

	if (switch_parse_uint_range(argv[3], 0U, 4095U, &cvid) != 0) {
		printf("wrong vlan id range, should be within 0~4095\n");
		return;
	}

	if (strlen(argv[4]) != 7) {
		printf("portmap format error, should be of length 7 (e.g. 1111001)\n");
		return;
	}

	port_mask = 0;
	for (i = 0; i < 7; i++) {
		if (argv[4][i] != '0' && argv[4][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		port_mask += (argv[4][i] - '0') * (1u << i);
	}

	value  = 1; // valid
	value2 = 0;

	if (argc > 5) {
		if (switch_parse_uint_range(argv[5], 0U, 4095U, &stag) != 0) {
			printf("wrong stag range, should be within 0~4095\n");
			return;
		}
		value |= (stag << 4);
	}

	if (argc > 6) {
		if (switch_parse_uint_range(argv[6], 0U, 1U, &eg_con) != 0) {
			printf("wrong eg_con range, should be within 0~1\n");
			return;
		}
		
		if (eg_con) {
			value |= (1u << 28);	// eg_tag control enable
			value |= (1u << 29);	// egress tag consistent
		}
	}

	if (argc > 7) {
		if (strlen(argv[7]) != 7) {
			printf("eg_tag per port format error, should be of length 7 (e.g. uuuuutt)\n");
			return;
		}
		
		egtag_mask = 0;
		for (i = 0; i < 7; i++) {
			if (argv[7][i] == 'u' || argv[7][i] == '-')
				;
			else if (argv[7][i] == 'w')
				egtag_mask |= (1u << (i*2));
			else if (argv[7][i] == 't')
				egtag_mask |= (2u << (i*2));
			else if (argv[7][i] == 's')
				egtag_mask |= (3u << (i*2));
			else {
				printf("eg_tag format error, should be of 'u' (untag), 't' (tag), 'w' (swap) or 's' (stack)\n");
				return;
			}
		}
		
		value |= (1u << 28);	// eg_tag control enable
		value2 |= egtag_mask;
	}

	// set vlan member
	value |= (port_mask << 16);
	value |= (1u << 30);		// IVL

	reg_write(REG_ESW_VLAN_VAWD1, value);
	reg_write(REG_ESW_VLAN_VAWD2, value2);

	value = (0x80001000 + cvid);  //w_vid_cmd
	reg_write(REG_ESW_VLAN_VTCR, value);

	wait_vtcr();
}
#else
void vlan_set(int argc, char *argv[])
{
	int i;
	unsigned int idx, vlan_id, port_mask, cvid, stag, eg_con, egtag_mask, value, value2;

	if (argc < 6 || argc > 9) {
		printf("invalid argument count!\n");
		return;
	}

	if (switch_parse_uint_range(argv[3], 0U, 15U, &idx) != 0) {
		printf("wrong member index range, should be within 0~15\n");
		return;
	}

	if (switch_parse_uint_range(argv[4], 0U, 4095U, &cvid) != 0) {
		printf("wrong vlan id range, should be within 0~4095\n");
		return;
	}

	if (strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 8 (e.g. 11111011)\n");
		return;
	}

	port_mask = 0;
	for (i = 0; i < 8; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		port_mask += (argv[5][i] - '0') * (1u << i);
	}

	value  = 1; // valid
	value2 = 0;

	if (argc > 6) {
		if (switch_parse_uint_range(argv[6], 0U, 4095U, &stag) != 0) {
			printf("wrong stag range, should be within 0~4095\n");
			return;
		}
		value |= (stag << 4);
	}

	if (argc > 7) {
		if (switch_parse_uint_range(argv[7], 0U, 1U, &eg_con) != 0) {
			printf("wrong eg_con range, should be within 0~1\n");
			return;
		}
		
		if (eg_con) {
			value |= (1u << 28);	// eg_tag control enable
			value |= (1u << 29);	// egress tag consistent
		}
	}

	if (argc > 8) {
		if (strlen(argv[8]) != 8) {
			printf("eg_tag per port format error, should be of length 8 (e.g. uuuuu-tt)\n");
			return;
		}
		
		egtag_mask = 0;
		for (i = 0; i < 8; i++) {
			if (argv[8][i] == 'u' || argv[8][i] == '-')
				;
			else if (argv[8][i] == 'w')
				egtag_mask |= (1u << (i*2));
			else if (argv[8][i] == 't')
				egtag_mask |= (2u << (i*2));
			else if (argv[8][i] == 's')
				egtag_mask |= (3u << (i*2));
			else {
				printf("eg_tag format error, should be of 'u' (untag), 't' (tag), 'w' (swap) or 's' (stack)\n");
				return;
			}
		}
		
		value |= (1u << 28);	// eg_tag control enable
		value2 |= egtag_mask;
	}

	// set vlan identifier
	reg_read(REG_ESW_VLAN_ID_BASE + 4*(idx/2), &vlan_id);
	if ((idx % 2) == 0) {
		vlan_id &= 0xfff000;
		vlan_id |= cvid;
	} else {
		vlan_id &= 0xfff;
		vlan_id |= (cvid << 12);
	}
	reg_write(REG_ESW_VLAN_ID_BASE + 4*(idx/2), vlan_id);

	// set vlan member
	value |= (port_mask << 16);
	value |= (1u << 30);		// IVL

	reg_write(REG_ESW_VLAN_VAWD1, value);
	reg_write(REG_ESW_VLAN_VAWD2, value2);

	value = (0x80001000 + idx);  //w_vid_cmd
	reg_write(REG_ESW_VLAN_VTCR, value);

	wait_vtcr();
}
#endif

int main(int argc, char *argv[])
{
	switch_init();

	if (argc < 2)
		usage(argv[0]);
#if RT_TABLE_MANIPULATE
	if (argc == 2) {
		if (!strncmp(argv[1], "dump", 5))
			table_dump();
		else if (!strncmp(argv[1], "clear", 6)) {
			table_clear();
			printf("done.\n");
		}
		else if (!strncmp(argv[1], "phy", 4)) {
			phy_dump(32); //dump all phy register
		}
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "add", 4))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "mymac", 4))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "filt", 5))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "del", 4))
		table_del(argc, argv);
	else if (!strncmp(argv[1], "phy", 4)) {
		if (argc == 3) {
			int phy_addr = strtoul(argv[2], NULL, 10);
			phy_dump(phy_addr);
		}else {
			phy_dump(32); //dump all phy register
		}
	}
	else if (!strncmp(argv[1], "sip", 5)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			sip_dump();
		else if (!strncmp(argv[2], "add", 4))
			sip_add(argc, argv);
		else if (!strncmp(argv[2], "del", 4))
			sip_del(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			sip_clear();
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "dip", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			dip_dump();
		else if (!strncmp(argv[2], "add", 4))
			dip_add(argc, argv);
		else if (!strncmp(argv[2], "del", 4))
			dip_del(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			dip_clear();
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "mirror", 7)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "monitor", 8))
			set_mirror_to(argc, argv);
		else if (!strncmp(argv[2], "target", 7))
			set_mirror_from(argc, argv);
		else
			usage(argv[0]);
	}

	else if (!strncmp(argv[1], "acl", 4)) {
		if (argc < 4)
			usage(argv[0]);
		if (!strncmp(argv[2], "dip", 4)){
			if (!strncmp(argv[3], "add", 4))
				acl_dip_add(argc, argv);
			else if (!strncmp(argv[3], "modup", 6))
				acl_dip_modify(argc, argv);
			else if (!strncmp(argv[3], "pppoe", 6))
				acl_dip_pppoe(argc, argv);
			else if (!strncmp(argv[3], "trtcm", 4))
				acl_dip_trtcm(argc, argv);
			else if (!strncmp(argv[3], "meter", 6))
				acl_dip_meter(argc, argv);
			else
				usage(argv[0]);
		}
		else if (!strncmp(argv[2], "etype", 6)){
			if (!strncmp(argv[3], "add", 4))
				acl_ethertype(argc, argv);
			else
				usage(argv[0]);
		}

		else if (!strncmp(argv[2], "port", 5)){
			if (!strncmp(argv[3], "add", 4))
				acl_sp_add(argc, argv);
			else
				usage(argv[0]);
		}
		else if (!strncmp(argv[2], "L4", 5)){
			if (!strncmp(argv[3], "add", 4))
				acl_l4_add(argc, argv);
			else
				usage(argv[0]);
		}
	}
#endif
	else if (!strncmp(argv[1], "vlan", 5)) {
		if (argc < 3)
			usage(argv[0]);
		
#if RT_TABLE_MANIPULATE
		if (!strncmp(argv[2], "dump", 5)) {
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_MT7530_GSW)
			int max_vid = 15;
			if (argc > 3)
				max_vid = strtoul(argv[3], NULL, 10);
			vlan_dump(max_vid);
#else
			vlan_dump();
#endif
		} else
#endif
		if (!strncmp(argv[2], "set", 4))
			vlan_set(argc, argv);
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "reg", 4)) {
		int i, j, off, val=0;
		if (argc < 4)
			usage(argv[0]);
		if (argv[2][0] == 'r') {
			off = strtoul(argv[3], NULL, 16);
			reg_read(off, &val);
			printf("switch reg read offset=%x, value=%x\n", off, val);
		}
		else if (argv[2][0] == 'w') {
			if (argc != 5)
				usage(argv[0]);
			off = strtoul(argv[3], NULL, 16);
			val = strtoul(argv[4], NULL, 16);
			printf("switch reg write offset=%x, value=%x\n", off, val);
			reg_write(off, val);
		}
		else if (argv[2][0] == 'd') {
			off = strtoul(argv[3], NULL, 16);
			for(i=0; i<16; i++) {
				printf("0x%08x: ", off+0x10*i);
				for(j=0; j<4; j++){
					reg_read(off+i*0x10+j*0x4, &val);
					printf(" 0x%08x", val);
				}
				printf("\n");
			}
		}
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "ingress-rate", 6)) {
		unsigned int port, bandwidth = 0U;
		int bw = 0;

		if (argc < 3)
			usage(argv[0]);
		if (strcmp(argv[2], "on") == 0) {
			if (argc != 5 ||
			    switch_parse_uint_range(argv[3], 0U, 4U, &port) != 0 ||
			    switch_parse_uint_range(argv[4], 0U, 2147483647U,
						    &bandwidth) != 0)
				usage(argv[0]);
			bw = (int)bandwidth;
			ingress_rate_set(1, (int)port, bw);
			printf("switch port=%u, bw=%d\n", port, bw);
		} else if (strcmp(argv[2], "off") == 0) {
			if (argc != 4 ||
			    switch_parse_uint_range(argv[3], 0U, 4U, &port) != 0)
				usage(argv[0]);
			ingress_rate_set(0, (int)port, 0);
			printf("switch port=%u ingress rate limit off\n", port);
		} else {
			usage(argv[0]);
		}
	}
	else if (!strncmp(argv[1], "egress-rate", 6)) {
		unsigned int port, bandwidth = 0U;
		int bw = 0;

		if (argc < 3)
			usage(argv[0]);
		if (strcmp(argv[2], "on") == 0) {
			if (argc != 5 ||
			    switch_parse_uint_range(argv[3], 0U, 4U, &port) != 0 ||
			    switch_parse_uint_range(argv[4], 0U, 2147483647U,
						    &bandwidth) != 0)
				usage(argv[0]);
			bw = (int)bandwidth;
			egress_rate_set(1, (int)port, bw);
			printf("switch port=%u, bw=%d\n", port, bw);
		} else if (strcmp(argv[2], "off") == 0) {
			if (argc != 4 ||
			    switch_parse_uint_range(argv[3], 0U, 4U, &port) != 0)
				usage(argv[0]);
			egress_rate_set(0, (int)port, 0);
			printf("switch port=%u egress rate limit off\n", port);
		} else {
			usage(argv[0]);
		}
	}
	else if (!strncmp(argv[1], "tag", 4)) {
		int offset=0, value=0;
		unsigned int port;

		if (argc != 4 ||
		    switch_parse_uint_range(argv[3], 0U, MAX_PORT, &port) != 0)
			usage(argv[0]);
		offset = 0x2004 + (int)port * 0x100;
		reg_read(offset, &value);
		if (strcmp(argv[2], "on") == 0) {
			value &= ~(0x30000000);
			value |=   0x20000000;
			reg_write(offset, value);
			printf("egress tag at port %d\n", port);
		}
		else if (strcmp(argv[2], "off") == 0) {
			value &= ~(0x30000000);
			reg_write(offset, value);
			printf("egress untag at port %d\n", port);
		}
		else if (strcmp(argv[2], "swap") == 0) {
			value &= ~(0x30000000);
			value |=   0x10000000;
			reg_write(offset, value);
			printf("egress swap cvid<->stag at port %d\n", port);
		}
		else if (strcmp(argv[2], "stack") == 0) {
			value |=   0x30000000;
			reg_write(offset, value);
			printf("egress stack stag at port %d\n", port);
		}
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "pvid", 5)) {
		int offset=0, value=0;
		unsigned int port, pvid;

		if (argc != 4 ||
		    switch_parse_uint_range(argv[2], 0U, MAX_PORT, &port) != 0 ||
		    switch_parse_uint_range(argv[3], 0U, 4095U, &pvid) != 0)
			usage(argv[0]);
		offset = 0x2014 + (int)port * 0x100;
		reg_read(offset, &value);
		value &= 0xfffff000;
		value |= pvid;
		reg_write(offset, value);
		printf("Set port %d pvid %d\n", port, pvid);
	}
	else
		usage(argv[0]);

	switch_fini();
	return 0;
}

