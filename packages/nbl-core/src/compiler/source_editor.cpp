#include "nbl/compiler/source_editor.h"

#include <algorithm>
#include <stdexcept>

namespace nbl::compiler {

SourceEditor::SourceEditor(const std::string &src) : src_(src) {
  offsets_.push_back(0);
  for (size_t i = 0; i < src.size(); ++i)
    if (src[i] == '\n')
      offsets_.push_back(i + 1);
}

void SourceEditor::replace(const Luau::Location &loc, const std::string &with) {
  edits_.push_back({offsetOf(loc.begin), offsetOf(loc.end), with});
}

std::string SourceEditor::apply() const {
  std::vector<Edit> sorted = edits_;
  std::sort(sorted.begin(), sorted.end(),
            [](const Edit &a, const Edit &b) { return a.begin > b.begin; });

  for (size_t i = 1; i < sorted.size(); ++i)
    if (sorted[i].end > sorted[i - 1].begin)
      throw std::logic_error("SourceEditor: overlapping edits");

  std::string out = src_;
  for (const auto &e : sorted)
    out.replace(e.begin, e.end - e.begin, e.with);
  return out;
}

size_t SourceEditor::offsetOf(const Luau::Position &pos) const {
  if (pos.line >= offsets_.size())
    return src_.size();
  return offsets_[pos.line] + pos.column;
}

} // namespace nbl::compiler