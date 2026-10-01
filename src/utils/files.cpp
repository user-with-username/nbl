#include "utils/files.h"

#include <fstream>
#include <sstream>

std::string read_file(const std::string& file)
{
    std::ifstream f(file, std::ios::binary);
    if (!f)
        return {};

    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}