add_executable(bendiff
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

# Prefer a clear, actionable error over CMake's default find_package output.
find_package(Qt6 QUIET COMPONENTS Widgets)
if(NOT Qt6_FOUND)
    message(FATAL_ERROR
        "Qt6 Widgets not found. Install Qt6 (including the Widgets module) and re-run CMake.\n"
        "If Qt is installed in a non-standard location, set Qt6_DIR or CMAKE_PREFIX_PATH."
    )
endif()

target_link_libraries(bendiff PRIVATE Qt6::Widgets)
target_link_libraries(bendiff PRIVATE bendiff_common)

# Warnings (keep reasonable; this is a UI app and will be built on multiple compilers).
if(MSVC)
    target_compile_options(bendiff PRIVATE /W4 /permissive-)
else()
    target_compile_options(bendiff PRIVATE -Wall -Wextra -Wpedantic)
endif()
