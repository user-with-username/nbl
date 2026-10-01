import re

from utils import find_tables, strip_md


class ReadmeParser:
    def __init__(self, text):
        self.text = text
        self.sections = self._parse_sections(text)

    @staticmethod
    def _parse_sections(text):
        sections, current = {}, "_root"
        sections.setdefault(current, [])
        for line in text.splitlines():
            m = re.match(r"^(#{1,6})\s+(.*)$", line)
            if m:
                current = m.group(2).strip().lower()
                sections.setdefault(current, [])
                continue
            sections[current].append(line)
        return sections

    def extract_helpers(self):
        out = []
        for sec_name, lines in self.sections.items():
            for t in find_tables(lines):
                if not t or len(t[0]) < 3:
                    continue
                header = [strip_md(c).lower() for c in t[0]]
                if (header[0] in ("название", "name") # noqa: SIM102
                        and header[2] in ("применение", "предназначение", "описание")):
                    if "вспомогательн" in sec_name or "helper" in sec_name:
                        for r in t[1:]:
                            if len(r) >= 2:
                                name = strip_md(r[0])
                                if name:
                                    out.append((name, r[1]))
        return out

    def extract_listener_types(self):
        out = []
        for sec_name, lines in self.sections.items():
            for t in find_tables(lines):
                if not t or len(t[0]) != 2:
                    continue
                header = [strip_md(c).lower() for c in t[0]]
                if (header[0] in ("название", "name")  # noqa: SIM102
                        and header[1] in ("тип", "type")):
                    if "подписк" in sec_name or "listener" in sec_name:
                        for r in t[1:]:
                            if len(r) >= 2:
                                name = strip_md(r[0])
                                if name:
                                    out.append((name, r[1]))
        return out

    def extract_globals(self):
        out = {}
        text = re.sub(r"\s+", " ", self.text)

        for m in re.finditer(
            r"класса\s+\[?([A-Z]\w*)\]?"
            r".{0,400}?"
            r"глобальной\s+переменной\s+\*{0,2}([a-zA-Z_]\w*)",
            text, re.IGNORECASE | re.DOTALL,
        ):
            out[m.group(2)] = m.group(1)
        return out