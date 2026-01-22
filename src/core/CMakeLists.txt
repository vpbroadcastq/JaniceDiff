add_library(bendiff_core STATIC
  model.cpp
  model.h
  repo_discovery.cpp
  repo_discovery.h
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
