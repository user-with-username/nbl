#pragma once

#include <iosfwd>
#include <string>

#include "Luau/Location.h"

namespace Luau {
struct LoadDefinitionFileResult;
}

namespace nbl::utils {

struct Diagnostics {
  Diagnostics();
  explicit Diagnostics(std::ostream &out);

  void error(const std::string &message);
  void warning(const std::string &message);
  void info(const std::string &message);

  void add_definition(const Luau::LoadDefinitionFileResult &result,
                      const std::string &types_name);

  void print_summary() const;
  bool has_errors() const;

private:
  std::ostream &out_;
  int error_count = 0;
  int warning_count = 0;
};

std::string format_location(const std::string &module_name,
                            const Luau::Location &location);

} // namespace nbl::utils