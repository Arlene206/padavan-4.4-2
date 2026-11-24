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
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include <ra_ioctl.h>

#define MII_MGR_HELP 1

void show_usage(void)
{
#if MII_MGR_HELP
	printf("mii_mgr -g -p [phy number] -r [register number]\n");
	printf("  Get: mii_mgr -g -p 3 -r 4\n\n");
	printf("mii_mgr -s -p [phy number] -r [register number] -v [0xvalue]\n");
	printf("  Set: mii_mgr -s -p 4 -r 1 -v 0xff11\n\n");
#endif
}

struct mii_mgr_options {
	struct ra_mii_ioctl_data mii;
	int method;
#if defined(CONFIG_RALINK_MT7628)
	struct ra_mii_ioctl_data page_mii;
	int page_select;
#endif
};

static int parse_unsigned_argument(const char *text, int base,
				   unsigned long maximum,
				   unsigned long *value)
{
	char *end;
	unsigned long parsed;

	if (text == NULL || text[0] == '\0' || text[0] == '-' ||
	    isspace((unsigned char)text[0]))
		return -1;

	errno = 0;
	parsed = strtoul(text, &end, base);
	if (errno == ERANGE || end == text || *end != '\0' || parsed > maximum)
		return -1;

	*value = parsed;
	return 0;
}

static int parse_arguments(int argc, char **argv,
			   struct mii_mgr_options *parsed)
{
	int opt;
	int method_seen = 0;
	int phy_seen = 0;
	int reg_seen = 0;
	int value_seen = 0;
	char *reg_text = NULL;
	unsigned long number;
#if defined(CONFIG_RALINK_MT7628)
	int page_seen = 0;
	const char *option_string = "gsp:r:v:L:G:t";
#else
	const char *option_string = "gsp:r:v:t";
#endif

	if (argc < 1 || argv == NULL || parsed == NULL)
		return -1;

	memset(parsed, 0, sizeof(*parsed));
	optind = 1;
	opterr = 0;

	while ((opt = getopt(argc, argv, option_string)) != -1) {
		switch (opt) {
		case 'g':
			if (method_seen)
				return -1;
			parsed->method = RAETH_MII_READ;
			method_seen = 1;
			break;
		case 's':
			if (method_seen)
				return -1;
			parsed->method = RAETH_MII_WRITE;
			method_seen = 1;
			break;
		case 'p':
			if (phy_seen ||
			    parse_unsigned_argument(optarg, 10, 31, &number) < 0)
				return -1;
			parsed->mii.phy_id = (__u32)number;
			phy_seen = 1;
			break;
		case 'r':
			if (reg_seen)
				return -1;
			reg_text = optarg;
			reg_seen = 1;
			break;
		case 'v':
			if (value_seen ||
			    parse_unsigned_argument(optarg, 16, 0xffff, &number) < 0)
				return -1;
			parsed->mii.val_in = (__u32)number;
			value_seen = 1;
			break;
#if defined(CONFIG_RALINK_MT7628)
		case 'L':
		case 'G':
			if (page_seen ||
			    parse_unsigned_argument(optarg, 16, UINT_MAX >> 12,
						    &number) < 0)
				return -1;
			parsed->page_mii.reg_num = 31;
			parsed->page_mii.val_in = (__u32)(number << 12);
			if (opt == 'L')
				parsed->page_mii.val_in |= 0x8000;
			parsed->page_select = 1;
			page_seen = 1;
			break;
#endif
		case 't':
			/* Retain the historical, intentionally unused option. */
			break;
		default:
			return -1;
		}
	}

	if (optind != argc || !phy_seen)
		return -1;

#if defined(CONFIG_RALINK_MT7628)
	parsed->page_mii.phy_id = parsed->mii.phy_id;
#endif

	if (!method_seen) {
#if defined(CONFIG_RALINK_MT7628)
		return parsed->page_select && !reg_seen && !value_seen ? 0 : -1;
#else
		return -1;
#endif
	}

	if (!reg_seen ||
	    (parsed->method == RAETH_MII_READ && value_seen) ||
	    (parsed->method == RAETH_MII_WRITE && !value_seen))
		return -1;

#if defined(CONFIG_RALINK_MT7621) || defined(CONFIG_MT7530_GSW)
	if (parsed->mii.phy_id == 31) {
		if (parse_unsigned_argument(reg_text, 16, 0xffff, &number) < 0)
			return -1;
	} else
#endif
	{
		if (parse_unsigned_argument(reg_text, 10, 31, &number) < 0)
			return -1;
	}

	parsed->mii.reg_num = (__u32)number;
	return 0;
}

int main(int argc, char *argv[])
{
	int sk;
	int status = EXIT_SUCCESS;
	struct ifreq ifr;
	struct mii_mgr_options options;

	if (argc == 1 || (argc == 2 && strcmp(argv[1], "-?") == 0)) {
		show_usage();
		return EXIT_SUCCESS;
	}

	if (parse_arguments(argc, argv, &options) < 0) {
		fprintf(stderr, "mii_mgr: invalid or incomplete arguments\n");
		show_usage();
		return EXIT_FAILURE;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth2", sizeof(ifr.ifr_name) - 1);
	ifr.ifr_data = (void *)&options.mii;

	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk < 0) {
		perror("mii_mgr: socket");
		return EXIT_FAILURE;
	}

#if defined(CONFIG_RALINK_MT7628)
	if (options.page_select) {
		struct ifreq ifr2;

		memset(&ifr2, 0, sizeof(ifr2));
		strncpy(ifr2.ifr_name, "eth2", sizeof(ifr2.ifr_name) - 1);
		ifr2.ifr_data = (void *)&options.page_mii;
		if (ioctl(sk, RAETH_MII_WRITE, &ifr2) < 0) {
			perror("mii_mgr: page-select ioctl");
			status = EXIT_FAILURE;
		} else {
			printf("Set: phy[%d].reg[%d] = %04x\n",
			       options.page_mii.phy_id, options.page_mii.reg_num,
			       options.page_mii.val_in);
		}
	}
#endif

	if (status == EXIT_SUCCESS && options.method != 0) {
		if (ioctl(sk, options.method, &ifr) < 0) {
			perror("mii_mgr: ioctl");
			status = EXIT_FAILURE;
		} else if (options.method == RAETH_MII_READ) {
			printf("Get: phy[%d].reg[%d] = %04x\n",
			       options.mii.phy_id, options.mii.reg_num,
			       options.mii.val_out);
		} else {
			printf("Set: phy[%d].reg[%d] = %04x\n",
			       options.mii.phy_id, options.mii.reg_num,
			       options.mii.val_in);
		}
	}

	if (close(sk) < 0) {
		perror("mii_mgr: close");
		if (status == EXIT_SUCCESS)
			status = EXIT_FAILURE;
	}

	return status;
}
