
if (MSVC)
    add_compile_options("/source-charset:utf-8" "/execution-charset:utf-8")
endif ()

set(HRDS
        Window.h
        gl/Shader.h
        gl/Scene.h
        gl/Map.h
        xy2/mapx.h
        xy2/ujpeg.h
)

set(SRCS
        main.cpp
        Window.cpp
        gl/Shader.cpp
        gl/Scene.cpp
        gl/Map.cpp
        xy2/mapx.cpp
        xy2/ujpeg.cpp
)

add_executable(XYTools ${SRCS} ${HRDS})

target_link_libraries(XYTools PRIVATE glm-header-only glad implot stb OpenGL::GL)

target_include_directories(XYTools PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


#set_target_properties(XYTools PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")