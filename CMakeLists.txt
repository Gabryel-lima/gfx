cmake_minimum_required(VERSION 3.10)
project(gfx_demo C)

set(CMAKE_C_STANDARD 99)

add_executable(gfx_demo
    examples/main.c
    src/gfx_math.c
    src/stubs.c
    src/tinyobj_loader.c
)
target_include_directories(gfx_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(gfx_demo PRIVATE dl)
add_executable(tinyobj_demo
    examples/tinyobj_demo.c
    src/gfx_math.c
    src/stubs.c
    src/tinyobj_loader.c
)

target_include_directories(tinyobj_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(tinyobj_demo PRIVATE dl)
