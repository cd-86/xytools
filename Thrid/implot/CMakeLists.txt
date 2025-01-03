project(implot)

set(SRCS
        implot.cpp
        implot_items.cpp
        implot_demo.cpp
)

add_library(implot STATIC ${SRCS})

target_link_libraries(implot PUBLIC imgui)

target_include_directories(implot PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)