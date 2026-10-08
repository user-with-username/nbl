#pragma once

#include <string>

#include "Luau/Ast.h"
#include "Luau/Location.h"

namespace nbl::lsp {

/// AST visitor that finds the innermost symbol containing a position:
/// global, local, field access, or `require(...)` target.
class SymbolFinder : public Luau::AstVisitor {
public:
  enum class Kind { None, Global, Local, Require, Field };

  struct Found {
    Kind kind = Kind::None;
    Luau::Location location;
    Luau::AstLocal *local = nullptr;
    std::string text;
  };

  explicit SymbolFinder(const Luau::Position &position);

  const Found &found() const { return found_; }

  bool visit(Luau::AstExprGlobal *expr) override;
  bool visit(Luau::AstExprLocal *expr) override;
  bool visit(Luau::AstExprIndexName *expr) override;
  bool visit(Luau::AstExprIndexExpr *expr) override;
  bool visit(Luau::AstExprCall *call) override;

private:
  void consider(const Luau::Location &location, Kind kind, std::string text,
                Luau::AstLocal *local);

  Luau::Position position_;
  Found found_;
};

} // namespace nbl::lsp
