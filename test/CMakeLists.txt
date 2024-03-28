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
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_Declare(
  yaml-cpp URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.zip)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0)

# ===== Golang Build ==========
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

# ===== Add Sources ==========
add_executable(
  dynamic_paper_test
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
  # sources
  ${MAIN_SRC_DIR}/background_set.cpp
  ${MAIN_SRC_DIR}/time_from_midnight.cpp
  ${MAIN_SRC_DIR}/background_setter.cpp
  ${MAIN_SRC_DIR}/command_executor.cpp
  ${MAIN_SRC_DIR}/config.cpp
  ${MAIN_SRC_DIR}/dynamic_background_set.cpp
  ${MAIN_SRC_DIR}/image_compositor.cpp
  ${MAIN_SRC_DIR}/logger.cpp
  ${MAIN_SRC_DIR}/static_background_set.cpp
  ${MAIN_SRC_DIR}/time_util.cpp
  ${MAIN_SRC_DIR}/yaml_helper.cpp
  ${MAIN_SRC_DIR}/cmdline_helper.cpp
  ${MAIN_SRC_DIR}/file_util.cpp
  "${MAIN_SRC_DIR}/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a")

# ===== Include Path ==========
target_include_directories(
  dynamic_paper_test
  PRIVATE "${CMAKE_SOURCE_DIR}/third-party/CImg/include"
          "${CMAKE_SOURCE_DIR}/third-party/argparse/include"
          "${CMAKE_SOURCE_DIR}/third-party/expected/include"
          "${CMAKE_SOURCE_DIR}/third-party/spdlog/include")

# ===== Linking ===========

target_link_libraries(
  dynamic_paper_test
  PRIVATE ${CMAKE_SOURCE_DIR}/src/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)

FetchContent_MakeAvailable(spdlog)
target_link_libraries(dynamic_paper_test PRIVATE spdlog::spdlog_header_only)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(yaml-cpp)
target_link_libraries(dynamic_paper_test PRIVATE gtest_main gmock_main yaml-cpp)

find_package(X11 REQUIRED) # TODO support wayland
target_link_libraries(dynamic_paper_test PRIVATE ${X11_LIBRARIES})

target_include_directories(dynamic_paper_test
                           PRIVATE ${dynamic_paper_SOURCE_DIR})

# ===== Clang Tidy ==========

# find_program(CLANG_TIDY_EXE NAMES "clang-tidy") set(CLANG_TIDY_COMMAND
# "${CLANG_TIDY_EXE}") set_target_properties(dynamic_paper_test PROPERTIES
# CXX_CLANG_TIDY
# "${CLANG_TIDY_COMMAND}")
