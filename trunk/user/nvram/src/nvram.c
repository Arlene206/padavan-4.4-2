/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Frontend command-line utility for Linux NVRAM layer
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram.c,v 1.1 2007/06/08 10:22:42 arthur Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <bcmnvram.h>

#include <nvram_linux.h>

#include "nvram_profile_policy.h"
#include "nvram_profile_file.h"
#include "nvram_cli_policy.h"

static void usage(void)
{
	printf("Usage: nvram [get name] [set name=value] [settmp name=value] [unset name]\n"
	       "             [show] [showall] [clear] [save fn] [restore fn] [commit]\n");
	exit(1);
}

static unsigned char get_rand()
{
	unsigned char buf[1];
	FILE *fp;

	buf[0] = 0;
	fp = fopen("/dev/urandom", "rb");
	if (fp == NULL) {
		return 0;
	}
	if (fread(buf, 1, 1, fp) != 1)
		buf[0] = 0;
	fclose(fp);

	return buf[0];
}

static int is_sys_param(char *p)
{
	struct nvram_pair *np;
	extern struct nvram_pair router_defaults[];
	extern struct nvram_pair tables_defaults[];

	for (np = router_defaults; np->name; np++)
	{
		if (strcmp(p, np->name)==0)
			return 1;
	}

	for (np = tables_defaults; np->name; np++)
	{
		if (strstr(p, np->name))
			return 1;
	}

	return 0;
}

static int nvram_save(char *file)
{
    char *name, *buf;
    unsigned long count, filelen, i, padding_len;
    unsigned char random_value, padding[1024];
    unsigned char header[NVRAM_PROFILE_HEADER_SIZE];
    int result;

    buf = malloc(NVRAM_SPACE);
    if (!buf) {
        perror ("Out of memory!\n");
        return -1;
    }

    buf[0] = 0;
    if (nvram_getall(buf, NVRAM_SPACE, 0) < 0 ||
        !nvram_profile_payload_valid((unsigned char *)buf, NVRAM_SPACE,
                                     NVRAM_MAX_PARAM_LEN-1, NVRAM_MAX_VALUE_LEN-1)) {
        free(buf);
        return -1;
    }

    count = 0;
    for (name = buf; *name; name += strlen(name) + 1)
        count += strlen(name) + 1;

    filelen = count + (1024 - count % 1024);
    if (filelen > 0xffffffUL) {
        free(buf);
        return -1;
    }
    random_value = get_rand() % 30;

    for (i = 0; i < count; i++)
    {
        if (buf[i] == 0)
            buf[i] = 0xfd + get_rand() % 3;
        else
            buf[i] = 0xff - buf[i] + random_value;
    }

    /* Every padding sentinel decodes to NUL; avoid opening urandom per byte. */
    memset(padding, 0xfd, sizeof(padding));
    padding_len = filelen - count;

    /* HDR2 stores the length as three little-endian bytes on every host. */
    memcpy(header, NVRAM_PROFILE_HDR2, 4);
    header[4] = (unsigned char)(filelen & 0xffUL);
    header[5] = (unsigned char)((filelen >> 8) & 0xffUL);
    header[6] = (unsigned char)((filelen >> 16) & 0xffUL);
    header[7] = random_value;

    result = nvram_profile_write_atomic(file, header, sizeof(header),
                                        buf, count, padding, padding_len);
    free(buf);
    return result;
}

static int nvram_restore(char *file)
{
	FILE *fp;
	unsigned char header[NVRAM_PROFILE_HEADER_SIZE], *buf, rand;
	char *p, *v;
	unsigned long count, filelen;
	unsigned long profile_capacity = (unsigned long)NVRAM_SPACE + 1024UL;
	int is_current_format, trailing;

	if ((fp = fopen(file, "rb")) == NULL)
		return -1;

	buf = malloc((size_t)profile_capacity);
	if (!buf) {
		fclose(fp);
		perror ("Out of memory!\n");
		return -1;
	}

	count = fread(header, 1, sizeof(header), fp);
	if (count != sizeof(header) ||
	    !nvram_profile_parse_header(header, NVRAM_SPACE, profile_capacity,
					&filelen, &rand, &is_current_format)) {
		fclose(fp);
		free(buf);
		return 1;
	}

	count = fread(buf, 1, (size_t)filelen, fp);
	if (count != filelen || ferror(fp)) {
		fclose(fp);
		free(buf);
		return 1;
	}
	trailing = fgetc(fp);
	if (trailing != EOF || ferror(fp)) {
		fclose(fp);
		free(buf);
		return 1;
	}
	if (fclose(fp) != 0) {
		free(buf);
		return 1;
	}

	if (is_current_format)
		nvram_profile_decode_payload(buf, filelen, rand);

	if (!nvram_profile_payload_valid(buf, filelen,
					 NVRAM_MAX_PARAM_LEN-1, NVRAM_MAX_VALUE_LEN-1)) {
		free(buf);
		return 1;
	}

	/* Never clear live configuration until the complete profile is validated. */
	if (nvram_clear() < 0) {
		free(buf);
		return -1;
	}

	p = (char *)buf;
	while (*p)
	{
		v = strchr(p, '=');
		if (v == NULL) {
			free(buf);
			return 1;
		}
		*v++ = 0;
		if (is_sys_param(p) && nvram_set(p, v) < 0) {
			free(buf);
			return -1;
		}
		p = v + strlen(v) + 1;
	}

	free(buf);
	return 0;
}

