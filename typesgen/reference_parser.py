import re

from utils import is_separator, split_row, strip_md


class ReferenceParser:
    CLASS_RE = re.compile(r"^#\s+([A-Z][A-Za-z0-9_]*)\s*$")
    SUBSECTION_RE = re.compile(r"^###\s+(.*?)\s*:?\s*$")
    INHERIT_RE = re.compile(r"Наследуется\s+от\s+\[?([A-Z][A-Za-z0-9_]*)\]?", re.IGNORECASE)

    def parse(self, text):
        classes, current, section, desc_buf = {}, None, None, []

        def flush():
            if current and not classes[current]["desc_done"]:
                classes[current]["desc"] = " ".join(desc_buf).strip()
                classes[current]["desc_done"] = True
            desc_buf.clear()

        for raw in text.splitlines():
            line = raw.rstrip()

            m = self.CLASS_RE.match(line)
            if m:
                flush()
                current = m.group(1)
                classes[current] = {
                    "desc": "", "fields": [], "methods": [],
                    "inherits": None, "desc_done": False,
                }
                section = None
                continue

            m = self.SUBSECTION_RE.match(line)
            if m:
                flush()
                sub = m.group(1).strip().lower()
                section = ("fields" if ("поля" in sub or "fields" in sub)
                           else "methods" if ("методы" in sub or "methods" in sub)
                           else None)
                continue

            if line.startswith("#"):
                flush()
                current, section = None, None
                continue

            if current and section is None and line.strip():
                plain = strip_md(line)
                inh = self.INHERIT_RE.search(plain)
                if inh:
                    classes[current]["inherits"] = inh.group(1)
                if not classes[current]["desc_done"]:
                    desc_buf.append(plain)
                continue

            if current and section and line.startswith("|"):
                cells = split_row(line)
                if not cells or len(cells) < 2 or is_separator(cells):
                    continue
                name = strip_md(cells[0])
                if not name or name.lower() in ("название", "name"):
                    continue
                classes[current][section].append({
                    "name": name,
                    "type": cells[1],
                    "desc": cells[2] if len(cells) > 2 else "",
                })

        flush()
        return classes
