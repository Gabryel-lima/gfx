#include <stdio.h>

#include "gfx.h"

static void stub_draw_mesh(void *ctx, Mesh *mesh, Mat4 transform, Material *mat) {
    (void)ctx;
    (void)mesh;
    (void)transform;
    (void)mat;
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

void gfx_draw_mesh(GfxContext *g, Mesh *mesh, Mat4 transform, Material *mat) {
    if (g && g->backend.draw_mesh) {
        g->backend.draw_mesh(g->backend_ctx, mesh, transform, mat);
    }
}

void gfx_set_camera(GfxContext *g, Vec3 pos, Vec3 target, float fov) {
    if (g && g->backend.set_camera) {
        g->backend.set_camera(g->backend_ctx, pos, target, fov);
    }
}

void gfx_begin_frame(GfxContext *g) {
    if (g && g->backend.begin_frame) {
        g->backend.begin_frame(g->backend_ctx);
    }
}

void gfx_end_frame(GfxContext *g) {
    if (g && g->backend.end_frame) {
        g->backend.end_frame(g->backend_ctx);
    }
}

void gfx_cleanup(GfxContext *g) {
    (void)g;
}

GfxBackend gfx_get_stub_backend(void) {
    GfxBackend backend = { stub_begin, stub_end, stub_draw_mesh, stub_set_camera };
    return backend;
}
