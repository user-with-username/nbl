#pragma once
#include <string>
#include "Luau/Frontend.h"

std::string format_location(const std::string& module, const Luau::Location& loc);

class Diagnostics
{
public:
    void error(const std::string& message);
    void warning(const std::string& message);
    void add(const Luau::CheckResult& result, const std::string& script);
    void print_summary() const;
    bool has_errors() const;

private:
    size_t error_count = 0;
    size_t warning_count = 0;
};