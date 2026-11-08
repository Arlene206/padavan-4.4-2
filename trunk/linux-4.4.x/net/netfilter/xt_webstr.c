/* Kernel module to match a string into a packet.
 *
 * Copyright (C) 2000 Emmanuel Roger  <winfield@freegates.be>
 *
 * ChangeLog
 *	19.02.2002: Gianni Tedesco <gianni@ecsc.co.uk>
 *		Fixed SMP re-entrancy problem using per-cpu data areas
 *		for the skip/shift tables.
 *	02.05.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 *		Fixed kernel panic, due to overrunning boyer moore string
 *		tables. Also slightly tweaked heuristic for deciding what
 * 		search algo to use.
 * 	27.01.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 * 		Implemented Boyer Moore Sublinear search algorithm
 * 		alongside the existing linear search based on memcmp().
 * 		Also a quick check to decide which method to use on a per
 * 		packet basis.
 * Download location
 *               http://svn.dd-wrt.com:8000/browser//src/linux/xscale/linux-2.6.34.6/net/ipv4/netfilter
 */

/* Kernel module to match a http header string into a packet.
 *
 * Copyright (C) 2009 - 2010, CyberTAN Corporation
 *
 * All Rights Reserved.
 * You can redistribute it and/or modify it under the terms of the GPL v2
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. CYBERTAN
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NON INFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * Description:
 *   This is kernel module for web content inspection. It was derived from
 *   'string' match module, declared as above.
 *
 *   The module follows the Netfilter framework, called extended packet
 *   matching modules.
 * Restriction:
 *            Can not search https headers, higly recomended to use  tinyproxy server to block contents
 ******************************************************************************************************
 Includes Intel Corporation's changes/modifications dated: 02.11.2011
 Changed/modified portions :
 * -  X-tables API adapted for kernel 2.6.39
 * - added, improved Debug prints
 * - Kernel warning /complains of a too big stack size in the xt_webstr_match() function fixed
 * - copy from packet sk_buff removed, search is done directly in the tcp payload (optimization)
 * Copyright © 2011-2014, Intel Corporation.
 ******************************************************************************************************

 */



#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <linux/netfilter/x_tables.h>
#if IS_ENABLED(CONFIG_IPV6)
#include <net/ipv6.h>
#endif

#include "xt_webstr_parser_policy.h"
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter/xt_webstr.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gianni Tedesco <gianni@ecsc.co.uk>");
MODULE_ALIAS("ip6t_webstr");
MODULE_ALIAS("ipt_webstr");

//MODULE_DESCRIPTION("Xtables: match --url or --host parameter in http header IPv4");
#define	isdigit(x) ((x) >= '0' && (x) <= '9')
#define	isupper(x) (((unsigned)(x) >= 'A') && ((unsigned)(x) <= 'Z'))
#define	islower(x) (((unsigned)(x) >= 'a') && ((unsigned)(x) <= 'z'))
#define	isalpha(x) (isupper(x) || islower(x))
#define	toupper(x) (isupper(x) ? (x) : (x) - 'a' + 'A')
#define tolower(x) (isupper(x) ? ((x) - 'A' + 'a') : (x))

#define URLSIZE 	1024
#define WORDSIZE 	256
#define HOSTSIZE 	256
#define WEBSTR_MAX_SCAN_SIZE 2048


/* Flags for get_http_info() */
#define HTTP_HOST	0x01
#define HTTP_URL	0x02
/* Flags for mangle_http_header() */
#define HTTP_COOKIE	0x04

//#define DEBUG
#ifdef DEBUG
#define DEBUGP printk
#define PRINTK_LEVEL KERN_CRIT
#else
#define PRINTK_LEVEL
#define DEBUGP( PRINTK_LEVEL, format, args...)
#endif

typedef struct httpinfo {
    char  host[HOSTSIZE];
    int hostlen;
    char  url[URLSIZE];
    int urllen;
} httpinfo_t;

