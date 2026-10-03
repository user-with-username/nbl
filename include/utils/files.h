#pragma once
#include <string>
#include <filesystem>

std::string read_file(const std::string& file);
bool write_file(const std::string& file, const std::string& content);

inline bool is_bundle_file(const std::string& path) {
    constexpr const char* suffix = ".bundle.luau";
    constexpr size_t n = 12; // strlen(".bundle.luau")
    return path.size() >= n && path.compare(path.size() - n, n, suffix) == 0;
}

inline std::string bundle_output_path(const std::string& input) {
    std::filesystem::path p(input);
    std::filesystem::path dir = p.parent_path();
    std::string stem = p.stem().string();
    if (stem.empty())
        stem = p.filename().string();
    return (dir / (stem + ".bundle.luau")).string();
}
