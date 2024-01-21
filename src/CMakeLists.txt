cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)
	set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(
  yaml-cpp
  URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.zip
)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v3.0
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
  expected
  GIT_REPOSITORY https://github.com/TartanLlama/expected.git
  GIT_TAG v1.1.0
)
FetchContent_MakeAvailable(expected)

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
  yaml_helper.cpp
  cmdline_helper.cpp
  file_util.cpp
)

target_link_libraries(
  dynamic_paper
  PRIVATE
  yaml-cpp
  spdlog
  argparse
  expected
)

target_include_directories(
  dynamic_paper
  PRIVATE
  ${dynamic_paper_SOURCE_DIR}
)

# clang tidy
#find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
#set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")
#set_target_properties(dynamic_paper PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
