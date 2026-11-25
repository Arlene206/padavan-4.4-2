#!/usr/bin/env python3
"""Check that redirect metadata allocation precedes firewall mutation."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr.c").read_text()
SOAP_SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/upnpsoap.c").read_text()
POLICY = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/miniupnpd_redirect_desc_policy.h").read_text()


def function_body(source, signature):
    position = source.index(signature)
    brace = source.find("{", position)
    semicolon = source.find(";", position)
    if semicolon >= 0 and semicolon < brace:
        position = source.index(signature, position + len(signature))
        brace = source.find("{", position)
    depth = 0
    state = "code"
    index = brace
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "/" and next_char == "*":
                state = "block"
                index += 1
            elif char == "/" and next_char == "/":
                state = "line"
                index += 1
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return " ".join(source[position:index + 1].split())
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif state == "string" and char == '"':
                state = "code"
            elif state == "char" and char == "'":
                state = "code"
        elif state == "block":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state == "line" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


def normalized(signature):
    return function_body(SOURCE, signature)


prepare = normalized("prepare_redirect_desc(unsigned short eport, int proto,")
assert "miniupnpd_redirect_desc_allocate(sizeof(*p), l, malloc)" in prepare
assert "saved_errno = errno ? errno : ENOMEM;" in prepare
assert "errno = saved_errno;" in prepare

add_redirect = normalized("add_redirect_rule2(const char * ifname,")
assert add_redirect.index("prepare_redirect_desc(") < add_redirect.index("addnatrule(")
assert "publish_redirect_desc(desc_entry);" in add_redirect
assert "free(desc_entry);" in add_redirect
assert "if(!desc_entry) return -1;" in add_redirect

add_peer = normalized("add_peer_redirect_rule2(const char * ifname,")
assert add_peer.index("prepare_redirect_desc(") < add_peer.index("addpeernatrule(")
assert "publish_redirect_desc(desc_entry);" in add_peer
assert "free(desc_entry);" in add_peer
assert "if(!desc_entry) return -1;" in add_peer

update_desc = normalized("update_portmapping_desc_timestamp(const char * ifname,")
assert update_desc.index("prepare_redirect_desc(") < update_desc.index("replace_redirect_desc(")
assert "if(!desc_entry) return -1;" in update_desc

update = normalized("update_portmapping(const char * ifname, unsigned short eport, int proto,")
assert update.index("prepare_redirect_desc(") < update.index('iptc_init("nat")')
assert "free(desc_entry); return -1;" in update
filter_commit = update.index('update_rule_and_commit("filter", miniupnpd_forward_chain, index, filter_e)')
publish = update.index("replace_redirect_desc(eport, proto, desc_entry);", filter_commit)
assert filter_commit < publish
rollback = update.index("rollback_nat:")
assert "free(desc_entry);" in update[rollback:]
assert "return update_portmapping_desc_timestamp(" not in update

get_description = normalized("get_redirect_desc(unsigned short eport, int proto,")
get_description_compact = get_description.replace("( ", "(")
assert "miniupnpd_redirect_desc_copy_bounded(desc, (size_t)desclen, p->str)" in get_description_compact
assert 'miniupnpd_redirect_desc_copy_bounded(desc, (size_t)desclen, "miniupnpd")' in get_description_compact
assert "strncpy(desc" not in get_description

soap_get_specific = function_body(SOAP_SOURCE, "GetSpecificPortMappingEntry(struct upnphttp * h,")
assert "char desc[64];" in soap_get_specific
assert "desc, sizeof(desc)" in soap_get_specific
assert soap_get_specific.index("upnp_get_redirection_infos(") < soap_get_specific.index("miniupnpd_soap_xml_escape(escaped_desc")

copy_policy = " ".join(POLICY.split())
assert "copy_size = strlen(source);" in copy_policy
assert "if (copy_size >= destination_size) copy_size = destination_size - 1;" in copy_policy
assert "destination[copy_size] = '\\0';" in copy_policy

replace = normalized("replace_redirect_desc(unsigned short eport, int proto, struct rdr_desc * p)")
assert replace.index("del_redirect_desc(") < replace.index("publish_redirect_desc(")

policy_text = " ".join(POLICY.split())
assert "text_size > (size_t)-1 - header_size" in policy_text
assert "errno = EOVERFLOW;" in policy_text
assert "errno = EINVAL;" in policy_text
assert "miniupnpd_redirect_desc_copy_bounded" in policy_text

print("MiniUPnP redirect metadata allocation/order source contract: PASS")
