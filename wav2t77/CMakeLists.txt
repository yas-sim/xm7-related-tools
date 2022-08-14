set(TARGET "wav2t77")
file(GLOB CPP_SOURCES *.cpp)
add_executable(${TARGET} ${CPP_SOURCES})
