cmake_minimum_required(VERSION 3.27)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===== Common Locations ==========

set(MAIN_SRC_DIR "${dynamic_paper_SOURCE_DIR}/src")
set(CURRENT_TARGET dynamic_paper)

# ===== Golang Build ==========
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

# ===== Add Sources ==========
add_executable(
  dynamic_paper
  # sources
  main.cpp
  background_set.cpp
  background_setter.cpp
  command_executor.cpp
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
  nolint/cimg_compositor.cpp
  "${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a")

# ===== Linking ===========

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
target_link_libraries(
  ${CURRENT_TARGET}
  PRIVATE ${CMAKE_SOURCE_DIR}/src/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)

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

set_property(
  TARGET ${CURRENT_TARGET}
  APPEND
  PROPERTY ADDITIONAL_CLEAN_FILES {GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a
           ${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h)

# ===== Clang Tidy ==========

# find_program(CLANG_TIDY_EXE NAMES "clang-tidy") set(CLANG_TIDY_COMMAND
# "${CLANG_TIDY_EXE}") set_target_properties(${CURRENT_TARGET} PROPERTIES
# CXX_CLANG_TIDY
# "${CLANG_TIDY_COMMAND}")
