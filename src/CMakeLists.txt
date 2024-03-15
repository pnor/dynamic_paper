cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

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

add_custom_command(
  POST_BUILD
  OUTPUT ${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a
         ${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h
  COMMAND ${GO_EXEC} mod tidy
  COMMAND ${GO_EXEC} build -buildmode=c-archive -o ${GOLANG_OUTPUT_NAME}.a
          ${GOLANG_SRC_FILE_NAMES}
  WORKING_DIRECTORY ${dynamic_paper_SOURCE_DIR}/src/golang
  DEPENDS ${GOLANG_SRCS}
  COMMENT "Building archive and header file for golang src")

# TODO make it so main build waits for existence of the archive?
add_custom_target(build-golang ALL DEPENDS ${GOLANG_SRCS})

# ===== Add Sources ==========

add_executable(
  dynamic_paper
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
  # golang archive
  ${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)

target_link_libraries(dynamic_paper PRIVATE yaml-cpp spdlog argparse expected)

# TODO can remove?
target_include_directories(dynamic_paper PRIVATE ${dynamic_paper_SOURCE_DIR})

# ===== Clang Tidy ==========
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")
set_target_properties(dynamic_paper PROPERTIES CXX_CLANG_TIDY
                                               "${CLANG_TIDY_COMMAND}")
