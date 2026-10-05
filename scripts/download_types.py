from __future__ import annotations

import sys
import urllib.error
import urllib.request
from pathlib import Path

REPO = "nulls-mods-community/scripting-docs"
FILES = ("globals.d.luau", "types.d.luau")
TIMEOUT = 30
REF = "main"
OUT_DIR = Path(__file__).resolve().parent.parent / "packages" / "nbl-types"


def raw_url(ref: str, name: str) -> str:
    return f"https://raw.githubusercontent.com/{REPO}/{ref}/{name}"


def fetch(url: str) -> bytes:
    req = urllib.request.Request(
        url,
        headers={
            "User-Agent": "nbl-fetch-docs/1.0",
            "Accept": "text/plain, */*",
        },
    )
    with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
        if resp.status != 200:
            raise urllib.error.HTTPError(
                url, resp.status, resp.reason, resp.headers, None
            )
        return resp.read()

def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    for name in FILES:
        url = raw_url(REF, name)
        dest = OUT_DIR / name

        data = fetch(url)

        if dest.exists() and dest.read_bytes() == data:
            continue

        tmp = dest.with_suffix(dest.suffix + ".tmp")
        tmp.write_bytes(data)
        tmp.replace(dest)

    return 0


sys.exit(main())