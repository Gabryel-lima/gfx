#include <stdio.h>
#include "gfx.h"

int main(void) {
    // Construir contexto mínimo com backend stub
    GfxContext ctx;
    ctx.backend = gfx_get_stub_backend();
    ctx.backend_ctx = NULL;

    // Exemplo simples: chamar begin/draw/end
    ctx.backend.begin_frame(ctx.backend_ctx);

    Mesh *m = NULL;
    Material *mat = NULL;
    Mat4 ident = { .col = { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} } };

    gfx_draw_mesh(&ctx, m, ident, mat);

    ctx.backend.end_frame(ctx.backend_ctx);

    printf("Demo finalizado.\n");
    return 0;
}
