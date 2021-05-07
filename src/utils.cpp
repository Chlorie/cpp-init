#define _CRT_SECURE_NO_WARNINGS

#include "utils.h"

#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>
#include <ctime>
#include <vector>
#include <ranges>
#include <fstream>

namespace utils
{
    namespace alg = boost::algorithm;
    namespace bp = boost::process;
    namespace sr = std::ranges;

    namespace
    {
        std::string insert_underscores_before_cap(const std::string_view str)
        {
            const auto is_upper_case = [](const char ch) { return ch >= 'A' && ch <= 'Z'; };
            std::string result;
            auto iter = str.begin();
            const auto end = str.end();
            while (iter != end)
            {
                const auto next_cap = sr::find_if(iter + 1, end, is_upper_case);
                result.append(iter, next_cap);
                if (next_cap != end) result += '_';
                iter = next_cap;
            }
            return result;
        }

        std::string make_macro_case(std::vector<std::string> splitted)
        {
            for (auto& str : splitted) alg::to_upper(str);
            return alg::join(splitted, "_");
        }

        std::string make_title_case(std::vector<std::string> splitted)
        {
            for (auto& str : splitted) str[0] = static_cast<char>(std::toupper(str[0]));
            return alg::join(splitted, " ");
        }
    }

    fs::path resource_directory()
    {
        const fs::path program_location = boost::dll::program_location().native();
        return program_location.parent_path() / "res";
    }

    std::vector<std::string> split_and_normalize(const std::string_view str)
    {
        const auto allowed = [](const char ch)
        {
            return
                (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') ||
                (ch >= '0' && ch <= '9') ||
                ch == '-' || ch == '_';
        };
        const auto is_lower_case = [](const char ch) { return ch >= 'a' && ch <= 'z'; };
        std::string filtered;
        sr::copy_if(str, std::back_inserter(filtered), allowed);
        if (!sr::none_of(filtered, is_lower_case)) // input is not MACRO_CASE
            filtered = insert_underscores_before_cap(filtered);
        std::vector<std::string> splitted;
        alg::split(splitted, filtered, [](const char ch) { return ch == '_' || ch == '-'; });
        std::erase_if(splitted, [](auto&& s) { return s.empty(); });
        return splitted;
    }

    std::string apply_style(const std::vector<std::string>& splitted, const string_style style)
    {
        switch (style)
        {
            case string_style::kebab: return alg::join(splitted, "-");
            case string_style::snake: return alg::join(splitted, "_");
            case string_style::macro: return make_macro_case(splitted);
            case string_style::title: return make_title_case(splitted);
            default: std::terminate();
        }
    }

    std::string apply_style(const std::string_view str, const string_style style)
    {
        return apply_style(split_and_normalize(str), style);
    }

    std::string read_all_text(const fs::path& path)
    {
        std::ifstream ifs(path);
        ifs.ignore(std::numeric_limits<std::streamsize>::max());
        const std::streamsize size = ifs.gcount();
        std::string result(size, '\0');
        ifs.seekg(0, std::ios::beg);
        ifs.read(result.data(), size);
        if (ifs.fail() && !ifs.eof())
            throw std::runtime_error("Failed to read text file: " + path.string());
        return result;
    }

    std::string config_vars(const std::string_view text, const std::map<std::string_view, std::string>& variables)
    {
        std::string result(text);
        for (const auto& [key, value] : variables)
            alg::replace_all(result, key, value);
        return result;
    }

    int current_year()
    {
        // TODO: use C++20 calendar when the STL supports it
        const std::time_t time = std::time(nullptr);
        return std::localtime(&time)->tm_year + 1900;
    }

    void system(const char* command, const std::string_view hint)
    {
        bp::child child(command, bp::std_out > bp::null);
        child.wait();
        if (const int code = child.exit_code())
        {
            if (hint.empty())
                error("[{}] exited with code {}", command, code);
            else
                error("[{}] exited with code {}, {}", command, code, hint);
        }
    }
}
