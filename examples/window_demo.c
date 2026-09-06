#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "gfx.h"
#include "gfx_platform_gl_window.h"
#include "tinyobj_utils.h"

/** Modelo desenhado pela demo.
 *
 *  Um cubo, e não um triângulo plano: com uma única face não há como ver que
 *  a iluminação funciona — a normal é constante e a superfície inteira sai da
 *  mesma cor. O cube.obj tem um só material justamente para que qualquer
 *  diferença entre as faces venha do sombreamento.
 */
#define GFX_WINDOW_DEMO_MODEL "examples/models/cube.obj"

/** Número de frames antes de a demo encerrar sozinha.
 *  @note Ela é usada como teste de fumaça no CTest, então precisa terminar.
 */
#define GFX_WINDOW_DEMO_FRAMES 120U

/** Frame em que as amostras de pixel são coletadas.
 *  Tarde o bastante para o cubo já ter girado o suficiente para mostrar mais
 *  de uma face, e cedo o bastante para sobrar frame depois.
 */
#define GFX_WINDOW_DEMO_SAMPLE_FRAME 60U

static int gfx_window_demo_resolve_model_path(char *out, size_t size) {
    char executable_path[TINYOBJ_UTILS_PATH_MAX];

    if (tinyobj_get_executable_path(executable_path, sizeof(executable_path)) == 0 &&
        tinyobj_resolve_path(executable_path, "../" GFX_WINDOW_DEMO_MODEL, out, size) == 0) {
        return 0;
    }

    if (tinyobj_copy_path(out, size, "../" GFX_WINDOW_DEMO_MODEL) == 0) {
        return 0;
    }

    return tinyobj_copy_path(out, size, GFX_WINDOW_DEMO_MODEL);
}

/** Matriz de modelo que gira o cubo em Y e em X.
 *  @param angle Ângulo em radianos.
 *  @return Matriz de modelo em colunas.
 *  @note Girar importa: é o que faz faces de orientações diferentes passarem
 *  pela luz, e portanto o que torna o sombreamento observável.
 */
static Mat4 gfx_window_demo_model_matrix(float angle) {
    float sin_y = (float)sin((double)angle);
    float cos_y = (float)cos((double)angle);
    float sin_x = (float)sin((double)angle * 0.35);
    float cos_x = (float)cos((double)angle * 0.35);
    Mat4 model = mat4_identity();

    /* Rotação em Y composta com rotação em X, escrita direto nas colunas. */
    model.col[0] = (Vec4){ cos_y,  sin_y * sin_x, -sin_y * cos_x, 0.0f };
    model.col[1] = (Vec4){ 0.0f,   cos_x,          sin_x,         0.0f };
    model.col[2] = (Vec4){ sin_y, -cos_y * sin_x,  cos_y * cos_x, 0.0f };
    model.col[3] = (Vec4){ 0.0f,   0.0f,           0.0f,          1.0f };
    return model;
}

/** Luminância percebida de uma cor linear.
 *  @param rgb Componentes em [0,1].
 *  @return Luminância aproximada.
 */
static float gfx_window_demo_luminance(const float rgb[3]) {
    return 0.2126f * rgb[0] + 0.7152f * rgb[1] + 0.0722f * rgb[2];
}

/** Variação mínima de luminância que conta como "há iluminação".
 *
 *  Sem sombreamento, todas as faces sairiam com a cor crua do material e a
 *  variação seria zero. O limiar é folgado de propósito: ele existe para
 *  pegar uma regressão que desligue a iluminação, não para fixar um valor
 *  exato que mudaria a cada ajuste de luz ou de ângulo.
 */
#define GFX_WINDOW_DEMO_MIN_SHADING_DELTA 0.05f

/** Varre uma grade de pontos sobre o cubo e resume o sombreamento observado.
 *
 *  Todas as faces do cube.obj usam o mesmo material, então a variação de
 *  luminância entre os pontos só pode vir da iluminação. Uma grade é mais
 *  robusta que dois pontos fixos: qual face cai em qual pixel depende do
 *  ângulo de rotação, e amostrar largo evita depender disso.
 *
 *  Imprimir o resultado é o que torna o backend verificável por script — o
 *  alvo roda sob Xvfb no CTest, onde não há ninguém para olhar a janela.
 *
 *  @param window Janela com o frame corrente ainda no back buffer.
 */
