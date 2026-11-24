#!/usr/bin/env python3
"""Check the HTTPD network-device stats snapshot/error contract."""

from pathlib import Path

WEB = Path("trunk/user/httpd/src/web_ex.c")
PAGE = Path("trunk/user/www/n56u_ribbon_fixed/Main_TrafficMonitor_realtime.asp")
WORKFLOW = Path(".github/workflows/httpd-security.yml")
POLICY = Path("trunk/user/httpd/src/httpd_netdev_policy.h")
TEST = Path("trunk/user/httpd/src/tests/httpd_netdev_policy.c")


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError("unterminated function: " + signature)


def main() -> None:
    web = WEB.read_text(encoding="utf-8")
    page = PAGE.read_text(encoding="utf-8")
    workflow = WORKFLOW.read_text(encoding="utf-8")
    policy = POLICY.read_text(encoding="utf-8")
    test = TEST.read_text(encoding="utf-8")

    assert '#include "httpd_netdev_policy.h"' in web
    handler = body(web, "ej_netdev(int eid, webs_t wp, int argc, char **argv)")
    assert "char buf[512];" in handler
    assert 'fopen("/proc/net/dev", "r")' in handler
    assert "httpd_netdev_status_from_errno(errno)" in handler
    assert "httpd_netdev_parse_counters(p + 1, &rx, &tx)" in handler
    assert "if (!strchr(buf, '\\n'))" in handler
    assert "if (status == 0 && ferror(fp))" in handler
    assert "if (fclose(fp) != 0 && status == 0)" in handler
    assert 'fprintf(wp, "\\nnetdev_status = %d;\\n", status);' in handler
    assert 'fprintf(wp, "\\nnetdev_status = 0;\\nnetdevs = {\\n");' in handler
    assert handler.index("fclose(fp)") < handler.index("netdevs = {")
    assert handler.index("if (status != 0)") < handler.index("netdevs = {")

    assert "strtoull(cursor, &end, 10)" in policy
    assert "errno == ERANGE" in policy
    assert "while (httpd_netdev_is_space((unsigned char)*cursor))" in policy
    assert "if (*cursor != '\\0')" in policy
    assert "*rx = (uint64_t)fields[0];" in policy
    assert "*tx = (uint64_t)fields[8];" in policy
    assert "assert(rx == 0x1122334455667788ULL);" in test
    assert "assert(tx == 0x8877665544332211ULL);" in test
    assert '15 999", &rx, &tx) == EPROTO' in test

    assert "var netdevs = {};" in page
    assert "var netdev_status = 0;" in page
    evaluator = body(page, "function eval_netdevs(response)")
    assert "var previous = netdevs;" in evaluator
    assert "netdev_status = 0;" in evaluator
    assert "if (netdev_status !== 0 || !netdevs" in evaluator
    assert "netdevs = previous;" in evaluator
    assert evaluator.index("if (netdev_status !== 0") < evaluator.index("calc_speed(now_perf);")
    assert "eval_netdevs(response);" in page

    push = workflow.split("  pull_request:", 1)[0]
    pr = workflow.split("  pull_request:", 1)[1].split("  workflow_dispatch:", 1)[0]
    required_paths = (
        "trunk/user/httpd/src/web_ex.c",
        "trunk/user/httpd/src/httpd_netdev_policy.h",
        "trunk/user/httpd/src/tests/httpd_netdev_policy.c",
        "trunk/user/www/n56u_ribbon_fixed/Main_TrafficMonitor_realtime.asp",
        "scripts/check_httpd_netdev_status.py",
    )
    for path in required_paths:
        assert f'      - "{path}"' in push, f"missing push path: {path}"
        assert f'      - "{path}"' in pr, f"missing PR path: {path}"

    assert "Verify HTTPD network-device counter snapshots fail closed" in workflow
    assert "python3 scripts/check_httpd_netdev_status.py" in workflow
    print("HTTPD netdev atomic snapshot, UI retention, and CI path contracts: PASS")


if __name__ == "__main__":
    main()
