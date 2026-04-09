#define _XOPEN_SOURCE 700  // Necessário para obter definições de tipos e funções do X11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "../src/internal/gl_loader.h"
#include "../src/internal/mesh.h"
#include "../src/internal/platform_window.h"
#include "../src/internal/x11_platform.h"

#define GFX_GLX_RGBA 4  // Tipo de visual RGBA
#define GFX_GLX_DOUBLEBUFFER 5  // Visual com buffer duplo
#define GFX_GLX_DEPTH_SIZE 12   // Atributo para especificar tamanho do buffer de profundidade

#define GFX_GL_ARRAY_BUFFER 0x8892u
#define GFX_GL_STATIC_DRAW 0x88E4u
#define GFX_GL_FLOAT 0x1406u
#define GFX_GL_FALSE 0u
#define GFX_GL_TRIANGLES 0x0004u
#define GFX_GL_DEPTH_TEST 0x0B71u
#define GFX_GL_LEQUAL 0x0203u

#define GFX_GL_VERTEX_SHADER 0x8B31u
#define GFX_GL_FRAGMENT_SHADER 0x8B30u
#define GFX_GL_COMPILE_STATUS 0x8B81u
#define GFX_GL_LINK_STATUS 0x8B82u
#define GFX_GL_INFO_LOG_LENGTH 0x8B84u

static const char *GFX_PLATFORM_WINDOW_VERTEX_SHADER =
    "#version 120\n"
    "attribute vec3 a_position;\n"
    "attribute vec3 a_color;\n"
    "uniform mat4 u_mvp;\n"
    "varying vec3 v_color;\n"
    "void main() {\n"
    "    v_color = a_color;\n"
    "    gl_Position = u_mvp * vec4(a_position, 1.0);\n"
    "}\n";

static const char *GFX_PLATFORM_WINDOW_FRAGMENT_SHADER =
    "#version 120\n"
    "varying vec3 v_color;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(v_color, 1.0);\n"
    "}\n";

#define GFX_GL_COLOR_BUFFER_BIT 0x00004000u // Máscara para limpar o buffer de cor
#define GFX_GL_DEPTH_BUFFER_BIT 0x00000100u // Máscara para limpar o buffer de profundidade

typedef struct MeshGpuRecord {
    const Mesh *mesh;           // Malha associada ao upload; ponteiro emprestado.
    GLuint position_buffer;     // VBO com posições em ordem triangulada.
    GLuint color_buffer;        // VBO com cores expandidas por vértice.
    size_t vertex_count;        // Número total de vértices no upload atual.
    struct MeshGpuRecord *next; // Próximo item da cache vinculada.
} MeshGpuRecord;

struct PlatformWindow {
    PlatformGL platform;        // Handles e funções X11/GLX carregadas dinamicamente.
    GLProcs gl;                 // Funções OpenGL usadas pelo backend.
    Display *display;           // Display X11 ativo.
    Window window;              // Janela X11 nativa.
    GLXContext context;         // Contexto GLX corrente.
    XVisualInfo *visual_info;   // Visual escolhido para a janela.
    Colormap colormap;          // Colormap associado ao visual.
    Atom wm_protocols;          // Atom WM_PROTOCOLS.
    Atom wm_delete_window;      // Atom WM_DELETE_WINDOW.
    unsigned int width;         // Largura atual da janela.
    unsigned int height;        // Altura atual da janela.
    float clear_color[4];       // Cor de limpeza aplicada em begin_frame.
    Vec3 camera_position;       // Posição da câmera usada para view/projection.
    Vec3 camera_target;         // Alvo da câmera.
    float camera_fov;           // Campo de visão em graus.
    Mat4 view_matrix;           // Matriz de visão calculada da câmera.
    Mat4 projection_matrix;     // Matriz de projeção calculada da câmera.
    int camera_dirty;           // Marca quando view/projection precisam ser recalculadas.
    GLuint shader_program;      // Programa GLSL usado para desenhar malhas.
    GLint mvp_location;         // Local do uniforme MVP no shader.
    MeshGpuRecord *mesh_records; // Cache de uploads GL por Mesh.
    int should_close;           // Pedido de fechamento da janela.
    GfxContext context_dispatch; // Contexto público que despacha para este backend.
};

