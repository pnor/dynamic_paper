cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(
  dynamic_paper_test
  helper.cpp
  test.cpp
)

target_link_libraries(
  dynamic_paper_test
  PRIVATE
  gtest_main
)

target_include_directories(
  dynamic_paper_test
  PRIVATE
  ${dynamic_paper_SOURCE_DIR}
)
