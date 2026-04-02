#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "rasterizer.h"
#include "tinyobj_loader.h"

#define PREVIEW_WIDTH 800
#define PREVIEW_HEIGHT 600
#define PREVIEW_MARGIN 32

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static void file_reader(void *ctx, const char *filename,
                        int is_mtl, const char *obj_filename,
                        char **buf, size_t *len) {
    (void)ctx;
    (void)is_mtl;
    (void)obj_filename;

    FILE *f = fopen(filename, "rb");

    if (!f) {
        *buf = NULL;
        *len = 0;
        return;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        *buf = NULL;
        *len = 0;
        return;
    }

    long n = ftell(f);
    if (n < 0) {
        fclose(f);
        *buf = NULL;
        *len = 0;
        return;
    }

    rewind(f);

    char *b = (char *)malloc((size_t)n + 1);
    if (!b) {
        fclose(f);
        *buf = NULL;
        *len = 0;
        return;
    }

    size_t r = fread(b, 1, (size_t)n, f);
    b[r] = '\0';
    fclose(f);

    *buf = b;
    *len = r + 1;
}

static int get_executable_path(char *buffer, size_t size) {
    ssize_t length = readlink("/proc/self/exe", buffer, size - 1);
    if (length < 0 || (size_t)length >= size) {
        return -1;
    }

    buffer[length] = '\0';
    return 0;
}

static int strip_basename(char *path) {
    char *slash = strrchr(path, '/');
    if (!slash) {
        return -1;
    }

    if (slash == path) {
        slash[1] = '\0';
        return 0;
    }

    *slash = '\0';
    return 0;
}

static int copy_path(char *dest, size_t size, const char *source) {
    size_t length = strlen(source);
    if (length + 1 > size) {
        return -1;
    }

    memcpy(dest, source, length + 1);
    return 0;
}

static int join_path(char *dest, size_t size, const char *left, const char *right) {
    size_t left_len = strlen(left);
    size_t right_len = strlen(right);

    if (left_len + 1 + right_len + 1 > size) {
        return -1;
    }

    memcpy(dest, left, left_len);
    dest[left_len] = '/';
    memcpy(dest + left_len + 1, right, right_len + 1);
    return 0;
}

static float minf(float a, float b) {
    return (a < b) ? a : b;
}

static float maxf(float a, float b) {
    return (a > b) ? a : b;
}

static int int_abs(int value) {
    return (value < 0) ? -value : value;
}

static Vec3 make_vec3(float x, float y, float z) {
    Vec3 v = { x, y, z };
    return v;
}

static Vec3 get_vertex(const TINYOBJ_ATTRIB *attrib, int index) {
    Vec3 v = { 0.0f, 0.0f, 0.0f };

    if (!attrib || !attrib->vertices || index < 0 || (unsigned int)index >= attrib->num_vertices) {
        return v;
    }

    size_t base = (size_t)index * 3;
    v.x = attrib->vertices[base + 0];
    v.y = attrib->vertices[base + 1];
    v.z = attrib->vertices[base + 2];
    return v;
}

static Vec3 material_color(const TINYOBJ_MATERIAL *materials, size_t num_materials, int material_id) {
    if (materials && material_id >= 0 && (size_t)material_id < num_materials) {
        return make_vec3(
            materials[material_id].diffuse[0],
            materials[material_id].diffuse[1],
            materials[material_id].diffuse[2]
        );
    }

    return make_vec3(0.75f, 0.75f, 0.80f);
}

static void compute_bounds(const TINYOBJ_ATTRIB *attrib, Vec3 *min_v, Vec3 *max_v) {
    if (!attrib || !attrib->vertices || attrib->num_vertices == 0) {
        *min_v = make_vec3(-1.0f, -1.0f, -1.0f);
        *max_v = make_vec3(1.0f, 1.0f, 1.0f);
        return;
    }

    *min_v = make_vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    *max_v = make_vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (unsigned int i = 0; i < attrib->num_vertices; ++i) {
        Vec3 v = get_vertex(attrib, (int)i);
        min_v->x = minf(min_v->x, v.x);
        min_v->y = minf(min_v->y, v.y);
        min_v->z = minf(min_v->z, v.z);
        max_v->x = maxf(max_v->x, v.x);
        max_v->y = maxf(max_v->y, v.y);
        max_v->z = maxf(max_v->z, v.z);
    }
}

