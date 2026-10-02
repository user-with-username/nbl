import re


def strip_md(s):
    s = s.replace("⇒", "->")
    s = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", s)
    s = re.sub(r"<br\s*/?>", " ", s, flags=re.IGNORECASE)
    s = re.sub(r"<sup>.*?</sup>", "", s, flags=re.IGNORECASE | re.DOTALL)
    s = re.sub(r"<[^>]+>", "", s)
    return s.replace("&nbsp;", " ").replace("`", "").strip()


def split_args(s):
    depth, cur, parts = 0, "", []
    for ch in s:
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur.strip()); cur = ""
        else:
            cur += ch
    if cur.strip():
        parts.append(cur.strip())
    return parts


def split_row(line):
    line = line.rstrip()
    if not (line.startswith("|") and line.endswith("|")):
        return None
    cells, cur, depth = [], "", 0
    for ch in line[1:-1]:
        if ch == "|" and depth == 0:
            cells.append(cur); cur = ""
        else:
            if ch in "([{":
                depth += 1
            elif ch in ")]}":
                depth -= 1
            cur += ch
    cells.append(cur)
    return [c.strip() for c in cells]


def is_separator(cells):
    return all(c and set(c.strip()) <= set("-: ") for c in cells)


def find_tables(lines):
    tables, cur = [], []
    for line in lines:
        cells = split_row(line)
        if cells is None:
            if cur:
                tables.append(cur); cur = []
            continue
        if is_separator(cells):
            continue
        cur.append(cells)
    if cur:
        tables.append(cur)
    return tables


def to_snake_upper(s):
    s = re.sub(r'(?<=[a-z0-9])(?=[A-Z])', '_', s)
    s = re.sub(r'(?<=[A-Z])(?=[A-Z][a-z])', '_', s)
    s = re.sub(r'(?<=[A-Za-z])(?=[0-9])', '_', s)
    s = re.sub(r'(?<=[0-9])(?=[A-Za-z])', '_', s)
    return s.upper()


IDENT_RE = re.compile(r"^[A-Z][A-Za-z0-9_]*$")


def is_identifier(v):
    return bool(IDENT_RE.match(v))
