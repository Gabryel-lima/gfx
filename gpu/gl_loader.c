#include <dlfcn.h>   // dlopen, dlsym — POSIX puro

#include "../src/internal/gl_loader.h"

GLProcs gfx_gl_load(void) {
    GLProcs gl = {0};
    gl.handle = dlopen("libGL.so.1", RTLD_LAZY);

    // Macro para reduzir boilerplate
    #define GL_LOAD(name) gl.name = dlsym(gl.handle, "gl" #name)
    GL_LOAD(GenBuffers);
    GL_LOAD(BindBuffer);
    GL_LOAD(BufferData);
    GL_LOAD(CreateShader);
    #undef GL_LOAD

    return gl;
}
