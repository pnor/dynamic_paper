cmake_minimum_required(VERSION 3.27)
cmake_policy(SET CMP0174 NEW)
cmake_policy(SET CMP0167 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(dynamic_paper CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(STATUS "Build Dynamic Paper")

# ====== Compile Options for All ============
add_compile_options(-Wall -Wextra -Werror=switch -Wno-error=sign-compare -Wfatal-errors)

# Debug Compile Options
set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -O0"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for Debug build")
    add_link_options(-fsanitize=address)
endif()


# ====== Release Compile Options ============
set(CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} -O3"
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring for Release build")
endif()

# ====== Uninstall =============

if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# ==================

add_subdirectory(third-party)
add_subdirectory(src)
add_subdirectory(test)

message(STATUS "Dynamic Paper : Done!")