/* Return 1 for match, 0 for accept, -1 for partial. */
static int find_pattern2(const char *data, size_t dlen,
	const char *pattern, size_t plen,
	char term,
	unsigned int *numoff,
	unsigned int *numlen)
{
    size_t i;
    int state = 0;
    *numoff = *numlen = 0;

    DEBUGP( PRINTK_LEVEL "%s: pattern = '%s', dlen = %u\n",__FUNCTION__, pattern, dlen);
    if (dlen == 0)
	return 0;

    if (dlen <= plen) {	/* Short packet: try for partial? */
	if (strncasecmp(data, pattern, dlen) == 0)
	    return -1;
	else 
	    return 0;
    }
    for (i = 0; i <= (dlen - plen); i++) {
	/* DFA : \r\n\r\n :: 1234 */
	if (*(data + i) == '\r') {
	    if (!(state % 2)) state++;	/* forwarding move */
	    else state = 0;		/* reset */
	}
	else if (*(data + i) == '\n') {
	    if (state % 2) state++;
	    else state = 0;
	}
	else state = 0;

	if (state >= 4)
	    break;

	/* pattern compare */
	if (memcmp(data + i, pattern, plen ) != 0)
	    continue;

	/* Here, it means pattern match. Bound the CR search before reading. */
	*numoff = i + plen;
	if (xt_webstr_find_terminator((const unsigned char *)data,
				      (unsigned int)dlen, *numoff,
				      (unsigned char)term, numlen) != 0)
	    return -1;

	return 1;
    }
    return 0;
}

static int mangle_http_header(int flags, unsigned char *data, unsigned int datalen)
{
    int found, ret = 0;
    unsigned int offset, len;

    DEBUGP( PRINTK_LEVEL "%s: processing HTTP header data\n", __FUNCTION__);

    /* Basic checking, is it HTTP packet? */
    if (datalen < 10)
	return ret;	/* Not enough length, ignore it */
    if (memcmp(data, "GET ", sizeof("GET ") - 1) != 0 &&
        memcmp(data, "POST ", sizeof("POST ") - 1) != 0 &&
        memcmp(data, "HEAD ", sizeof("HEAD ") - 1) != 0)
	return ret;	/* Pass it */

    /* COOKIE modification */
    if (flags & HTTP_COOKIE) {
	found = find_pattern2(data, datalen, "Cookie: ", 
		sizeof("Cookie: ")-1, '\r', &offset, &len);
	if (found == 1) {
	    char c;
	    offset -= (sizeof("Cookie: ") - 1);
	    /* Swap the 2rd and 4th bit */
	    c = *(data + offset + 2) ;
	    *(data + offset + 2) = *(data + offset + 4) ;
	    *(data + offset + 4) = c ;
	    ret++;
	}
    }

    return ret;
}

