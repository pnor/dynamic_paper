cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

# ===== Common Locations ==========

set(MAIN_SRC_DIR "${dynamic_paper_SOURCE_DIR}/src")

# ===== Libraries ==========

include(FetchContent)
FetchContent_Declare(
  yaml-cpp URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.zip)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v3.0)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
  expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected.git
  GIT_TAG v1.1.0)
FetchContent_MakeAvailable(expected)

# ===== Golang ==========
find_program(GO_EXEC NAMES "go")

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

message("${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a")
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
  yaml_helper.cpp
  cmdline_helper.cpp
  file_util.cpp
  "${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a")

target_link_libraries(
  dynamic_paper
  PRIVATE ${CMAKE_SOURCE_DIR}/src/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)

target_link_libraries(dynamic_paper PRIVATE yaml-cpp spdlog argparse expected)

set_property(
  TARGET dynamic_paper
  APPEND
  PROPERTY ADDITIONAL_CLEAN_FILES {GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a
           ${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h)

# ===== Clang Tidy ==========

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")
set_target_properties(dynamic_paper PROPERTIES CXX_CLANG_TIDY
                                               "${CLANG_TIDY_COMMAND}")
