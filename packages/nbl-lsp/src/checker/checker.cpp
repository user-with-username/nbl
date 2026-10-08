#include "nbl/lsp/checker/checker.h"

#include <utility>

#include "Luau/Frontend.h"

#include "nbl/lsp/fs/path_equal.h"
#include "nbl/utils/files.h"

namespace nbl::lsp {

Checker::Checker(std::string definitions, std::string definitions_path)
    : definitions_(std::move(definitions)),
      definitions_path_(std::move(definitions_path)),
      files_(sources_) {}

Checker::~Checker() = default;

bool Checker::is_definitions(const std::string &path) const {
  return path_equal(nbl::utils::canonicalize(path),
                    nbl::utils::canonicalize(definitions_path_));
}

} // namespace nbl::lsp
