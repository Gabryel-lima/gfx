#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <time.h>

#include "gfx.h"
#include "../src/internal/platform_window.h"

int main(void) {
    PlatformWindow *window;
    GfxContext *context;
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

    frame_delay.tv_sec = 0;
    frame_delay.tv_nsec = 16L * 1000L * 1000L;

    for (unsigned int frame = 0; frame < 240U && !gfx_platform_window_should_close(window); ++frame) {
        gfx_platform_window_pump_events(window);

        gfx_platform_window_set_clear_color(window,
                                            (float)((frame * 5U) % 255U) / 255.0f,
                                            (float)((frame * 3U + 85U) % 255U) / 255.0f,
                                            (float)((frame * 7U + 170U) % 255U) / 255.0f,
                                            1.0f);

        gfx_begin_frame(context);
        gfx_end_frame(context);

        nanosleep(&frame_delay, NULL);
    }

    gfx_platform_window_destroy(window);
    printf("gfx_window_demo finalizado.\n");
    return 0;
}
