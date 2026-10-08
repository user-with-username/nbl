#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>

#include "Luau/Location.h"

namespace nbl::lsp {

/// First line matching `match`, as a zero-length location at column 0.
std::optional<Luau::Location> find_line(
    std::string_view text,
    const std::function<bool(std::string_view)> &match);

/// How many lines match `match`.
size_t count_matches(std::string_view text,
                     const std::function<bool(std::string_view)> &match);

} // namespace nbl::lsp
