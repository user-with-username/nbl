#include "analysis/visitor.h"

#include <cstring>
#include <string>
#include <vector>

namespace {

bool isGlobalNamed(Luau::AstExpr *expr, const char *name) {
  auto *global = expr->as<Luau::AstExprGlobal>();
  return global && std::strcmp(global->name.value, name) == 0;
}

bool isFunctionValue(Luau::AstExpr *expr) {
  return expr->is<Luau::AstExprFunction>();
}

void note(TickInfo &info, const Luau::Location &loc,
          const std::string &message) {
  info.errors.push_back({loc, message});
}

class TickScanner : public Luau::AstVisitor {
public:
  struct Ref {
    enum Kind { TopLevelFn, LocalFn, NonFn, Nested } kind;
    Luau::Location loc;
  };

  std::vector<Ref> scan(Luau::AstStatBlock *root) {
    for (Luau::AstStat *stat : root->body) {
      depth_ = 0;
      stat->visit(this);
    }
    return std::move(refs_);
  }

  bool visit(Luau::AstStatBlock *block) override {
    ++depth_;
    for (Luau::AstStat *stat : block->body)
      stat->visit(this);
    --depth_;
    return false;
  }

  bool visit(Luau::AstStatFunction *fn) override {
    if (isGlobalNamed(fn->name, "tick")) {
      refs_.push_back(
          {depth_ == 0 ? Ref::TopLevelFn : Ref::Nested, fn->location});
    }
    return true;
  }

  bool visit(Luau::AstStatLocalFunction *fn) override {
    if (std::strcmp(fn->name->name.value, "tick") == 0)
      refs_.push_back({Ref::LocalFn, fn->location});
    return true;
  }

  bool visit(Luau::AstStatLocal *stat) override {
    for (size_t i = 0; i < stat->vars.size; ++i)
      if (std::strcmp(stat->vars.data[i]->name.value, "tick") == 0)
        refs_.push_back({Ref::LocalFn, stat->vars.data[i]->location});
    return true;
  }

  bool visit(Luau::AstStatAssign *assign) override {
    if (assign->vars.size != 1 || assign->values.size != 1)
      return true;
    if (!isGlobalNamed(assign->vars.data[0], "tick"))
      return true;

    if (!isFunctionValue(assign->values.data[0])) {
      refs_.push_back({Ref::NonFn, assign->location});
    } else {
      refs_.push_back(
          {depth_ == 0 ? Ref::TopLevelFn : Ref::Nested, assign->location});
    }
    return true;
  }

private:
  std::vector<Ref> refs_;
  int depth_ = 0;
};

} // namespace

TickInfo analyzeTick(Luau::AstStatBlock *root) {
  TickScanner scanner;
  std::vector<TickScanner::Ref> refs = scanner.scan(root);

  TickInfo info;
  std::vector<Luau::Location> top_level;
  bool had_other_errors = false;

  for (const auto &ref : refs) {
    switch (ref.kind) {
    case TickScanner::Ref::TopLevelFn:
      top_level.push_back(ref.loc);
      break;
    case TickScanner::Ref::LocalFn:
      note(info, ref.loc, "`tick` must not be declared `local`");
      had_other_errors = true;
      break;
    case TickScanner::Ref::NonFn:
      note(info, ref.loc, "`tick` must be a function, not a value");
      had_other_errors = true;
      break;
    case TickScanner::Ref::Nested:
      note(info, ref.loc, "`tick` must be defined at module top-level");
      had_other_errors = true;
      break;
    }
  }

  if (!top_level.empty()) {
    info.has_tick = true;
    info.tick_loc = top_level.front();

    if (top_level.size() > 1 && !had_other_errors)
      for (size_t i = 1; i < top_level.size(); ++i)
        note(info, top_level[i], "duplicate definition of `tick`");
  }

  return info;
}