static Vec3 gfx_platform_window_vec3_sub(Vec3 a, Vec3 b) {
    Vec3 result = { a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

static Vec3 gfx_platform_window_vec3_cross(Vec3 a, Vec3 b) {
    Vec3 result = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
    return result;
}

static float gfx_platform_window_vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static float gfx_platform_window_vec3_length(Vec3 v) {
    return sqrtf(gfx_platform_window_vec3_dot(v, v));
}

static Vec3 gfx_platform_window_vec3_normalize_or(Vec3 v, Vec3 fallback) {
    float length = gfx_platform_window_vec3_length(v);

    if (!(length > 0.0f) || length != length) {
        return fallback;
    }

    return (Vec3){ v.x / length, v.y / length, v.z / length };
}

static Mat4 gfx_platform_window_mat4_identity(void) {
    return (Mat4){
        .col = {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        }
    };
}

static Vec4 gfx_platform_window_mat4_mul_vec4(Mat4 matrix, Vec4 v) {
    Vec4 result;

    result.x = matrix.col[0].x * v.x + matrix.col[1].x * v.y + matrix.col[2].x * v.z + matrix.col[3].x * v.w;
    result.y = matrix.col[0].y * v.x + matrix.col[1].y * v.y + matrix.col[2].y * v.z + matrix.col[3].y * v.w;
    result.z = matrix.col[0].z * v.x + matrix.col[1].z * v.y + matrix.col[2].z * v.z + matrix.col[3].z * v.w;
    result.w = matrix.col[0].w * v.x + matrix.col[1].w * v.y + matrix.col[2].w * v.z + matrix.col[3].w * v.w;
    return result;
}

static Mat4 gfx_platform_window_mat4_mul(Mat4 a, Mat4 b) {
    Mat4 result;

    result.col[0] = gfx_platform_window_mat4_mul_vec4(a, b.col[0]);
    result.col[1] = gfx_platform_window_mat4_mul_vec4(a, b.col[1]);
    result.col[2] = gfx_platform_window_mat4_mul_vec4(a, b.col[2]);
    result.col[3] = gfx_platform_window_mat4_mul_vec4(a, b.col[3]);
    return result;
}

static Mat4 gfx_platform_window_mat4_look_at(Vec3 eye, Vec3 target) {
    Vec3 forward = gfx_platform_window_vec3_normalize_or(gfx_platform_window_vec3_sub(target, eye),
                                                         (Vec3){ 0.0f, 0.0f, -1.0f });
    Vec3 world_up = { 0.0f, 1.0f, 0.0f };
    Vec3 right = gfx_platform_window_vec3_cross(forward, world_up);

    if (gfx_platform_window_vec3_length(right) <= 0.000001f) {
        world_up = (Vec3){ 0.0f, 0.0f, 1.0f };
        right = gfx_platform_window_vec3_cross(forward, world_up);
    }

    right = gfx_platform_window_vec3_normalize_or(right, (Vec3){ 1.0f, 0.0f, 0.0f });
    Vec3 up = gfx_platform_window_vec3_cross(right, forward);

    Mat4 view = gfx_platform_window_mat4_identity();
    view.col[0] = (Vec4){ right.x, up.x, -forward.x, 0.0f };
    view.col[1] = (Vec4){ right.y, up.y, -forward.y, 0.0f };
    view.col[2] = (Vec4){ right.z, up.z, -forward.z, 0.0f };
    view.col[3] = (Vec4){
        -gfx_platform_window_vec3_dot(right, eye),
        -gfx_platform_window_vec3_dot(up, eye),
        gfx_platform_window_vec3_dot(forward, eye),
        1.0f,
    };
    return view;
}

static Mat4 gfx_platform_window_mat4_perspective(float fov_degrees, float aspect, float near_plane, float far_plane) {
    const float pi = 3.14159265358979323846f;
    float fov_radians;
    float focal_length;
    Mat4 projection = { 0 };

    if (!(aspect > 0.0f)) {
        aspect = 1.0f;
    }
    if (fov_degrees < 1.0f) {
        fov_degrees = 1.0f;
    }
    if (fov_degrees > 179.0f) {
        fov_degrees = 179.0f;
    }
    if (!(near_plane > 0.0f) || !(far_plane > near_plane)) {
        return gfx_platform_window_mat4_identity();
    }

    fov_radians = fov_degrees * (pi / 180.0f);
    focal_length = 1.0f / tanf(fov_radians * 0.5f);

    projection.col[0].x = focal_length / aspect;
    projection.col[1].y = focal_length;
    projection.col[2].z = (far_plane + near_plane) / (near_plane - far_plane);
    projection.col[2].w = -1.0f;
    projection.col[3].z = (2.0f * far_plane * near_plane) / (near_plane - far_plane);
    return projection;
}

static void gfx_platform_window_mat4_to_array(Mat4 matrix, GLfloat out[16]) {
    out[0] = matrix.col[0].x; out[1] = matrix.col[0].y; out[2] = matrix.col[0].z; out[3] = matrix.col[0].w;
    out[4] = matrix.col[1].x; out[5] = matrix.col[1].y; out[6] = matrix.col[1].z; out[7] = matrix.col[1].w;
    out[8] = matrix.col[2].x; out[9] = matrix.col[2].y; out[10] = matrix.col[2].z; out[11] = matrix.col[2].w;
    out[12] = matrix.col[3].x; out[13] = matrix.col[3].y; out[14] = matrix.col[3].z; out[15] = matrix.col[3].w;
}

static GLuint gfx_platform_window_compile_shader(const GLProcs *gl,
                                                 GLenum stage,
                                                 const char *source,
                                                 const char *stage_name) {
    GLuint shader_id;
    GLint status = 0;
    GLint log_length = 0;
    const char *sources[1];

    if (!gl || !gl->CreateShader || !gl->ShaderSource || !gl->CompileShader || !gl->GetShaderiv ||
        !gl->GetShaderInfoLog || !gl->DeleteShader || !source || !source[0]) {
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
                fprintf(stderr, "gfx_window_demo: %s shader compile failed:\n%s\n", stage_name, log);
                free(log);
            }
        }

        gl->DeleteShader(shader_id);
        return 0;
    }

    return shader_id;
}

