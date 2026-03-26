#pragma once

/**
 * @file include/x11_platform.h
 * @brief Tipos e wrappers mínimos para interação com X11 e GLX.
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

/** Estrutura com ponteiros para funções X11/GLX e handles */
typedef struct {
    void *x11; /**< Handle para biblioteca X11 */
    void *gl;  /**< Handle para biblioteca GL/GLX */
    PFN_XOpenDisplay         XOpenDisplay;        /**< Ponteiro para XOpenDisplay */
    PFN_XCreateSimpleWindow  XCreateSimpleWindow; /**< Ponteiro para XCreateSimpleWindow */
    PFN_glXCreateContext     glXCreateContext;    /**< Ponteiro para glXCreateContext */
    PFN_glXMakeCurrent       glXMakeCurrent;      /**< Ponteiro para glXMakeCurrent */
} PlatformGL; // Abstração mínima da plataforma para GL

/** Inicializa a abstração PlatformGL carregando funções necessárias. */
PlatformGL platform_gl_init(void);
