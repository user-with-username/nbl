#pragma once

#include <string>

namespace CLI {
class App;
}

namespace nbl::cli {

enum class Command { Lint, Run, Update };

struct Args {
  Command command = Command::Lint;
  std::string script;
  std::string output;
  std::string types;
};

void configure_cli(CLI::App &app, Args &args);

} // namespace nbl::cli
