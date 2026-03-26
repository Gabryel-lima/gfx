#include <stdio.h>
#include "gfx.h"

// Tipos opacos para compilar com a API mínima
struct Mesh { int dummy; };
struct Material { int dummy; };

static void stub_begin(void *ctx) {
    (void)ctx;
    printf("[gfx_stub] begin_frame\n");
}

static void stub_end(void *ctx) {
    (void)ctx;
    printf("[gfx_stub] end_frame\n");
}

static void stub_draw(void *ctx, Mesh *m, Mat4 transform, Material *mat) {
    (void)ctx; (void)m; (void)transform; (void)mat;
    printf("[gfx_stub] draw_mesh\n");
}

static void stub_set_camera(void *ctx, Vec3 pos, Vec3 target, float fov) {
    (void)ctx; (void)pos; (void)target; (void)fov;
    printf("[gfx_stub] set_camera\n");
}

// Implementação da função prototipada em gfx.h
void gfx_draw_mesh(GfxContext *g, Mesh *m, Mat4 t, Material *mat) {
    if (g && g->backend.draw_mesh) {
        g->backend.draw_mesh(g->backend_ctx, m, t, mat);
    }
}

// Função auxiliar para obter um backend stub
GfxBackend gfx_get_stub_backend(void) {
    GfxBackend b = { stub_begin, stub_end, stub_draw, stub_set_camera };
    return b;
}
