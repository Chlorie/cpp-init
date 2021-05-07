#include <fmt/format.h>
#include <fmt/ostream.h>
#include <console/utility.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include "utils.h"

namespace fs = std::filesystem;
namespace nl = nlohmann;

class Initializer
{
private:
    fs::path current_dir_;
    fs::path resource_dir_;
    std::vector<std::string> splitted_name_;
    std::string project_name_;
    enum class ProjectType { app, headers, lib } project_type_{};
    std::map<std::string_view, std::string> variable_map_;

    void copy_resource(const fs::path& res_path) const
    {
        copy(resource_dir_ / res_path, current_dir_ / res_path);
    }

    void copy_resource(const fs::path& res_path, const fs::path& target_path) const
    {
        copy(resource_dir_ / res_path, current_dir_ / target_path);
    }

    void config_resource(const fs::path& res_path) const
    {
        const std::string text = utils::read_all_text(resource_dir_ / res_path);
        std::ofstream ofs(current_dir_ / res_path);
        ofs << utils::config_vars(text, variable_map_);
    }

    void config_resource(const fs::path& res_path, const fs::path& target_path) const
    {
        const std::string text = utils::read_all_text(resource_dir_ / res_path);
        std::ofstream ofs(current_dir_ / target_path);
        ofs << utils::config_vars(text, variable_map_);
    }

    void setup_paths()
    {
        current_dir_ = fs::current_path();
        resource_dir_ = utils::resource_directory();
        if (!is_empty(current_dir_))
            utils::error("Current working directory is not empty");
        splitted_name_ = utils::split_and_normalize(current_dir_.stem().string());
    }

    void project_settings()
    {
        project_name_ = (variable_map_["%project_name%"] =
            cnsl::read_line("Project name", apply_style(splitted_name_, utils::string_style::kebab)));
        splitted_name_ = utils::split_and_normalize(project_name_);
        variable_map_["%version%"] =
            cnsl::read_line("Package version", "0.1.0");
        project_type_ = static_cast<ProjectType>(cnsl::selection("What type of project?",
            { "Application", "Header-only library", "Static/Shared library" }));
    }

    void universal_res() const
    {
        create_directory(current_dir_ / "cmake");
        copy_resource(".editorconfig");
        copy_resource(".gitignore");
        copy_resource("CMakeSettings_.json", "CMakeSettings.json");
        copy_resource("cmake/ProjectSettings.cmake");
    }

    void setup_vcpkg()
    {
        const size_t vcpkg = cnsl::selection("Set up vcpkg settings?",
            { "Yes", "Yes, and also add vcpkg_configuration.json", "No" });
        if (vcpkg != 2)
        {
            const std::string vcpkg_name =
                cnsl::read_line("vcpkg package name", apply_style(splitted_name_, utils::string_style::kebab));
            std::ofstream ofs("vcpkg.json");
            ofs << nl::json{
                { "name", vcpkg_name },
                { "version-string", variable_map_["%version%"] },
                { "dependencies", nl::json::array_t{} }
            }.dump(4);
        }
        if (vcpkg == 1)
        {
            std::ofstream ofs("vcpkg_configuration.json");
            ofs << nl::json{
                { "registries", nl::json::array_t{} }
            }.dump(4);
        }
    }

    void initialize_app() const
    {
        config_resource("CMakeLists.txt");
        create_directory(current_dir_ / "src");
        config_resource("src/CMakeLists.txt");
        copy_resource("src/main.cpp");
        cnsl::success_message("Created CMake project for the app");
    }

    void initialize_lib()
    {
        config_resource("CMakeListsLibrary.txt", "CMakeLists.txt");
        create_directory(current_dir_ / "examples");
        config_resource("examples/CMakeLists.txt");
        copy_resource("examples/playground.cpp");
        cnsl::success_message("Created CMake directory for examples");
        const auto& include_dir_str = (variable_map_["%include_dir%"] =
            cnsl::read_line("Include directory name", apply_style(splitted_name_, utils::string_style::snake)));
        const fs::path include_dir = "lib/include/" + include_dir_str;
        create_directories(current_dir_ / include_dir);
        if (project_type_ == ProjectType::headers)
            config_resource("lib/CMakeListsHeaderOnly.txt", "lib/CMakeLists.txt");
        else // Linked library
        {
            variable_map_["%macro_namespace%"] =
                cnsl::read_line("Preprocessor macro namespace", apply_style(splitted_name_, utils::string_style::macro));
            config_resource("lib/include/dir/export.h", include_dir / "export.h");
            config_resource("lib/CMakeLists.txt");
            create_directory(current_dir_ / "lib/src");
        }
        config_resource("cmake/libConfig.cmake.in", fmt::format("cmake/{}Config.cmake.in", project_name_));
        cnsl::success_message("Created CMake project for the library");
    }

    void add_docs()
    {
        if (!cnsl::boolean("Add Doxygen & Sphinx based docs support?")) return;
        copy_resource("cmake/FindSphinx.cmake");
        std::ofstream ofs(current_dir_ / "CMakeLists.txt", std::ios::app);
        ofs << R"(
option(BUILD_DOCS "Build documentation using Doxygen & Sphinx" OFF)
if (BUILD_DOCS)
    add_subdirectory(docs)
endif ()
)";
        create_directories(current_dir_ / "docs/custom");
        copy_resource("docs/custom/custom.css");
        variable_map_["%project_title%"] =
            cnsl::read_line("Project title", apply_style(splitted_name_, utils::string_style::title));
        const auto& author = (variable_map_["%author%"] =
            cnsl::read_line("Author of the library/documentation"));
        variable_map_["%doc_copyright%"] =
            cnsl::read_line("Docs copyright", fmt::format("{}, {}", utils::current_year(), author));
        variable_map_["%cpp_namespace%"] =
            cnsl::read_line("C++ namespace of the project", apply_style(splitted_name_, utils::string_style::snake));
        config_resource("docs/CMakeLists.txt");
        config_resource("docs/conf.py");
        config_resource("docs/Doxyfile.in");
        config_resource("docs/index.rst");
    }

    void add_readme()
    {
        if (!cnsl::boolean("Add a readme file")) return;
        const auto& title = [this]
        {
            if (const auto iter = variable_map_.find("%project_title%");
                iter != variable_map_.end())
                return iter->second;
            else
                return (variable_map_["%project_title%"] =
                    cnsl::read_line("Project title", apply_style(splitted_name_, utils::string_style::title)));
        }();
        std::ofstream ofs("README.md");
        ofs << fmt::format("# {}\n", title);
    }

    void git()
    {
        const size_t git_sel = cnsl::selection("Set up git repo?",
            { "Initialize and make an initial commit", "Just initialize", "Skip" });
        if (git_sel == 2) return;
        utils::system("git init", "make sure you have git properly installed");
        if (git_sel == 1) return;
        utils::system("git add -A");
        utils::system("git commit -m \"Initialized project\"");
    }

public:
    void run()
    {
        setup_paths();
        project_settings();
        universal_res();
        setup_vcpkg();
        if (project_type_ == ProjectType::app)
            initialize_app();
        else
        {
            initialize_lib();
            add_docs();
        }
        add_readme();
        git();
        cnsl::success_message("Finished project initialization");
    }
};

int main()
{
    try
    {
        Initializer init;
        init.run();
    }
    catch (const std::exception& exc) { cnsl::error_message(exc.what()); }
}
