cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)
	set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

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
  dynamic_paper_test
  general_config_test.cpp
  background_set_test.cpp
  time_conversion_test.cpp
  logging_config_parsing.cpp
  ${dynamic_paper_SOURCE_DIR}/src/background_set.cpp
  ${dynamic_paper_SOURCE_DIR}/src/background_setter.cpp
  ${dynamic_paper_SOURCE_DIR}/src/command_executor.cpp
  ${dynamic_paper_SOURCE_DIR}/src/config.cpp
  ${dynamic_paper_SOURCE_DIR}/src/dynamic_background_set.cpp
  ${dynamic_paper_SOURCE_DIR}/src/image_compositor.cpp
  ${dynamic_paper_SOURCE_DIR}/src/logger.cpp
  ${dynamic_paper_SOURCE_DIR}/src/static_background_set.cpp
  ${dynamic_paper_SOURCE_DIR}/src/time_util.cpp
  ${dynamic_paper_SOURCE_DIR}/src/yaml_helper.cpp
  ${dynamic_paper_SOURCE_DIR}/src/cmdline_helper.cpp
  ${dynamic_paper_SOURCE_DIR}/src/file_util.cpp
)

target_link_libraries(
  dynamic_paper_test
  PRIVATE
  gtest_main
  gmock_main
  yaml-cpp
  spdlog
  argparse
  expected
)

target_include_directories(
  dynamic_paper_test
  PRIVATE
  ${dynamic_paper_SOURCE_DIR}
)

# clang tidy
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")
set_target_properties(dynamic_paper_test PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
