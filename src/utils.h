#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <map>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace utils
{
    namespace fs = std::filesystem;

    fs::path resource_directory();

    enum class string_style { kebab, snake, macro, title };
    std::vector<std::string> split_and_normalize(std::string_view str);
    std::string apply_style(const std::vector<std::string>& splitted, string_style style);
    std::string apply_style(std::string_view str, string_style style);

    std::string read_all_text(const fs::path& path);
    std::string config_vars(std::string_view text,
        const std::map<std::string_view, std::string>& variables);

    int current_year();

    template <typename... Args>
    [[noreturn]] void error(Args&&... args) { throw std::runtime_error(fmt::format(std::forward<Args>(args)...)); }
    void system(const char* command, std::string_view hint = "");
}
