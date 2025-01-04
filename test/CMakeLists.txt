cmake_minimum_required(VERSION 3.27)
if(POLICY CMP0174)
    cmake_policy(SET CMP0174 NEW)
endif()
cmake_policy(SET CMP0167 NEW)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

# ===== OS check ==================
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOSX TRUE)
else()
  set(LINUX TRUE) # default to Linux
endif()

# ===== Common Locations ==========

set(MAIN_SRC_DIR "${dynamic_paper_SOURCE_DIR}/src")
set(CURRENT_TARGET dynamic_paper_test)
set(BENCHMARKING_TARGET dynamic_paper_benchmarking)

# ===== Golang Build ==========
if(LINUX)
  set(GOLANG_DIR_NAME golang)

  set(GOLANG_SRC_FILE_NAMES background.go)
  set(GOLANG_SRCS ${GOLANG_SRC_FILE_NAMES})
  list(TRANSFORM GOLANG_SRCS PREPEND "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/")

  set(GOLANG_OUTPUT_NAME go-background)
  set(GOLANG_COMPILE_SCRIPT compile_go.sh)

  set(GOLANG_DIR_FULL_PATH "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/")

  add_custom_command(
    OUTPUT "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a"
    "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h"
    COMMAND "./compile_go.sh"
    WORKING_DIRECTORY "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}"
    DEPENDS ${GOLANG_SRCS}
    COMMENT "[test] Building archive and header file for golang src")

  # add_custom_target(build-golang-test ALL DEPENDS ${GOLANG_SRCS})
  add_custom_target(
    build-golang-test ALL
    DEPENDS "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a"
    "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h"
    "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_COMPILE_SCRIPT}")

endif()

# ===== Profiling ==========

FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG v0.11.1
  GIT_SHALLOW TRUE
  GIT_PROCESS TRUE
)

FetchContent_MakeAvailable(tracy)

option(TRACY_ENABLE "" ON)

# ===== Add Sources ==========
if(MACOSX)
  set(BACKGROUND_SETTER_CALLER_SRC_FILE ${MAIN_SRC_DIR}/background_setter_macos.cpp)
else()
  set(BACKGROUND_SETTER_CALLER_SRC_FILE ${MAIN_SRC_DIR}/background_setter.cpp)
  set(BACKGROUND_SETTER_FILE ${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)
endif()

add_executable(
  ${CURRENT_TARGET}
  # test sources
  general_config_test.cpp
  background_set_test.cpp
  time_conversion_test.cpp
  logging_config_parsing.cpp
  background_test_setter.cpp
  static_background_test.cpp
  dynamic_background_test.cpp
  time_from_midnight_test.cpp
  current_time_test.cpp
  cmdline_helper_tests.cpp
  helper.cpp
  # sources
  ${MAIN_SRC_DIR}/background_set.cpp
  ${MAIN_SRC_DIR}/time_from_midnight.cpp
  ${MAIN_SRC_DIR}/config.cpp
  ${MAIN_SRC_DIR}/dynamic_background_set.cpp
  ${MAIN_SRC_DIR}/image_compositor.cpp
  ${MAIN_SRC_DIR}/logger.cpp
  ${MAIN_SRC_DIR}/static_background_set.cpp
  ${MAIN_SRC_DIR}/time_util.cpp
  ${MAIN_SRC_DIR}/cmdline_helper.cpp
  ${MAIN_SRC_DIR}/file_util.cpp
  ${MAIN_SRC_DIR}/location.cpp
  ${MAIN_SRC_DIR}/solar_day_provider.cpp
  ${MAIN_SRC_DIR}/hook_script_executor.cpp
  ${MAIN_SRC_DIR}/magick_compositor.cpp
  ${MAIN_SRC_DIR}/networking.cpp
  #${MAIN_SRC_DIR}/nolint/cimg_compositor.cpp
  "${BACKGROUND_SETTER_CALLER_SRC_FILE}"
  "${BACKGROUND_SETTER_FILE}")

add_executable(
  ${BENCHMARKING_TARGET}
  # benchmarking
  ${MAIN_SRC_DIR}/benchmarking/benchmarking.cpp
  # sources
  ${MAIN_SRC_DIR}/background_set.cpp
  ${MAIN_SRC_DIR}/time_from_midnight.cpp
  ${MAIN_SRC_DIR}/config.cpp
  ${MAIN_SRC_DIR}/dynamic_background_set.cpp
  ${MAIN_SRC_DIR}/image_compositor.cpp
  ${MAIN_SRC_DIR}/logger.cpp
  ${MAIN_SRC_DIR}/static_background_set.cpp
  ${MAIN_SRC_DIR}/time_util.cpp
  ${MAIN_SRC_DIR}/cmdline_helper.cpp
  ${MAIN_SRC_DIR}/file_util.cpp
  ${MAIN_SRC_DIR}/location.cpp
  ${MAIN_SRC_DIR}/solar_day_provider.cpp
  ${MAIN_SRC_DIR}/hook_script_executor.cpp
  ${MAIN_SRC_DIR}/magick_compositor.cpp
  ${MAIN_SRC_DIR}/networking.cpp
  "${BACKGROUND_SETTER_CALLER_SRC_FILE}"
  "${BACKGROUND_SETTER_FILE}")

# ===== Linking ===========

# googletest
FetchContent_MakeAvailable(googletest)
target_link_libraries(${CURRENT_TARGET} PRIVATE gtest_main gmock_main)

# Boost Xpressive
find_package(Boost 1.83.0)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found; needed to use Boost Xpressive!")
endif()
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${CURRENT_TARGET} PRIVATE ${Boost_LIBRARIES})
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE ${Boost_LIBRARIES})