static GLuint gfx_platform_window_create_program(PlatformWindow *window) {
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    GLuint program = 0;

    if (!window || !window->gl.CreateProgram || !window->gl.BindAttribLocation || !window->gl.AttachShader ||
        !window->gl.LinkProgram || !window->gl.GetProgramiv || !window->gl.GetProgramInfoLog ||
        !window->gl.DeleteProgram || !window->gl.DeleteShader || !window->gl.GetUniformLocation) {
        return 0;
    }

    vertex_shader = gfx_platform_window_compile_shader(&window->gl,
                                                       GFX_GL_VERTEX_SHADER,
                                                       GFX_PLATFORM_WINDOW_VERTEX_SHADER,
                                                       "vertex");
    if (!vertex_shader) {
        return 0;
    }

    fragment_shader = gfx_platform_window_compile_shader(&window->gl,
                                                         GFX_GL_FRAGMENT_SHADER,
                                                         GFX_PLATFORM_WINDOW_FRAGMENT_SHADER,
                                                         "fragment");
    if (!fragment_shader) {
        window->gl.DeleteShader(vertex_shader);
        return 0;
    }

    program = window->gl.CreateProgram();
    if (!program) {
        window->gl.DeleteShader(vertex_shader);
        window->gl.DeleteShader(fragment_shader);
        return 0;
    }

    window->gl.BindAttribLocation(program, 0U, "a_position");
    window->gl.BindAttribLocation(program, 1U, "a_color");
    window->gl.AttachShader(program, vertex_shader);
    window->gl.AttachShader(program, fragment_shader);
    window->gl.LinkProgram(program);

    {
        GLint link_status = 0;
        GLint log_length = 0;

        window->gl.GetProgramiv(program, GFX_GL_LINK_STATUS, &link_status);
        if (link_status == 0) {
            window->gl.GetProgramiv(program, GFX_GL_INFO_LOG_LENGTH, &log_length);
            if (log_length > 1) {
                char *log = (char *)malloc((size_t)log_length + 1U);
                if (log) {
                    GLsizei written = 0;
                    window->gl.GetProgramInfoLog(program, (GLsizei)log_length, &written, log);
                    if (written < 0) {
                        written = 0;
                    }
                    log[(size_t)written] = '\0';
                    fprintf(stderr, "gfx_window_demo: shader program link failed:\n%s\n", log);
                    free(log);
                }
            }

            window->gl.DeleteProgram(program);
            program = 0;
        }
    }

    window->gl.DeleteShader(vertex_shader);
    window->gl.DeleteShader(fragment_shader);

    if (program) {
        GLint mvp_location = window->gl.GetUniformLocation(program, "u_mvp");
        if (mvp_location < 0) {
            fprintf(stderr, "gfx_window_demo: shader uniform u_mvp not found\n");
            window->gl.DeleteProgram(program);
            program = 0;
        } else {
            window->mvp_location = mvp_location;
        }
    }

    return program;
}

