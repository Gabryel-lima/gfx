#define _XOPEN_SOURCE 700  // Necessário para obter definições de tipos e funções do X11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/internal/gl_loader.h"
#include "../src/internal/platform_window.h"
#include "../src/internal/x11_platform.h"

#define GFX_GLX_RGBA 4  // Tipo de visual RGBA
#define GFX_GLX_DOUBLEBUFFER 5  // Visual com buffer duplo
#define GFX_GLX_DEPTH_SIZE 12   // Atributo para especificar tamanho do buffer de profundidade

#define GFX_GL_COLOR_BUFFER_BIT 0x00004000u // Máscara para limpar o buffer de cor
#define GFX_GL_DEPTH_BUFFER_BIT 0x00000100u // Máscara para limpar o buffer de profundidade

/** Estrutura que representa uma janela nativa Linux com contexto GLX. 
 *  @param platform Estrutura que encapsula o estado da janela, contexto GLX e recursos associados.
 *  @param gl Tabela de ponteiros para funções OpenGL carregadas dinamicamente.
 *  @param display Ponteiro para o display X11.
 *  @param window Identificador da janela X11.
 *  @param context Contexto GLX associado à janela.
 *  @param visual_info Informações sobre o visual X11 usado para criar a janela.
 *  @param colormap Colormap X11 usado pela janela.
 *  @param wm_protocols Atom para os protocolos de janela do gerenciador de janelas.
 *  @param wm_delete_window Atom para o protocolo de fechamento de janela.
 *  @param width Largura da janela em pixels.
 *  @param height Altura da janela em pixels.
 *  @param clear_color Cor de limpeza usada em `gfx_begin_frame`.
 *  @param should_close Flag que indica se a janela deve ser fechada.
 *  @param context_dispatch Contexto público que despacha para a janela.
 *  @note A estrutura é opaca e deve ser manipulada apenas por meio das funções definidas em `platform_window.h`.
*/
struct PlatformWindow {
    PlatformGL platform; // Plataforma GLX - Tabela de ponteiros para funções e handles de X11/GLX
    GLProcs gl;          // Tabela de ponteiros para funções OpenGL carregadas dinamicamente
    Display *display;    // Ponteiro para o display X11
    Window window;       // Identificador da janela X11
    GLXContext context;  // Contexto GLX associado à janela
    XVisualInfo *visual_info; // Informações sobre o visual X11 usado para criar a janela
    Colormap colormap;  // Colormap X11 usado pela janela
    Atom wm_protocols;  // Atom para os protocolos de janela do gerenciador de janelas
    Atom wm_delete_window;  // Atom para o protocolo de fechamento de janela
    unsigned int width; // Largura da janela em pixels
    unsigned int height; // Altura da janela em pixels
    float clear_color[4]; // Cor de limpeza usada em `gfx_begin_frame`
    int should_close; // Flag que indica se a janela deve ser fechada
    GfxContext context_dispatch; // Contexto público que despacha para a janela
};

/** Desenha uma malha usando o backend da janela. 
 *  @param ctx Ponteiro para o contexto da janela.
 *  @param mesh Ponteiro para a malha a ser desenhada.
 *  @param transform Matriz de transformação a ser aplicada à malha.
 *  @param mat Material a ser usado para desenhar a malha.
 *  @note Esta função é um stub e não implementa realmente o desenho de malhas. 
 *  Ela deve ser implementada posteriormente para suportar renderização real.
*/
static void gfx_platform_window_draw_mesh(void *ctx, Mesh *mesh, Mat4 transform, Material *mat) {
    // TODO: Implementar renderização de malhas usando OpenGL
    (void)ctx;
    (void)mesh;
    (void)transform;
    (void)mat;
}

static void gfx_platform_window_set_camera(void *ctx, Vec3 pos, Vec3 target, float fov) {
    // TODO: Implementar configuração de câmera usando OpenGL
    (void)ctx;
    (void)pos;
    (void)target;
    (void)fov;
}

