project(stb)

file(GLOB SRCS *.c)

add_library(stb STATIC ${SRCS})

target_include_directories(stb PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)