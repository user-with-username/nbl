from .csv_enums import CsvEnumResolver
from .fetch import fetch
from .luau_generator import LuauGenerator
from .readme_parser import ReadmeParser
from .reference_parser import ReferenceParser

README_URL = (
    "https://raw.githubusercontent.com/"
    "nulls-mods-community/scripting-docs/main/readme.md"
)
REFERENCE_URL = (
    "https://raw.githubusercontent.com/"
    "nulls-mods-community/scripting-docs/main/reference.md"
)

OUTPUT_FILE = "types.d.luau"


def main():
    readme_text = fetch(README_URL)
    reference_text = fetch(REFERENCE_URL)

    classes = ReferenceParser().parse(reference_text)

    readme = ReadmeParser(readme_text)
    helpers = readme.extract_helpers()
    listeners = readme.extract_listener_types()
    globals_ = readme.extract_globals()

    enum_names = {n for n, c in classes.items()
                  if "перечислен" in c["desc"].lower()}

    enum_constants = CsvEnumResolver().resolve(enum_names)

    result = LuauGenerator(
        classes, helpers, listeners, globals_, enum_constants
    ).generate()

    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write(result)

    print(f"wrote {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
