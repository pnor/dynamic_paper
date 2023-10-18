cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)
FetchContent_Declare(
  yaml-cpp
  URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.zip
)
FetchContent_MakeAvailable(yaml-cpp)

add_executable(
  dynamic_paper_test
  helper.cpp
  test.cpp
  yaml_parsing_test.cpp
  ${dynamic_paper_SOURCE_DIR}/src/background_set.cpp
  ${dynamic_paper_SOURCE_DIR}/src/config.cpp
)

target_link_libraries(
  dynamic_paper_test
  PRIVATE
  gtest_main
  yaml-cpp
)

target_include_directories(
  dynamic_paper_test
  PRIVATE
  ${dynamic_paper_SOURCE_DIR}
)
