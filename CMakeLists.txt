cmake_minimum_required(VERSION 3.10)
project(vulkan_abstraction_layer)

string(TIMESTAMP NOW "%Y %m %d")
string(REPLACE " " ";" DATE_LIST ${NOW})
list(GET DATE_LIST 0 YEAR)
list(GET DATE_LIST 1 MONTH)
list(GET DATE_LIST 2 DAY)

math(EXPR YEAR ${YEAR})
math(EXPR MONTH ${MONTH})
math(EXPR DAY ${DAY})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)

add_compile_definitions(__YEAR__=${YEAR})
add_compile_definitions(__MONTH__=${MONTH})
add_compile_definitions(__DAY__=${DAY})

find_package(Vulkan REQUIRED)

option(VKLIB_BUILD_SAMPLE "Build sample app" ON)

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

add_compile_definitions(VKLIB_BUILD)

if (UNIX)
    set(CMAKE_C_FLAGS_DEBUG "-g -ggdb")
    set(CMAKE_C_FLAGS_RELEASE "-g")
endif()

add_subdirectory(vklib)
if(VKLIB_BUILD_SAMPLE)
    add_subdirectory(sample)
endif()
