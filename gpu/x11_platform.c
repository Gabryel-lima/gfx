#include <dlfcn.h>

#include "../src/internal/x11_platform.h"

PlatformGL gfx_platform_gl_init(void) {
    PlatformGL p = {0};
    p.x11 = dlopen("libX11.so",  RTLD_LAZY);
    p.gl  = dlopen("libGL.so.1", RTLD_LAZY);

    p.XOpenDisplay        = dlsym(p.x11, "XOpenDisplay");
    p.XCreateSimpleWindow = dlsym(p.x11, "XCreateSimpleWindow");
    p.glXCreateContext    = dlsym(p.gl,  "glXCreateContext");
    p.glXMakeCurrent      = dlsym(p.gl,  "glXMakeCurrent");
    return p;
}
