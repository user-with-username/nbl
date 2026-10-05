#include "args.h"

#include <cstdlib>
#include <iostream>
#include <vector>

#include "nbl/utils/files.h"

namespace nbl::cli {

namespace {

constexpr const char *kUsage =
    "usage:\n"
    "  nbl [--types <path|embedded:>] <file> [script.bundle.luau]\n"
    "  nbl update";

void print_usage_and_exit() {
  std::cout << kUsage << '\n';
  std::exit(0);
}

} // namespace

std::optional<Args> parse_args(int argc, char *argv[],
                               nbl::utils::Diagnostics &diagnostics) {
  Args args;
  std::vector<std::string> positional;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      print_usage_and_exit();
    }

    if (arg == "--types") {
      if (i + 1 >= argc) {
        diagnostics.error("--types requires a value");
        return std::nullopt;
      }
      args.types = argv[++i];
      continue;
    }

    positional.push_back(std::move(arg));
  }

  if (positional.empty()) {
    diagnostics.error(kUsage);
    return std::nullopt;
  }

  if (positional[0] == "update") {
    if (positional.size() != 1 || !args.types.empty()) {
      diagnostics.error("`update` takes no arguments");
      return std::nullopt;
    }
    args.command = Command::Update;
    return args;
  }

  if (positional.size() > 2) {
    diagnostics.error(kUsage);
    return std::nullopt;
  }

  args.command = Command::Check;
  args.script = positional[0];
  args.output = (positional.size() == 2)
                    ? positional[1]
                    : nbl::utils::bundle_output_path(args.script);
  return args;
}

} // namespace nbl::cli