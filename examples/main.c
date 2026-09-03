#include <stdio.h>
#include "gfx.h"

int main(void) {
    GfxContext ctx;
    gfx_context_init(&ctx, gfx_get_stub_backend(), NULL);

    Mat4 ident = mat4_identity();

    gfx_begin_frame(&ctx);
    gfx_draw_mesh(&ctx, NULL, ident);
    gfx_end_frame(&ctx);
    gfx_cleanup(&ctx);

    printf("Demo finalizado.\n");
    return 0;
}
