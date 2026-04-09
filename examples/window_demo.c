#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gfx.h"
#include "internal/mesh.h"
#include "tinyobj_utils.h"
#include "../src/internal/platform_window.h"

static int gfx_window_demo_resolve_model_path(char *out, size_t size) {
    char executable_path[TINYOBJ_UTILS_PATH_MAX];

    if (tinyobj_get_executable_path(executable_path, sizeof(executable_path)) == 0 &&
        tinyobj_resolve_path(executable_path, "../examples/models/triangle.obj", out, size) == 0) {
        return 0;
    }

    if (tinyobj_copy_path(out, size, "../examples/models/triangle.obj") == 0) {
        return 0;
    }

    return tinyobj_copy_path(out, size, "examples/models/triangle.obj");
}

int main(void) {
    PlatformWindow *window;
    GfxContext *context;
    Mesh *mesh;
    Mat4 model;
    char model_path[TINYOBJ_UTILS_PATH_MAX];
    struct timespec frame_delay;

    window = gfx_platform_window_create("gfx window demo", 960U, 540U);
    if (!window) {
        fprintf(stderr, "falha ao criar janela nativa\n");
        return 1;
    }

    context = gfx_platform_window_context(window);
    if (!context) {
        fprintf(stderr, "falha ao obter contexto da janela\n");
        gfx_platform_window_destroy(window);
        return 1;
    }

    if (gfx_window_demo_resolve_model_path(model_path, sizeof(model_path)) != 0) {
        fprintf(stderr, "falha ao resolver caminho do modelo\n");
        gfx_platform_window_destroy(window);
        return 1;
    }

    mesh = gfx_mesh_load(model_path);
    if (!mesh) {
        fprintf(stderr, "falha ao carregar malha: %s\n", model_path);
        gfx_platform_window_destroy(window);
        return 1;
    }

    model = (Mat4){
        .col = {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        }
    };

    gfx_set_camera(context,
                   (Vec3){ 0.5f, 0.5f, 2.5f },
                   (Vec3){ 0.5f, 0.5f, 0.0f },
                   45.0f);

    frame_delay.tv_sec = 0;
    frame_delay.tv_nsec = 16L * 1000L * 1000L;

    for (unsigned int frame = 0; frame < 120U && !gfx_platform_window_should_close(window); ++frame) {
        gfx_platform_window_pump_events(window);

        gfx_platform_window_set_clear_color(window, 0.08f, 0.10f, 0.15f, 1.0f);

        gfx_begin_frame(context);
        gfx_draw_mesh(context, mesh, model, NULL);
        gfx_end_frame(context);

        nanosleep(&frame_delay, NULL);
    }

    gfx_platform_window_destroy(window);
    gfx_mesh_free(mesh);
    printf("gfx_window_demo finalizado.\n");
    return 0;
}
