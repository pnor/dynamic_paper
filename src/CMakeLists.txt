cmake_minimum_required(VERSION 3.27)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===== OS check ==================
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
  else()
    set(LINUX TRUE) # default to Linux
endif()

# ===== Common Locations ==========

set(MAIN_SRC_DIR "${dynamic_paper_SOURCE_DIR}/src")
set(CURRENT_TARGET dynamic_paper)

# ===== Golang Build ==========
if(LINUX)
  set(GOLANG_SRC_FILE_NAMES background.go)
  set(GOLANG_DIR_NAME golang)

  # prepend full path to the srcs for GOLANG_SRCS
  set(GOLANG_SRCS ${GOLANG_SRC_FILE_NAMES})
  list(TRANSFORM GOLANG_SRCS
    PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/${GOLANG_DIR_NAME}/")
  set(GOLANG_OUTPUT_NAME go-background)
  set(GOLANG_COMPILE_SCRIPT compile_go.sh)

  set(FULL_GO "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}")

  add_custom_command(
    OUTPUT "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a"
    "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h"
    COMMAND "./compile_go.sh"
    WORKING_DIRECTORY "${dynamic_paper_SOURCE_DIR}/src/${GOLANG_DIR_NAME}"
    DEPENDS ${GOLANG_SRCS}
    COMMENT "[src] Building archive and header file for golang src")

  add_custom_target(
    build-golang ALL
    DEPENDS "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a"
    "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h"
    "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_COMPILE_SCRIPT}")
endif()

# ===== Add Sources ==========
if(MACOSX)
  set(BACKGROUND_SETTER_CALLER_SRC_FILE background_setter_macos.cpp)
else()
  set(BACKGROUND_SETTER_CALLER_SRC_FILE background_setter.cpp)
  set(BACKGROUND_SETTER_FILE "${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a")
endif()

add_executable(
  dynamic_paper
  # sources
  main.cpp
  background_set.cpp
  # background_setter.cpp
  config.cpp
  dynamic_background_set.cpp
  image_compositor.cpp
  logger.cpp
  static_background_set.cpp
  time_util.cpp
  time_from_midnight.cpp
  cmdline_helper.cpp
  file_util.cpp
  location.cpp
  solar_day_provider.cpp
  hook_script_executor.cpp
  nolint/cimg_compositor.cpp
  "${BACKGROUND_SETTER_CALLER_SRC_FILE}"
  "${BACKGROUND_SETTER_FILE}")

# ===== Linking ===========

# Boost Xpressive
find_package(Boost 1.83.0)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found; needed to use Boost Xpressive!")
endif()
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${CURRENT_TARGET} PRIVATE ${Boost_LIBRARIES})

# argparse
target_include_directories(argparse INTERFACE ${argparse_SOURCE_DIR}/include)
target_link_libraries(${CURRENT_TARGET} PRIVATE argparse)

# CImg
target_include_directories(CImg INTERFACE ${cimg_SOURCE_DIR})
target_link_libraries(${CURRENT_TARGET} PRIVATE CImg)

# expected
target_include_directories(expected INTERFACE ${expected_SOURCE_DIR}/include)
target_link_libraries(${CURRENT_TARGET} PRIVATE expected)

# golang lib
if (LINUX)
  target_link_libraries(
    ${CURRENT_TARGET}
    PRIVATE ${CMAKE_SOURCE_DIR}/src/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)
endif()

# cpr
target_link_libraries(${CURRENT_TARGET} PRIVATE cpr::cpr)

# yaml-cpp
target_link_libraries(${CURRENT_TARGET} PRIVATE yaml-cpp)

# spdlog
target_link_libraries(${CURRENT_TARGET} PRIVATE spdlog::spdlog_header_only)

# sunset
target_link_libraries(${CURRENT_TARGET} PRIVATE sunset)

# X11
find_package(X11 REQUIRED) # TODO support wayland
target_link_libraries(${CURRENT_TARGET} PRIVATE ${X11_LIBRARIES})

# ===== Clean Command ==========

if(LINUX)
  set_property(
    TARGET ${CURRENT_TARGET}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES {GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a
    ${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h)
endif()

# ===== Clang Tidy ==========

# find_program(CLANG_TIDY_EXE NAMES "clang-tidy") set(CLANG_TIDY_COMMAND
# "${CLANG_TIDY_EXE}") set_target_properties(${CURRENT_TARGET} PROPERTIES
# CXX_CLANG_TIDY
# "${CLANG_TIDY_COMMAND}")
