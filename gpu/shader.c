#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/internal/gl_loader.h"
#include "../src/internal/shader.h"

#define GFX_GL_VERTEX_SHADER   0x8B31u
#define GFX_GL_FRAGMENT_SHADER 0x8B30u
#define GFX_GL_COMPILE_STATUS  0x8B81u
#define GFX_GL_LINK_STATUS     0x8B82u
#define GFX_GL_INFO_LOG_LENGTH 0x8B84u

struct Shader {
    GLProcs gl;
    GLuint program;
};

static int gfx_shader_has_required_procs(const GLProcs *gl) {
    return gl && gl->handle &&
           gl->CreateShader && gl->ShaderSource && gl->CompileShader &&
           gl->GetShaderiv && gl->GetShaderInfoLog && gl->DeleteShader &&
           gl->CreateProgram && gl->AttachShader && gl->LinkProgram &&
           gl->GetProgramiv && gl->GetProgramInfoLog && gl->DeleteProgram;
}

static void gfx_shader_close_gl(GLProcs *gl) {
    if (gl && gl->handle) {
        dlclose(gl->handle);
        gl->handle = NULL;
    }
}

static GLuint gfx_shader_compile_stage(const GLProcs *gl,
                                       GLenum stage,
                                       const char *source,
                                       const char *stage_name) {
    GLuint shader_id;
    GLint status = 0;
    GLint log_length = 0;
    const char *sources[1];

    if (!gl || !source || !source[0]) {
        return 0;
    }

    shader_id = gl->CreateShader(stage);
    if (!shader_id) {
        return 0;
    }

    sources[0] = source;
    gl->ShaderSource(shader_id, 1, sources, NULL);
    gl->CompileShader(shader_id);
    gl->GetShaderiv(shader_id, GFX_GL_COMPILE_STATUS, &status);

    if (status == 0) {
        gl->GetShaderiv(shader_id, GFX_GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 1) {
            char *log = (char *)malloc((size_t)log_length + 1U);
            if (log) {
                GLsizei written = 0;
                gl->GetShaderInfoLog(shader_id, (GLsizei)log_length, &written, log);
                if (written < 0) {
                    written = 0;
                }
                log[(size_t)written] = '\0';
                fprintf(stderr, "[gfx_shader] %s compile failed:\n%s\n", stage_name, log);
                free(log);
            }
        }
        gl->DeleteShader(shader_id);
        return 0;
    }

    return shader_id;
}

static void gfx_shader_print_program_log(const GLProcs *gl, GLuint program_id) {
    GLint log_length = 0;

    if (!gl || !gl->GetProgramiv || !gl->GetProgramInfoLog) {
        return;
    }

    gl->GetProgramiv(program_id, GFX_GL_INFO_LOG_LENGTH, &log_length);
    if (log_length <= 1) {
        return;
    }

    {
        char *log = (char *)malloc((size_t)log_length + 1U);
        if (!log) {
            return;
        }

        {
            GLsizei written = 0;
            gl->GetProgramInfoLog(program_id, (GLsizei)log_length, &written, log);
            if (written < 0) {
                written = 0;
            }
            log[(size_t)written] = '\0';
            fprintf(stderr, "[gfx_shader] program link failed:\n%s\n", log);
        }

        free(log);
    }
}

Shader *gfx_shader_create_from_source(const char *vert_src, const char *frag_src) {
    GLProcs gl;
    Shader *shader = NULL;
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    GLuint program = 0;

    if (!vert_src || !frag_src || !vert_src[0] || !frag_src[0]) {
        return NULL;
    }

    gl = gfx_gl_load();
    if (!gfx_shader_has_required_procs(&gl)) {
        gfx_shader_close_gl(&gl);
        return NULL;
    }

    shader = (Shader *)calloc(1, sizeof(*shader));
    if (!shader) {
        gfx_shader_close_gl(&gl);
        return NULL;
    }

    vertex_shader = gfx_shader_compile_stage(&gl, GFX_GL_VERTEX_SHADER, vert_src, "vertex");
    if (!vertex_shader) {
        goto fail;
    }

    fragment_shader = gfx_shader_compile_stage(&gl, GFX_GL_FRAGMENT_SHADER, frag_src, "fragment");
    if (!fragment_shader) {
        goto fail;
    }

    program = gl.CreateProgram();
    if (!program) {
        goto fail;
    }

    gl.AttachShader(program, vertex_shader);
    gl.AttachShader(program, fragment_shader);
    gl.LinkProgram(program);

    {
        GLint link_status = 0;
        gl.GetProgramiv(program, GFX_GL_LINK_STATUS, &link_status);
        if (link_status == 0) {
            gfx_shader_print_program_log(&gl, program);
            gl.DeleteProgram(program);
            program = 0;
            goto fail;
        }
    }

    gl.DeleteShader(vertex_shader);
    gl.DeleteShader(fragment_shader);

    shader->gl = gl;
    shader->program = program;
    return shader;

fail:
    if (vertex_shader) {
        gl.DeleteShader(vertex_shader);
    }
    if (fragment_shader) {
        gl.DeleteShader(fragment_shader);
    }
    if (program) {
        gl.DeleteProgram(program);
    }
    free(shader);
    gfx_shader_close_gl(&gl);
    return NULL;
}

void gfx_shader_destroy(Shader *shader) {
    if (!shader) {
        return;
    }

    if (shader->program && shader->gl.DeleteProgram) {
        shader->gl.DeleteProgram(shader->program);
    }

    gfx_shader_close_gl(&shader->gl);
    free(shader);
}
