#include "nbl/lsp/definition/symbol_finder.h"

#include <cstring>
#include <utility>

namespace nbl::lsp {

namespace {

bool contains(const Luau::Location &outer, const Luau::Position &position) {
  if (position.line < outer.begin.line || position.line > outer.end.line)
    return false;
  if (position.line == outer.begin.line && position.column < outer.begin.column)
    return false;
  if (position.line == outer.end.line && position.column > outer.end.column)
    return false;
  return true;
}

} // namespace

SymbolFinder::SymbolFinder(const Luau::Position &position)
    : position_(position) {}

bool SymbolFinder::visit(Luau::AstExprGlobal *expr) {
  consider(expr->location, Kind::Global, expr->name.value, nullptr);
  return true;
}

bool SymbolFinder::visit(Luau::AstExprLocal *expr) {
  consider(expr->location, Kind::Local, expr->local->name.value, expr->local);
  return true;
}

bool SymbolFinder::visit(Luau::AstExprIndexName *expr) {
  consider(expr->location, Kind::Field, expr->index.value, nullptr);
  return true;
}

bool SymbolFinder::visit(Luau::AstExprIndexExpr *expr) {
  if (auto *str = expr->index->as<Luau::AstExprConstantString>())
    consider(expr->location, Kind::Field,
             std::string(str->value.data, str->value.size), nullptr);
  return true;
}

bool SymbolFinder::visit(Luau::AstExprCall *call) {
  auto *global = call->func->as<Luau::AstExprGlobal>();
  if (global && std::strcmp(global->name.value, "require") == 0 &&
      call->args.size == 1) {
    if (auto *str = call->args.data[0]->as<Luau::AstExprConstantString>())
      consider(str->location, Kind::Require,
               std::string(str->value.data, str->value.size), nullptr);
  }
  return true;
}

void SymbolFinder::consider(const Luau::Location &location, Kind kind,
                            std::string text, Luau::AstLocal *local) {
  if (!contains(location, position_))
    return;

  if (found_.kind != Kind::None && !contains(found_.location, location.begin))
    return;

  found_ = Found{kind, location, local, std::move(text)};
}

} // namespace nbl::lsp
