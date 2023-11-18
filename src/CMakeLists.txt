cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  yaml-cpp
  URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.zip
)
FetchContent_MakeAvailable(yaml-cpp)

add_executable(
  dynamic_paper
  main.cpp
  background_set.cpp
  background_setter.cpp
  command_executor.cpp
  config.cpp
  dynamic_background_set.cpp
  static_background_set.cpp
  time_util.cpp
  yaml_helper.cpp
)

target_link_libraries(
  dynamic_paper
  PRIVATE
  yaml-cpp
)

target_include_directories(
  dynamic_paper
  PRIVATE
  ${dynamic_paper_SOURCE_DIR}
)