static int get_http_info(int flags, httpinfo_t *info, const unsigned char *data, unsigned int datalen)
{
    int found, offset = 0;
    int hostlen = 0, pathlen;
    unsigned int match_offset = 0, match_length = 0;
    int ret = 0;

    if (!info || !data)
	return 0;
    memset(info, 0, sizeof(*info));

    if (datalen < 10)
	return ret;
    if (memcmp(data, "GET ", sizeof("GET ") - 1) != 0 &&
        memcmp(data, "POST ", sizeof("POST ") - 1) != 0 &&
        memcmp(data, "HEAD ", sizeof("HEAD ") - 1) != 0)
	return ret;
    if (!(flags & (HTTP_HOST | HTTP_URL)))
	return ret;

    if (flags & HTTP_HOST) {
	found = find_pattern2(data, datalen, "Host: ",
			      sizeof("Host: ") - 1, '\r',
			      &match_offset, &match_length);
	if (found != 1 || !match_length)
		return ret;

	offset = (int)match_offset;
	hostlen = (int)xt_webstr_copy_length(match_length, HOSTSIZE);
	memcpy(info->host, data + offset, (unsigned int)hostlen);
	info->host[hostlen] = '\0';
	info->hostlen = hostlen;
	ret++;
    }

    if (!(flags & HTTP_URL))
	return ret;

    found = find_pattern2(data, datalen, "GET ",
			  sizeof("GET ") - 1, '\r',
			  &match_offset, &match_length);
    if (found != 1) {
	found = find_pattern2(data, datalen, "POST ",
			      sizeof("POST ") - 1, '\r',
			      &match_offset, &match_length);
    }
    if (found != 1) {
	found = find_pattern2(data, datalen, "HEAD ",
			      sizeof("HEAD ") - 1, '\r',
			      &match_offset, &match_length);
    }
    if (found != 1)
	return ret;

    offset = (int)match_offset;
    pathlen = (int)match_length;
    if (pathlen <= (int)(sizeof(" HTTP/x.x") - 1))
	return ret;
    pathlen -= (int)(sizeof(" HTTP/x.x") - 1);

    ret++;
    pathlen = (int)xt_webstr_url_path_length((unsigned int)hostlen,
					     (unsigned int)pathlen, URLSIZE);
    memcpy(info->url, info->host, (unsigned int)hostlen);
    memcpy(info->url + hostlen, data + offset, (unsigned int)pathlen);
    info->url[hostlen + pathlen] = '\0';
    info->urllen = hostlen + pathlen;

    return ret;
}

/* Linear string search based on memcmp() */
static char *search_linear(char *needle, char *haystack,
			    int needle_len, int haystack_len)
{
	char *end, *cursor;

	if (!needle || !haystack || needle_len <= 0 ||
	    haystack_len < needle_len)
		return NULL;
	end = haystack + (haystack_len - needle_len);
	for (cursor = haystack; cursor <= end; cursor++) {
		if (strncasecmp(cursor, needle, needle_len) == 0)
			return cursor;
	}
	return NULL;
}

#define TOKEN   "<&nbsp;>"

static int
webstr_match_token_list(const char *wordlist, unsigned int wordlist_len,
			char *haystack, int haystack_len)
{
	const unsigned int delimiter_len = sizeof(TOKEN) - 1;
	unsigned int offset = 0, token_start, token_len;
	char needle[WORDSIZE];
	char *chtemp;
	int nlen, status;

	if (!wordlist || !haystack || wordlist_len > BM_MAX_NLEN ||
	    haystack_len <= 0)
		return 0;

	while (offset < wordlist_len) {
		token_start = offset;
		status = xt_webstr_next_token(wordlist, wordlist_len, &offset,
					      TOKEN, delimiter_len, &token_len);
		if (status <= 0)
			break;
		if (token_len >= sizeof(needle))
			token_len = sizeof(needle) - 1;
		memcpy(needle, wordlist + token_start, token_len);
		needle[token_len] = '\0';
		chtemp = strstr(needle, "://");
		if (chtemp)
			memmove(needle, chtemp + 3, strlen(chtemp + 3) + 1);
		nlen = strlen(needle);
		if (!nlen || nlen > haystack_len)
			continue;
		if (search_linear(needle, haystack, nlen, haystack_len) != NULL)
			return 1;
	}
	return 0;
}