static MeshGpuRecord *gfx_platform_window_find_mesh_record(PlatformWindow *window, const Mesh *mesh) {
    MeshGpuRecord *record;

    if (!window || !mesh) {
        return NULL;
    }

    for (record = window->mesh_records; record; record = record->next) {
        if (record->mesh == mesh) {
            return record;
        }
    }

    return NULL;
}

static void gfx_platform_window_release_mesh_cache(PlatformWindow *window) {
    MeshGpuRecord *record;

    if (!window) {
        return;
    }

    record = window->mesh_records;
    while (record) {
        MeshGpuRecord *next = record->next;

        if (window->gl.DeleteBuffers) {
            if (record->position_buffer) {
                window->gl.DeleteBuffers(1, &record->position_buffer);
            }
            if (record->color_buffer) {
                window->gl.DeleteBuffers(1, &record->color_buffer);
            }
        }

        free(record);
        record = next;
    }

    window->mesh_records = NULL;
}

static void gfx_platform_window_release_gl_resources(PlatformWindow *window) {
    if (!window) {
        return;
    }

    if (window->display && window->context && window->platform.glXMakeCurrent && window->window) {
        window->platform.glXMakeCurrent(window->display, window->window, window->context);
    }

    gfx_platform_window_release_mesh_cache(window);

    if (window->shader_program && window->gl.DeleteProgram) {
        window->gl.DeleteProgram(window->shader_program);
        window->shader_program = 0;
        window->mvp_location = -1;
    }
}

static void gfx_platform_window_update_camera(PlatformWindow *window) {
    float aspect;

    if (!window || !window->camera_dirty) {
        return;
    }

    aspect = (window->height > 0U) ? ((float)window->width / (float)window->height) : 1.0f;
    window->view_matrix = gfx_platform_window_mat4_look_at(window->camera_position, window->camera_target);
    window->projection_matrix = gfx_platform_window_mat4_perspective(window->camera_fov, aspect, 0.1f, 100.0f);
    window->camera_dirty = 0;
}

