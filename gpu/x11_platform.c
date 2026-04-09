#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "../src/internal/x11_platform.h"

/** Plataforma GLX  - Tabela de ponteiros para funções e handles de X11/GLX.
 *  @param platform Ponteiro para a estrutura de plataforma a ser liberada.
*/
static void gfx_platform_gl_dispose(PlatformGL *platform) {
    if (!platform) {
        return;
    }

    if (platform->gl) {
        dlclose(platform->gl);
        platform->gl = NULL;
    }

    if (platform->x11) {
        dlclose(platform->x11);
        platform->x11 = NULL;
    }

    memset(platform, 0, sizeof(*platform));
}

/** Inicializa a plataforma GLX e carrega as funções necessárias. 
 *  @return Estrutura `PlatformGL` preenchida com os ponteiros para as funções, ou uma estrutura zerada em caso de falha.
*/
PlatformGL gfx_platform_gl_init(void) {
    PlatformGL p = {0};
    const char *error = NULL;

    p.x11 = dlopen("libX11.so", RTLD_NOW | RTLD_LOCAL);
    if (!p.x11) {
        fprintf(stderr, "gfx_platform_gl_init: dlopen(libX11.so) failed: %s\n", dlerror());
        return p;
    }

    p.gl = dlopen("libGL.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!p.gl) {
        fprintf(stderr, "gfx_platform_gl_init: dlopen(libGL.so.1) failed: %s\n", dlerror());
        gfx_platform_gl_dispose(&p);
        return p;
    }

    /** Carrega uma função necessária da biblioteca dinâmica.
     *  @param handle Handle da biblioteca.
     *  @param dst Ponteiro para a função a ser carregada.
     *  @param type Tipo da função.
     *  @param symbol Nome da função.
     *  @param library Nome da biblioteca.
     */
    #define LOAD_REQUIRED(handle, dst, type, symbol, library) do { \
        dlerror(); \
        (dst) = (type)dlsym((handle), (symbol)); \
        error = dlerror(); \
        if (error) { \
            fprintf(stderr, "gfx_platform_gl_init: dlsym(%s from %s) failed: %s\n", (symbol), (library), error); \
            gfx_platform_gl_dispose(&p); \
            return p; \
        } \
    } while (0)

    LOAD_REQUIRED(p.x11, p.XOpenDisplay, PFN_XOpenDisplay, "XOpenDisplay", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XCloseDisplay, PFN_XCloseDisplay, "XCloseDisplay", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XDefaultScreen, PFN_XDefaultScreen, "XDefaultScreen", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XRootWindow, PFN_XRootWindow, "XRootWindow", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XBlackPixel, PFN_XBlackPixel, "XBlackPixel", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XWhitePixel, PFN_XWhitePixel, "XWhitePixel", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XCreateColormap, PFN_XCreateColormap, "XCreateColormap", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XFreeColormap, PFN_XFreeColormap, "XFreeColormap", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XCreateWindow, PFN_XCreateWindow, "XCreateWindow", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XCreateSimpleWindow, PFN_XCreateSimpleWindow, "XCreateSimpleWindow", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XStoreName, PFN_XStoreName, "XStoreName", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XSelectInput, PFN_XSelectInput, "XSelectInput", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XInternAtom, PFN_XInternAtom, "XInternAtom", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XSetWMProtocols, PFN_XSetWMProtocols, "XSetWMProtocols", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XMapWindow, PFN_XMapWindow, "XMapWindow", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XPending, PFN_XPending, "XPending", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XNextEvent, PFN_XNextEvent, "XNextEvent", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XDestroyWindow, PFN_XDestroyWindow, "XDestroyWindow", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XFlush, PFN_XFlush, "XFlush", "libX11.so");
    LOAD_REQUIRED(p.x11, p.XFree, PFN_XFree, "XFree", "libX11.so");
    LOAD_REQUIRED(p.gl, p.glXChooseVisual, PFN_glXChooseVisual, "glXChooseVisual", "libGL.so.1");
    LOAD_REQUIRED(p.gl, p.glXCreateContext, PFN_glXCreateContext, "glXCreateContext", "libGL.so.1");
    LOAD_REQUIRED(p.gl, p.glXDestroyContext, PFN_glXDestroyContext, "glXDestroyContext", "libGL.so.1");
    LOAD_REQUIRED(p.gl, p.glXMakeCurrent, PFN_glXMakeCurrent, "glXMakeCurrent", "libGL.so.1");
    LOAD_REQUIRED(p.gl, p.glXSwapBuffers, PFN_glXSwapBuffers, "glXSwapBuffers", "libGL.so.1");

    #undef LOAD_REQUIRED
    return p;
}

/** Fecha a plataforma GLX e libera os recursos associados.
 *  @param platform Ponteiro para a estrutura de plataforma a ser fechada.
 */
void gfx_platform_gl_close(PlatformGL *platform) {
    gfx_platform_gl_dispose(platform);
}
