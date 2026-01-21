add_library(bendiff_common STATIC
    logging.cpp
    logging.h
    invocation.cpp
    invocation.h
    startup_policy.cpp
    startup_policy.h
)

# src/common is intentionally Qt-free.
set_target_properties(bendiff_common PROPERTIES
    AUTOMOC OFF
    AUTOUIC OFF
    AUTORCC OFF
)

target_include_directories(bendiff_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(bendiff_common PUBLIC cxx_std_23)

# Warnings (match the app target so common code stays clean).
if(MSVC)
    target_compile_options(bendiff_common PRIVATE /W4 /permissive-)
else()
    target_compile_options(bendiff_common PRIVATE -Wall -Wextra -Wpedantic)
endif()
