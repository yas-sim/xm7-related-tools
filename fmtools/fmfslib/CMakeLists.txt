file(GLOB CPP_SOURCES *.cpp)
#include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories(".")
add_library(fmfs SHARED ${CPP_SOURCES})
