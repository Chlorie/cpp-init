find_package(fmt CONFIG REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(console CONFIG REQUIRED)

add_executable(cpp-init
    "main.cpp"
    "utils.h"
    "utils.cpp")
target_include_directories(cpp-init PRIVATE ${Boost_INCLUDE_DIR})
target_set_options(cpp-init)
target_link_libraries(cpp-init PRIVATE
    project_options
    fmt::fmt
    console::console
    ${Boost_LIBRARIES}
    nlohmann_json::nlohmann_json)

add_custom_command(
    TARGET cpp-init POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res)
