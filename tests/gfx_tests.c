#define _XOPEN_SOURCE 700

#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gfx.h"
#include "tinyobj_loader.h"
#include "internal/gl_loader.h"
#include "internal/framebuffer.h"
#include "internal/rasterizer.h"
#include "internal/x11_platform.h"

typedef struct DispatchCounters {
    int begin_count;
    int end_count;
    int draw_count;
    int camera_count;
} DispatchCounters;

static void test_begin_frame(void *ctx) {
    DispatchCounters *counters = (DispatchCounters *)ctx;
    counters->begin_count++;
}

static void test_end_frame(void *ctx) {
    DispatchCounters *counters = (DispatchCounters *)ctx;
    counters->end_count++;
}

static void test_draw_mesh(void *ctx, Mesh *mesh, Mat4 transform, Material *mat) {
    DispatchCounters *counters = (DispatchCounters *)ctx;
    (void)mesh;
    (void)transform;
    (void)mat;
    counters->draw_count++;
}

static void test_set_camera(void *ctx, Vec3 pos, Vec3 target, float fov) {
    DispatchCounters *counters = (DispatchCounters *)ctx;
    (void)pos;
    (void)target;
    (void)fov;
    counters->camera_count++;
}

static void mat4_identity(Mat4 *matrix) {
    *matrix = (Mat4){
        .col = {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        }
    };
}

static float absf_local(float value) {
    return value < 0.0f ? -value : value;
}

static int float_close(float a, float b, float epsilon) {
    return absf_local(a - b) <= epsilon;
}

static int file_starts_with(const char *path, const char *prefix) {
    FILE *file;
    char buffer[8] = {0};
    size_t prefix_len = strlen(prefix);
    size_t read_len;

    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }

    read_len = fread(buffer, 1, prefix_len, file);
    fclose(file);

    if (read_len != prefix_len) {
        return 0;
    }

    return memcmp(buffer, prefix, prefix_len) == 0;
}

