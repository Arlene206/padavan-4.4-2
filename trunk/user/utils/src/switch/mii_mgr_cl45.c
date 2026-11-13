#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if.h>
#include <linux/mii.h>
#include <linux/types.h>

#include <ra_ioctl.h>

#define MII_MGR_HELP 1

void show_usage(void)
{
#if MII_MGR_HELP
	printf("Get: mii_mgr_cl45 -g -p [port number] -d [dev number] -r [register number]\n");
	printf("Example: mii_mgr_cl45 -g -p 3 -d 0x5 -r 0x4\n\n");
	printf("Set: mii_mgr_cl45 -s -p [port number] -d [dev number] -r [register number] -v [value]\n");
	printf("Example: mii_mgr_cl45 -s -p 4 -d 0x6 -r 0x1 -v 0xff11\n\n");
#endif
}

static int parse_hex_argument(const char *text, unsigned long max_value,
			      __u32 *value)
{
	char *end;
	unsigned long parsed;

	if (text == NULL || *text == '\0' || *text == '+' || *text == '-' ||
	    isspace((unsigned char)*text))
		return -1;

	errno = 0;
	parsed = strtoul(text, &end, 16);
	if (errno == ERANGE || end == text || *end != '\0' ||
	    parsed > max_value)
		return -1;

	*value = (__u32)parsed;
	return 0;
}

static int parse_arguments(int argc, char *argv[],
			   struct ra_mii_ioctl_data *mii, int *method)
{
	int opt;
	int have_port = 0;
	int have_device = 0;
	int have_register = 0;
	int have_value = 0;
	char options[] = "gsp:d:r:v:?t";

	if (mii == NULL || method == NULL)
		return -1;

	memset(mii, 0, sizeof(*mii));
	*method = 0;
	optind = 1;
	opterr = 0;

	while ((opt = getopt(argc, argv, options)) != -1) {
		switch (opt) {
		case 'g':
			if (*method == RAETH_MII_WRITE_CL45)
				return -1;
			*method = RAETH_MII_READ_CL45;
			break;
		case 's':
			if (*method == RAETH_MII_READ_CL45)
				return -1;
			*method = RAETH_MII_WRITE_CL45;
			break;
		case 'p':
			if (parse_hex_argument(optarg, 0x1fUL, &mii->port_num) < 0)
				return -1;
			have_port = 1;
			break;
		case 'd':
			if (parse_hex_argument(optarg, 0x1fUL, &mii->dev_addr) < 0)
				return -1;
			have_device = 1;
			break;
		case 'r':
			if (parse_hex_argument(optarg, 0xffffUL, &mii->reg_addr) < 0)
				return -1;
			have_register = 1;
			break;
		case 'v':
			if (parse_hex_argument(optarg, 0xffffUL, &mii->val_in) < 0)
				return -1;
			have_value = 1;
			break;
		case 't':
			/* Retain the legacy no-op option. */
			break;
		case '?':
		default:
			return -1;
		}
	}

	if (optind != argc || *method == 0 || !have_port || !have_device ||
	    !have_register || ((*method == RAETH_MII_WRITE_CL45) != have_value))
		return -1;

	return 0;
}

int main(int argc, char *argv[])
{
	int sk;
	int method;
	int status = EXIT_SUCCESS;
	struct ifreq ifr;
	struct ra_mii_ioctl_data mii;

	if (argc == 1 || (argc == 2 && strcmp(argv[1], "-?") == 0)) {
		show_usage();
		return EXIT_SUCCESS;
	}

	if (parse_arguments(argc, argv, &mii, &method) < 0) {
		fprintf(stderr, "mii_mgr_cl45: invalid or incomplete options\n");
		show_usage();
		return EXIT_FAILURE;
	}

	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk < 0) {
		perror("mii_mgr_cl45: socket");
		return EXIT_FAILURE;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth2", sizeof(ifr.ifr_name) - 1);
	ifr.ifr_data = (char *)&mii;

	if (ioctl(sk, method, &ifr) < 0) {
		perror("mii_mgr_cl45: ioctl");
		status = EXIT_FAILURE;
	} else if (method == RAETH_MII_READ_CL45) {
		printf("Get: port%u dev%Xh_reg%Xh = 0x%04X\n",
		       mii.port_num, mii.dev_addr, mii.reg_addr, mii.val_out);
	} else {
		printf("Set: port%u dev%Xh_reg%Xh = 0x%04X\n",
		       mii.port_num, mii.dev_addr, mii.reg_addr, mii.val_in);
	}

	if (close(sk) < 0 && status == EXIT_SUCCESS) {
		perror("mii_mgr_cl45: close");
		status = EXIT_FAILURE;
	}

	return status;
}
