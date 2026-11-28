#!/usr/bin/env python3
"""Verify IAPP descriptor, receive, and event-log persistence safety."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "trunk/user/ralinkiappd/src/rtmpiapp.c"
POLICY = ROOT / "trunk/user/ralinkiappd/src/iapp_select_fd_policy.h"
DEADLINE_POLICY = ROOT / "trunk/user/ralinkiappd/src/iapp_tcp_deadline_policy.h"
DEADLINE_TEST = ROOT / "trunk/user/ralinkiappd/tests/iapp_select_fd_policy.c"
EVENT_LOG_POLL_POLICY = ROOT / "trunk/user/ralinkiappd/src/iapp_event_log_poll_policy.h"


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
    start_handler = extract_function(source, "IAPP_Start")
    require("struct timespec now, backuptime;" in start_handler,
            "event-log interval must use monotonic timespec samples")
    require("clock_gettime(CLOCK_MONOTONIC, &backuptime)" in start_handler and
            "clock_gettime(CLOCK_MONOTONIC, &now)" in start_handler,
            "event-log timer must initialize and sample CLOCK_MONOTONIC")
    require("gettimeofday(" not in start_handler and "timersub(" not in start_handler,
            "event-log polling must not depend on adjustable wall-clock time")
    poll_gate = start_handler.find("iapp_event_log_poll_due(&now, &backuptime)")
    query_call = start_handler.find("IAPP_EventLog_Query(pCtrlBK)", poll_gate)
    update_sample = start_handler.find("backuptime = now;", poll_gate)
    require(poll_gate >= 0 and update_sample > poll_gate and query_call > update_sample,
            "event-log query must be gated by a monotonic one-second deadline")
    require("!event_log_clock_ready" in start_handler and
            "event_log_clock_ready = 1;" in start_handler,
            "event-log polling must recover after a transient monotonic-clock error")
    event_log_poll_policy = EVENT_LOG_POLL_POLICY.read_text(encoding="utf-8")
    require("iapp_event_log_poll_due(const struct timespec *now" in
            event_log_poll_policy and "now->tv_sec < 0" in event_log_poll_policy and
            "last->tv_sec < 0" in event_log_poll_policy and
            "now->tv_nsec" in event_log_poll_policy and
            "last->tv_nsec" in event_log_poll_policy,
            "event-log polling helper must validate monotonic timespec samples")
    require("msgrcv(" not in start_handler and "rtmpiapp.MsgId" not in start_handler and
            "MsgSubType msg" not in start_handler,
            "event-log loop must not read the obsolete, never-initialized 802.1X message queue")
    require(re.search(
                r"static\s+VOID\s+IAPP_EventLog_Query\s*\(\s*IAPP_IN\s+RTMP_IAPP\s*\*\s*pCtrlBK\s*\)\s*;",
                source) is not None,
            "event-log query prototype must accept the IAPP control block")
    query_handler = extract_function(source, "IAPP_EventLog_Query")
    require('#define EVENT_LOG_FILE "/etc/Wireless/RT61AP/RT61APEvt.dat"' in source,
            "event-log path must be defined when the optional logger is enabled")
    require("RT_802_11_EVENT_TABLE EventTab;" in query_handler and
            "EventTabLen = sizeof(EventTab)" in query_handler,
            "event-log query must use a fully sized event-table output buffer")
    require("IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP" in query_handler and
            "RT_QUERY_EVENT_TABLE" in query_handler,
            "event-log query must issue the real driver ioctl, not the no-op legacy message subtype")
    require("IAPP_EventLogHandle(&EventTab)" in query_handler and
            "if_idx < pCtrlBK->IfNameWlanCount" in query_handler,
            "event-log query must persist every configured radio's returned event table")
    require("pCtrlBK->MsgId" not in query_handler and "IAPP_MsgProcess" not in query_handler,
            "event-log query must not depend on the uninitialized legacy message queue")
    require("IAPP_EventLog_Query();" not in start_handler,
            "event-log query must not be called with a missing control block")

    require("iapp_event_log_table_fits" in query_handler and
            "EventTabLen < (INT32)offsetof(RT_802_11_EVENT_TABLE, Log)" in query_handler and
            "EventTabLen > (INT32)sizeof(EventTab)" in query_handler,
            "driver event-table results must be bounded by returned length and output capacity")
    event_dispatch_start = source.index("case IAPP_CMD_SECURITY_MONITOR:")
    event_dispatch_end = source.index("case IAPP_CMD_MOVE_NOTIFY:", event_dispatch_start)
    event_dispatch = source[event_dispatch_start:event_dispatch_end]
    require("memcpy(&EventTab" in event_dispatch and
            "offsetof(RT_IAPP_SECURITY_MONITOR, EvtTab)" in event_dispatch and
            "IAPP_EventLogHandle(&SM_p->EvtTab)" not in event_dispatch,
            "packed security-monitor event tables must be copied to aligned storage before access")

    write_start = source.index("static BOOLEAN IAPP_TCP_WriteFrame(")
    write_end = source.index("\n}\n", write_start) + 3
    write_handler = source[write_start:write_end]
    connect_start = source.index("static BOOLEAN IAPP_TCP_Connect(")
    connect_end = source.index("\n}\n", connect_start) + 3
    connect_handler = source[connect_start:connect_end]
    for name, handler in (("write", write_handler), ("connect", connect_handler)):
        require("struct timespec Deadline;" in handler and
                "clock_gettime(CLOCK_MONOTONIC, &Deadline)" in handler and
                "Deadline.tv_sec += IAPP_TCP_FRAME_TIMEOUT_SEC;" in handler and
                "IAPP_TCP_Wait(Socket, &Deadline, TRUE)" in handler and
                "time_t Deadline" not in handler,
                "Linux TCP " + name + " must pass one absolute monotonic timespec deadline by address")

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
