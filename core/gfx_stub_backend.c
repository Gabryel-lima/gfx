#include <stdio.h>

#include "gfx.h"

/**
 * @brief Backend de depuração que só imprime cada chamada. Usado pelos
 *        exemplos e smoke tests quando não há um backend real disponível.
 * @file core/gfx_stub_backend.c
 */

static void stub_draw_mesh(void *ctx, Mesh *mesh, Mat4 transform) {
    (void)ctx;
    (void)mesh;
    (void)transform;
    printf("[gfx_stub] draw_mesh\n");
}

static void stub_set_camera(void *ctx, Vec3 pos, Vec3 target, float fov) {
    (void)ctx;
    (void)pos;
    (void)target;
    (void)fov;
    printf("[gfx_stub] set_camera\n");
}

static void stub_begin(void *ctx) {
    (void)ctx;
    printf("[gfx_stub] begin_frame\n");
}

static void stub_end(void *ctx) {
    (void)ctx;
    printf("[gfx_stub] end_frame\n");
}

static void stub_cleanup(void *ctx) {
    (void)ctx;
    printf("[gfx_stub] cleanup\n");
}

GfxBackend gfx_get_stub_backend(void) {
    GfxBackend backend = { stub_begin, stub_end, stub_draw_mesh, stub_set_camera, stub_cleanup };
    return backend;
}
