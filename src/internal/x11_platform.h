#pragma once

/**
 * @brief Wrappers mínimos para X11 e GLX carregados dinamicamente.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file src/internal/x11_platform.h
*/

/** Tipo para identificadores X (XID) */
typedef unsigned long  XID; // Identificador X11
/** Tipo para janelas X */
typedef XID            Window; // Handle de janela X11
/** Tipo para drawables GLX */
typedef XID            GLXDrawable; // Handle para GLX drawables
/** Tipo opaco para Display do X11 */
typedef void*          Display; // Ponteiro para Display X11
/** Evento X genérico (estrutura simplificada) */
typedef struct { long l[5]; } XEvent; // Representação simplificada de XEvent

/** Ponteiros para funções X11/GLX carregadas dinamicamente */
typedef Display* (*PFN_XOpenDisplay)    (const char*); // Abre Display
typedef Window   (*PFN_XCreateSimpleWindow)(Display*, Window, int, int,
                                            unsigned, unsigned, unsigned,
                                            unsigned long, unsigned long); // Cria janela simples
typedef void*    (*PFN_glXCreateContext) (Display*, void*, void*, int); // Cria contexto GLX
typedef void     (*PFN_glXMakeCurrent)   (Display*, GLXDrawable, void*); // Faz contexto atual

/** Tabela com ponteiros e handles para X11/GLX.
 * @param x11 Handle para biblioteca X11 (usado para dlclose)
 * @param gl Handle para biblioteca GL/GLX (usado para dlclose)
 * @param XOpenDisplay Ponteiro para função XOpenDisplay
 * @param XCreateSimpleWindow Ponteiro para função XCreateSimpleWindow
 * @param glXCreateContext Ponteiro para função glXCreateContext
 * @param glXMakeCurrent Ponteiro para função glXMakeCurrent
*/
typedef struct PlatformGL {
    void *x11; /**< Handle para biblioteca X11 */
    void *gl;  /**< Handle para biblioteca GL/GLX */
    PFN_XOpenDisplay         XOpenDisplay;        /**< Ponteiro para XOpenDisplay */
    PFN_XCreateSimpleWindow  XCreateSimpleWindow; /**< Ponteiro para XCreateSimpleWindow */
    PFN_glXCreateContext     glXCreateContext;    /**< Ponteiro para glXCreateContext */
    PFN_glXMakeCurrent       glXMakeCurrent;      /**< Ponteiro para glXMakeCurrent */
} PlatformGL; // Abstração mínima da plataforma para GL

/** Carrega X11 e GL/GLX e preenche PlatformGL. */
PlatformGL gfx_platform_gl_init(void);
