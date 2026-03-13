cmake_minimum_required(VERSION 3.10)
project(vklib)

set(project_sources
    vklib/set.c
    vklib/vk.c
    vklib/vkdev.c
    vklib/vkdisplay.c
    vklib/vkutil.c
)

add_library(${PROJECT_NAME} SHARED ${project_sources})

target_link_libraries(${PROJECT_NAME} PRIVATE
    volk
    glfw
)