/** Inicia um novo frame de renderização.
 *  @param ctx Ponteiro para o contexto da janela.
 *  @note Esta função deve ser chamada no início de cada frame de renderização.
 */
static void gfx_platform_window_begin_frame(void *ctx) {
    PlatformWindow *window = (PlatformWindow *)ctx;

    if (!window || !window->gl.ClearColor || !window->gl.Clear || !window->gl.Viewport) {
        return;
    }

    window->gl.Viewport(0, 0, (GLsizei)window->width, (GLsizei)window->height);
    window->gl.ClearColor(window->clear_color[0],
                          window->clear_color[1],
                          window->clear_color[2],
                          window->clear_color[3]);
    window->gl.Clear(GFX_GL_COLOR_BUFFER_BIT | GFX_GL_DEPTH_BUFFER_BIT);
}

/** Finaliza o frame de renderização.
 *  @param ctx Ponteiro para o contexto da janela.
 *  @note Esta função deve ser chamada no final de cada frame de renderização.
 */
static void gfx_platform_window_end_frame(void *ctx) {
    PlatformWindow *window = (PlatformWindow *)ctx;

    if (!window || !window->display || !window->platform.glXSwapBuffers) {
        return;
    }

    window->platform.glXSwapBuffers(window->display, window->window);
}

/** Retorna o backend da janela.
 *  @return Estrutura GfxBackend contendo os ponteiros para as funções do backend.
 */
static GfxBackend gfx_platform_window_backend(void) {
    GfxBackend backend = {
        gfx_platform_window_begin_frame,
        gfx_platform_window_end_frame,
        gfx_platform_window_draw_mesh,
        gfx_platform_window_set_camera,
    };
    return backend;
}

/** Libera os recursos associados à janela.
 *  @param window Ponteiro para a janela a ser liberada.
 */
static void gfx_platform_window_release(PlatformWindow *window) {
    if (!window) {
        return;
    }

    if (window->display && window->context && window->platform.glXMakeCurrent) {
        window->platform.glXMakeCurrent(window->display, 0, NULL);
    }

    if (window->display && window->context && window->platform.glXDestroyContext) {
        window->platform.glXDestroyContext(window->display, window->context);
        window->context = NULL;
    }

    if (window->display && window->window && window->platform.XDestroyWindow) {
        window->platform.XDestroyWindow(window->display, window->window);
        window->window = 0;
    }

    if (window->visual_info && window->platform.XFree) {
        window->platform.XFree(window->visual_info);
        window->visual_info = NULL;
    }

    if (window->display && window->colormap && window->platform.XFreeColormap) {
        window->platform.XFreeColormap(window->display, window->colormap);
        window->colormap = 0;
    }

    if (window->display && window->platform.XCloseDisplay) {
        window->platform.XCloseDisplay(window->display);
        window->display = NULL;
    }

    gfx_gl_close(&window->gl);
    gfx_platform_gl_close(&window->platform);
}

/** Valida se todos os ponteiros de função necessários estão disponíveis.
 *  @param window Ponteiro para a janela a ser validada.
 *  @return 1 se todos os ponteiros de função estiverem disponíveis, 0 caso contrário.
 */
static int gfx_platform_window_validate_procs(const PlatformWindow *window) {
    return window && window->platform.x11 && window->platform.gl &&
           window->platform.XOpenDisplay && window->platform.XCloseDisplay &&
           window->platform.XDefaultScreen && window->platform.XRootWindow &&
           window->platform.XBlackPixel && window->platform.XWhitePixel &&
           window->platform.XCreateColormap && window->platform.XFreeColormap &&
           window->platform.XCreateWindow &&
           window->platform.XStoreName && window->platform.XSelectInput &&
           window->platform.XInternAtom && window->platform.XSetWMProtocols &&
           window->platform.XMapWindow && window->platform.XPending &&
           window->platform.XNextEvent && window->platform.XDestroyWindow &&
           window->platform.XFlush && window->platform.XFree &&
           window->platform.glXChooseVisual && window->platform.glXCreateContext &&
           window->platform.glXDestroyContext && window->platform.glXMakeCurrent &&
           window->platform.glXSwapBuffers && window->gl.handle &&
           window->gl.ClearColor && window->gl.Clear && window->gl.Viewport;
}