# argparse
target_include_directories(argparse INTERFACE ${argparse_SOURCE_DIR}/include)
target_link_libraries(${CURRENT_TARGET} PRIVATE argparse)
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE argparse)

# CImg
# target_include_directories(CImg INTERFACE ${cimg_SOURCE_DIR}/)
# target_link_libraries(${CURRENT_TARGET} PRIVATE CImg)
# target_link_libraries(${BENCHMARKING_TARGET} PRIVATE CImg)

# expected
target_include_directories(expected INTERFACE ${expected_SOURCE_DIR}/include)
target_link_libraries(${CURRENT_TARGET} PRIVATE expected)
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE expected)

# golang lib
if(LINUX)
  target_link_libraries(
    ${CURRENT_TARGET}
    PRIVATE ${CMAKE_SOURCE_DIR}/src/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)
  target_link_libraries(
    ${BENCHMARKING_TARGET}
    PRIVATE ${CMAKE_SOURCE_DIR}/src/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)
endif()

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(${CURRENT_TARGET} PRIVATE CURL::libcurl)
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE CURL::libcurl)

# yaml-cpp
target_link_libraries(${CURRENT_TARGET} PRIVATE yaml-cpp)
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE yaml-cpp)

# spdlog
target_link_libraries(${CURRENT_TARGET} PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE spdlog::spdlog_header_only)

# sunset
target_link_libraries(${CURRENT_TARGET} PRIVATE sunset)
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE sunset)

# X11
find_package(X11 REQUIRED) # TODO support wayland
target_link_libraries(${CURRENT_TARGET} PRIVATE ${X11_LIBRARIES})
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE ${X11_LIBRARIES})

# Tracy
target_link_libraries(${BENCHMARKING_TARGET} PUBLIC TracyClient)

# ImageMagick
add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )
find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${CURRENT_TARGET} PRIVATE ${ImageMagick_LIBRARIES})
target_link_libraries(${BENCHMARKING_TARGET} PRIVATE ${ImageMagick_LIBRARIES})

# fmt (MacOS)
if(MACOSX)
  target_link_libraries(${CURRENT_TARGET} PRIVATE fmt::fmt)
endif()

# The main project
target_include_directories(${CURRENT_TARGET}
  PRIVATE ${dynamic_paper_SOURCE_DIR})
target_include_directories(${BENCHMARKING_TARGET}
  PRIVATE ${dynamic_paper_SOURCE_DIR})

# ===== Debug / Release ==================

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Setting Link Time Optimization to true")
  set_property(TARGET ${CURRENT_TARGET}
    PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ===== Clang Tidy ==========

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")
  set_target_properties(${CURRENT_TARGET} PROPERTIES CXX_CLANG_TIDY
    "${CLANG_TIDY_COMMAND}")
endif()

set_source_files_properties(image_compositor.cpp PROPERTIES SKIP_LINTING ON)

add_test(NAME runTests COMMAND dynamic_background_test)

