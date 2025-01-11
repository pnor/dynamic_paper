cmake_minimum_required(VERSION 3.27)
if(POLICY CMP0174)
  cmake_policy(SET CMP0174 NEW)
endif()
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(dynamic_paper CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(STATUS "Build Dynamic Paper")

# ====== Build Options =====================
option(DYNAMIC_PAPER_BUILD_LIBRARIES_STATIC
  "Build library dependencies as static archives to include in the executable"
  TRUE)
option(BUILD_SHARED_LIBS "Build using shared libraries" FALSE)

# ===== OS Specific ========================
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOSX TRUE)
  add_compile_definitions(dynamic_paper_use_fmt_lib)
else()
  set(LINUX TRUE) # default to Linux
  add_compile_definitions(dynamic_paper_use_std_chrono_zoned_time)
endif()

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
