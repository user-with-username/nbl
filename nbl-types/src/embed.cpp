#include "nbl/types/resolve.h"

namespace nbl::types {

namespace {

constexpr unsigned char kEmbeddedGlobals[] = {
#embed "../globals.d.luau"
};

constexpr unsigned char kEmbeddedTypes[] = {
#embed "../types.d.luau"
};

static_assert(sizeof(kEmbeddedGlobals) > 0,
              "globals.d.luau was embedded as an empty file");
static_assert(sizeof(kEmbeddedTypes) > 0,
              "types.d.luau was embedded as an empty file");

} // namespace

std::string_view embedded_globals() {
  return std::string_view(reinterpret_cast<const char *>(kEmbeddedGlobals),
                          sizeof(kEmbeddedGlobals));
}

std::string_view embedded_types() {
  return std::string_view(reinterpret_cast<const char *>(kEmbeddedTypes),
                          sizeof(kEmbeddedTypes));
}

} // namespace nbl::types