cmake_minimum_required(VERSION 3.12)
project(vklib)

include_directories(
    inc
)

set(project_sources
    vklib/set.c
    vklib/vk.c
    vklib/vkbuffer.c
    vklib/vkcmd.c
    vklib/vkdev.c
    vklib/vkdisplay.c
    vklib/vkframebuffer.c
    vklib/vkindexbuffer.c
    vklib/vkpipeline.c
    vklib/vkrenderer.c
    vklib/vkuniform.c
    vklib/vkutil.c
    vklib/vkvertexbuffer.c
)

add_library(${PROJECT_NAME} STATIC ${project_sources})

target_link_libraries(${PROJECT_NAME} PRIVATE
    volk
    glfw
)
