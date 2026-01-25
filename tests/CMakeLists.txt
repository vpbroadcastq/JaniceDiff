include(FetchContent)

# This directory (and GoogleTest) is Qt-free; suppress AUTOGEN warnings caused
# by the top-level project enabling AUTOMOC/AUTOUIC/AUTORCC for the app.
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

# GoogleTest
# - FetchContent keeps this repo dependency-free while still being cross-platform.
# - On MSVC, this avoids mismatched CRT linkage.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(bendiff_tests
  test_file_list_rows.cpp
  test_content_sources.cpp
  test_loaded_text_file.cpp
  test_render_model.cpp
  test_diff_api.cpp
  test_dir_diff_model.cpp
  test_dir_walk.cpp
  test_file_compare.cpp
  test_dir_diff.cpp
  test_smoke.cpp
  test_invocation.cpp
  test_core_model.cpp
  test_repo_discovery.cpp
  test_repo_status.cpp
  test_porcelain.cpp
  test_process.cpp
)

set_target_properties(bendiff_tests PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)

target_compile_features(bendiff_tests PRIVATE cxx_std_23)

if(MSVC)
  target_compile_options(bendiff_tests PRIVATE /W4 /permissive-)
else()
  target_compile_options(bendiff_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(bendiff_tests PRIVATE GTest::gtest_main bendiff_common bendiff_core)

include(GoogleTest)

gtest_discover_tests(bendiff_tests)
