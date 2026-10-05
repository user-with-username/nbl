from __future__ import annotations

import sys
import urllib.error
import urllib.request
from pathlib import Path

REPO = "nulls-mods-community/scripting-docs"
FILES = ("globals.d.luau", "types.d.luau")
TIMEOUT = 30
REF = "main"
OUT_DIR = Path("../nbl-types")


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


def looks_like_html(data: bytes) -> bool:
    head = data.lstrip()[:64].lower()
    return head.startswith((b"<!doctype html", b"<html"))


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    for name in FILES:
        url = raw_url(REF, name)
        dest = OUT_DIR / name

        try:
            data = fetch(url)
        except (urllib.error.URLError, urllib.error.HTTPError, TimeoutError) as e:
            raise SystemExit(f"failed to download {url}: {e}")

        if not data:
            raise SystemExit(f"empty response for {url}")
        if looks_like_html(data):
            raise SystemExit(f"{url} returned HTML, not raw file")

        if dest.exists() and dest.read_bytes() == data:
            continue

        tmp = dest.with_suffix(dest.suffix + ".tmp")
        tmp.write_bytes(data)
        tmp.replace(dest)

    return 0


sys.exit(main())