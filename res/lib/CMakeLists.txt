add_library(%project_name%)
add_library(%project_name%::%project_name% ALIAS %project_name%)

set_target_properties(%project_name% PROPERTIES LINKER_LANGUAGE CXX)
target_set_output_dirs(%project_name%)

function (add_headers TGT)
    set(headers ${ARGN})
    list(TRANSFORM headers PREPEND "include/%include_dir%/")
    set(${TGT} ${headers} PARENT_SCOPE)
endfunction ()
function (add_sources TGT)
    set(sources ${ARGN})
    list(TRANSFORM sources PREPEND "src/")
    set(${TGT} ${sources} PARENT_SCOPE)
endfunction ()

add_headers(HEADERS
    # Header files here (relative to ./include/%include_dir%/)
    "export.h"
)
add_sources(SOURCES
    # Source files here (relative to ./src/)
)

# find_package(fmt CONFIG REQUIRED)

target_sources(%project_name% PRIVATE ${HEADERS} ${SOURCES})
target_set_options(%project_name% PRIVATE)
target_set_warnings(%project_name% PRIVATE)
target_compile_features(%project_name% PRIVATE cxx_std_20)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(%project_name%
        PRIVATE %macro_namespace%_EXPORT_SHARED
        PUBLIC %macro_namespace%_BUILD_SHARED)
endif ()

target_include_directories(%project_name%
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(%project_name%
PUBLIC
# fmt::fmt
)

if (ENABLE_INSTALL)
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/%project_name%ConfigVersion.cmake"
        VERSION ${PACKAGE_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(TARGETS %project_name%
        EXPORT %project_name%Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/%project_name%Config.cmake.in"
        "${PROJECT_BINARY_DIR}/%project_name%Config.cmake"
        INSTALL_DESTINATION lib/cmake/%project_name%
    )

    export(TARGETS %project_name% NAMESPACE %project_name%:: FILE %project_name%Config.cmake)
    install(EXPORT %project_name%Targets NAMESPACE %project_name%:: DESTINATION lib/cmake/%project_name%)
    install(FILES "${PROJECT_BINARY_DIR}/%project_name%ConfigVersion.cmake"
                  "${PROJECT_BINARY_DIR}/%project_name%Config.cmake"
            DESTINATION lib/cmake/%project_name%)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/lib/include/ DESTINATION include)
endif ()
