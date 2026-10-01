import re

from utils import split_args, strip_md


class LuauGenerator:
    READONLY_RE = re.compile(r"<sup>\s*\(?readonly\)?\s*</sup>", re.IGNORECASE)
    NIL_RE = re.compile(r"\bnil\b", re.IGNORECASE)
    LIST_RE = re.compile(r"список\s+объектов\s+(?:класса\s+)?([A-Z][A-Za-z0-9_]*)")
    LISTENER_HINT_RE = re.compile(r"Использует\s+класс\s+([A-Z][A-Za-z0-9_]*)", re.IGNORECASE)

    def __init__(self, classes, helpers, listeners, globals_, enum_constants):
        self.classes = classes
        self.helpers = helpers
        self.listeners = listeners
        self.globals_ = globals_
        self.enum_constants = enum_constants

        self.enum_names = []
        self.abstract_names = []
        self.children = {}
        self.aliases = {}
        self._built = set()

    @staticmethod
    def to_luau_type(t):
        t = t.strip()
        if t in ("void", ""):
            return "()"
        if t in ("Object", "JavaObject"):
            return "any"
        if t == "function":
            return "(...any) -> ()"
        return t

    @classmethod
    def parse_sig(cls, raw):
        s = strip_md(raw)
        m = re.match(r"^\s*\((.*?)\)\s*(?:->\s*(.*))?$", s, re.DOTALL)
        if not m:
            return [], s.strip() or "()"
        args_str, ret = m.group(1).strip(), (m.group(2) or "").strip() or "()"
        args = []
        if args_str:
            for idx, p in enumerate(split_args(args_str)):
                if ":" in p:
                    n, t = p.split(":", 1)
                    args.append((n.strip(), t.strip()))
                else:
                    args.append((f"arg{idx}", p.strip()))
        return args, ret

    def _render_params(self, args):
        parts = []
        for name, typ in args:
            typ = self.to_luau_type(typ)
            if typ.endswith("..."):
                base = typ[:-3].strip()
                parts.append(f"...: {self._subst(base)}")
            else:
                parts.append(f"{name}: {self._subst(typ)}")
        return ", ".join(parts)

    @staticmethod
    def _fill_iter(t, hint):
        if hint and t in ("Iterable", "List"):
            return f"{t}<{hint}>"
        return t

    def _subst(self, t):
        for name in sorted(self.aliases, key=len, reverse=True):
            t = re.sub(rf"\b{re.escape(name)}\b", "Any" + name, t)
        return t

    def _annotate(self):
        for c in self.classes.values():
            for f in c["fields"]:
                f["readonly"] = bool(self.READONLY_RE.search(f["type"]))
                desc = strip_md(f["desc"])
                f["nullable"] = bool(self.NIL_RE.search(desc))
                mm = self.LIST_RE.search(desc) or self.LISTENER_HINT_RE.search(desc)
                f["list_hint"] = mm.group(1) if mm else None
            for m in c["methods"]:
                desc = strip_md(m["desc"])
                m["nullable"] = bool(self.NIL_RE.search(desc))
                mm = self.LIST_RE.search(desc) or self.LISTENER_HINT_RE.search(desc)
                m["list_hint"] = mm.group(1) if mm else None

    def _build_alias(self, name):
        if name in self._built:
            return
        for ch in self.children.get(name, []):
            if ch in self.abstract_names:
                self._build_alias(ch)
        members = []
        for ch in self.children.get(name, []):
            if ch in self.abstract_names and ch in self.aliases:
                members.append("Any" + ch)
            else:
                members.append(ch)
        if members:
            self.aliases[name] = members
        self._built.add(name)

    def _resolved_members(self, name, _seen=None):
        _seen = _seen or set()
        if name in _seen:
            return [], []
        _seen = _seen | {name}
        parent = self.classes[name]["inherits"]
        base_f, base_m = [], []
        if parent and parent in self.classes:
            base_f, base_m = self._resolved_members(parent, _seen)
        own_fn = {f["name"] for f in self.classes[name]["fields"]}
        own_mn = {m["name"] for m in self.classes[name]["methods"]}
        fields = [f for f in base_f if f["name"] not in own_fn] + self.classes[name]["fields"]
        methods = [m for m in base_m if m["name"] not in own_mn] + self.classes[name]["methods"]
        return fields, methods

    def _field_type(self, f):
        t = self.to_luau_type(strip_md(f["type"]))
        t = self._fill_iter(t, f.get("list_hint"))
        t = self._subst(t)
        if f["nullable"] and t != "()" and not t.endswith("?"):
            t += "?"
        return t

    def _method_type(self, m, self_cls):
        args, ret = self.parse_sig(m["type"])
        ret = self._fill_iter(self.to_luau_type(ret), m.get("list_hint"))
        ret = self._subst(ret)
        if m["nullable"] and ret != "()" and not ret.endswith("?"):
            ret += "?"
        params = self._render_params(args)
        head = f"self: {self_cls}"
        allp = f"{head}, {params}" if params else head
        return f"({allp}) -> {ret}"

    def generate(self):
        self._annotate()

        self.enum_names = [n for n, c in self.classes.items()
                            if "перечислен" in c["desc"].lower()]
        self.abstract_names = [n for n, c in self.classes.items()
                                if "абстрактный" in c["desc"].lower()]

        for n, c in self.classes.items():
            if c["inherits"] and c["inherits"] in self.classes:
                self.children.setdefault(c["inherits"], []).append(n)

        for n in self.abstract_names:
            self._build_alias(n)

        out = []
        w = out.append

        w("--!strict")
        w("-- Auto-generated. Do not edit by paws")
        w("")
        w("type Iterable<T = any> = { T }")
        w("type List<T = any> = { T }")
        w("type JavaObject = any")
        w("")

        for name in self.abstract_names:
            if name in self.aliases:
                w(f"type Any{name} =")
                for m in self.aliases[name]:
                    w(f"    | {m}")
                w("")

        listener_names = []
        for lname, lsig in self.listeners:
            args, ret = self.parse_sig(lsig)
            ret = self._subst(self.to_luau_type(ret))
            params = self._render_params(args)
            w(f"type {lname} = ({params}) -> {ret}")
            listener_names.append(lname)
        if self.listeners:
            w("")

        # This Luau revision does not support `declare class`.
        # Emit `type X = { ... }` + `declare X: X` instead.
        for name in self.enum_names:
            consts = sorted(self.enum_constants.get(name, []))
            if consts:
                w(f"type {name} = {{")
                for c in consts:
                    w(f"    {c}: {name},")
                w("}")
                w(f"declare {name}: {name}")
            else:
                w(f"-- no {name}.CONSTANT found in CSV")
                w(f"type {name} = any")
                w(f"declare {name}: {name}")
            w("")

        for name in self.classes:
            if name in self.enum_names:
                continue
            fields, methods = self._resolved_members(name)
            if not fields and not methods:
                w(f"type {name} = {{}}")
                w(f"declare {name}: {name}")
            else:
                w(f"type {name} = {{")
                for f in fields:
                    w(f"    {f['name']}: {self._field_type(f)},")
                for m in methods:
                    w(f"    {m['name']}: {self._method_type(m, name)},")
                w("}")
                w(f"declare {name}: {name}")
            w("")

        for gname, gclass in self.globals_.items():
            w(f"declare {gname}: {gclass}")
        if self.globals_:
            w("")

        # Luau definition files do not support overloads.
        callback_handled = False
        for hname, hsig in self.helpers:
            if hname == "createCallback" and listener_names:
                if not callback_handled:
                    singles = " | ".join(f'"{n}"' for n in listener_names)
                    union = " | ".join(listener_names)
                    w(f"declare function createCallback("
                      f"class: {singles}, callback: {union}"
                      f"): JavaObject")
                    callback_handled = True
                continue
            args, ret = self.parse_sig(hsig)
            ret = self._subst(self.to_luau_type(ret))
            params = self._render_params(args)
            w(f"declare function {hname}({params}): {ret}")

        w("")
        w("declare json: {")
        w("    encode: (value: any, pretty: boolean?) -> string,")
        w("    decode: (str: string) -> any,")
        w("    null: any,")
        w("}")

        return "\n".join(out) + "\n"