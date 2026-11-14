#!/usr/bin/env python3
"""Tracked-source lexical inventory for process and service lifecycle usage."""
from __future__ import annotations
import argparse, collections, datetime, json, os, re, subprocess, sys
from pathlib import Path

ROOT = "trunk/user"
SYMBOLS = ("pids", "pids_main", "find_pid_by_name", "procps_scan",
           "alloc_procps_scan", "free_procps_scan", "kill_services",
           "kill_pidfile", "kill_pidfile_s")
SUFFIXES = {".c",".h",".cc",".cpp",".cxx",".hh",".hpp",".hxx",".s",".sh",
".bash",".mk",".mak",".am",".ac",".in",".lua",".pl",".py",".awk",".y",".l",
".asp",".js",".m4",".inc",".def",".tcl",".tests",".test",".testsx",".init",
".service",".conf",".rules",".ipset",".bound",".deconfig",".nak",".renew",
".loop",".rc",".script",".html",".htm",".cgi",".txt",".x",".sub",".templ",
".socket",".firewall",".up",".down",".mksuid",".mkfs"}
BASENAMES = {"Makefile","makefile","GNUmakefile","CMakeLists.txt"}
API = {s: re.compile(r"\b"+re.escape(s)+r"\s*\(") for s in SYMBOLS}
LIFE = {
"service_lifecycle": re.compile(r"\bservice_lifecycle_[A-Za-z0-9_]*\b"),
"process_discovery": re.compile(r"\b(?:pids_main|find_pid_by_name|procps_scan|pids|pidof|pgrep|pkill|killall)\b"),
"pidfile": re.compile(r"\b(?:kill_pidfile(?:_s)?|pidfile|pid_file|pidof)\b", re.I),
"signals": re.compile(r"\b(?:SIGTERM|SIGKILL|SIGHUP|SIGINT|SIGUSR1|SIGUSR2)\b|\bkill\s*\("),
"wait_reap": re.compile(r"\b(?:waitpid|wait4|waitid)\s*\(|\bwait\s*(?:\(|$)"),
"procfs": re.compile(r"/proc(?:/[0-9]+)?(?:/|\b)"),
"process_commands": re.compile(r"\b(?:pidof|pgrep|pkill|killall|start-stop-daemon|fuser)\b"),
"service_function_candidates": re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*(?:start|stop|restart|reload)[A-Za-z0-9_]*\s*\("),
}

def run_git(repo, args):
    p = subprocess.run(["git","-C",str(repo),*args],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if p.returncode: raise RuntimeError(p.stderr.decode("utf-8","replace"))
    return p.stdout

def entries(repo, rev):
    raw=run_git(repo,["ls-tree","-r","-z","--full-tree",rev,"--",ROOT])
    out=[]
    for item in raw.split(b"\0"):
        if not item: continue
        meta,path=item.split(b"\t",1)
        mode,typ,sha=meta.decode("ascii").split()
        out.append((mode,typ,sha,path.decode("utf-8","surrogateescape")))
    return out

def candidate(entry):
    mode,typ,sha,path=entry
    base=path.rsplit("/",1)[-1]
    return typ=="blob" and (Path(base).suffix.lower() in SUFFIXES or
        base in BASENAMES or base.startswith("Kconfig") or mode=="100755")

def esc_sparse(path):
    return "/"+re.sub(r"([\\*?\[\]])",r"\\\1",path)

def emit_patterns(repo,rev):
    paths={e[3] for e in entries(repo,rev) if candidate(e)}
    paths.add("scripts/audit_process_lifecycle.py")
    for path in sorted(paths): print(esc_sparse(path))

def close_paren(s,op):
    depth=0; state="code"; i=op
    while i<len(s):
        c=s[i]; n=s[i+1] if i+1<len(s) else ""
        if state=="code":
            if c=='"': state="str"
            elif c=="'": state="char"
            elif c=="/" and n=="*": state="comment"; i+=1
            elif c=="/" and n=="/": state="line"; i+=1
            elif c=="(": depth+=1
            elif c==")":
                depth-=1
                if depth==0: return i
        elif state=="str":
            if c=="\\": i+=1
            elif c=='"': state="code"
        elif state=="char":
            if c=="\\": i+=1
            elif c=="'": state="code"
        elif state=="comment" and c=="*" and n=="/":
            state="code"; i+=1
        elif state=="line" and c=="\n": state="code"
        i+=1
    return None

def role(s,m):
    op=s.find("(",m.start(),m.end()); cp=close_paren(s,op)
    if cp is None: return "unclassified"
    tail=s[cp+1:cp+96].lstrip()
    start=s.rfind("\n",0,m.start())+1
    prefix=s[start:m.start()].strip()
    if tail.startswith("{"): return "definition"
    if tail.startswith(";") and prefix and not re.search(r"\b(?:if|while|for|switch|return)\s*$",prefix):
        if re.search(r"[A-Za-z_*]\s*$",prefix) and not prefix.endswith((".","->")):
            return "prototype_or_statement"
    return "call_or_macro"

def scan(repo,rev,outdir):
    es=entries(repo,rev); blobs=[e for e in es if e[1]=="blob"]
    cand=[e for e in blobs if candidate(e)]
    api={s:[] for s in SYMBOLS}; life={k:[] for k in LIFE}
    ext=collections.Counter(); scanned=skipped_binary=symlinks=0; missing=[]; errors=[]
    for mode,typ,sha,rel in cand:
        path=repo/rel
        if not path.exists() and not path.is_symlink():
            missing.append(rel); continue
        if mode=="120000" or path.is_symlink():
            symlinks+=1
            try: text=os.readlink(path)
            except OSError as e: errors.append((rel,str(e))); continue
        else:
            try: data=path.read_bytes()
            except OSError as e: errors.append((rel,str(e))); continue
            if b"\0" in data: skipped_binary+=1; continue
            text=data.decode("utf-8","replace")
        scanned+=1; ext[Path(rel).suffix.lower() or "<no-extension>"]+=1
        lines=text.splitlines()
        for sym,pat in API.items():
            for m in pat.finditer(text):
                ln=text.count("\n",0,m.start())+1
                api[sym].append({"path":rel,"line":ln,"role":role(text,m),
                    "context":lines[ln-1].strip()[:220] if ln<=len(lines) else ""})
        for group,pat in LIFE.items():
            for m in pat.finditer(text):
                ln=text.count("\n",0,m.start())+1
                life[group].append({"path":rel,"line":ln,"token":m.group(0)[:80],
                    "context":lines[ln-1].strip()[:220] if ln<=len(lines) else ""})
    if missing: raise RuntimeError(f"{len(missing)} candidate files absent from sparse checkout: {missing[:15]}")
    if errors: raise RuntimeError(f"{len(errors)} candidate files unreadable: {errors[:10]}")
    for group in (api,life):
        for key in group: group[key].sort(key=lambda x:(x["path"],x["line"],x.get("token","")))
    tree=run_git(repo,["rev-parse",f"{rev}:{ROOT}"]).decode().strip()
    report={"schema_version":1,"repository":"Arlene206/padavan-4.4-2","revision":rev,
      "source_root":ROOT,"source_tree_sha":tree,
      "generated_at_utc":datetime.datetime.now(datetime.timezone.utc).replace(microsecond=0).isoformat(),
      "method":"tracked Git tree + exact-path sparse checkout + lexical scan",
      "limitations":["Lexical inventory only; not a compiler AST or interprocedural call graph.",
        "Dynamic commands, generated sources, preprocessor aliases and external submodules may need separate review.",
        "Binary blobs are excluded by NUL-byte detection; matches do not prove runtime reachability."],
      "coverage":{"tracked_tree_entries":len(es),"tracked_blobs":len(blobs),
        "candidate_files":len(cand),"scanned_text_files":scanned,
        "binary_blobs_skipped":skipped_binary,"symlink_link_text_scanned":symlinks,
        "missing_candidates":len(missing),"read_errors":len(errors),
        "files_with_api_hits":len({h["path"] for hits in api.values() for h in hits}),
        "scanned_suffix_counts":dict(sorted(ext.items()))},
      "api_symbols":{k:{"count":len(v),"hits":v} for k,v in api.items()},
      "lifecycle_categories":{k:{"count":len(v),"hits":v} for k,v in life.items()}}
    outdir.mkdir(parents=True,exist_ok=True)
    (outdir/"process-lifecycle-inventory.json").write_text(json.dumps(report,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    md=["# Padavan process and service lifecycle source inventory","",
      f"- Exact commit: {rev}",f"- trunk/user tree: {tree}",
      f"- Tracked entries/blobs: {len(es)}/{len(blobs)}",
      f"- Candidate/scanned text files: {len(cand)}/{scanned}",
      f"- Binary skipped: {skipped_binary}; missing/read errors: {len(missing)}/{len(errors)}","",
      "> Lexical inventory only. Manual semantic review is required before API migration.","",
      "## API occurrence summary","","| Symbol | Count | Definitions | Prototype candidates | Call/macro candidates |",
      "|---|---:|---:|---:|---:|"]
    for sym,hits in api.items():
        c=collections.Counter(x["role"] for x in hits)
        md.append(f"| {sym} | {len(hits)} | {c['definition']} | {c['prototype_or_statement']} | {c['call_or_macro']} |")
    for sym,hits in api.items():
        md.extend(["",f"## {sym} ({len(hits)})",""])
        for h in hits: md.append(f"- {h['path']}:{h['line']} [{h['role']}] {h['context']}")
    for group,hits in life.items():
        md.extend(["",f"## Lifecycle candidates: {group} ({len(hits)})",""])
        for h in hits: md.append(f"- {h['path']}:{h['line']} [{h['token']}] {h['context']}")
    md.extend(["","## Review gate","",
      "1. Trace every declaration, definition, macro and call site through callers/callees and conditional builds.",
      "2. Classify process disappearance separately from permission, I/O, allocation and directory iteration failures.",
      "3. Check PID-file ownership, stale PID/reuse, signal, wait/reap, readiness and destructive cleanup behavior.",
      "4. Do not migrate the shared API until every consumer is classified; UNKNOWN must fail closed.",
      "5. Review generated code, submodules and unusual script formats separately."])
    (outdir/"process-lifecycle-inventory.md").write_text("\n".join(md)+"\n",encoding="utf-8")
    summary=[f"revision={rev}",f"source_tree_sha={tree}",f"tracked_tree_entries={len(es)}",
      f"tracked_blobs={len(blobs)}",f"candidate_files={len(cand)}",f"scanned_text_files={scanned}",
      f"binary_blobs_skipped={skipped_binary}",f"api_occurrences={sum(map(len,api.values()))}",
      f"lifecycle_occurrences={sum(map(len,life.values()))}"]
    (outdir/"summary.txt").write_text("\n".join(summary)+"\n",encoding="utf-8")
    print("\n".join(summary))

def self_test():
    sample='extern int pids(char *x);\nint pids(char *x) { return 0; }\nvoid f(void) { if (pids("x")) kill(1,SIGTERM); }\n'
    hits=list(API["pids"].finditer(sample))
    assert [role(sample,m) for m in hits]==["prototype_or_statement","definition","call_or_macro"]
    assert candidate(("100644","blob","x","test.c"))
    assert candidate(("100755","blob","x","bin/helper"))
    assert not candidate(("100644","blob","x","archive.tar.gz"))
    assert esc_sparse("trunk/user/a b/[x].c")=="/trunk/user/a b/\\[x\\].c"
    print("scanner self-test: PASS")

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--repo-root",type=Path,default=Path.cwd())
    p.add_argument("--revision",default="HEAD")
    p.add_argument("--output-dir",type=Path,default=Path("process-lifecycle-report"))
    p.add_argument("--emit-sparse-patterns",action="store_true")
    p.add_argument("--self-test",action="store_true")
    a=p.parse_args()
    if a.self_test: self_test(); return 0
    try:
        if a.emit_sparse_patterns: emit_patterns(a.repo_root.resolve(),a.revision)
        else: scan(a.repo_root.resolve(),a.revision,a.output_dir.resolve())
    except Exception as e:
        print(f"process lifecycle inventory FAILED: {e}",file=sys.stderr); return 1
    return 0

if __name__=="__main__": raise SystemExit(main())
