cmake_minimum_required(VERSION 3.27)
cmake_policy(SET CMP0167 NEW)
cmake_policy(SET CMP0169 OLD)
cmake_policy(SET CMP0174 NEW)

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

# CImg
# FetchContent_Declare(
#   CImg
#   GIT_REPOSITORY https://github.com/GreycLab/CImg.git
#   GIT_TAG v.3.3.5
#   CONFIGURE_COMMAND "" BUILD_COMMAND "")
#
# FetchContent_GetProperties(CImg)
# if(NOT CImg_POPULATED)
#   FetchContent_Populate(CImg)
# endif()

# add_library(CImg INTERFACE)
# set(cimg_SOURCE_DIR
#     "${cimg_SOURCE_DIR}"
#     PARENT_SCOPE)
# target_compile_options(cimg PRIVATE "-w")
# TODO replace CImg with ImageMagick

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
target_compile_options(yaml-cpp PRIVATE "-w")

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0)
FetchContent_MakeAvailable(spdlog)
target_compile_options(spdlog PRIVATE "-w")

# cpr
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5)
FetchContent_MakeAvailable(cpr)
target_compile_options(cpr PRIVATE "-w")

# sunset
FetchContent_Declare(
  sunset
  GIT_REPOSITORY https://github.com/buelowp/sunset.git
  GIT_TAG 1.1.7)
FetchContent_MakeAvailable(sunset)
target_compile_options(sunset PRIVATE "-w")

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
