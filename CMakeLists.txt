cmake_minimum_required(VERSION 3.24)

project(BenDiff LANGUAGES CXX)

# Help IDEs (clangd / VS Code IntelliSense) by exporting a compilation database.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json" FORCE)

option(BENDIFF_BUILD_TESTS "Build unit tests" ON)

include(CTest)
if(BENDIFF_BUILD_TESTS)
	enable_testing()
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(src/common)
add_subdirectory(src/app)

if(BENDIFF_BUILD_TESTS)
	add_subdirectory(tests)
endif()
