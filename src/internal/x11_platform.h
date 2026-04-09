#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>

/**
 * @brief Abstração mínima para carregar X11/GLX dinamicamente no Linux.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file src/internal/x11_platform.h
*/

typedef void *GLXContext;  // Tipo opaco para contexto GLX
typedef unsigned long GLXDrawable;  // Tipo opaco para drawable GLX

/** Ponteiros para funções X11/GLX carregadas dinamicamente. */
typedef Display *(*PFN_XOpenDisplay)(const char *); // Ponteiro para XOpenDisplay
typedef int      (*PFN_XCloseDisplay)(Display *);   // Ponteiro para XCloseDisplay
typedef int      (*PFN_XDefaultScreen)(Display *);  // Ponteiro para XDefaultScreen
typedef Window   (*PFN_XRootWindow)(Display *, int); // Ponteiro para XRootWindow
typedef unsigned long (*PFN_XBlackPixel)(Display *, int); // Ponteiro para XBlackPixel
typedef unsigned long (*PFN_XWhitePixel)(Display *, int); // Ponteiro para XWhitePixel
typedef Colormap (*PFN_XCreateColormap)(Display *, Window, Visual *, int); // Ponteiro para XCreateColormap
typedef int      (*PFN_XFreeColormap)(Display *, Colormap); // Ponteiro para XFreeColormap
typedef Window   (*PFN_XCreateWindow)(Display *, Window, int, int, unsigned int,
                                      unsigned int, unsigned int, int, unsigned int,
                                      Visual *, unsigned long, XSetWindowAttributes *); // Ponteiro para XCreateWindow
typedef Window   (*PFN_XCreateSimpleWindow)(Display *, Window, int, int,
                                            unsigned int, unsigned int, unsigned int,
                                            unsigned long, unsigned long); // Ponteiro para XCreateSimpleWindow
typedef int      (*PFN_XStoreName)(Display *, Window, const char *); // Ponteiro para XStoreName
typedef int      (*PFN_XSelectInput)(Display *, Window, long); // Ponteiro para XSelectInput
typedef Atom     (*PFN_XInternAtom)(Display *, const char *, Bool); // Ponteiro para XInternAtom
typedef int      (*PFN_XSetWMProtocols)(Display *, Window, Atom *, int); // Ponteiro para XSetWMProtocols
typedef int      (*PFN_XMapWindow)(Display *, Window); // Ponteiro para XMapWindow
typedef int      (*PFN_XPending)(Display *); // Ponteiro para XPending
typedef int      (*PFN_XNextEvent)(Display *, XEvent *); // Ponteiro para XNextEvent
typedef int      (*PFN_XDestroyWindow)(Display *, Window); // Ponteiro para XDestroyWindow
typedef int      (*PFN_XFlush)(Display *); // Ponteiro para XFlush
typedef int      (*PFN_XFree)(void *); // Ponteiro para XFree
typedef XVisualInfo *(*PFN_glXChooseVisual)(Display *, int, int *); // Ponteiro para glXChooseVisual
typedef GLXContext (*PFN_glXCreateContext)(Display *, XVisualInfo *, GLXContext, Bool); // Ponteiro para glXCreateContext
typedef void     (*PFN_glXDestroyContext)(Display *, GLXContext); // Ponteiro para glXDestroyContext
typedef Bool     (*PFN_glXMakeCurrent)(Display *, GLXDrawable, GLXContext); // Ponteiro para glXMakeCurrent
typedef void     (*PFN_glXSwapBuffers)(Display *, GLXDrawable); // Ponteiro para glXSwapBuffers

/** Tabela com ponteiros e handles para X11/GLX. */
typedef struct PlatformGL {
    void *x11; /**< Handle para biblioteca X11 */
    void *gl;  /**< Handle para biblioteca GL/GLX */
    PFN_XOpenDisplay         XOpenDisplay;         /**< Ponteiro para XOpenDisplay */
    PFN_XCloseDisplay        XCloseDisplay;        /**< Ponteiro para XCloseDisplay */
    PFN_XDefaultScreen       XDefaultScreen;       /**< Ponteiro para XDefaultScreen */
    PFN_XRootWindow          XRootWindow;          /**< Ponteiro para XRootWindow */
    PFN_XBlackPixel          XBlackPixel;          /**< Ponteiro para XBlackPixel */
    PFN_XWhitePixel          XWhitePixel;          /**< Ponteiro para XWhitePixel */
    PFN_XCreateColormap      XCreateColormap;      /**< Ponteiro para XCreateColormap */
    PFN_XFreeColormap        XFreeColormap;        /**< Ponteiro para XFreeColormap */
    PFN_XCreateWindow        XCreateWindow;        /**< Ponteiro para XCreateWindow */
    PFN_XCreateSimpleWindow  XCreateSimpleWindow;  /**< Ponteiro para XCreateSimpleWindow */
    PFN_XStoreName           XStoreName;           /**< Ponteiro para XStoreName */
    PFN_XSelectInput         XSelectInput;         /**< Ponteiro para XSelectInput */
    PFN_XInternAtom          XInternAtom;          /**< Ponteiro para XInternAtom */
    PFN_XSetWMProtocols      XSetWMProtocols;      /**< Ponteiro para XSetWMProtocols */
    PFN_XMapWindow           XMapWindow;           /**< Ponteiro para XMapWindow */
    PFN_XPending              XPending;            /**< Ponteiro para XPending */
    PFN_XNextEvent           XNextEvent;           /**< Ponteiro para XNextEvent */
    PFN_XDestroyWindow       XDestroyWindow;       /**< Ponteiro para XDestroyWindow */
    PFN_XFlush               XFlush;               /**< Ponteiro para XFlush */
    PFN_XFree                XFree;                /**< Ponteiro para XFree */
    PFN_glXChooseVisual      glXChooseVisual;      /**< Ponteiro para glXChooseVisual */
    PFN_glXCreateContext     glXCreateContext;     /**< Ponteiro para glXCreateContext */
    PFN_glXDestroyContext    glXDestroyContext;    /**< Ponteiro para glXDestroyContext */
    PFN_glXMakeCurrent       glXMakeCurrent;       /**< Ponteiro para glXMakeCurrent */
    PFN_glXSwapBuffers       glXSwapBuffers;       /**< Ponteiro para glXSwapBuffers */
} PlatformGL;

/** Carrega X11 e GL/GLX e preenche PlatformGL. */
PlatformGL gfx_platform_gl_init(void);

/** Libera os handles carregados por `gfx_platform_gl_init`. */
void gfx_platform_gl_close(PlatformGL *platform);
