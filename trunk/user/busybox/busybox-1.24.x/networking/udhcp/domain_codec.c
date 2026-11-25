/* vi: set sw=4 ts=4: */

/* RFC1035 domain compression routines (C) 2007 Gabriel Somlo <somlo at cmu.edu>
 *
 * Loosely based on the isc-dhcpd implementation by dhankins@isc.org
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifdef DNS_COMPR_TESTING
# define FAST_FUNC /* nothing */
# define xmalloc malloc
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <stdio.h>
#else
# include "common.h"
#endif

#define NS_MAXDNAME  1025	/* max domain name length */
#define NS_MAXCDNAME  255	/* max compressed domain name length */
#define NS_MAXLABEL    63	/* max label length */
#define NS_MAXDNSRCH    6	/* max domains in search path */
#define NS_CMPRSFLGS 0xc0	/* name compression pointer flag */


/* Expand a RFC1035-compressed list of domain names "cstr", of length "clen";
 * returns a newly allocated string containing the space-separated domains,
 * prefixed with the contents of string pre, or NULL if an error occurs.
 */
char* FAST_FUNC dname_dec(const uint8_t *cstr, int clen, const char *pre)
{
	char *ret = ret; /* for compiler */
	char *dst = NULL;

	/* We make two passes over the cstr string. First, we compute
	 * how long the resulting string would be. Then we allocate a
	 * new buffer of the required length, and fill it in with the
	 * expanded content. The advantage of this approach is not
	 * having to deal with requiring callers to supply their own
	 * buffer, then having to check if it's sufficiently large, etc.
	 */
	while (1) {
		/* note: "return NULL" below are leak-safe since
		 * dst isn't yet allocated */
		const uint8_t *c;
		unsigned crtpos, retpos, depth, len;
		int name_terminated;

		crtpos = retpos = depth = len = 0;
		name_terminated = 0;
		while (crtpos < clen) {
			c = cstr + crtpos;

			if ((*c & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
				unsigned target;

				/* pointer */
				if (crtpos + 2 > clen) /* no offset to jump to? abort */
					return NULL;
				target = ((c[0] & 0x3f) << 8) | c[1];
				/* RFC1035 pointers must reference a preceding name. */
				if (target >= crtpos)
					return NULL;
				if (retpos == 0) /* toplevel? save return spot */
					retpos = crtpos + 2;
				depth++;
				name_terminated = 0;
				crtpos = target; /* jump */
			} else if (*c) {
				/* label */
				if (crtpos + *c + 1 > clen) /* label too long? abort */
					return NULL;
				name_terminated = 0;
				if (dst)
					/* \3com ---> "com." */
					((char*)mempcpy(dst + len, c + 1, *c))[0] = '.';
				len += *c + 1;
				crtpos += *c + 1;
			} else {
				/* NUL: end of current domain name */
				name_terminated = 1;
				if (retpos == 0) {
					/* toplevel? keep going */
					crtpos++;
				} else {
					/* return to toplevel saved spot */
					crtpos = retpos;
					retpos = depth = 0;
				}
				if (dst && len != 0)
					/* \4host\3com\0\4host and we are at \0:
					 * \3com was converted to "com.", change dot to space.
					 */
					dst[len - 1] = ' ';
			}

			if (depth > NS_MAXDNSRCH /* too many jumps? abort, it's a loop */
			 || len > NS_MAXDNAME * NS_MAXDNSRCH /* result too long? abort */
			) {
				return NULL;
			}
		}

		/* A truncated label or pointer must not be accepted as a name. */
		if (!name_terminated || !len)
			return NULL;

		if (!dst) { /* first pass? */
			/* allocate dst buffer and copy pre */
			unsigned plen = strlen(pre);
			ret = dst = xmalloc(plen + len);
			memcpy(dst, pre, plen);
			dst += plen;
		} else {
			dst[len - 1] = '\0';
			break;
		}
	}

	return ret;
}

/* Convert a domain name (src) from human-readable "foo.blah.com" format into
 * RFC1035 encoding "\003foo\004blah\003com\000". Return allocated string, or
 * NULL if an error occurs.
 */
static uint8_t *convert_dname(const char *src)
{
	uint8_t c, *res, *lenptr, *dst;
	int len;

	res = xmalloc(strlen(src) + 2);
	dst = lenptr = res;
	dst++;

	for (;;) {
		c = (uint8_t)*src++;
		if (c == '.' || c == '\0') {  /* end of label */
			len = dst - lenptr - 1;
			/* label too long, too short, or two '.'s in a row? abort */
			if (len > NS_MAXLABEL || len == 0 || (c == '.' && *src == '.')) {
				free(res);
				return NULL;
			}
			*lenptr = len;
			if (c == '\0' || *src == '\0')	/* "" or ".": end of src */
				break;
			lenptr = dst++;
			continue;
		}
		if (c >= 'A' && c <= 'Z')  /* uppercase? convert to lower */
			c += ('a' - 'A');
		*dst++ = c;
	}

	if (dst - res >= NS_MAXCDNAME) {  /* dname too long? abort */
		free(res);
		return NULL;
	}

	*dst = 0;
	return res;
}

/* Returns the offset within cstr at which dname can be found, or -1 */
static int find_offset(const uint8_t *cstr, int clen, const uint8_t *dname)
{
	const uint8_t *c, *d;
	int off;

	/* find all labels in cstr */
	off = 0;
	while (off < clen) {
		c = cstr + off;

		if ((*c & NS_CMPRSFLGS) == NS_CMPRSFLGS) {  /* pointer, skip */
			off += 2;
			continue;
		}
		if (*c) {  /* label, try matching dname */
			d = dname;
			while (1) {
				unsigned len1 = *c + 1;
				if (memcmp(c, d, len1) != 0)
					break;
				if (len1 == 1)  /* at terminating NUL - match, return offset */
					return off;
				d += len1;
				c += len1;
				if ((*c & NS_CMPRSFLGS) == NS_CMPRSFLGS)  /* pointer, jump */
					c = cstr + (((c[0] & 0x3f) << 8) | c[1]);
			}
			off += cstr[off] + 1;
			continue;
		}
		/* NUL, skip */
		off++;
	}

	return -1;
}

/* Computes string to be appended to cstr so that src would be added to
 * the compression (best case, it's a 2-byte pointer to some offset within
 * cstr; worst case, it's all of src, converted to <4>host<3>com<0> format).
 * The computed string is returned directly; its length is returned via retlen;
 * NULL and 0, respectively, are returned if an error occurs.
 */
uint8_t* FAST_FUNC dname_enc(const uint8_t *cstr, int clen, const char *src, int *retlen)
{
	uint8_t *d, *dname;
	int off;

	dname = convert_dname(src);
	if (dname == NULL) {
		*retlen = 0;
		return NULL;
	}

	d = dname;
	while (*d) {
		if (cstr) {
			off = find_offset(cstr, clen, d);
			if (off >= 0) {	/* found a match, add pointer and return */
				*d++ = NS_CMPRSFLGS | (off >> 8);
				*d = off;
				break;
			}
		}
		d += *d + 1;
	}

	*retlen = d - dname + 1;
	return dname;
}

#ifdef DNS_COMPR_TESTING
/* gcc -Wall -DDNS_COMPR_TESTING domain_codec.c -o domain_codec && ./domain_codec */
static int expect_decoded(const uint8_t *wire, int wire_len, const char *expected)
{
	char *decoded = dname_dec(wire, wire_len, "");

	if (decoded == NULL || strcmp(decoded, expected) != 0) {
		fprintf(stderr, "unexpected decode result for %s: %s\n",
			expected, decoded ? decoded : "(null)");
		free(decoded);
		return -1;
	}
	free(decoded);
	return 0;
}

static int expect_rejected(const uint8_t *wire, int wire_len)
{
	char *decoded = dname_dec(wire, wire_len, "");

	if (decoded != NULL) {
		fprintf(stderr, "accepted malformed compressed domain: %s\n", decoded);
		free(decoded);
		return -1;
	}
	return 0;
}

static int test_domain_decoder_bounds(void)
{
	static const uint8_t valid_compressed[] = {
		3, 'f', 'o', 'o', 3, 'b', 'a', 'r', 0,
		3, 'w', 'w', 'w', 0xc0, 0x04
	};
	static const uint8_t truncated_label[] = { 3, 'f', 'o', 'o' };
	static const uint8_t truncated_after_complete_name[] = {
		1, 'a', 0, 3, 'f', 'o', 'o'
	};
	static const uint8_t pointer_past_end[] = { 3, 'f', 'o', 'o', 0xc0, 0x20 };
	static const uint8_t forward_pointer[] = { 0xc0, 0x02, 1, 'x', 0 };

	if (expect_decoded(valid_compressed, sizeof(valid_compressed),
			   "foo.bar www.bar") != 0 ||
	    expect_rejected(truncated_label, sizeof(truncated_label)) != 0 ||
	    expect_rejected(truncated_after_complete_name, sizeof(truncated_after_complete_name)) != 0 ||
	    expect_rejected(pointer_past_end, sizeof(pointer_past_end)) != 0 ||
	    expect_rejected(forward_pointer, sizeof(forward_pointer)) != 0)
		return -1;

	return 0;
}

int main(int argc, char **argv)
{
	int len;
	uint8_t *encoded;

	(void)argc;
	(void)argv;
	if (test_domain_decoder_bounds() != 0)
		return 1;
	puts("RFC1035 domain decoder malformed-input bounds: PASS");

	uint8_t str[6] = { 0x00, 0x00, 0x02, 0x65, 0x65, 0x00 };
	printf("NUL:'%s'\n",   dname_dec(str, 6, ""));

#define DNAME_DEC(encoded,pre) dname_dec((uint8_t*)(encoded), sizeof(encoded), (pre))
	printf("'%s'\n",       DNAME_DEC("\4host\3com\0", "test1:"));
	printf("test2:'%s'\n", DNAME_DEC("\4host\3com\0\4host\3com\0", ""));
	printf("test3:'%s'\n", DNAME_DEC("\4host\3com\0\xC0\0", ""));
	printf("test4:'%s'\n", DNAME_DEC("\4host\3com\0\xC0\5", ""));
	printf("test5:'%s'\n", DNAME_DEC("\4host\3com\0\xC0\5\1z\xC0\xA", ""));

#define DNAME_ENC(cache,source,lenp) dname_enc((uint8_t*)(cache), sizeof(cache), (source), (lenp))
	encoded = dname_enc(NULL, 0, "test.net", &len);
	printf("test6:'%s' len:%d\n", dname_dec(encoded, len, ""), len);
	encoded = DNAME_ENC("\3net\0", "test.net", &len);
	printf("test7:'%s' len:%d\n", dname_dec(encoded, len, ""), len);
	encoded = DNAME_ENC("\4test\3net\0", "test.net", &len);
	printf("test8:'%s' len:%d\n", dname_dec(encoded, len, ""), len);
	return 0;
}
#endif