static int write_text_file(const char *path, const char *content) {
    FILE *file = fopen(path, "wb");
    size_t length;

    if (!file) {
        return -1;
    }

    length = strlen(content);
    if (fwrite(content, 1, length, file) != length) {
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

static void test_file_reader(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len) {
    FILE *file;
    long file_size;
    char *data;
    size_t read_size;

    (void)ctx;
    (void)is_mtl;
    (void)obj_filename;

    *buf = NULL;
    *len = 0;

    file = fopen(filename, "rb");
    if (!file) {
        return;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return;
    }

    rewind(file);

    data = (char *)malloc((size_t)file_size + 1U);
    if (!data) {
        fclose(file);
        return;
    }

    read_size = fread(data, 1, (size_t)file_size, file);
    fclose(file);
    if (read_size != (size_t)file_size) {
        free(data);
        return;
    }

    data[file_size] = '\0';
    *buf = data;
    *len = (size_t)file_size;
}

static int test_public_facade(void) {
    DispatchCounters counters = {0};
    GfxBackend backend;
    GfxContext context;
    Mat4 identity;

    backend.begin_frame = test_begin_frame;
    backend.end_frame = test_end_frame;
    backend.draw_mesh = test_draw_mesh;
    backend.set_camera = test_set_camera;

    context.backend = backend;
    context.backend_ctx = &counters;

    mat4_identity(&identity);

    gfx_begin_frame(&context);
    gfx_set_camera(&context, (Vec3){ 1.0f, 2.0f, 3.0f }, (Vec3){ 0.0f, 0.0f, 0.0f }, 60.0f);
    gfx_draw_mesh(&context, NULL, identity, NULL);
    gfx_end_frame(&context);

    if (counters.begin_count != 1 || counters.end_count != 1 || counters.draw_count != 1 || counters.camera_count != 1) {
        fprintf(stderr, "public facade dispatch failed\n");
        return 1;
    }

    backend = gfx_get_stub_backend();
    if (!backend.begin_frame || !backend.end_frame || !backend.draw_mesh || !backend.set_camera) {
        fprintf(stderr, "stub backend missing callbacks\n");
        return 1;
    }

    return 0;
}

static int test_dynamic_loaders(void) {
    GLProcs gl = gfx_gl_load();
    PlatformGL platform = gfx_platform_gl_init();

    if (!gl.handle || !gl.ClearColor || !gl.Clear || !gl.Viewport ||
        !gl.CreateShader || !gl.CreateProgram || !gl.DeleteProgram) {
        fprintf(stderr, "OpenGL runtime loader did not resolve the expected symbols\n");
        gfx_gl_close(&gl);
        gfx_platform_gl_close(&platform);
        return 1;
    }

    gfx_gl_close(&gl);
    if (gl.handle != NULL) {
        fprintf(stderr, "gfx_gl_close did not reset the loader state\n");
        gfx_platform_gl_close(&platform);
        return 1;
    }

    if (!platform.x11 || !platform.gl || !platform.XOpenDisplay || !platform.XCloseDisplay ||
        !platform.XCreateWindow || !platform.XFreeColormap || !platform.XPending ||
        !platform.glXChooseVisual || !platform.glXCreateContext || !platform.glXMakeCurrent ||
        !platform.glXSwapBuffers) {
        fprintf(stderr, "X11/GLX runtime loader did not resolve the expected symbols\n");
        gfx_platform_gl_close(&platform);
        return 1;
    }

    gfx_platform_gl_close(&platform);
    if (platform.x11 != NULL || platform.gl != NULL || platform.XOpenDisplay != NULL || platform.glXSwapBuffers != NULL) {
        fprintf(stderr, "gfx_platform_gl_close did not reset the loader state\n");
        return 1;
    }

    return 0;
}

static int test_math_framebuffer_and_rasterizer(void) {
    uint32_t pixels[16];
    float zbuf[20];
    Framebuffer fb;
    size_t i;
    float min_zero;
    float max_zero;
    float sentinel = 12345.0f;

    if (!float_close(gfx_fminf(NAN, 3.0f), 3.0f, 0.0f)) {
        fprintf(stderr, "gfx_fminf should ignore NaN\n");
        return 1;
    }
    if (!float_close(gfx_fmaxf(3.0f, NAN), 3.0f, 0.0f)) {
        fprintf(stderr, "gfx_fmaxf should ignore NaN\n");
        return 1;
    }
    if (!float_close(gfx_fminf(INFINITY, 2.0f), 2.0f, 0.0f)) {
        fprintf(stderr, "gfx_fminf should handle infinity\n");
        return 1;
    }
    if (!float_close(gfx_fmaxf(-INFINITY, 2.0f), 2.0f, 0.0f)) {
        fprintf(stderr, "gfx_fmaxf should handle infinity\n");
        return 1;
    }

    min_zero = gfx_fminf(-0.0f, 0.0f);
    max_zero = gfx_fmaxf(-0.0f, 0.0f);
    if (!(min_zero == 0.0f && signbit(min_zero))) {
        fprintf(stderr, "gfx_fminf should preserve negative zero\n");
        return 1;
    }
    if (!(max_zero == 0.0f && !signbit(max_zero))) {
        fprintf(stderr, "gfx_fmaxf should preserve positive zero\n");
        return 1;
    }

    fb.pixels = pixels;
    fb.fd = -1;
    fb.width = 2;
    fb.height = 2;
    fb.pitch = 12;

    gfx_fb_clear(&fb, 0x11223344U);
    for (i = 0; i < 6; ++i) {
        if (pixels[i] != 0x11223344U) {
            fprintf(stderr, "gfx_fb_clear did not respect pitch stride\n");
            return 1;
        }
    }

    gfx_fb_set_pixel(&fb, 1, 1, 0xAABBCCDDU);
    if (pixels[4] != 0xAABBCCDDU) {
        fprintf(stderr, "gfx_fb_set_pixel used the wrong stride\n");
        return 1;
    }
    if (pixels[3] != 0x11223344U) {
        fprintf(stderr, "gfx_fb_set_pixel clobbered a neighboring pixel\n");
        return 1;
    }

    gfx_fb_set_pixel(&fb, -1, 0, 0xFFFFFFFFU);
    if (pixels[0] != 0x11223344U) {
        fprintf(stderr, "gfx_fb_set_pixel accepted an out-of-bounds coordinate\n");
        return 1;
    }

    {
        Framebuffer invalid = gfx_fb_open("/definitely/not/a/real/framebuffer");
        if (invalid.fd != -1 || invalid.pixels != NULL || invalid.width != 0 || invalid.height != 0 || invalid.pitch != 0) {
            fprintf(stderr, "gfx_fb_open failed to return an invalid framebuffer on error\n");
            return 1;
        }
        gfx_fb_close(&invalid);
    }

    for (i = 0; i < 16; ++i) {
        pixels[i] = 0U;
    }
    for (i = 0; i < 20; ++i) {
        zbuf[i] = FLT_MAX;
    }
    zbuf[16] = sentinel;
    zbuf[17] = sentinel;
    zbuf[18] = sentinel;
    zbuf[19] = sentinel;

    fb.pixels = pixels;
    fb.width = 4;
    fb.height = 4;
    fb.pitch = 16;

    gfx_rasterize_triangle(
        &fb,
        zbuf,
        (Vec4){ -1.5f, -1.0f, 0.25f, 1.0f },
        (Vec4){ 3.5f, 0.0f, 0.25f, 1.0f },
        (Vec4){ 0.0f, 3.5f, 0.25f, 1.0f },
        (Vec3){ 1.0f, 0.0f, 0.0f },
        (Vec3){ 0.0f, 1.0f, 0.0f },
        (Vec3){ 0.0f, 0.0f, 1.0f });

    {
        int changed = 0;
        for (i = 0; i < 16; ++i) {
            if (pixels[i] != 0U) {
                changed++;
            }
        }
        if (changed == 0) {
            fprintf(stderr, "rasterizer did not draw any pixels\n");
            return 1;
        }
    }

    if (zbuf[16] != sentinel || zbuf[17] != sentinel || zbuf[18] != sentinel || zbuf[19] != sentinel) {
        fprintf(stderr, "rasterizer wrote past the visible z-buffer\n");
        return 1;
    }

    for (i = 0; i < 16; ++i) {
        pixels[i] = 0U;
        zbuf[i] = FLT_MAX;
    }

    gfx_rasterize_triangle(
        &fb,
        zbuf,
        (Vec4){ 1.0f, 1.0f, 0.0f, 1.0f },
        (Vec4){ 2.0f, 2.0f, 0.0f, 1.0f },
        (Vec4){ 3.0f, 3.0f, 0.0f, 1.0f },
        (Vec3){ 1.0f, 1.0f, 1.0f },
        (Vec3){ 1.0f, 1.0f, 1.0f },
        (Vec3){ 1.0f, 1.0f, 1.0f });

    for (i = 0; i < 16; ++i) {
        if (pixels[i] != 0U) {
            fprintf(stderr, "degenerate triangle should not draw pixels\n");
            return 1;
        }
    }

    return 0;
}

static int test_tinyobj_loader_and_preview(void) {
    char dir_template[] = "/tmp/gfx-tests-XXXXXX";
    char *dir;
    char obj_path[512];
    char mtl_path[512];
    char preview_path[512];
    TinyObj_Attrib attrib;
    TinyObj_Shape *shapes = NULL;
    TinyObj_Material *materials = NULL;
    size_t num_shapes = 0;
    size_t num_materials = 0;
    int rc;
    size_t i;
    Vec3 color;

    dir = mkdtemp(dir_template);
    if (!dir) {
        fprintf(stderr, "mkdtemp failed: %s\n", strerror(errno));
        return 1;
    }

    snprintf(obj_path, sizeof(obj_path), "%s/scene.obj", dir);
    snprintf(mtl_path, sizeof(mtl_path), "%s/scene.mtl", dir);
    snprintf(preview_path, sizeof(preview_path), "%s/preview.ppm", dir);

    if (write_text_file(mtl_path,
        "newmtl Blue\n"
        "Ka 0.05 0.05 0.05\n"
        "Kd 0.20 0.40 0.80\n"
        "Ks 0.00 0.00 0.00\n"
        "foo 1 2 3\n") != 0) {
        fprintf(stderr, "failed to write test MTL\n");
        return 1;
    }

    if (write_text_file(obj_path,
        "mtllib scene.mtl\n"
        "o Octagon\n"
        "v 0.0 1.0 0.0\n"
        "v 0.707 0.707 0.0\n"
        "v 1.0 0.0 0.0\n"
        "v 0.707 -0.707 0.0\n"
        "v 0.0 -1.0 0.0\n"
        "v -0.707 -0.707 0.0\n"
        "v -1.0 0.0 0.0\n"
        "v -0.707 0.707 0.0\n"
        "usemtl Blue\n"
        "f 1 2 3 4 5 6 7 8\n"
        "g GroupA GroupB\n"
        "o Triangle\n"
        "v 2.0 0.0 0.0\n"
        "v 3.0 0.0 0.0\n"
        "v 2.0 1.0 0.0\n"
        "usemtl Blue\n"
        "f 9 10 11\n") != 0) {
        fprintf(stderr, "failed to write test OBJ\n");
        return 1;
    }

    tinyobj_attrib_init(&attrib);
    rc = tinyobj_load_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                          obj_path, test_file_reader, NULL, TINYOBJ_FLAG_TRIANGULATE);
    if (rc != TINYOBJ_SUCCESS) {
        fprintf(stderr, "tinyobj_load_obj failed: %d\n", rc);
        return 1;
    }

    if (attrib.num_vertices != 11 || attrib.num_faces != 21 || attrib.num_face_num_verts != 7) {
        fprintf(stderr, "unexpected OBJ counts: vertices=%u faces=%u face_num_verts=%u\n",
                attrib.num_vertices, attrib.num_faces, attrib.num_face_num_verts);
        return 1;
    }

    if (num_shapes != 2) {
        fprintf(stderr, "unexpected shape count: %zu\n", num_shapes);
        return 1;
    }

    if (!shapes[0].name || !shapes[1].name || strcmp(shapes[0].name, "Octagon") != 0 || strcmp(shapes[1].name, "Triangle") != 0) {
        fprintf(stderr, "shape names were not preserved as expected\n");
        return 1;
    }

    if (shapes[0].length != 1 || shapes[1].length != 1) {
        fprintf(stderr, "unexpected shape lengths: %u %u\n", shapes[0].length, shapes[1].length);
        return 1;
    }

    if (num_materials != 1) {
        fprintf(stderr, "unexpected material count: %zu\n", num_materials);
        return 1;
    }

    color = tinyobj_material_color(materials, num_materials, 0);
    if (!float_close(color.x, 0.20f, 0.0001f) || !float_close(color.y, 0.40f, 0.0001f) || !float_close(color.z, 0.80f, 0.0001f)) {
        fprintf(stderr, "material diffuse color mismatch\n");
        return 1;
    }

    for (i = 0; i < attrib.num_face_num_verts; ++i) {
        if (attrib.face_num_verts[i] != 3) {
            fprintf(stderr, "face %zu was not triangulated\n", i);
            return 1;
        }
    }

    rc = tinyobj_save_preview_ppm(&attrib, shapes, num_shapes, materials, num_materials,
                                  preview_path, 256, 256, 0x101018FFU, 16);
    if (rc != TINYOBJ_SUCCESS) {
        fprintf(stderr, "tinyobj_save_preview_ppm failed: %d\n", rc);
        return 1;
    }

    if (!file_starts_with(preview_path, "P6\n")) {
        fprintf(stderr, "preview file does not start with a PPM header\n");
        return 1;
    }

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    remove(preview_path);
    remove(obj_path);
    remove(mtl_path);
    rmdir(dir);

    return 0;
}

static int run_test(const char *name, int (*test_fn)(void)) {
    int rc = test_fn();
    if (rc == 0) {
        printf("[PASS] %s\n", name);
    } else {
        printf("[FAIL] %s\n", name);
    }
    return rc;
}

int main(void) {
    int failures = 0;

    failures += run_test("public facade", test_public_facade);
    failures += run_test("dynamic loaders", test_dynamic_loaders);
    failures += run_test("math framebuffer rasterizer", test_math_framebuffer_and_rasterizer);
    failures += run_test("tinyobj loader and preview", test_tinyobj_loader_and_preview);

    if (failures != 0) {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return 1;
    }

    printf("all tests passed\n");
    return 0;
}
