#pragma once
#include <string>
#include "Luau/Frontend.h"
#include "../utils/logs.h"

struct CheckOutput {
    bool ok = false;
    std::string bundle;
};

CheckOutput check_script(Luau::Frontend& frontend,
                         const std::string& script,
                         Diagnostics& diagnostics);