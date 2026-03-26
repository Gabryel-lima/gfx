#include <stdio.h>

#include "gfx.h"

// Tipos opacos para compilar com a API mínima
struct Mesh { int dummy; };
struct Material { int dummy; };

// Implementação de backend de stub (apenas imprime mensagens)

static void stub_draw_mesh(void *ctx, Mesh *m, Mat4 transform, Material *mat) {
    gfx_draw_mesh((GfxContext *)ctx, m, transform, mat); // Chama draw_mesh para mostrar que o stub funciona
    printf("[gfx_stub] draw_mesh\n");
}

static void stub_set_camera(void *ctx, Vec3 pos, Vec3 target, float fov) {
    gfx_set_camera((GfxContext *)ctx, pos, target, fov); // Chama set_camera para mostrar que o stub funciona
    printf("[gfx_stub] set_camera\n");
}

static void stub_begin(void *ctx) {
    gfx_draw_mesh((GfxContext *)ctx, NULL, (Mat4){{{0}}}, NULL); // Chama draw_mesh para mostrar que o stub funciona
    printf("[gfx_stub] begin_frame\n");
}

static void stub_end(void *ctx) {
    gfx_end_frame((GfxContext *)ctx); // Chama end_frame para mostrar que o stub funciona
    printf("[gfx_stub] end_frame\n");
}

// Implementação da função prototipada em gfx.h

void gfx_draw_mesh(GfxContext *g, Mesh *m, Mat4 t, Material *mat) {
    if (g && g->backend.draw_mesh) {
        g->backend.draw_mesh(g->backend_ctx, m, t, mat);
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
    // Nenhuma alocação dinâmica no stub, então nada a limpar
    (void)g; // Evita warning de parâmetro não usado
}

// Função auxiliar para obter um backend stub
GfxBackend gfx_get_stub_backend(void) {
    GfxBackend b = { stub_begin, stub_end, stub_draw_mesh, stub_set_camera };
    return b;
}
