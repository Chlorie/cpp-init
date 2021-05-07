cmake_minimum_required(VERSION 3.14)

project(cpp-init CXX)

set(CMAKE_SCRIPTS "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SCRIPTS})

add_library(project_options INTERFACE)
include("cmake/ProjectSettings.cmake")
target_compile_features(project_options INTERFACE cxx_std_20)
target_set_warnings(project_options)
target_set_options(project_options)

add_subdirectory(src)
