file(
  DOWNLOAD
  https://raw.githubusercontent.com/elnormous/HTTPRequest/master/include/HTTPRequest.hpp
  ${CMAKE_CURRENT_LIST_DIR}/include/HTTPRequest.hpp)

target_include_directories(
  ${INCLUDE_TARGET}
  PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
          "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
