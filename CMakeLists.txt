cmake_minimum_required(VERSION 3.30)
project(XYTools)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options("/source-charset:utf-8" "/execution-charset:utf-8")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

find_package(OpenGL REQUIRED)

add_subdirectory(Thrid/glm)
add_subdirectory(Thrid/glad)
add_subdirectory(Thrid/glfw)
add_subdirectory(Thrid/imgui)
add_subdirectory(Thrid/implot)
add_subdirectory(Thrid/ImGuiFileDialog)

add_executable(XYTools main.cpp Window.cpp Window.h Scene.cpp Scene.h)

target_link_libraries(XYTools PRIVATE glm-header-only glad implot ImGuiFileDialog OpenGL::GL)


set_target_properties(XYTools PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
