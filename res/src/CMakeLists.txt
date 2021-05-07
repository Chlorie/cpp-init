# find_package(fmt CONFIG REQUIRED)

add_executable(%project_name%)

target_sources(%project_name% PRIVATE
    "main.cpp")

target_set_output_dirs(%project_name%)
target_link_libraries(%project_name% PRIVATE
    project_options)
