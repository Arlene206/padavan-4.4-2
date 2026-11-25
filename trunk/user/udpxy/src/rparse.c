/* @(#) parsing functions for udpxy
 *
 * Copyright 2008-2011 Pavel V. Cherenkov (pcherenkov@gmail.com)
 *
 *  This file is part of udpxy.
 *
 *  udpxy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  udpxy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with udpxy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>

#include "rparse.h"
#include "mtrace.h"

/* parse and copy parameters of HTTP GET request into
 * request buffer
 *
 * @param src       buffer with raw source data
 * @param srclen    length of raw data
 * @param request   destination buffer for the request
 * @param rqlen     length of request buffer on entry
 *                  length of request on exit
 *
 * @return 0 if success: request buffer gets populated,
 *         non-zero if error
 */
int
get_request( const char* src, size_t srclen,
             char* request, size_t* rqlen )
{
    static const char HEAD[] = "GET /";
    const size_t head_len = sizeof(HEAD) - 1U;
    const char *nul, *end, *p = NULL, *space;
    size_t input_len = srclen, i, remaining, n;

    if (!src || !request || !rqlen || *rqlen == 0U)
        return 3;

    /* recv() does not append a NUL when the buffer is filled exactly. */
    nul = (const char *)memchr(src, '\0', srclen);
    if (nul)
        input_len = (size_t)(nul - src);
    if (input_len < head_len)
        return 1;

    for (i = 0; i <= input_len - head_len; ++i) {
        if (memcmp(src + i, HEAD, head_len) == 0) {
            p = src + i + head_len;
            break;
        }
    }
    if (!p)
        return 1;

    end = src + input_len;
    remaining = (size_t)(end - p);
    if (remaining <= 1U)
        return 2;

    space = (const char *)memchr(p, ' ', remaining);
    if (!space)
        return 3;
    n = (size_t)(space - p);
    if (n >= *rqlen)
        return 3;

    memcpy(request, p, n);
    request[n] = '\0';
    *rqlen = n;
    return 0;
}



/* parse (GET) request into command and parameters (options)
 * c-strings
 *
 * @param s         source c-string
 * @param cmd       buffer for the parsed command c-string
 * @param clen      length of command buffer
 * @param opt       buffer for the parsed options c-string
 * @param optlen    length of options buffer
 * @param tail      buffer for tail (whatever is beyond options)
 * @param tlen      length of tail buffer
 *
 * @return 0 if success: cmd and opt get get populated
 *         non-zero if an error ocurred
 */
int
parse_param( const char* s, size_t slen,
             char* cmd,     size_t clen,
             char* opt,     size_t optlen,
             char* tail,    size_t tlen)
{
    const char DLM = '/';
    const char *nul, *query;
    size_t len = slen, i, j, n, tail_len, copy_len;

    if (!s || !cmd || clen == 0U || !opt || optlen == 0U)
        return EINVAL;

    *cmd = '\0';
    *opt = '\0';
    if (tail && tlen > 0U)
        tail[0] = '\0';
    if (slen == 0U)
        return 0;

    /* Treat an embedded NUL as the end of the supplied byte span. */
    nul = (const char *)memchr(s, '\0', slen);
    if (nul)
        len = (size_t)(nul - s);
    if (len == 0U)
        return 0;

    i = (DLM == s[0]) ? 1U : 0U;
    for (j = 0; (i < len) && (j < clen) &&
                (s[i] != '\0') && (s[i] != DLM); ) {
        cmd[j++] = s[i++];
    }
    if (j >= clen)
        return EOVERFLOW;
    cmd[j] = '\0';

    if ((i < len) && (s[i] == DLM))
        ++i;
    if (i >= len)
        return 0;

    query = (const char *)memchr(s + i, '?', len - i);
    n = query ? (size_t)(query - (s + i)) : len - i;
    if (n >= optlen)
        return EOVERFLOW;
    memcpy(opt, s + i, n);
    opt[n] = '\0';

    if (query && tail && tlen > 0U) {
        tail_len = len - (size_t)(query - s);
        copy_len = (tail_len < (tlen - 1U)) ? tail_len : (tlen - 1U);
        memcpy(tail, query, copy_len);
        tail[copy_len] = '\0';
    }
    return 0;
}




/* parse options of upd-relay command into IP address
 * and port
 *
 * @param opt       options string
 * @param addr      destination for address string
 * @param addrlen   length of address string buffer
 * @param port      port to populate
 *
 * @return 0 if success: inaddr and port get populated
 *         non-zero if error
 */
int
parse_udprelay( const char* opt, size_t optlen,
                char* s_addr,     size_t s_addrlen,
                char* addr,       size_t addrlen,
                uint16_t* port )
{
    const char *nul, *source;
    char buffer[512];
    char *host;
    size_t input_len, at, host_len, host_remaining, sep, i;
    unsigned int value = 0U;
    int has_source = 0;

    if (s_addr && s_addrlen > 0U)
        s_addr[0] = '\0';
    if (addr && addrlen > 0U)
        addr[0] = '\0';
    if (!opt || !s_addr || !addr || !port ||
        s_addrlen == 0U || addrlen == 0U || optlen == 0U)
        return 1;

    /* optlen includes the terminator; reject truncated/nonterminated input. */
    nul = (const char *)memchr(opt, '\0', optlen);
    if (!nul)
        return 1;
    input_len = (size_t)(nul - opt);
    if (input_len == 0U || input_len >= sizeof(buffer))
        return 1;
    memcpy(buffer, opt, input_len);
    buffer[input_len] = '\0';

    at = input_len;
    for (i = 0; i < input_len; ++i) {
        if (buffer[i] == '@') {
            at = i;
            has_source = 1;
            break;
        }
    }

    source = buffer;
    host = buffer;
    if (has_source) {
        if (at == 0U || (at + 1U) >= input_len)
            return 1;
        host = buffer + at + 1U;
    }

    host_remaining = input_len - (size_t)(host - buffer);
    sep = host_remaining;
    for (i = 0; i < host_remaining; ++i) {
        /* Compare bytes directly: strchr() is undefined for negative
         * signed-char values received from an untrusted HTTP request. */
        if (host[i] == ':' || host[i] == '%' || host[i] == '~' ||
            host[i] == '+' || host[i] == '-' || host[i] == '^') {
            sep = i;
            break;
        }
        if (host[i] == '@')
            return 1;
    }
    if (sep == 0U || sep >= host_remaining)
        return 1;
    host_len = sep;

    if (sep + 1U >= host_remaining)
        return 3;
    for (i = sep + 1U; i < host_remaining; ++i) {
        unsigned int digit;
        if (host[i] < '0' || host[i] > '9')
            return 3;
        digit = (unsigned int)(host[i] - '0');
        if (value > (65534U - digit) / 10U)
            return 3;
        value = value * 10U + digit;
    }
    if (value == 0U)
        return 3;

    /* Never truncate a source/group literal into a different address. */
    if ((has_source && at >= s_addrlen) || host_len >= addrlen)
        return 3;

    if (has_source) {
        memcpy(s_addr, source, at);
        s_addr[at] = '\0';
    }
    else {
        s_addr[0] = '\0';
    }
    memcpy(addr, host, host_len);
    addr[host_len] = '\0';
    *port = (uint16_t)value;
    return 0;
}


/* __EOF__ */

