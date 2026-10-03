#include "utils/files.h"

#include <fstream>
#include <sstream>
#include <string>

std::string read_file(const std::string &file) {
  std::ifstream f(file, std::ios::binary);
  if (!f)
    return {};

  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

bool write_file(const std::string &file, const std::string &content) {
  std::ofstream f(file, std::ios::binary);

  if (!f)
    return false;

  f << content;
  return f.good();
}
