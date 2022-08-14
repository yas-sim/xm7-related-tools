set(TARGET "fmdir")
file(GLOB CPP_SOURCES *.cpp)

include_directories(../fmfslib)
add_executable(${TARGET} ${CPP_SOURCES})
target_link_libraries(${TARGET} PRIVATE fmfs)
