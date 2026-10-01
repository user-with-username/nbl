import csv
import io

from fetch import get_asset

from utils import is_identifier, to_snake_upper

CSV_FILES = ("csv_logic/characters.csv", "csv_logic/records.csv", "csv_logic/traits.csv")

TYPE_ROW_VALUES = {"string", "boolean", "int", "float", "double",
                   "int64", "uint32", "short", "", "void"}

ENUM_COLUMN_HINTS = {
    "CharacterType": [
        ("characters.csv", "name", "disabled", {"*true"}),
    ],
    "GameMode": [
        ("records.csv", "TargetGameModes"),
    ],
    "TraitType": [
        ("traits.csv", "type"),
    ],
}

KNOWN_ENUM_CONSTANTS = {
    "AttackOrigin": [
        "ACCESSORY",
        "COMPONENT",
        "EXTERNAL",
        "GAME_MODE",
        "INCOMING_DAMAGE",
        "MODIFIER",
        "NEW_ROUND",
        "OVERCHARGE",
        "OVERCHARGE_ABILITY",
        "PASSIVE_CHARGING",
        "PASSIVE_HEALING",
        "REDIRECT_SHIELD",
        "STAR_POWER",
        "ULTI",
        "UNUSED_9",
        "UNUSED_14",
        "UNUSED_15",
        "UNUSED_16",
        "UNUSED_17",
        "WEAPON",
    ],
}


class CsvEnumResolver:
    def __init__(self):
        self.files = self._load_files()

    @staticmethod
    def _parse_csv(content):
        try:
            raw = list(csv.reader(io.StringIO(content)))
        except Exception:  # noqa: BLE001
            return {}, []
        if not raw:
            return {}, []
        header = [c.strip().strip('"') for c in raw[0]]
        data_start = 1
        if len(raw) > 1:
            second = [c.strip().strip('"').lower() for c in raw[1]]
            if second and all(c in TYPE_ROW_VALUES for c in second):
                data_start = 2

        columns = {col: [] for col in header}
        rows = []
        for raw_row in raw[data_start:]:
            row = {}
            for i, col in enumerate(header):
                if i < len(raw_row):
                    v = raw_row[i].strip().strip('"')
                    if v:
                        columns[col].append(v)
                        row[col] = v
            rows.append(row)
        return columns, rows

    def _load_files(self):
        files = []
        for file in CSV_FILES:
            content = get_asset(file)
            cols, rows = self._parse_csv(content)
            if cols:
                files.append((file, cols, rows))
        return files

    @staticmethod
    def _hint_parts(hint):
        if isinstance(hint, str):
            return None, hint, None, None
        if len(hint) == 2:
            fsub, csub = hint
            return fsub, csub, None, None
        if len(hint) == 4:
            fsub, csub, fcol, fvals = hint
            return fsub, csub, fcol, fvals
        raise ValueError(f"bad hint: {hint!r}")

    @staticmethod
    def _row_passes_filter(row, filter_col, filter_vals):
        if not filter_col:
            return True
        v = row.get(filter_col)

        banned = {x[1:] for x in filter_vals
                  if isinstance(x, str) and x.startswith("*")}
        allowed = {x for x in filter_vals
                   if not (isinstance(x, str) and x.startswith("*"))}

        if v is not None and v in banned:
            return False
        if allowed:
            return v is not None and v in allowed
        return True

    @staticmethod
    def _consts_from_column(vals):
        return {to_snake_upper(v) for v in vals if is_identifier(v)}

    def _match_enum(self, enum_name):
        for hint in ENUM_COLUMN_HINTS.get(enum_name, []):
            fsub, csub, fcol, fvals = self._hint_parts(hint)
            for fname, cols, rows in self.files:
                if fsub and fsub.lower() not in fname.lower():
                    continue
                for col, vals in cols.items():
                    if csub.lower() not in col.lower():
                        continue
                    if fcol:
                        picked = []
                        for row in rows:
                            if self._row_passes_filter(row, fcol, fvals):
                                v = row.get(col)
                                if v:
                                    picked.append(v)
                        consts = self._consts_from_column(picked)
                    else:
                        consts = self._consts_from_column(vals)
                    if consts:
                        return consts

        enum_l = enum_name.lower()
        for fname, cols, _rows in self.files:
            for col, vals in cols.items():
                if enum_l in col.lower():
                    consts = self._consts_from_column(vals)
                    if consts:
                        return consts

        return set()

    def resolve(self, enum_names):
        result = {}
        for ename in enum_names:
            merged = self._match_enum(ename)
            if not merged:
                merged = set(KNOWN_ENUM_CONSTANTS.get(ename, set()))
            if merged:
                result[ename] = merged
        return result
