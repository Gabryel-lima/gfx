#include <dlfcn.h>   // dlopen, dlsym — POSIX puro
#include <stdio.h>
#include <string.h>

#include "../src/internal/gl_loader.h"

/** Fecha o handle do OpenGL e limpa a estrutura GLProcs.
 *  @param gl Ponteiro para a estrutura GLProcs a ser liberada.
 */
static void gfx_gl_close_handle(GLProcs *gl) {
    if (gl && gl->handle) {
        dlclose(gl->handle);
        gl->handle = NULL;
    }

    if (gl) {
        memset(gl, 0, sizeof(*gl));
    }
}

GLProcs gfx_gl_load(void) {
    GLProcs gl = {0};
    const char *error = NULL;

    gl.handle = dlopen("libGL.so.1", RTLD_NOW | RTLD_LOCAL);

    if (!gl.handle) {
        fprintf(stderr, "gfx_gl_load: dlopen(libGL.so.1) failed: %s\n", dlerror());
        return gl;
    }

    /** Carrega os ponteiros de função do OpenGL.
     *  @param gl Ponteiro para a estrutura GLProcs onde os ponteiros serão armazenados.
     *  @note Esta macro deve ser usada para cada função do OpenGL que deseja carregar.
     */
    #define GL_LOAD(name)  do { \
        dlerror(); \
        gl.name = (PFN_gl##name)dlsym(gl.handle, "gl" #name); \
        error = dlerror(); \
        if (error) { \
            fprintf(stderr, "gfx_gl_load: dlsym(gl%s) failed: %s\n", #name, error); \
        } \
    } while (0) 
    GL_LOAD(GenBuffers);
    GL_LOAD(BindBuffer);
    GL_LOAD(BufferData);
    GL_LOAD(ClearColor);
    GL_LOAD(Clear);
    GL_LOAD(Viewport);
    GL_LOAD(Enable);
    GL_LOAD(DepthFunc);
    GL_LOAD(CreateShader);
    GL_LOAD(ShaderSource);
    GL_LOAD(CompileShader);
    GL_LOAD(GetShaderiv);
    GL_LOAD(GetShaderInfoLog);
    GL_LOAD(DeleteShader);
    GL_LOAD(CreateProgram);
    GL_LOAD(BindAttribLocation);
    GL_LOAD(AttachShader);
    GL_LOAD(LinkProgram);
    GL_LOAD(GetProgramiv);
    GL_LOAD(GetProgramInfoLog);
    GL_LOAD(DeleteProgram);
    GL_LOAD(UseProgram);
    GL_LOAD(GetUniformLocation);
    GL_LOAD(UniformMatrix4fv);
    GL_LOAD(DeleteBuffers);
    GL_LOAD(EnableVertexAttribArray);
    GL_LOAD(VertexAttribPointer);
    GL_LOAD(DrawArrays);
    #undef GL_LOAD

    return gl;
}

/** Fecha o handle do OpenGL e limpa a estrutura GLProcs.
 *  @param gl Ponteiro para a estrutura GLProcs a ser liberada.
 */
void gfx_gl_close(GLProcs *gl) {
    gfx_gl_close_handle(gl);
}
