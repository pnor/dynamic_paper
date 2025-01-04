cmake_minimum_required(VERSION 3.27)
if(POLICY CMP0174)
    cmake_policy(SET CMP0174 NEW)
endif()
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===== Common Locations ==========

set(MAIN_SRC_DIR "${dynamic_paper_SOURCE_DIR}/src")
set(CURRENT_TARGET dynamic_paper)

# ===== Golang Build ==========
if(LINUX)
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
endif()

# ===== Add Sources ==========
if(MACOSX)
  set(BACKGROUND_SETTER_CALLER_SRC_FILE background_setter_macos.cpp)
else()
  set(BACKGROUND_SETTER_CALLER_SRC_FILE background_setter.cpp)
  set(BACKGROUND_SETTER_FILE "${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a")
endif()

add_executable(
  dynamic_paper
  # sources
  main.cpp
  background_set.cpp
  config.cpp
  dynamic_background_set.cpp
  image_compositor.cpp
  logger.cpp
  static_background_set.cpp
  time_util.cpp
  time_from_midnight.cpp
  cmdline_helper.cpp
  file_util.cpp
  location.cpp
  solar_day_provider.cpp
  hook_script_executor.cpp
  magick_compositor.cpp
  networking.cpp
  "${BACKGROUND_SETTER_CALLER_SRC_FILE}"
  "${BACKGROUND_SETTER_FILE}")

# ===== Release ===========

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  # TODO only install the executable?
  set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
  install(TARGETS ${CURRENT_TARGET} DESTINATION . OPTIONAL)
endif()

# ===== Linking ===========

# Boost Xpressive
find_package(Boost 1.83.0)
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found; needed to use Boost Xpressive!")
endif()
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${CURRENT_TARGET} PRIVATE ${Boost_LIBRARIES})

# argparse
target_include_directories(argparse INTERFACE ${argparse_SOURCE_DIR}/include)
target_link_libraries(${CURRENT_TARGET} PRIVATE argparse)
# target_link_libraries(${CURRENT_TARGET} PRIVATE argparse -static)

# expected
target_include_directories(expected INTERFACE ${expected_SOURCE_DIR}/include)
target_link_libraries(${CURRENT_TARGET} PRIVATE expected)
# target_link_libraries(${CURRENT_TARGET} PRIVATE expected -static)

# golang lib
if (LINUX)
  target_link_libraries(
    ${CURRENT_TARGET}
    PRIVATE ${CMAKE_SOURCE_DIR}/src/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a)
    # PRIVATE ${CMAKE_SOURCE_DIR}/src/${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a -static)
endif()

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(${CURRENT_TARGET} PRIVATE CURL::libcurl)

# yaml-cpp
# target_link_libraries(${CURRENT_TARGET} PRIVATE yaml-cpp)
target_link_libraries(${CURRENT_TARGET} PRIVATE yaml-cpp)

# spdlog
target_link_libraries(${CURRENT_TARGET} PRIVATE spdlog::spdlog_header_only)
# target_link_libraries(${CURRENT_TARGET} PRIVATE spdlog::spdlog_header_only -static)

# sunset
# target_link_libraries(${CURRENT_TARGET} PRIVATE sunset)
target_link_libraries(${CURRENT_TARGET} PRIVATE sunset)

# X11
find_package(X11 REQUIRED) # TODO support wayland
target_link_libraries(${CURRENT_TARGET} PRIVATE ${X11_LIBRARIES})
# target_link_libraries(${CURRENT_TARGET} PRIVATE ${X11_LIBRARIES} -static)

# ImageMagick
add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )
find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})
target_link_libraries(${CURRENT_TARGET} PRIVATE ${ImageMagick_LIBRARIES})

# fmt (MacOS)
if(MACOSX)
  target_link_libraries(${CURRENT_TARGET} PRIVATE fmt::fmt)
endif()

# ===== Clean Command ==========

if(LINUX)
  set_property(
    TARGET ${CURRENT_TARGET}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES {GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.a
    ${GOLANG_DIR_NAME}/${GOLANG_OUTPUT_NAME}.h)
endif()

# ===== Debug / Release ==================

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Setting Link Time Optimization to true")
  set_property(TARGET ${CURRENT_TARGET}
    PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ===== Clang Tidy ==========

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")
  set_target_properties(${CURRENT_TARGET} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()
