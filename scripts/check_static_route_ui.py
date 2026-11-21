#!/usr/bin/env python3
"""Regression check for the gateway static-route WebUI capacity boundary."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
PAGE = ROOT / "trunk/user/www/n56u_ribbon_fixed/Advanced_GWStaticRoute_Content.asp"


def main():
    source = PAGE.read_text(encoding="utf-8")
    add_guard = re.search(
        r"document\\.form\\.sr_num_x_0\\.value\\s*(>=|>)\\s*c",
        source,
    )
    assert add_guard, "static-route add guard was not found"
    assert add_guard.group(1) == ">=", (
        "adding a route when 64 entries already exist must be rejected"
    )

    assert re.search(
        r"GWStatic_markGroup\\(this,\\s*64,\\s*' Add '\\)",
        source,
    ), "static-route Add button must keep the 64-entry limit"
    assert re.search(
        r"GWStatic_markGroup\\(this,\\s*64,\\s*' Del '\\)",
        source,
    ), "static-route Delete button must keep the same 64-entry limit"


if __name__ == "__main__":
    main()
    print("Static-route WebUI capacity boundary: OK")
