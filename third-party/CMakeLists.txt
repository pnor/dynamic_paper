cmake_minimum_required(VERSION 3.15)

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

# CImg
FetchContent_Declare(
  CImg
  GIT_REPOSITORY https://github.com/GreycLab/CImg.git
  GIT_TAG v.3.3.5
  CONFIGURE_COMMAND "" BUILD_COMMAND "")

FetchContent_GetProperties(CImg)
if(NOT CImg_POPULATED)
  FetchContent_Populate(CImg)
endif()

add_library(CImg INTERFACE)
set(cimg_SOURCE_DIR
    "${cimg_SOURCE_DIR}"
    PARENT_SCOPE)

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

# yaml-cpp
FetchContent_Declare(
  yaml-cpp URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.zip)
FetchContent_MakeAvailable(yaml-cpp)

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0)
FetchContent_MakeAvailable(spdlog)

# cpr
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5)
FetchContent_MakeAvailable(cpr)

# googletest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