/** Cria uma nova janela.
 *  @param title Título da janela.
 *  @param width Largura da janela em pixels.
 *  @param height Altura da janela em pixels.
 *  @return Ponteiro para a janela criada, ou NULL em caso de falha.
 */
PlatformWindow *gfx_platform_window_create(const char *title,
                                           unsigned int width,
                                           unsigned int height) {
    PlatformWindow *window;
    int visual_attribs[] = {
        GFX_GLX_RGBA,
        GFX_GLX_DOUBLEBUFFER,
        GFX_GLX_DEPTH_SIZE,
        24,
        None,
    };
    XSetWindowAttributes attributes;
    const char *window_title = (title && title[0]) ? title : "gfx window demo";
    Window root_window;
    int screen_index;

    if (width == 0 || height == 0) {
        fprintf(stderr, "gfx_platform_window_create: invalid window size %ux%u\n", width, height);
        return NULL;
    }

    window = (PlatformWindow *)calloc(1, sizeof(*window));
    if (!window) {
        fprintf(stderr, "gfx_platform_window_create: out of memory\n");
        return NULL;
    }

    window->platform = gfx_platform_gl_init();
    window->gl = gfx_gl_load();
    if (!gfx_platform_window_validate_procs(window)) {
        fprintf(stderr, "gfx_platform_window_create: failed to load X11/GLX/GL symbols\n");
        goto fail;
    }

    window->display = window->platform.XOpenDisplay(NULL);
    if (!window->display) {
        fprintf(stderr, "gfx_platform_window_create: XOpenDisplay failed\n");
        goto fail;
    }

    window->width = width;
    window->height = height;
    window->clear_color[0] = 0.10f;
    window->clear_color[1] = 0.12f;
    window->clear_color[2] = 0.18f;
    window->clear_color[3] = 1.0f;

    screen_index = window->platform.XDefaultScreen(window->display);
    root_window = window->platform.XRootWindow(window->display, screen_index);

    window->visual_info = window->platform.glXChooseVisual(window->display, screen_index, visual_attribs);
    if (!window->visual_info) {
        fprintf(stderr, "gfx_platform_window_create: glXChooseVisual failed\n");
        goto fail;
    }

    window->colormap = window->platform.XCreateColormap(window->display,
                                                        root_window,
                                                        window->visual_info->visual,
                                                        AllocNone);
    if (!window->colormap) {
        fprintf(stderr, "gfx_platform_window_create: XCreateColormap failed\n");
        goto fail;
    }

    memset(&attributes, 0, sizeof(attributes));
    attributes.border_pixel = 0;
    attributes.background_pixel = 0;
    attributes.colormap = window->colormap;
    attributes.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask;

    window->window = window->platform.XCreateWindow(window->display,
                                                    root_window,
                                                    0,
                                                    0,
                                                    width,
                                                    height,
                                                    0,
                                                    window->visual_info->depth,
                                                    InputOutput,
                                                    window->visual_info->visual,
                                                    CWBorderPixel | CWBackPixel | CWColormap | CWEventMask,
                                                    &attributes);
    if (!window->window) {
        fprintf(stderr, "gfx_platform_window_create: XCreateWindow failed\n");
        goto fail;
    }

    if (!window->platform.XSelectInput(window->display, window->window, attributes.event_mask)) {
        fprintf(stderr, "gfx_platform_window_create: XSelectInput failed\n");
        goto fail;
    }

    if (!window->platform.XStoreName(window->display, window->window, window_title)) {
        fprintf(stderr, "gfx_platform_window_create: XStoreName failed\n");
        goto fail;
    }

    window->wm_protocols = window->platform.XInternAtom(window->display, "WM_PROTOCOLS", False);
    window->wm_delete_window = window->platform.XInternAtom(window->display, "WM_DELETE_WINDOW", False);
    if (window->wm_protocols == None || window->wm_delete_window == None) {
        fprintf(stderr, "gfx_platform_window_create: XInternAtom failed\n");
        goto fail;
    }

    {
        Atom protocols[1];
        protocols[0] = window->wm_delete_window;
        if (!window->platform.XSetWMProtocols(window->display, window->window, protocols, 1)) {
            fprintf(stderr, "gfx_platform_window_create: XSetWMProtocols failed\n");
            goto fail;
        }
    }

    if (!window->platform.XMapWindow(window->display, window->window)) {
        fprintf(stderr, "gfx_platform_window_create: XMapWindow failed\n");
        goto fail;
    }

    window->context = window->platform.glXCreateContext(window->display,
                                                        window->visual_info,
                                                        NULL,
                                                        True);
    if (!window->context) {
        fprintf(stderr, "gfx_platform_window_create: glXCreateContext failed\n");
        goto fail;
    }

    if (!window->platform.glXMakeCurrent(window->display, window->window, window->context)) {
        fprintf(stderr, "gfx_platform_window_create: glXMakeCurrent failed\n");
        goto fail;
    }

    window->platform.XFlush(window->display);
    window->gl.Viewport(0, 0, (GLsizei)window->width, (GLsizei)window->height);

    window->context_dispatch.backend = gfx_platform_window_backend();
    window->context_dispatch.backend_ctx = window;

    return window;

fail:
    gfx_platform_window_release(window);
    free(window);
    return NULL;
}

