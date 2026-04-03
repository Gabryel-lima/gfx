#include <float.h>
#include <stdio.h>
#include <stdlib.h>

/** @brief Esboço de visualização de modelos 
 *  3D usando TinyOBJ e rasterização. 
 *  @author Gabriel-lima
 *  @date 2026-05-02
 *  @file src/tinyobj_preview.c
 *  @note Este arquivo é um esboço inicial para renderizar 
 *  um modelo 3D carregado com TinyOBJ usando uma 
 *  abordagem de rasterização software. 
 *  Ele é destinado a ser um ponto de partida para desenvolvimento 
 *  futuro, e não é uma implementação completa ou otimizada.
*/

#include "internal/rasterizer.h"
#include "tinyobj_loader.h"

/** Esta função calcula os deslocamentos dos vértices de cada face 
 *  @param attrib Ponteiro para a estrutura `TinyObj_Attrib` que contém os atributos do objeto
 *  @return Array de deslocamentos dos vértices de cada face, ou NULL em caso de erro
 *  @note O array retornado deve ser liberado pelo chamador usando `free()`
*/
static size_t *tinyobj_preview_face_vertex_offsets(const TinyObj_Attrib *attrib) {
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

/** Esta função desenha uma linha no framebuffer usando o algoritmo de Bresenham
 *  @param fb Ponteiro para o framebuffer
 *  @param x0 Coordenada x do ponto inicial
 *  @param y0 Coordenada y do ponto inicial
 *  @param x1 Coordenada x do ponto final
 *  @param y1 Coordenada y do ponto final
 *  @param rgba Cor da linha no formato RGBA
 */
static void tinyobj_preview_draw_line(Framebuffer *fb, int x0, int y0, int x1, int y1, uint32_t rgba) {
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -((y1 > y0) ? (y1 - y0) : (y0 - y1));
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        if (x0 >= 0 && y0 >= 0 && x0 < (int)fb->width && y0 < (int)fb->height) {
            fb->pixels[(size_t)y0 * fb->width + (size_t)x0] = rgba;
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

/** Esta função salva o framebuffer em um arquivo PPM
 *  @param path Caminho do arquivo de saída
 *  @param fb Ponteiro para o framebuffer
 *  @return Código de erro ou TINYOBJ_SUCCESS em caso de sucesso
 */
static int tinyobj_preview_save_ppm(const char *path, const Framebuffer *fb) {
    FILE *f = fopen(path, "wb");
    if (!f) {
        return TINYOBJ_ERROR_FILE_OPERATION;
    }

    if (fprintf(f, "P6\n%u %u\n255\n", fb->width, fb->height) < 0) {
        fclose(f);
        return TINYOBJ_ERROR_FILE_OPERATION;
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
                return TINYOBJ_ERROR_FILE_OPERATION;
            }
        }
    }

    fclose(f);
    return TINYOBJ_SUCCESS;
}

/** Esta função renderiza as faces de um objeto 3D no framebuffer
 *  @param attrib Ponteiro para a estrutura `TinyObj_Attrib` que contém os atributos do objeto
 *  @param materials Ponteiro para a lista de materiais
 *  @param num_materials Número de materiais na lista
 *  @param shape Ponteiro para a estrutura `TinyObj_Shape` que contém as informações da forma
 *  @param min_v Coordenadas mínimas do objeto
 *  @param max_v Coordenadas máximas do objeto
 *  @param face_vertex_offsets Array de deslocamentos dos vértices de cada face
 *  @param fb Ponteiro para o framebuffer
 *  @param zbuf Ponteiro para o buffer de profundidade
 *  @param margin Margem para o desenho
 */
static void tinyobj_preview_render_faces(const TinyObj_Attrib *attrib,
                                         const TinyObj_Material *materials, size_t num_materials,
                                         const TinyObj_Shape *shape, Vec3 min_v, Vec3 max_v,
                                         const size_t *face_vertex_offsets,
                                         Framebuffer *fb, float *zbuf, int margin) {
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
        Vec3 color = tinyobj_material_color(materials, num_materials, material_id);

        for (int tri = 1; tri < verts_in_face - 1; ++tri) {
            TinyObj_Vertex_Index i0 = attrib->faces[vertex_offset + 0];
            TinyObj_Vertex_Index i1 = attrib->faces[vertex_offset + (size_t)tri];
            TinyObj_Vertex_Index i2 = attrib->faces[vertex_offset + (size_t)tri + 1];

            if (i0.v_idx < 0 || i1.v_idx < 0 || i2.v_idx < 0) {
                continue;
            }

            Vec3 v0 = tinyobj_attrib_get_vertex(attrib, i0.v_idx);
            Vec3 v1 = tinyobj_attrib_get_vertex(attrib, i1.v_idx);
            Vec3 v2 = tinyobj_attrib_get_vertex(attrib, i2.v_idx);

            Vec4 p0 = tinyobj_project_vertex(v0, min_v, max_v, (int)fb->width, (int)fb->height, margin);
            Vec4 p1 = tinyobj_project_vertex(v1, min_v, max_v, (int)fb->width, (int)fb->height, margin);
            Vec4 p2 = tinyobj_project_vertex(v2, min_v, max_v, (int)fb->width, (int)fb->height, margin);

            gfx_rasterize_triangle(fb, zbuf, p0, p1, p2, color, color, color);

            tinyobj_preview_draw_line(fb, (int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, 0xFFFFFFFF);
            tinyobj_preview_draw_line(fb, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, 0xFFFFFFFF);
            tinyobj_preview_draw_line(fb, (int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, 0xFFFFFFFF);
        }
    }
}

/** Esta função salva uma pré-visualização do objeto 3D em um arquivo PPM
 *  @param attrib Ponteiro para a estrutura `TinyObj_Attrib` que contém os atributos do objeto
 *  @param shapes Ponteiro para a lista de formas
 *  @param num_shapes Número de formas na lista
 *  @param materials Ponteiro para a lista de materiais
 *  @param num_materials Número de materiais na lista
 *  @param path Caminho do arquivo de saída
 *  @param width Largura da imagem
 *  @param height Altura da imagem
 */
int tinyobj_save_preview_ppm(const TinyObj_Attrib *attrib,
                             const TinyObj_Shape *shapes, size_t num_shapes,
                             const TinyObj_Material *materials, size_t num_materials,
                             const char *path,
                             uint32_t width, uint32_t height,
                             uint32_t background_rgba,
                             int margin) {
    if (!attrib || !path || width == 0 || height == 0) {
        return TINYOBJ_ERROR_INVALID_PARAMETER;
    }

    if (!attrib->face_num_verts || attrib->num_face_num_verts == 0) {
        return TINYOBJ_ERROR_EMPTY;
    }

    Vec3 min_v, max_v;
    tinyobj_attrib_compute_bounds(attrib, &min_v, &max_v);

    size_t pixel_count = (size_t)width * (size_t)height;
    if (width != 0 && pixel_count / (size_t)width != (size_t)height) {
        return TINYOBJ_ERROR_INVALID_PARAMETER;
    }

    Framebuffer fb = { 0 };
    fb.width = width;
    fb.height = height;
    fb.pitch = width * (uint32_t)sizeof(uint32_t);
    fb.fd = -1;
    fb.pixels = (uint32_t *)calloc(pixel_count, sizeof(uint32_t));

    float *zbuf = (float *)malloc(pixel_count * sizeof(float));
    size_t *face_vertex_offsets = tinyobj_preview_face_vertex_offsets(attrib);

    if (!fb.pixels || !zbuf || !face_vertex_offsets) {
        free(fb.pixels);
        free(zbuf);
        free(face_vertex_offsets);
        return TINYOBJ_ERROR_INVALID_PARAMETER;
    }

    for (size_t i = 0; i < pixel_count; ++i) {
        fb.pixels[i] = background_rgba;
        zbuf[i] = FLT_MAX;
    }

    if (num_shapes > 0 && shapes) {
        for (size_t i = 0; i < num_shapes; ++i) {
            tinyobj_preview_render_faces(attrib, materials, num_materials, &shapes[i], min_v, max_v,
                                         face_vertex_offsets, &fb, zbuf, margin);
        }
    } else {
        tinyobj_preview_render_faces(attrib, materials, num_materials, NULL, min_v, max_v,
                                     face_vertex_offsets, &fb, zbuf, margin);
    }

    int rc = tinyobj_preview_save_ppm(path, &fb);

    free(fb.pixels);
    free(zbuf);
    free(face_vertex_offsets);

    return rc;
}