static int nvram_show(int show_all)
{
	char *name, *buf;
	int buf_len, size;

	buf_len = NVRAM_SPACE;
	if (show_all)
		buf_len *= 2;

	buf = malloc(buf_len);
	if (!buf) {
		perror ("Out of memory!\n");
		return -1;
	}

	buf[0] = 0;
	nvram_getall(buf, buf_len, show_all);

	for (name = buf; *name; name += strlen(name) + 1)
		puts(name);

	if (!show_all) {
		size = sizeof(struct nvram_header) + (int) name - (int) buf;
		printf("\nsize: %d bytes (%d left)\n", size, NVRAM_SPACE - size);
	}

	free(buf);

	return 0;
}

/* NVRAM utility */
int
main(int argc, char **argv)
{
	char *name, *value;
	int ret = 0;

	/* Skip program name */
	--argc;
	++argv;

	if (!*argv)
		usage();

	/* Process the remaining arguments. */
	for (; *argv; argv++) {
		if (!strcmp(*argv, "get")) {
			if (*++argv) {
				if ((value = nvram_get(*argv)))
					puts(value);
			}
		}
		else if (!strcmp(*argv, "settmp")) {
			if (*++argv) {
				char buf[1024];
				if (nvram_cli_parse_assignment(buf, sizeof(buf), *argv,
						NVRAM_MAX_PARAM_LEN - 1,
						NVRAM_MAX_VALUE_LEN - 1,
						&name, &value) < 0) {
					fprintf(stderr, "Invalid NVRAM assignment; expected name=value.\n");
					if (ret == 0)
						ret = -1;
				} else if (nvram_set_temp(name, value) < 0 && ret == 0) {
					ret = -1;
				}
			} else if (ret == 0) {
				ret = -1;
			}
		}
		else if (!strcmp(*argv, "set")) {
			if (*++argv) {
				char buf[1024];
				if (nvram_cli_parse_assignment(buf, sizeof(buf), *argv,
						NVRAM_MAX_PARAM_LEN - 1,
						NVRAM_MAX_VALUE_LEN - 1,
						&name, &value) < 0) {
					fprintf(stderr, "Invalid NVRAM assignment; expected name=value.\n");
					if (ret == 0)
						ret = -1;
				} else if (nvram_set(name, value) < 0 && ret == 0) {
					ret = -1;
				}
			} else if (ret == 0) {
				ret = -1;
			}
		}
		else if (!strcmp(*argv, "unset")) {
			if (*++argv) {
				if (nvram_unset(*argv) < 0 && ret == 0)
					ret = -1;
			} else if (ret == 0) {
				ret = -1;
			}
		}
		else if (!strcmp(*argv, "clear")) {
			if (nvram_clear() < 0 && ret == 0)
				ret = -1;
		}
		else if (!strcmp(*argv, "commit")) {
			if (nvram_commit() < 0 && ret == 0)
				ret = -1;
			break;
		}
		else if (!strcmp(*argv, "save")) {
			if (*++argv)
				ret |= nvram_save(*argv);
			break;
		}
		else if (!strcmp(*argv, "restore")) {
			if (*++argv)
				ret |= nvram_restore(*argv);
			break;
		}
		else if (!strcmp(*argv, "show")) {
			ret |= nvram_show(0);
			break;
		}
		else if (!strcmp(*argv, "showall")) {
			ret |= nvram_show(1);
			break;
		}
		if (!*argv)
			break;
	}

	return ret;
}
