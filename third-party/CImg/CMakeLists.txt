file(DOWNLOAD https://raw.githubusercontent.com/GreycLab/CImg/master/CImg.h
     ${CMAKE_CURRENT_LIST_DIR}/include/CImg.h)

target_include_directories(
  ${INCLUDE_TARGET}
  PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
          "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
