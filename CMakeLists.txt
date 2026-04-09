cmake_minimum_required(VERSION 3.10)
project(gfx_demo C)

set(CMAKE_C_STANDARD 99)
include(CTest)

set(GFX_PUBLIC_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(GFX_INTERNAL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)

add_executable(gfx_demo
    examples/main.c
    src/gfx_math.c
    src/stubs.c
    src/tinyobj_loader.c
)
target_include_directories(gfx_demo PRIVATE
    ${GFX_PUBLIC_INCLUDE_DIR}
    ${GFX_INTERNAL_INCLUDE_DIR}
)
target_link_libraries(gfx_demo PRIVATE dl)
add_executable(tinyobj_demo
    examples/tinyobj_demo.c
    cpu/fb0_platform.c
    cpu/framebuffer.c
    cpu/rasterizer.c
    src/gfx_math.c
    src/tinyobj_preview.c
    src/tinyobj_loader.c
)

target_include_directories(tinyobj_demo PRIVATE
    ${GFX_PUBLIC_INCLUDE_DIR}
    ${GFX_INTERNAL_INCLUDE_DIR}
)
target_link_libraries(tinyobj_demo PRIVATE dl)

add_executable(gfx_tests
    tests/gfx_tests.c
    cpu/fb0_platform.c
    cpu/framebuffer.c
    cpu/rasterizer.c
    gpu/gl_loader.c
    gpu/mesh.c
    gpu/x11_platform.c
    src/gfx_math.c
    src/stubs.c
    src/tinyobj_loader.c
    src/tinyobj_preview.c
)

target_include_directories(gfx_tests PRIVATE
    ${GFX_PUBLIC_INCLUDE_DIR}
    ${GFX_INTERNAL_INCLUDE_DIR}
)
target_link_libraries(gfx_tests PRIVATE dl)

add_test(NAME gfx_tests COMMAND gfx_tests)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(gfx_window_demo
        examples/window_demo.c
        gpu/mesh.c
        gpu/linux_window.c
        gpu/gl_loader.c
        gpu/x11_platform.c
        src/gfx_math.c
        src/stubs.c
        src/tinyobj_loader.c
    )

    target_include_directories(gfx_window_demo PRIVATE
        ${GFX_PUBLIC_INCLUDE_DIR}
        ${GFX_INTERNAL_INCLUDE_DIR}
    )
    target_link_libraries(gfx_window_demo PRIVATE dl m)

    find_program(XVFB_RUN xvfb-run)
    if (XVFB_RUN)
        add_test(NAME gfx_window_demo_smoke COMMAND ${XVFB_RUN} -a $<TARGET_FILE:gfx_window_demo>)
    elseif(DEFINED ENV{DISPLAY} AND NOT "$ENV{DISPLAY}" STREQUAL "")
        add_test(NAME gfx_window_demo_smoke COMMAND $<TARGET_FILE:gfx_window_demo>)
    endif()
endif()
