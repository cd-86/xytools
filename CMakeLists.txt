cmake_minimum_required(VERSION 3.30)
project(XYTools)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

find_package(OpenGL REQUIRED)

add_subdirectory(Thrid/glm)
add_subdirectory(Thrid/glad)
add_subdirectory(Thrid/glfw)
add_subdirectory(Thrid/imgui)
add_subdirectory(Thrid/implot)
add_subdirectory(Thrid/stb)

add_subdirectory(src)
