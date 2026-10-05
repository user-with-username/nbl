#include "nbl/analysis/tick_checker.h"

#include <cstring>
#include <string>
#include <vector>

namespace nbl::analysis {

namespace {

bool isTickName(const Luau::AstName &name) {
  return std::strcmp(name.value, "tick") == 0;
}

bool isGlobalTick(Luau::AstExpr *expr) {
  auto *global = expr->as<Luau::AstExprGlobal>();
  return global && isTickName(global->name);
}

class TickScanner : public Luau::AstVisitor {
public:
  TickScanner(TickInfo &info, std::vector<Luau::Location> &topLevel)
      : info_(info), topLevel_(topLevel) {}

  void scan(Luau::AstStatBlock *root) { root->visit(this); }

  bool visit(Luau::AstStatBlock *block) override {
    ++depth_;
    for (Luau::AstStat *stat : block->body)
      stat->visit(this);
    --depth_;
    return false;
  }

  bool visit(Luau::AstStatFunction *fn) override {
    if (isGlobalTick(fn->name))
      recordGlobal(fn->location);
    return true;
  }

  bool visit(Luau::AstStatLocalFunction *fn) override {
    if (depth_ == 0 && isTickName(fn->name->name))
      note(fn->location, "`tick` must not be declared `local`");
    return true;
  }

  bool visit(Luau::AstStatLocal *stat) override {
    if (depth_ != 0)
      return true;
    for (size_t i = 0; i < stat->vars.size; ++i)
      if (isTickName(stat->vars.data[i]->name))
        note(stat->vars.data[i]->location,
             "`tick` must not be declared `local`");
    return true;
  }

  bool visit(Luau::AstStatAssign *assign) override {
    if (assign->vars.size != 1 || assign->values.size != 1)
      return true;
    if (!isGlobalTick(assign->vars.data[0]))
      return true;

    if (depth_ != 0) {
      note(assign->location, "`tick` must be defined at module top-level");
    } else if (!assign->values.data[0]->is<Luau::AstExprFunction>()) {
      note(assign->location, "`tick` must be a function, not a value");
    } else {
      topLevel_.push_back(assign->location);
    }
    return true;
  }

private:
  void recordGlobal(const Luau::Location &loc) {
    if (depth_ == 0)
      topLevel_.push_back(loc);
    else
      note(loc, "`tick` must be defined at module top-level");
  }

  void note(const Luau::Location &loc, const std::string &message) {
    info_.errors.push_back({loc, message});
  }

  TickInfo &info_;
  std::vector<Luau::Location> &topLevel_;
  int depth_ = -1;
};

} // namespace

TickInfo analyzeTick(Luau::AstStatBlock *root) {
  TickInfo info;
  std::vector<Luau::Location> topLevel;

  TickScanner(info, topLevel).scan(root);

  if (topLevel.empty())
    return info;

  info.has_tick = true;
  info.tick_loc = topLevel.front();

  if (topLevel.size() > 1 && info.errors.empty())
    for (size_t i = 1; i < topLevel.size(); ++i)
      info.errors.push_back({topLevel[i], "duplicate definition of `tick`"});

  return info;
}

} // namespace nbl::analysis