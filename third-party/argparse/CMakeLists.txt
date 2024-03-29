file(
  DOWNLOAD
  https://raw.githubusercontent.com/p-ranav/argparse/master/include/argparse/argparse.hpp
  ${CMAKE_CURRENT_LIST_DIR}/include/argparse/argparse.hpp)

target_include_directories(
  ${INCLUDE_TARGET}
  PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
          "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
