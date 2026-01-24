add_library(bendiff_core STATIC
  content_sources.cpp
  content_sources.h
  dir_diff.cpp
  dir_diff.h
  dir_diff_model.h
  dir_walk.cpp
  dir_walk.h
  file_compare.cpp
  file_compare.h
  file_list_rows.cpp
  file_list_rows.h
  loaded_text_file.cpp
  loaded_text_file.h
  model.cpp
  model.h
  porcelain.cpp
  porcelain.h
  repo_discovery.cpp
  repo_discovery.h
  repo_status.cpp
  repo_status.h
  process.cpp
  process.h
)

# src/core is intentionally Qt-free.
set_target_properties(bendiff_core PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)

target_include_directories(bendiff_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(bendiff_core PUBLIC cxx_std_23)

# Warnings (match other targets).
if(MSVC)
  target_compile_options(bendiff_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(bendiff_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
