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
add_subdirectory(dmygen)
add_subdirectory(fdump)
add_subdirectory(mot2bin)
add_subdirectory(mot2bin2)
add_subdirectory(romcut)
add_subdirectory(seven2av)
add_subdirectory(t77dec)
add_subdirectory(t772wav)
add_subdirectory(wav2t77)

add_subdirectory(fontp)

if(WIN32)
	add_subdirectory(dmp2mot)
	add_subdirectory(fmtools)
	add_subdirectory(krom)
endif()

