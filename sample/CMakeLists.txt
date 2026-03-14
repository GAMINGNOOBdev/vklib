cmake_minimum_required(VERSION 3.10)
project(sample)

include_directories(
    ../vklib/inc
)

set(project_sources
    main.c
)

add_executable(${PROJECT_NAME} ${project_sources})

target_link_libraries(${PROJECT_NAME} PRIVATE
    vklib
    volk
    glfw
)
