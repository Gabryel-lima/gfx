cmake_minimum_required(VERSION 3.10)
project(gfx_demo C)

set(CMAKE_C_STANDARD 99)
include(CTest)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(GFX_ENABLE_GL_BACKEND_DEFAULT ON)
else()
    set(GFX_ENABLE_GL_BACKEND_DEFAULT OFF)
endif()
option(GFX_ENABLE_GL_BACKEND "Build the optional Linux/X11/OpenGL window backend" ${GFX_ENABLE_GL_BACKEND_DEFAULT})
option(GFX_CORE_FREESTANDING_CHECK "Compile-only check that the portable gfx core has no OS dependency" OFF)

set(GFX_PUBLIC_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# gfx_core: the portable library. Math, software rasterizer, framebuffer
# primitives, OBJ/MTL loading and the public facade dispatcher. No OS
# dependency beyond the C standard library — no dl/m link, no platform code.
# This is what a consumer (including a future freestanding kernel) embeds.
add_library(gfx_core STATIC
    core/gfx_math.c
    core/gfx_raster.c
    core/gfx_framebuffer.c
    core/gfx_mesh.c
    core/gfx_facade.c
    core/gfx_stub_backend.c
    core/tinyobj_loader.c
    core/tinyobj_preview.c
)
target_include_directories(gfx_core PUBLIC ${GFX_PUBLIC_INCLUDE_DIR})

add_executable(gfx_demo examples/main.c)
target_link_libraries(gfx_demo PRIVATE gfx_core)

add_executable(tinyobj_demo examples/tinyobj_demo.c)
target_link_libraries(tinyobj_demo PRIVATE gfx_core)

add_executable(gfx_tests
    tests/gfx_tests.c
    platform/linux/fb_open.c
    platform/linux/gl_loader.c
    platform/linux/x11_platform.c
)
target_link_libraries(gfx_tests PRIVATE gfx_core dl)

add_test(NAME gfx_tests COMMAND gfx_tests)

if (GFX_ENABLE_GL_BACKEND)
    add_executable(gfx_window_demo
        examples/window_demo.c
        platform/linux/window.c
        platform/linux/gl_loader.c
        platform/linux/x11_platform.c
    )
    target_link_libraries(gfx_window_demo PRIVATE gfx_core dl m)

    find_program(XVFB_RUN xvfb-run)
    if (XVFB_RUN)
        add_test(NAME gfx_window_demo_smoke COMMAND ${XVFB_RUN} -a $<TARGET_FILE:gfx_window_demo>)
    elseif(DEFINED ENV{DISPLAY} AND NOT "$ENV{DISPLAY}" STREQUAL "")
        add_test(NAME gfx_window_demo_smoke COMMAND $<TARGET_FILE:gfx_window_demo>)
    endif()
endif()

# Compile-only proof that the truly bare-metal-ready subset of gfx_core
# (math + rasterizer + framebuffer primitives) has zero libc/OS dependency
# beyond <stdint.h>/<stddef.h>. Not linked, not run — just compiled with
# -ffreestanding to catch an accidental hosted dependency creeping back in.
# gfx_mesh.c/tinyobj_*.c are excluded: they need fopen/malloc and are meant
# for hosted use, not for vendoring into a kernel.
if (GFX_CORE_FREESTANDING_CHECK)
    if (NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "GFX_CORE_FREESTANDING_CHECK requires GCC or Clang")
    endif()

    add_library(gfx_core_freestanding_check OBJECT
        core/gfx_math.c
        core/gfx_raster.c
        core/gfx_framebuffer.c
    )
    target_include_directories(gfx_core_freestanding_check PRIVATE ${GFX_PUBLIC_INCLUDE_DIR})
    target_compile_options(gfx_core_freestanding_check PRIVATE
        -ffreestanding -fno-builtin -Wall -Wextra
    )
endif()