static int gfx_platform_window_prepare_mesh_record(PlatformWindow *window, Mesh *mesh, MeshGpuRecord **record_out) {
    MeshGpuRecord *record = NULL;
    const Vec3 *positions;
    const Vec3 *triangle_colors;
    Vec3 *expanded_colors = NULL;
    size_t vertex_count;
    size_t triangle_count;
    size_t triangle_index;

    if (!window || !mesh || !record_out || !window->gl.GenBuffers || !window->gl.BindBuffer ||
        !window->gl.BufferData) {
        return -1;
    }

    vertex_count = gfx_mesh_vertex_count(mesh);
    triangle_count = gfx_mesh_triangle_count(mesh);
    positions = gfx_mesh_positions(mesh);
    triangle_colors = gfx_mesh_triangle_colors(mesh);

    if (!positions || !triangle_colors || vertex_count == 0U || triangle_count == 0U ||
        vertex_count != triangle_count * 3U || vertex_count > (size_t)INT_MAX) {
        return -1;
    }

    expanded_colors = (Vec3 *)malloc(vertex_count * sizeof(*expanded_colors));
    if (!expanded_colors) {
        return -1;
    }

    for (triangle_index = 0; triangle_index < triangle_count; ++triangle_index) {
        Vec3 color = triangle_colors[triangle_index];
        size_t vertex_base = triangle_index * 3U;

        expanded_colors[vertex_base + 0U] = color;
        expanded_colors[vertex_base + 1U] = color;
        expanded_colors[vertex_base + 2U] = color;
    }

    record = (MeshGpuRecord *)calloc(1, sizeof(*record));
    if (!record) {
        free(expanded_colors);
        return -1;
    }

    window->gl.GenBuffers(1, &record->position_buffer);
    window->gl.BindBuffer(GFX_GL_ARRAY_BUFFER, record->position_buffer);
    window->gl.BufferData(GFX_GL_ARRAY_BUFFER,
                          (GLsizeiptr)(vertex_count * sizeof(*positions)),
                          positions,
                          GFX_GL_STATIC_DRAW);

    window->gl.GenBuffers(1, &record->color_buffer);
    window->gl.BindBuffer(GFX_GL_ARRAY_BUFFER, record->color_buffer);
    window->gl.BufferData(GFX_GL_ARRAY_BUFFER,
                          (GLsizeiptr)(vertex_count * sizeof(*expanded_colors)),
                          expanded_colors,
                          GFX_GL_STATIC_DRAW);

    free(expanded_colors);

    if (!record->position_buffer || !record->color_buffer) {
        if (record->position_buffer && window->gl.DeleteBuffers) {
            window->gl.DeleteBuffers(1, &record->position_buffer);
        }
        if (record->color_buffer && window->gl.DeleteBuffers) {
            window->gl.DeleteBuffers(1, &record->color_buffer);
        }
        free(record);
        return -1;
    }

    record->mesh = mesh;
    record->vertex_count = vertex_count;
    record->next = window->mesh_records;
    window->mesh_records = record;
    *record_out = record;
    return 0;
}

