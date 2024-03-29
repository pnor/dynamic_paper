file(
  DOWNLOAD
  https://raw.githubusercontent.com/TartanLlama/expected/master/include/tl/expected.hpp
  ${CMAKE_CURRENT_LIST_DIR}/include/tl/expected.hpp)

target_include_directories(
  ${INCLUDE_TARGET}
  PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
          "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
