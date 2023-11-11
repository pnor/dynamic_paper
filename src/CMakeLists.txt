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
  config.cpp
  background_set.cpp
  time_util.cpp
  command_executor.cpp

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