static size_t *build_face_vertex_offsets(const TINYOBJ_ATTRIB *attrib) {
    if (!attrib || !attrib->face_num_verts || attrib->num_face_num_verts == 0) {
        return NULL;
    }

    size_t *offsets = (size_t *)malloc(((size_t)attrib->num_face_num_verts + 1) * sizeof(size_t));
    if (!offsets) {
        return NULL;
    }

    size_t offset = 0;
    for (unsigned int face = 0; face < attrib->num_face_num_verts; ++face) {
        offsets[face] = offset;
        offset += (size_t)attrib->face_num_verts[face];
    }

    offsets[attrib->num_face_num_verts] = offset;
    return offsets;
}

static Vec4 project_vertex(Vec3 v, Vec3 min_v, Vec3 max_v, int width, int height) {
    float range_x = max_v.x - min_v.x;
    float range_y = max_v.y - min_v.y;
    float range_z = max_v.z - min_v.z;

    if (range_x <= 0.0f) {
        range_x = 1.0f;
    }
    if (range_y <= 0.0f) {
        range_y = 1.0f;
    }

    float usable_w = (float)width - (2.0f * PREVIEW_MARGIN);
    float usable_h = (float)height - (2.0f * PREVIEW_MARGIN);
    float scale = minf(usable_w / range_x, usable_h / range_y);
    float draw_w = range_x * scale;
    float draw_h = range_y * scale;

    float x = (float)PREVIEW_MARGIN + 0.5f * ((float)width - (2.0f * PREVIEW_MARGIN) - draw_w) + (v.x - min_v.x) * scale;
    float y = (float)PREVIEW_MARGIN + 0.5f * ((float)height - (2.0f * PREVIEW_MARGIN) - draw_h) + (max_v.y - v.y) * scale;
    float z = (range_z > 0.0f) ? ((v.z - min_v.z) / range_z) : 0.5f;

    if (z < 0.0f) {
        z = 0.0f;
    }
    if (z > 1.0f) {
        z = 1.0f;
    }

    Vec4 p = { x, y, z, 1.0f };
    return p;
}

static void draw_line(Framebuffer *fb, int x0, int y0, int x1, int y1, uint32_t rgba) {
    int dx = int_abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -int_abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        if (x0 >= 0 && y0 >= 0 && x0 < (int)fb->width && y0 < (int)fb->height) {
            fb_set_pixel(fb, x0, y0, rgba);
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static int save_ppm(const char *path, const Framebuffer *fb) {
    FILE *f = fopen(path, "wb");
    if (!f) {
        return -1;
    }

    if (fprintf(f, "P6\n%u %u\n255\n", fb->width, fb->height) < 0) {
        fclose(f);
        return -1;
    }

    for (uint32_t y = 0; y < fb->height; ++y) {
        for (uint32_t x = 0; x < fb->width; ++x) {
            uint32_t px = fb->pixels[(size_t)y * fb->width + x];
            unsigned char rgb[3];
            rgb[0] = (unsigned char)((px >> 24) & 0xFF);
            rgb[1] = (unsigned char)((px >> 16) & 0xFF);
            rgb[2] = (unsigned char)((px >> 8) & 0xFF);

            if (fwrite(rgb, 1, 3, f) != 3) {
                fclose(f);
                return -1;
            }
        }
    }

    fclose(f);
    return 0;
}

static void print_summary(const char *filename,
                          const TINYOBJ_ATTRIB *attrib,
                          const TINYOBJ_SHAPE *shapes, size_t num_shapes,
                          const TINYOBJ_MATERIAL *materials, size_t num_materials,
                          Vec3 min_v, Vec3 max_v) {
    printf("Modelo carregado com sucesso\n");
    printf("Arquivo: %s\n", filename);
    printf("Vertices=%u normals=%u texcoords=%u faces=%u\n",
           attrib->num_vertices, attrib->num_normals, attrib->num_texcoords, attrib->num_faces);
    printf("Bounds min=(%.3f, %.3f, %.3f) max=(%.3f, %.3f, %.3f)\n",
           min_v.x, min_v.y, min_v.z, max_v.x, max_v.y, max_v.z);

    for (size_t i = 0; i < num_shapes; ++i) {
        const char *name = shapes[i].name ? shapes[i].name : "(sem nome)";
        printf("Shape %zu: %s face_offset=%u length=%u\n",
               i, name, shapes[i].face_offset, shapes[i].length);
    }

    for (size_t i = 0; i < num_materials; ++i) {
        const char *name = materials[i].name ? materials[i].name : "(sem nome)";
        printf("Material %zu: %s Ka=(%.2f, %.2f, %.2f) Kd=(%.2f, %.2f, %.2f) Ks=(%.2f, %.2f, %.2f) Ns=%.2f d=%.2f illum=%d\n",
               i,
               name,
               materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2],
               materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2],
               materials[i].specular[0], materials[i].specular[1], materials[i].specular[2],
               materials[i].shininess,
               materials[i].dissolve,
               materials[i].illum);
    }
}

