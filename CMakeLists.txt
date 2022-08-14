cmake_minimum_required(VERSION 3.12)
project(fdc_bitstream)

set(CMAKE_CXX_FLAGS "-std=c++14")

include_directories("${CMAKE_SOURCE_DIR}/include")
link_directories("${CMAKE_SOURCE_DIR}/bin/debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(bin2mot)
add_subdirectory(bincut)
add_subdirectory(d77enc_dec)
add_subdirectory(d77uty)
add_subdirectory(dmp2mot)
add_subdirectory(dmygen)
add_subdirectory(fdump)
add_subdirectory(fmtools)