static int gfx_window_demo_sample_shading(PlatformWindow *window) {
    /* Região central da janela 960x540; o cubo ocupa cerca de 260 px. */
    const unsigned int origin_x = 380U;
    const unsigned int origin_y = 170U;
    const unsigned int span = 200U;
    const unsigned int steps = 9U;

    float minimum = 2.0f;
    float maximum = -1.0f;
    unsigned int samples = 0U;
    unsigned int row;
    unsigned int column;

    for (row = 0U; row < steps; ++row) {
        for (column = 0U; column < steps; ++column) {
            float rgb[3];
            float luminance;

            unsigned int x = origin_x + column * (span / (steps - 1U));
            unsigned int y = origin_y + row * (span / (steps - 1U));

            if (gfx_platform_window_read_pixel(window, x, y, rgb) != 0) {
                continue;
            }

            /* Descarta o fundo: sem isso o "mais escuro" seria sempre a cor
             * de limpeza, e a variação medida não diria nada sobre a luz. */
            if (rgb[0] < 0.20f && rgb[1] < 0.20f && rgb[2] < 0.25f) {
                continue;
            }

            luminance = gfx_window_demo_luminance(rgb);
            if (luminance < minimum) {
                minimum = luminance;
            }
            if (luminance > maximum) {
                maximum = luminance;
            }
            samples++;
        }
    }

    if (samples == 0U) {
        fprintf(stderr, "gfx_window_demo: nenhuma amostra atingiu o cubo\n");
        return 1;
    }

    printf("gfx_window_demo: shading amostras=%u min=%.4f max=%.4f delta=%.4f\n",
           samples, minimum, maximum, maximum - minimum);

    if ((maximum - minimum) < GFX_WINDOW_DEMO_MIN_SHADING_DELTA) {
        fprintf(stderr,
                "gfx_window_demo: sombreamento plano (delta=%.4f < %.4f); "
                "a iluminacao nao esta chegando ao fragmento\n",
                maximum - minimum, GFX_WINDOW_DEMO_MIN_SHADING_DELTA);
        return 1;
    }

    return 0;
}

int main(void) {
    PlatformWindow *window;
    GfxContext *context;
    Mesh *mesh;
    char model_path[TINYOBJ_UTILS_PATH_MAX];
    struct timespec frame_delay;
    int shading_failed = 0;

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

    gfx_set_camera(context,
                   (Vec3){ 0.0f, 0.0f, 2.5f },
                   (Vec3){ 0.0f, 0.0f, 0.0f },
                   45.0f);

    /* A luz padrão já vem de cima e da frente; explicitá-la aqui documenta o
     * que a demo está exercitando e serve de exemplo de uso da API. */
    gfx_platform_window_set_light(window, gfx_platform_window_default_light());

    frame_delay.tv_sec = 0;
    frame_delay.tv_nsec = 16L * 1000L * 1000L;

    for (unsigned int frame = 0;
         frame < GFX_WINDOW_DEMO_FRAMES && !gfx_platform_window_should_close(window);
         ++frame) {
        Mat4 model = gfx_window_demo_model_matrix((float)frame * 0.03f);

        gfx_platform_window_pump_events(window);

        gfx_platform_window_set_clear_color(window, 0.08f, 0.10f, 0.15f, 1.0f);

        gfx_begin_frame(context);
        gfx_draw_mesh(context, mesh, model);

        /* Antes do end_frame: a leitura pega o back buffer, que o swap troca. */
        if (frame == GFX_WINDOW_DEMO_SAMPLE_FRAME) {
            shading_failed = gfx_window_demo_sample_shading(window);
        }

        gfx_end_frame(context);

        nanosleep(&frame_delay, NULL);
    }

    gfx_cleanup(context);
    gfx_mesh_free(mesh);

    if (shading_failed) {
        return 1;
    }

    printf("gfx_window_demo finalizado.\n");
    return 0;
}
