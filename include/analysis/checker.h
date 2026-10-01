#pragma once
#include <string>
#include "Luau/Frontend.h"
#include "../utils/logs.h"

void check_script(Luau::Frontend& frontend,
                  const std::string& script,
                  Diagnostics& diagnostics);