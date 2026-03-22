cmake_minimum_required(VERSION 3.12)
project(sample)

include_directories(
    ../vklib/inc
    ../imgui
    ../glm
)

set(project_sources
    ../imgui/backends/imgui_impl_vulkan.cpp
    ../imgui/backends/imgui_impl_glfw.cpp
    ../imgui/misc/cpp/imgui_stdlib.cpp
    ../imgui/imgui_demo.cpp
    ../imgui/imgui_draw.cpp
    ../imgui/imgui_tables.cpp
    ../imgui/imgui_widgets.cpp
    ../imgui/imgui.cpp

    main.cpp
)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_BINARY_DIR}/bin/assets
)

add_executable(${PROJECT_NAME} ${project_sources})
add_dependencies(${PROJECT_NAME} copy_assets)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME} PRIVATE
    vklib
    volk
    glfw
)
