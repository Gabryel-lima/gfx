#include "gfx.h"

void gfx_context_init(GfxContext *g, GfxBackend backend, void *backend_ctx) {
    if (!g) {
        return;
    }
    g->backend = backend;
    g->backend_ctx = backend_ctx;
}

void gfx_draw_mesh(GfxContext *g, Mesh *mesh, Mat4 transform) {
    if (g && g->backend.draw_mesh) {
        g->backend.draw_mesh(g->backend_ctx, mesh, transform);
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
    if (g && g->backend.cleanup) {
        g->backend.cleanup(g->backend_ctx);
    }
}
