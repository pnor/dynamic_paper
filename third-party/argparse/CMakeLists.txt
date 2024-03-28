# add_library(${TARGET_LOGGER} INTERFACE)
target_include_directories(
  ${TARGET_LOGGER}
  INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(example_header_only PRIVATE spdlog::spdlog_header_only)
