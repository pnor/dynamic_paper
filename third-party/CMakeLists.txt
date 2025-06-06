cmake_minimum_required(VERSION 3.27)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()
if(POLICY CMP0174)
  cmake_policy(SET CMP0174 NEW)
endif()

include(FetchContent)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

# argparse
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v3.0
  CONFIGURE_COMMAND "" BUILD_COMMAND "")

FetchContent_GetProperties(argparse)
if(NOT argparse_POPULATED)
  FetchContent_Populate(argparse)
endif()

add_library(argparse INTERFACE)
set(argparse_SOURCE_DIR
    "${argparse_SOURCE_DIR}"
    PARENT_SCOPE)
target_compile_options(argparse INTERFACE "-w")

# expected
FetchContent_Declare(
  expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected.git
  GIT_TAG v1.1.0
  CONFIGURE_COMMAND "" BUILD_COMMAND "")

FetchContent_GetProperties(expected)
if(NOT expected_POPULATED)
  FetchContent_Populate(expected)
endif()

add_library(expected INTERFACE)
set(expected_SOURCE_DIR
    "${expected_SOURCE_DIR}"
    PARENT_SCOPE)
target_compile_options(expected INTERFACE "-w")

# yaml-cpp
FetchContent_Declare(
  yaml-cpp URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.zip)
FetchContent_MakeAvailable(yaml-cpp)
if(${DYNAMIC_PAPER_BUILD_LIBRARIES_STATIC})
  set_target_properties(yaml-cpp PROPERTIES IMPORTED_LOCATION ${yaml-cpp_BINARY_DIR}/yaml-cpp.a)
endif()
target_compile_options(yaml-cpp PRIVATE "-w")

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.15.0)
FetchContent_MakeAvailable(spdlog)
target_compile_options(spdlog PRIVATE "-w")
if (MACOSX)
  target_compile_definitions(spdlog_header_only INTERFACE SPDLOG_FMT_EXTERNAL)
else()
  target_compile_definitions(spdlog_header_only INTERFACE SPDLOG_USE_STD_FORMAT)
endif()

# sunset
FetchContent_Declare(
  sunset
  GIT_REPOSITORY https://github.com/buelowp/sunset.git
  GIT_TAG 1.1.7
)
if(${DYNAMIC_PAPER_BUILD_LIBRARIES_STATIC})
  set(SUNSET_BUILD_SHARED FALSE)
else()
  set(SUNSET_BUILD_SHARED TRUE)
endif()
FetchContent_MakeAvailable(sunset)
set_target_properties(sunset PROPERTIES IMPORTED_LOCATION ${sunset_BINARY_DIR}/libsunset.a)
target_compile_options(sunset PRIVATE "-w")

# fmt
# Only for MacOS because Apple Clang is out of date and doesn't have a constexpr format...
if(MACOSX)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.0.2)
# TODO NOTE: using this version because at the time, spdlog didn't compile with the latest fmt
FetchContent_MakeAvailable(fmt)
endif()

# googletest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# Boost Xpressive
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.83.0)
