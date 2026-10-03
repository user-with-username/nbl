#pragma once
#include <string>
#include <vector>

#include "Luau/Ast.h"
#include "Luau/Location.h"

struct TickError {
    Luau::Location loc;
    std::string message;
};

struct TickInfo {
    bool has_tick = false;
    Luau::Location tick_loc{};
    std::vector<TickError> errors;
};

TickInfo analyzeTick(Luau::AstStatBlock* root);