static void render_faces(const TINYOBJ_ATTRIB *attrib,
                         const TINYOBJ_MATERIAL *materials, size_t num_materials,
                         const TINYOBJ_SHAPE *shape, Vec3 min_v, Vec3 max_v,
                         size_t *face_vertex_offsets,
                         Framebuffer *fb, float *zbuf) {
    size_t face_begin = shape ? (size_t)shape->face_offset : 0;
    size_t face_end = shape ? (size_t)shape->face_offset + (size_t)shape->length : (size_t)attrib->num_face_num_verts;

    if (face_end > (size_t)attrib->num_face_num_verts) {
        face_end = (size_t)attrib->num_face_num_verts;
    }

    for (size_t face = face_begin; face < face_end; ++face) {
        int verts_in_face = attrib->face_num_verts[face];
        if (verts_in_face < 3) {
            continue;
        }

        size_t vertex_offset = face_vertex_offsets[face];
        int material_id = attrib->material_ids ? attrib->material_ids[face] : -1;
        Vec3 color = material_color(materials, num_materials, material_id);

        for (int tri = 1; tri < verts_in_face - 1; ++tri) {
            TINYOBJ_VERTEX_INDEX i0 = attrib->faces[vertex_offset + 0];
            TINYOBJ_VERTEX_INDEX i1 = attrib->faces[vertex_offset + (size_t)tri];
            TINYOBJ_VERTEX_INDEX i2 = attrib->faces[vertex_offset + (size_t)tri + 1];

            if (i0.v_idx < 0 || i1.v_idx < 0 || i2.v_idx < 0) {
                continue;
            }

            Vec3 v0 = get_vertex(attrib, i0.v_idx);
            Vec3 v1 = get_vertex(attrib, i1.v_idx);
            Vec3 v2 = get_vertex(attrib, i2.v_idx);

            Vec4 p0 = project_vertex(v0, min_v, max_v, (int)fb->width, (int)fb->height);
            Vec4 p1 = project_vertex(v1, min_v, max_v, (int)fb->width, (int)fb->height);
            Vec4 p2 = project_vertex(v2, min_v, max_v, (int)fb->width, (int)fb->height);

            rasterize_triangle(fb, zbuf, p0, p1, p2, color, color, color);

            draw_line(fb, (int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, 0xFFFFFFFF);
            draw_line(fb, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, 0xFFFFFFFF);
            draw_line(fb, (int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, 0xFFFFFFFF);
        }
    }
}

int main(int argc, char **argv) {
    /** @todo Facilitar a inicialização com novos prototipos para a api, 
     *  e criar as funções que faltam, pois estão muito complexas de usar, 
     *  e isso é só um exemplo de como usar a API, não deveria ser tão verboso. 
     *  @todo Tmabém criar um script, talvez em ruby para visualizar por animação
     *  os blocos de memória alocados e liberados, para facilitar o entendimento de como usar a API,
     *  e também para ajudar a identificar possíveis vazamentos de memória, 
     *  ou usos incorretos dos ponteiros, como acessar um ponteiro após liberar a memória, ou esquecer de liberar a
     */

    char executable_path[PATH_MAX];
    char build_dir[PATH_MAX];
    char project_root[PATH_MAX];
    char default_model_path[PATH_MAX];
    char default_output_path[PATH_MAX];
    const char *filename = (argc > 1) ? argv[1] : NULL;
    const char *output_path = (argc > 2) ? argv[2] : NULL;

    if (get_executable_path(executable_path, sizeof(executable_path)) == 0) {
        if (copy_path(build_dir, sizeof(build_dir), executable_path) == 0 &&
            strip_basename(build_dir) == 0 &&
            copy_path(project_root, sizeof(project_root), build_dir) == 0 &&
            strip_basename(project_root) == 0 &&
            join_path(default_model_path, sizeof(default_model_path), project_root, "examples/models/triangle.obj") == 0 &&
            join_path(default_output_path, sizeof(default_output_path), build_dir, "tinyobj_triangle.ppm") == 0) {
            /* default paths resolved from the executable location */
        } else {
            snprintf(default_model_path, sizeof(default_model_path), "examples/models/triangle.obj");
            snprintf(default_output_path, sizeof(default_output_path), "tinyobj_triangle.ppm");
        }
    } else {
        snprintf(default_model_path, sizeof(default_model_path), "examples/models/triangle.obj");
        snprintf(default_output_path, sizeof(default_output_path), "tinyobj_triangle.ppm");
    }

    if (!filename) {
        filename = default_model_path;
    }
    if (!output_path) {
        output_path = default_output_path;
    }

    TINYOBJ_ATTRIB attrib = { 0 };
    tinyobj_attrib_init(&attrib);

    TINYOBJ_SHAPE *shapes = NULL;
    TINYOBJ_MATERIAL *materials = NULL;
    size_t num_shapes = 0;
    size_t num_materials = 0;

    int rc = tinyobj_load_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                               filename, file_reader, NULL, TINYOBJ_FLAG_TRIANGULATE);
    if (rc != TINYOBJ_SUCCESS) {
        fprintf(stderr, "Falha ao carregar modelo: %d\n", rc);
        return 1;
    }

    Vec3 min_v, max_v;
    compute_bounds(&attrib, &min_v, &max_v);
    print_summary(filename, &attrib, shapes, num_shapes, materials, num_materials, min_v, max_v);

    if (!attrib.face_num_verts || attrib.num_face_num_verts == 0) {
        fprintf(stderr, "Modelo sem faces para renderizar\n");
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return 1;
    }

    size_t pixel_count = (size_t)PREVIEW_WIDTH * (size_t)PREVIEW_HEIGHT;
    Framebuffer fb = { 0 };
    fb.width = PREVIEW_WIDTH;
    fb.height = PREVIEW_HEIGHT;
    fb.pitch = PREVIEW_WIDTH * (uint32_t)sizeof(uint32_t);
    fb.fd = -1;
    fb.pixels = (uint32_t *)calloc(pixel_count, sizeof(uint32_t));

    float *zbuf = (float *)malloc(pixel_count * sizeof(float));
    size_t *face_vertex_offsets = build_face_vertex_offsets(&attrib);

    if (!fb.pixels || !zbuf || !face_vertex_offsets) {
        fprintf(stderr, "Falha ao alocar memoria para o preview\n");
        free(fb.pixels);
        free(zbuf);
        free(face_vertex_offsets);
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return 1;
    }

    for (size_t i = 0; i < pixel_count; ++i) {
        zbuf[i] = FLT_MAX;
    }

    fb_clear(&fb, 0x101018FF);

    if (num_shapes > 0) {
        for (size_t i = 0; i < num_shapes; ++i) {
            render_faces(&attrib, materials, num_materials, &shapes[i], min_v, max_v, face_vertex_offsets, &fb, zbuf);
        }
    } else {
        render_faces(&attrib, materials, num_materials, NULL, min_v, max_v, face_vertex_offsets, &fb, zbuf);
    }

    if (save_ppm(output_path, &fb) != 0) {
        fprintf(stderr, "Falha ao salvar preview em %s\n", output_path);
        free(fb.pixels);
        free(zbuf);
        free(face_vertex_offsets);
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return 1;
    }

    printf("Preview salvo em %s\n", output_path);

    free(fb.pixels);
    free(zbuf);
    free(face_vertex_offsets);
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    return 0;
}
