#include "nbl/lsp/fs/path_equal.h"

#include <cctype>

namespace nbl::lsp {

bool path_equal(const std::string &a, const std::string &b) {
  if (a.size() != b.size())
    return false;

  for (size_t i = 0; i < a.size(); ++i) {
    const char ca = static_cast<char>(
        std::tolower(static_cast<unsigned char>(a[i])));
    const char cb = static_cast<char>(
        std::tolower(static_cast<unsigned char>(b[i])));
    if (ca != cb)
      return false;
  }
  return true;
}

} // namespace nbl::lsp
