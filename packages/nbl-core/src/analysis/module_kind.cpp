#include "nbl/analysis/module_kind.h"

#include <cctype>
#include <cstring>
#include <string_view>

#include "Luau/Ast.h"
#include "Luau/Parser.h"

namespace nbl::analysis {

namespace {

std::string_view first_line(std::string_view source) {
  const size_t end = source.find('\n');
  return source.substr(0, end == std::string_view::npos ? source.size() : end);
}

std::string_view first_code_line(std::string_view source) {
  size_t offset = 0;

  while (offset < source.size()) {
    const size_t end = source.find('\n', offset);
    std::string_view line = source.substr(
        offset, end == std::string_view::npos ? source.size() - offset
                                              : end - offset);
    while (!line.empty() &&
           std::isspace(static_cast<unsigned char>(line.front())))
      line.remove_prefix(1);
    while (!line.empty() &&
           std::isspace(static_cast<unsigned char>(line.back())))
      line.remove_suffix(1);

    if (!line.empty() &&
        !(line.size() >= 2 && line[0] == '-' && line[1] == '-'))
      return line;

    if (end == std::string_view::npos)
      break;
    offset = end + 1;
  }

  return {};
}

bool has_directive(std::string_view source, std::string_view directive) {
  return first_line(source).find(directive) != std::string_view::npos;
}

class TopLevelShape : public Luau::AstVisitor {
public:
  bool visit(Luau::AstStatBlock *block) override {
    for (Luau::AstStat *stat : block->body)
      stat->visit(this);
    return false;
  }

  bool visit(Luau::AstStatReturn *) override {
    has_return = true;
    return false;
  }

  bool visit(Luau::AstStatLocal *stat) override {
    for (size_t i = 0; i < stat->values.size; ++i)
      if (stat->values.data[i]->is<Luau::AstExprCall>())
        has_call = true;
    return false;
  }

  bool visit(Luau::AstStatAssign *stat) override {
    for (size_t i = 0; i < stat->values.size; ++i)
      if (stat->values.data[i]->is<Luau::AstExprCall>())
        has_call = true;
    return false;
  }

  bool visit(Luau::AstStatExpr *stat) override {
    if (stat->expr->is<Luau::AstExprCall>())
      has_call = true;
    return false;
  }

  bool visit(Luau::AstStatFunction *) override {
    has_function = true;
    return false;
  }

  bool visit(Luau::AstStatLocalFunction *) override {
    has_function = true;
    return false;
  }

  bool has_return = false;
  bool has_call = false;
  bool has_function = false;
};

} // namespace

ModuleKind classify_module(std::string_view source, bool has_requirers) {
  if (has_directive(source, "!nbl-library"))
    return ModuleKind::Library;
  if (has_directive(source, "!nbl-entrypoint"))
    return ModuleKind::Entrypoint;

  // Пусто / только комментарии — «только что созданный файл». Молчим.
  if (first_code_line(source).empty())
    return ModuleKind::Unknown;

  // Есть родители — точно библиотека.
  if (has_requirers)
    return ModuleKind::Library;

  Luau::Allocator allocator;
  Luau::AstNameTable names(allocator);
  Luau::ParseResult parsed = Luau::Parser::parse(
      source.data(), source.size(), names, allocator, {});

  if (!parsed.root)
    return ModuleKind::Unknown;

  TopLevelShape shape;
  parsed.root->visit(&shape);

  if (shape.has_return)
    return ModuleKind::Library;

  if (shape.has_call)
    return ModuleKind::Entrypoint;

  if (shape.has_function)
    return ModuleKind::Unknown;

  return ModuleKind::Unknown;
}

} // namespace nbl::analysis