/** Destroi a janela e libera os recursos associados.
 *  @param window Ponteiro para a janela a ser destruída.
 */
void gfx_platform_window_destroy(PlatformWindow *window) {
    if (!window) {
        return;
    }

    gfx_platform_window_release(window);
    free(window);
}

/** Processa os eventos da janela.
 *  @param window Ponteiro para a janela cujos eventos serão processados.
 *  @return 1 se a janela deve ser fechada, 0 caso contrário.
 */
int gfx_platform_window_pump_events(PlatformWindow *window) {
    XEvent event;

    if (!window || !window->display) {
        return 1;
    }

    while (window->platform.XPending(window->display) > 0) {
        window->platform.XNextEvent(window->display, &event);

        switch (event.type) {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == window->wm_delete_window &&
                    event.xclient.message_type == window->wm_protocols) {
                    window->should_close = 1;
                }
                break;

            case DestroyNotify:
                window->should_close = 1;
                break;

            case ConfigureNotify:
                if (event.xconfigure.width > 0 && event.xconfigure.height > 0) {
                    window->width = (unsigned int)event.xconfigure.width;
                    window->height = (unsigned int)event.xconfigure.height;
                }
                break;

            default:
                break;
        }
    }

    return window->should_close;
}

/** Verifica se a janela deve ser fechada.
 *  @param window Ponteiro para a janela a ser verificada.
 *  @return 1 se a janela deve ser fechada, 0 caso contrário.
 */
int gfx_platform_window_should_close(const PlatformWindow *window) {
    return window ? window->should_close : 1;
}

/** Define a cor de limpeza da janela.
 *  @param window Ponteiro para a janela.
 *  @param red Componente vermelho da cor.
 *  @param green Componente verde da cor.
 *  @param blue Componente azul da cor.
 *  @param alpha Componente alfa da cor.
 */
void gfx_platform_window_set_clear_color(PlatformWindow *window,
                                         float red,
                                         float green,
                                         float blue,
                                         float alpha) {
    if (!window) {
        return;
    }

    window->clear_color[0] = red;
    window->clear_color[1] = green;
    window->clear_color[2] = blue;
    window->clear_color[3] = alpha;
}

/** Retorna o contexto da janela.
 *  @param window Ponteiro para a janela.
 *  @return Ponteiro para o contexto da janela, ou NULL se a janela for inválida.
 */
GfxContext *gfx_platform_window_context(PlatformWindow *window) {
    if (!window) {
        return NULL;
    }

    return &window->context_dispatch;
}
