#!/usr/bin/env python3
"""Verify IAPP descriptor, receive, and event-log persistence safety."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "trunk/user/ralinkiappd/src/rtmpiapp.c"
POLICY = ROOT / "trunk/user/ralinkiappd/src/iapp_select_fd_policy.h"
DEADLINE_POLICY = ROOT / "trunk/user/ralinkiappd/src/iapp_tcp_deadline_policy.h"
DEADLINE_TEST = ROOT / "trunk/user/ralinkiappd/tests/iapp_select_fd_policy.c"


def extract_function(source: str, name: str) -> str:
    matches = list(re.finditer(r"\b(?:static\s+)?VOID\s+" + re.escape(name) + r"\s*\(", source))
    if not matches:
        raise SystemExit("FAIL: missing " + name + " definition")
    start = matches[-1].start()
    opening = source.find("{", matches[-1].end())
    if opening < 0:
        raise SystemExit("FAIL: missing " + name + " body")

    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "\"":
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and nxt == "*":
                state = "block_comment"
                i += 1
            elif ch == "/" and nxt == "/":
                state = "line_comment"
                i += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[start:i + 1]
        elif state == "string":
            if ch == "\\":
                i += 1
            elif ch == "\"":
                state = "code"
        elif state == "char":
            if ch == "\\":
                i += 1
            elif ch == "'":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 1
        elif state == "line_comment":
            if ch == "\n":
                state = "code"
        i += 1

    raise SystemExit("FAIL: unterminated " + name + " body")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("FAIL: " + message)


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    policy = POLICY.read_text(encoding="utf-8")
    require('#include "iapp_select_fd_policy.h"' in source,
            "Linux event loop must include the checked select descriptor policy")
    require("fd >= FD_SETSIZE" in policy,
            "policy must reject descriptors at and above FD_SETSIZE")

    body = extract_function(source, "IAPP_RcvHandler")
    loop_start = body.find("while(1)")
    require(loop_start >= 0, "IAPP receive event loop is missing")
    linux_start = body.find("#ifdef IAPP_OS_LINUX", loop_start)
    linux_else_marker = "#else\n\t\tFD_SET(pCtrlBK->SocketUdpSend, &FdSet);"
    linux_else = body.find(linux_else_marker, linux_start)
    require(linux_start >= 0 and linux_else > linux_start,
            "Linux event loop must have a checked registration branch")
    linux_branch = body[linux_start:linux_else]

    required_fds = (
        "pCtrlBK->SocketUdpSend",
        "pCtrlBK->SocketTcpRcv",
        "pCtrlBK->SocketRawRRB",
        "pCtrlBK->SocketRawDrv",
        "pCtrlBK->PipeRawDrv[FT_KDP_PIPE_ID_READ]",
    )
    for descriptor in required_fds:
        call = "iapp_select_fd_add(&FdSet, " + descriptor + ", &SockMax)"
        require(call in linux_branch,
                "Linux FD_SET registration bypasses the guard for " + descriptor)

    require("FD_SET(" not in linux_branch,
            "Linux branch must not call FD_SET outside the checked helper")
    require("SockMax = -1;" in linux_branch,
            "Linux select maximum must be rebuilt from validated descriptors")
    require("DBGPRINT(RT_DEBUG_ERROR" in linux_branch and "break;" in linux_branch,
            "invalid select descriptors must abort the event loop before select")
    require(body.find("select(SockMax+1") > linux_else,
            "select must run only after the Linux descriptor validation branch")
    require(body.find("FD_ISSET(pCtrlBK->SocketUdpSend") > body.find("select(SockMax+1"),
            "event dispatch must remain after select")

    datagram_handlers = (
        ("IAPP_RcvHandlerUdp", "SocketUdpSend", "pIappHdr = (RT_IAPP_HEADER *)(pPktBuf);"),
        ("IAPP_RcvHandlerRawDrv", "SocketRawDrv", "NdisCopyMemory(WifiMAC, pPktBuf, ETH_ALEN);"),
        ("IAPP_RcvHandlerRawRRB", "SocketRawRRB", "pFrameRRB = (FT_RRB_FRAME *)pPktBuf;"),
    )
    for name, socket_name, parse_marker in datagram_handlers:
        handler = extract_function(source, name)
        receive = handler.find("recvfrom(pCtrlBK->" + socket_name)
        trunc_flag = handler.find("MSG_TRUNC", receive)
        length_guard = handler.find("iapp_rx_length_within_capacity", trunc_flag)
        parse = handler.find(parse_marker, length_guard)
        require(receive >= 0 and trunc_flag > receive,
                name + " must request the original datagram length with MSG_TRUNC")
        require(length_guard > trunc_flag and
                "IAPP_MAX_RCV_PKT_SIZE" in handler[length_guard:length_guard + 140],
                name + " must reject datagrams larger than the receive buffer")
        require(parse > length_guard,
                name + " must reject oversized datagrams before reading the frame")

    require('#include "iapp_event_log_poll_policy.h"' in source,
            "IAPP event loop must include the event-log poll policy")
    require("if (iapp_event_log_poll_due(res.tv_sec))" in source,
            "event snapshots must use the one-second polling policy")
    require("res.tv_sec >= 1000" not in source,
            "event-log poll interval must not regress to 1000 seconds")
    start_handler = extract_function(source, "IAPP_Start")
    poll_gate = start_handler.find("iapp_event_log_poll_due(res.tv_sec)")
    query_call = start_handler.find("IAPP_EventLog_Query(pCtrlBK)", poll_gate)
    require(poll_gate >= 0 and query_call > poll_gate,
            "event-log query must remain gated by the elapsed-time policy")
    require(re.search(
                r"static\s+VOID\s+IAPP_EventLog_Query\s*\(\s*IAPP_IN\s+RTMP_IAPP\s*\*\s*pCtrlBK\s*\)\s*;",
                source) is not None,
            "event-log query prototype must accept the IAPP control block")
    query_handler = extract_function(source, "IAPP_EventLog_Query")
    require("IAPP_MsgProcess(pCtrlBK, IAPP_QUERY_OID_REQ" in query_handler,
            "event-log query must dispatch through its supplied control block")
    require("IAPP_EventLog_Query();" not in start_handler,
            "event-log query must not be called with a missing control block")

    require('#include "iapp_event_log_policy.h"' in source,
            "IAPP event log must include the append-size policy")
    event_start = source.rfind("static BOOLEAN IAPP_EventLogHandle(")
    event_end = source.find("} /* End of IAPP_EventLogHandle */", event_start)
    require(event_start >= 0 and event_end > event_start,
            "IAPP event-log append function is missing")
    event_body = source[event_start:event_end]
    require("FileLen = ftell(pFile);" in event_body and "FileLen < 0" in event_body,
            "IAPP event-log append must reject ftell errors")
    require("iapp_event_log_append_fits(" in event_body and
            event_body.find("iapp_event_log_append_fits(") < event_body.find("fwrite("),
            "IAPP event-log append must preflight the full batch size")
    require("if (fwrite(" in event_body and "Status = FALSE;" in event_body and
            "break;" in event_body,
            "IAPP event-log write failures must stop and propagate")
    require("if (fflush(pFile) != 0)" in event_body and
            "if (fclose(pFile) != 0)" in event_body,
            "IAPP event-log flush and close errors must propagate")
    clean_start = source.rfind("static BOOLEAN IAPP_EventLogClean(")
    clean_end = source.find("} /* End of IAPP_EventLogClean */", clean_start)
    require(clean_start >= 0 and clean_end > clean_start and
            "if (fclose(pFile) != 0)" in source[clean_start:clean_end],
            "IAPP event-log cleanup must propagate close errors")

    deadline_policy = DEADLINE_POLICY.read_text(encoding="utf-8")
    deadline_test = DEADLINE_TEST.read_text(encoding="utf-8")
    require('#include "iapp_tcp_deadline_policy.h"' in source,
            "Linux TCP receive path must include the monotonic deadline policy")
    wait_start = source.find("static INT32 IAPP_TCP_Wait(")
    wait_end = source.find("\n}\n\n/*\n * Each TCP connection", wait_start)
    require(wait_start >= 0 and wait_end > wait_start,
            "IAPP TCP wait helper is missing")
    wait_body = source[wait_start:wait_end]
    require("clock_gettime(CLOCK_MONOTONIC, &Now)" in wait_body and
            "iapp_tcp_deadline_remaining(&Now, Deadline, &Timeout)" in wait_body,
            "Linux TCP select waits must recompute the same absolute monotonic deadline")
    require("time(NULL)" in wait_body and "#else" in wait_body,
            "non-Linux TCP wait fallback must remain available")
    read_start = source.find("static INT32 IAPP_TCP_ReadFrame(")
    read_end = source.find("\n}\n\nstatic VOID IAPP_RcvHandlerTcp", read_start)
    require(read_start >= 0 and read_end > read_start,
            "IAPP TCP frame reader is missing")
    read_body = source[read_start:read_end]
    require("clock_gettime(CLOCK_MONOTONIC, &Deadline)" in read_body and
            "Deadline.tv_sec += IAPP_TCP_FRAME_TIMEOUT_SEC" in read_body,
            "Linux TCP frame reader must establish one absolute monotonic deadline")
    require(read_body.count("IAPP_TCP_Wait(Socket, &Deadline, FALSE)") == 2,
            "all TCP frame reads and overflow checks must share the same deadline")
    require("time_t Deadline = time(NULL) + IAPP_TCP_FRAME_TIMEOUT_SEC" in read_body and
            "#else" in read_body,
            "non-Linux TCP frame reader fallback must remain available")
    for token in (
        "iapp_tcp_deadline_remaining",
        "now->tv_nsec >= 1000000000L",
        "deadline->tv_nsec >= 1000000000L",
        "(nanoseconds + 999L) / 1000L",
        "remaining->tv_usec = 0",
    ):
        require(token in deadline_policy,
                "monotonic deadline conversion lost required boundary handling: " + token)
    for token in (
        "test_tcp_deadline_fractional_remaining_time",
        "test_tcp_deadline_rounds_sub_microsecond_up",
        "test_tcp_deadline_expiry_and_second_borrow",
        "test_tcp_deadline_invalid_inputs",
        "Ralink IAPP monotonic TCP deadline policy: PASS",
    ):
        require(token in deadline_test,
                "missing IAPP monotonic deadline regression: " + token)
    workflow_text = (ROOT / ".github/workflows/iappd-lifecycle.yml").read_text(encoding="utf-8")
    deadline_path = "trunk/user/ralinkiappd/src/iapp_tcp_deadline_policy.h"
    require(workflow_text.count('      - "' + deadline_path + '"') == 2,
            "both IAPP workflow path filters must include the deadline policy")

    print("Ralink IAPP descriptor, receive, and event-log source contracts: PASS")


if __name__ == "__main__":
    main()