/** Desenha uma malha usando o backend da janela. 
 *  @param ctx Ponteiro para o contexto da janela.
 *  @param mesh Ponteiro para a malha a ser desenhada.
 *  @param transform Matriz de transformação a ser aplicada à malha.
 *  @param mat Material a ser usado para desenhar a malha.
*/
static void gfx_platform_window_draw_mesh(void *ctx, Mesh *mesh, Mat4 transform, Material *mat) {
    PlatformWindow *window = (PlatformWindow *)ctx;
    MeshGpuRecord *record;
    Mat4 model_view_projection;
    GLfloat mvp[16];

    (void)mat;

    if (!window || !mesh || !window->shader_program || window->mvp_location < 0 ||
        !window->gl.UseProgram || !window->gl.UniformMatrix4fv || !window->gl.BindBuffer ||
        !window->gl.EnableVertexAttribArray || !window->gl.VertexAttribPointer || !window->gl.DrawArrays) {
        return;
    }

    gfx_platform_window_update_camera(window);

    record = gfx_platform_window_find_mesh_record(window, mesh);
    if (!record && gfx_platform_window_prepare_mesh_record(window, mesh, &record) != 0) {
        fprintf(stderr, "gfx_window_demo: failed to upload mesh\n");
        return;
    }

    if (!record) {
        return;
    }

    model_view_projection = gfx_platform_window_mat4_mul(window->projection_matrix,
                                                         gfx_platform_window_mat4_mul(window->view_matrix, transform));
    gfx_platform_window_mat4_to_array(model_view_projection, mvp);

    window->gl.UseProgram(window->shader_program);
    window->gl.UniformMatrix4fv(window->mvp_location, 1, GFX_GL_FALSE, mvp);

    window->gl.BindBuffer(GFX_GL_ARRAY_BUFFER, record->position_buffer);
    window->gl.EnableVertexAttribArray(0U);
    window->gl.VertexAttribPointer(0U, 3, GFX_GL_FLOAT, GFX_GL_FALSE, 0, (const void *)0);

    window->gl.BindBuffer(GFX_GL_ARRAY_BUFFER, record->color_buffer);
    window->gl.EnableVertexAttribArray(1U);
    window->gl.VertexAttribPointer(1U, 3, GFX_GL_FLOAT, GFX_GL_FALSE, 0, (const void *)0);

    window->gl.DrawArrays(GFX_GL_TRIANGLES, 0, (GLsizei)record->vertex_count);
}

static void gfx_platform_window_set_camera(void *ctx, Vec3 pos, Vec3 target, float fov) {
    PlatformWindow *window = (PlatformWindow *)ctx;

    if (!window) {
        return;
    }

    window->camera_position = pos;
    window->camera_target = target;
    window->camera_fov = fov;
    window->camera_dirty = 1;
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

    gfx_platform_window_update_camera(window);

    if (window->width == 0U || window->height == 0U) {
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

    gfx_platform_window_release_gl_resources(window);

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
           window->gl.ClearColor && window->gl.Clear && window->gl.Viewport &&
           window->gl.Enable && window->gl.DepthFunc && window->gl.GenBuffers &&
           window->gl.BindBuffer && window->gl.BufferData && window->gl.DeleteBuffers &&
           window->gl.CreateShader && window->gl.ShaderSource && window->gl.CompileShader &&
           window->gl.GetShaderiv && window->gl.GetShaderInfoLog && window->gl.DeleteShader &&
           window->gl.CreateProgram && window->gl.AttachShader && window->gl.LinkProgram &&
           window->gl.GetProgramiv && window->gl.GetProgramInfoLog && window->gl.DeleteProgram &&
           window->gl.BindAttribLocation && window->gl.UseProgram && window->gl.GetUniformLocation &&
           window->gl.UniformMatrix4fv && window->gl.EnableVertexAttribArray &&
           window->gl.VertexAttribPointer && window->gl.DrawArrays;
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
    window->camera_position = (Vec3){ 0.0f, 0.0f, 3.0f };
    window->camera_target = (Vec3){ 0.0f, 0.0f, 0.0f };
    window->camera_fov = 60.0f;
    window->view_matrix = gfx_platform_window_mat4_identity();
    window->projection_matrix = gfx_platform_window_mat4_identity();
    window->camera_dirty = 1;
    window->shader_program = 0;
    window->mvp_location = -1;
    window->mesh_records = NULL;
    window->should_close = 0;

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

    if (window->gl.Enable && window->gl.DepthFunc) {
        window->gl.Enable(GFX_GL_DEPTH_TEST);
        window->gl.DepthFunc(GFX_GL_LEQUAL);
    }

    window->shader_program = gfx_platform_window_create_program(window);
    if (!window->shader_program) {
        fprintf(stderr, "gfx_platform_window_create: failed to create shader program\n");
        goto fail;
    }

    window->platform.XFlush(window->display);
    window->gl.Viewport(0, 0, (GLsizei)window->width, (GLsizei)window->height);
    gfx_platform_window_update_camera(window);

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
                    window->camera_dirty = 1;
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
