#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Luau/Location.h"

namespace compiler {

class SourceEditor {
public:
    explicit SourceEditor(const std::string& src);

    void replace(const Luau::Location& loc, const std::string& with);

    std::string apply() const;

private:
    struct Edit {
        size_t begin, end;
        std::string with;
    };

    size_t offsetOf(const Luau::Position& pos) const;

    const std::string& src_;
    std::vector<size_t> offsets_;
    std::vector<Edit> edits_;
};

} // namespace compiler