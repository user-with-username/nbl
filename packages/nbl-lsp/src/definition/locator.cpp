#include "nbl/lsp/definition/locator.h"

#include "Luau/Allocator.h"
#include "Luau/Ast.h"
#include "Luau/Parser.h"

#include "nbl/analysis/fs_resolver.h"
#include "nbl/lsp/definition/line_matchers.h"
#include "nbl/lsp/definition/symbol_finder.h"
#include "nbl/lsp/text/line_search.h"

namespace nbl::lsp {

namespace {

std::optional<Definition>
locate_global(const std::string &module, std::string_view module_text,
              std::string_view name, std::string_view definitions,
              const std::string &definitions_path) {
  if (auto declared = find_line(definitions, [&](std::string_view line) {
        return declares(line, name);
      }))
    return Definition{definitions_path, *declared};

  if (auto defined = find_line(module_text, [&](std::string_view line) {
        return defines_global(line, name);
      }))
    return Definition{module, *defined};

  return std::nullopt;
}

std::optional<Definition>
locate_field(const std::string &module, std::string_view module_text,
             std::string_view name, std::string_view definitions,
             const std::string &definitions_path) {
  const auto is_field = [&](std::string_view line) {
    return is_property(line, name);
  };

  // A field name is only trusted when it is declared exactly once.
  if (count_matches(definitions, is_field) == 1)
    if (auto property = find_line(definitions, is_field))
      return Definition{definitions_path, *property};

  if (count_matches(module_text, is_field) == 1)
    if (auto property = find_line(module_text, is_field))
      return Definition{module, *property};

  return std::nullopt;
}

} // namespace

std::optional<Definition>
locate_definition(const std::string &module, std::string_view module_text,
                  Luau::Position position, std::string_view definitions,
                  const std::string &definitions_path) {
  Luau::Allocator allocator;
  Luau::AstNameTable names(allocator);
  const Luau::ParseResult parsed = Luau::Parser::parse(
      module_text.data(), module_text.size(), names, allocator, {});

  if (parsed.root == nullptr)
    return std::nullopt;

  SymbolFinder finder(position);
  parsed.root->visit(&finder);
  const SymbolFinder::Found &found = finder.found();

  switch (found.kind) {
  case SymbolFinder::Kind::Local:
    if (found.local == nullptr)
      return std::nullopt;
    return Definition{module, found.local->location};

  case SymbolFinder::Kind::Require: {
    auto target = nbl::analysis::resolve_lua_path(module, found.text);
    if (!target)
      return std::nullopt;
    return Definition{*target, Luau::Location{}};
  }

  case SymbolFinder::Kind::Global:
    return locate_global(module, module_text, found.text, definitions,
                         definitions_path);

  case SymbolFinder::Kind::Field:
    return locate_field(module, module_text, found.text, definitions,
                        definitions_path);

  case SymbolFinder::Kind::None:
    break;
  }

  return std::nullopt;
}

} // namespace nbl::lsp