static bool xt_webstr_match(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_webstr_info *info;
	const struct tcphdr *tcph;
	struct tcphdr _tcph;
	unsigned char method_buf[sizeof("POST ") - 1];
	const unsigned char *method;
	httpinfo_t *htinfo = NULL;
	unsigned char *data = NULL;
	unsigned char *allocated_data = NULL;
	char *wordlist;
	unsigned int datalen, payload_offset, tcp_header_len;
	int network_offset, tcp_offset;
	int flags = 0;
	int found = 0;
	int result = 0;
	long int opt = 0;

	if (!skb || !par || !par->matchinfo)
		return 0;
	info = par->matchinfo;
	if (info->len < 1 || info->len >= sizeof(info->string) ||
	    info->string[info->len] != '\0')
		return 0;
	wordlist = (char *)&info->string;

	network_offset = skb_network_offset(skb);
	if (network_offset < 0)
		return 0;

	if (par->family == NFPROTO_IPV4) {
		struct iphdr _iph;
		const struct iphdr *iph;
		unsigned int ip_header_len;

		iph = skb_header_pointer(skb, network_offset, sizeof(_iph), &_iph);
		if (!iph || iph->version != 4 || iph->ihl < 5 ||
		    iph->protocol != IPPROTO_TCP)
			return 0;
		if (ntohs(iph->frag_off) & IP_OFFSET)
			return 0;
		ip_header_len = (unsigned int)iph->ihl * 4;
		if (!xt_webstr_span_is_valid(skb->len,
					     (unsigned int)network_offset,
					     ip_header_len))
			return 0;
		tcp_offset = network_offset + (int)ip_header_len;
	}
#if IS_ENABLED(CONFIG_IPV6)
	else if (par->family == NFPROTO_IPV6) {
		struct ipv6hdr _iph;
		const struct ipv6hdr *iph;
		u8 nexthdr;
		__be16 frag_off = 0;

		iph = skb_header_pointer(skb, network_offset, sizeof(_iph), &_iph);
		if (!iph || iph->version != 6)
			return 0;
		nexthdr = iph->nexthdr;
		tcp_offset = ipv6_skip_exthdr(skb,
					      network_offset + (int)sizeof(*iph),
					      &nexthdr, &frag_off);
		if (tcp_offset < 0 || nexthdr != IPPROTO_TCP ||
		    !xt_webstr_ipv6_fragment_is_initial(ntohs(frag_off)))
			return 0;
	}
#endif
	else
		return 0;

	if (tcp_offset < 0 ||
	    !xt_webstr_span_is_valid(skb->len, (unsigned int)tcp_offset,
				     sizeof(_tcph)))
		return 0;
	tcph = skb_header_pointer(skb, tcp_offset, sizeof(_tcph), &_tcph);
	if (!tcph || tcph->doff < 5)
		return 0;
	tcp_header_len = (unsigned int)tcph->doff * 4;
	if (!xt_webstr_tcp_header_is_valid(skb->len,
					   (unsigned int)tcp_offset,
					   tcp_header_len))
		return 0;

	payload_offset = (unsigned int)tcp_offset + tcp_header_len;
	if (!xt_webstr_span_is_valid(skb->len, payload_offset, 0))
		return 0;
	datalen = skb->len - payload_offset;
	if (datalen > WEBSTR_MAX_SCAN_SIZE)
		datalen = WEBSTR_MAX_SCAN_SIZE;
	if (datalen < 10)
		return 0;

	/* Reject non-HTTP TCP payloads before allocating a linear copy. */
	method = skb_header_pointer(skb, (int)payload_offset,
				    sizeof(method_buf), method_buf);
	if (!method)
		return 0;
	if (memcmp(method, "GET ", sizeof("GET ") - 1) != 0 &&
	    memcmp(method, "POST ", sizeof("POST ") - 1) != 0 &&
	    memcmp(method, "HEAD ", sizeof("HEAD ") - 1) != 0)
		return 0;

	if (xt_webstr_span_is_valid(skb_headlen(skb), payload_offset, datalen)) {
		data = skb->data + payload_offset;
	} else {
		allocated_data = kmalloc(datalen, GFP_ATOMIC);
		if (!allocated_data)
			goto out;
		if (skb_copy_bits(skb, (int)payload_offset, allocated_data,
				  (int)datalen) != 0)
			goto out;
		data = allocated_data;
	}

	switch (info->type) {
	case XT_WEBSTR_URL:
		flags |= HTTP_URL;
		/* fall through */
	case XT_WEBSTR_HOST:
		flags |= HTTP_HOST;
		break;
	case XT_WEBSTR_CONTENT:
		opt = simple_strtol(wordlist, (char **)NULL, 10);
		if (opt & (BLK_JAVA | BLK_ACTIVE | BLK_PROXY))
			flags |= HTTP_URL;
		if (opt & BLK_PROXY)
			flags |= HTTP_HOST;
		if (opt & BLK_COOKIE) {
			int changed = mangle_http_header(HTTP_COOKIE, data, datalen);

			if (changed > 0 && allocated_data &&
			    skb_store_bits((struct sk_buff *)skb, (int)payload_offset,
				   data, (int)datalen) != 0)
				goto out;
		}
		break;
	default:
		goto out;
	}

	if (!flags)
		goto out;

	/* Per-invocation state avoids the original cross-CPU parser data race.
	 * Keep the >1 KiB object off the stack on CONFIG_FRAME_WARN boards. */
	htinfo = kmalloc(sizeof(*htinfo), GFP_ATOMIC);
	if (!htinfo)
		goto out;
	if (get_http_info(flags, htinfo, data, datalen) < 1)
		goto out;

	if (info->type == XT_WEBSTR_HOST || info->type == XT_WEBSTR_URL) {
		char *haystack;
		int hlen;

		if (info->type == XT_WEBSTR_HOST) {
			haystack = htinfo->host;
			hlen = htinfo->hostlen;
		} else {
			haystack = htinfo->url;
			hlen = htinfo->urllen;
		}
		found = webstr_match_token_list(wordlist, info->len, haystack, hlen);
	} else {
		int vicelen;

		if (opt & BLK_JAVA) {
			vicelen = sizeof(".js") - 1;
			if (htinfo->urllen >= vicelen &&
			    strncasecmp(htinfo->url + htinfo->urllen - vicelen,
					".js", vicelen) == 0) {
				found = 1;
				goto match_ret;
			}
			vicelen = sizeof(".class") - 1;
			if (htinfo->urllen >= vicelen &&
			    strncasecmp(htinfo->url + htinfo->urllen - vicelen,
					".class", vicelen) == 0) {
				found = 1;
				goto match_ret;
			}
		}
		if (opt & BLK_ACTIVE) {
			vicelen = sizeof(".ocx") - 1;
			if (htinfo->urllen >= vicelen &&
			    strncasecmp(htinfo->url + htinfo->urllen - vicelen,
					".ocx", vicelen) == 0) {
				found = 1;
				goto match_ret;
			}
			vicelen = sizeof(".cab") - 1;
			if (htinfo->urllen >= vicelen &&
			    strncasecmp(htinfo->url + htinfo->urllen - vicelen,
					".cab", vicelen) == 0) {
				found = 1;
				goto match_ret;
			}
		}
		if ((opt & BLK_PROXY) &&
		    htinfo->urllen >= htinfo->hostlen +
			(int)(sizeof("http://") - 1) &&
		    strncasecmp(htinfo->url + htinfo->hostlen,
				"http://", sizeof("http://") - 1) == 0)
			found = 1;
	}

match_ret:
	result = found ^ info->invert;
out:
	kfree(htinfo);
	kfree(allocated_data);
	return result;
}

static struct xt_match webstr_match[] __read_mostly = {
	{
		.name		= "webstr",
		.family		= NFPROTO_IPV4,
		.match		= xt_webstr_match,
		.matchsize      = sizeof(struct xt_webstr_info),
		.me		= THIS_MODULE,
	},
#if IS_ENABLED(CONFIG_IPV6)
	{
		.name		= "webstr",
		.family		= NFPROTO_IPV6,
		.match		= xt_webstr_match,
		.matchsize      = sizeof(struct xt_webstr_info),
		.me		= THIS_MODULE,
	}
#endif
};


static int __init xt_webstr_init(void)
{
	return xt_register_matches(webstr_match, ARRAY_SIZE(webstr_match));
}

static void __exit xt_webstr_fini(void)
{
	xt_unregister_matches(webstr_match, ARRAY_SIZE(webstr_match));
}

module_init(xt_webstr_init);
module_exit(xt_webstr_fini);
