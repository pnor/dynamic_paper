cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
  dynamic_paper
  main.cpp
)

target_include_directories(
  dynamic_paper
  PRIVATE
  ${dynamic_paper_SOURCE_DIR}
)
