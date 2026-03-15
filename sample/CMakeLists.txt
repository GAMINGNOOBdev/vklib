cmake_minimum_required(VERSION 3.10)
project(sample)

include_directories(
    ../vklib/inc
)

set(project_sources
    main.c
)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_BINARY_DIR}/bin/assets
)

add_executable(${PROJECT_NAME} ${project_sources})
add_dependencies(${PROJECT_NAME} copy_assets)

target_link_libraries(${PROJECT_NAME} PRIVATE
    vklib
    volk
    glfw
)
