#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/internal/mesh.h"
#include "../include/tinyobj_loader.h"

/** Estrutura que representa uma malha 3D carregada. 
 *  @param positions Array de posições dos vértices (x, y, z)
 *  @param texcoords Array de coordenadas UV para cada vértice
 *  @param normals Array de normais para cada vértice (x, y, z)
 *  @param triangle_material_ids Array de IDs de material para cada triângulo
 *  @param triangle_colors Cor difusa derivada de cada triângulo, usada pelo backend GPU
 *  @param vertex_count Número total de vértices na malha
 *  @param triangle_count Número total de triângulos na malha
*/
struct Mesh {
    Vec3 *positions;    // Array de posições dos vértices (x, y, z)
    Vec2 *texcoords;    // Array de coordenadas UV para cada vértice
    Vec3 *normals;      // Array de normais para cada vértice (x, y, z)
    int *triangle_material_ids; // Array de IDs de material para cada triângulo
    Vec3 *triangle_colors; // Cor difusa calculada para cada triângulo, paralela a triangle_material_ids
    size_t vertex_count; // Número total de vértices na malha
    size_t triangle_count; // Número total de triângulos na malha
};

/** Função auxiliar para duplicar uma string. 
 *  @param source String de origem a ser duplicada
 *  @return Ponteiro para a nova string duplicada ou NULL em caso de erro
 */
static char *gfx_mesh_strdup(const char *source) {
    size_t length;
    char *copy;

    if (!source) {
        return NULL;
    }

    length = strlen(source);
    copy = (char *)malloc(length + 1);
    if (!copy) {
        return NULL;
    }

    memcpy(copy, source, length + 1);
    return copy;
}

/** Função auxiliar para resolver o caminho completo de um arquivo. 
 *  @param filename Nome do arquivo a ser resolvido
 *  @param is_mtl Indica se o arquivo é um MTL
 *  @param obj_filename Nome do arquivo OBJ de referência
 *  @return Ponteiro para a string com o caminho resolvido ou NULL em caso de erro
 */
static char *gfx_mesh_resolve_path(const char *filename, int is_mtl, const char *obj_filename) {
    const char *slash;
    size_t base_length;
    size_t filename_length;
    char *resolved;

    if (!filename) {
        return NULL;
    }

    if (!is_mtl || !obj_filename || filename[0] == '/' || strchr(filename, '/') != NULL ||
        strcmp(filename, obj_filename) == 0) {
        return gfx_mesh_strdup(filename);
    }

    slash = strrchr(obj_filename, '/');
    if (!slash) {
        return gfx_mesh_strdup(filename);
    }

    base_length = (size_t)(slash - obj_filename) + 1;
    filename_length = strlen(filename);
    resolved = (char *)malloc(base_length + filename_length + 1);
    if (!resolved) {
        return NULL;
    }

    memcpy(resolved, obj_filename, base_length);
    memcpy(resolved + base_length, filename, filename_length + 1);
    return resolved;
}

/** Função auxiliar para ler o conteúdo de um arquivo. 
 *  @param path Caminho do arquivo a ser lido
 *  @param data_out Ponteiro para armazenar o conteúdo lido
 *  @param len_out Ponteiro para armazenar o tamanho do conteúdo lido
 *  @return 0 em caso de sucesso, -1 em caso de erro
 */
static int gfx_mesh_read_file(const char *path, char **data_out, size_t *len_out) {
    FILE *file;
    long file_size;
    size_t read_size;
    char *data;

    if (!path || !data_out || !len_out) {
        return -1;
    }

    *data_out = NULL;
    *len_out = 0;

    file = fopen(path, "rb");
    if (!file) {
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return -1;
    }

    rewind(file);

    data = (char *)malloc((size_t)file_size + 1);
    if (!data) {
        fclose(file);
        return -1;
    }

    read_size = fread(data, 1, (size_t)file_size, file);
    if (read_size != (size_t)file_size) {
        free(data);
        fclose(file);
        return -1;
    }

    data[file_size] = '\0';
    fclose(file);

    *data_out = data;
    *len_out = (size_t)file_size;
    return 0;
}

/** Função auxiliar para ler o conteúdo de um arquivo para o tinyobj_loader.
 *  @param ctx Contexto opcional, atualmente não utilizado
 *  @param filename Nome do arquivo a ser lido
 *  @param is_mtl Indica se o arquivo é um MTL
 *  @param obj_filename Nome do arquivo OBJ de referência
 *  @param buf Ponteiro para armazenar o buffer lido
 *  @param len Ponteiro para armazenar o tamanho do buffer lido
 */
static void gfx_mesh_file_reader(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len) {
    char *resolved_path;
    char *data = NULL;
    size_t data_len = 0;

    (void)ctx;

    if (!buf || !len) {
        return;
    }

    *buf = NULL;
    *len = 0;

    resolved_path = gfx_mesh_resolve_path(filename, is_mtl, obj_filename);
    if (!resolved_path) {
        return;
    }

    if (gfx_mesh_read_file(resolved_path, &data, &data_len) != 0) {
        free(resolved_path);
        return;
    }

    free(resolved_path);

    *buf = data;
    *len = data_len;
}

/** Função auxiliar para liberar uma malha.
 *  @param mesh Malha a ser liberada
 */
static void gfx_mesh_release(Mesh *mesh) {
    if (!mesh) {
        return;
    }

    free(mesh->positions);
    free(mesh->texcoords);
    free(mesh->normals);
    free(mesh->triangle_material_ids);
    free(mesh->triangle_colors);
    free(mesh);
}

/** Função auxiliar para copiar os dados de um vértice.
 *  @param attrib Atributos da malha
 *  @param index Índice do vértice
 *  @param position Ponteiro para armazenar a posição do vértice
 *  @param texcoord Ponteiro para armazenar as coordenadas de textura do vértice
 *  @param normal Ponteiro para armazenar a normal do vértice
 *  @return 0 em caso de sucesso, -1 em caso de erro
 */
static int gfx_mesh_copy_vertex(const TinyObj_Attrib *attrib,
                                TinyObj_Vertex_Index index,
                                Vec3 *position,
                                Vec2 *texcoord,
                                Vec3 *normal) {
    Vec2 uv = { 0.0f, 0.0f };
    Vec3 nrm = { 0.0f, 0.0f, 0.0f };

    if (!attrib || !position || !texcoord || !normal) {
        return -1;
    }

    *position = tinyobj_attrib_get_vertex(attrib, index.v_idx);

    if (attrib->texcoords && index.vt_idx >= 0 && (unsigned int)index.vt_idx < attrib->num_texcoords) {
        size_t texcoord_index = (size_t)index.vt_idx * 2U;
        uv.x = attrib->texcoords[texcoord_index + 0];
        uv.y = attrib->texcoords[texcoord_index + 1];
    }

    if (attrib->normals && index.vn_idx >= 0 && (unsigned int)index.vn_idx < attrib->num_normals) {
        size_t normal_index = (size_t)index.vn_idx * 3U;
        nrm.x = attrib->normals[normal_index + 0];
        nrm.y = attrib->normals[normal_index + 1];
        nrm.z = attrib->normals[normal_index + 2];
    }

    *texcoord = uv;
    *normal = nrm;
    return 0;
}

/** Função auxiliar para contar o número de triângulos em uma malha.
 *  @param attrib Atributos da malha
 *  @param triangle_count_out Ponteiro para armazenar o número de triângulos
 *  @return 0 em caso de sucesso, -1 em caso de erro
 */
static int gfx_mesh_count_triangles(const TinyObj_Attrib *attrib, size_t *triangle_count_out) {
    size_t triangle_count = 0;
    size_t face;

    if (!attrib || !triangle_count_out || !attrib->face_num_verts || attrib->num_face_num_verts == 0) {
        return -1;
    }

    for (face = 0; face < (size_t)attrib->num_face_num_verts; ++face) {
        int face_vertices = attrib->face_num_verts[face];

        if (face_vertices < 3) {
            continue;
        }

        if (triangle_count > SIZE_MAX - ((size_t)face_vertices - 2U)) {
            return -1;
        }

        triangle_count += (size_t)face_vertices - 2U;
    }

    if (triangle_count == 0) {
        return -1;
    }

    *triangle_count_out = triangle_count;
    return 0;
}

/** Função principal para carregar uma malha a partir de um arquivo OBJ.
 *  @param path Caminho do arquivo OBJ
 *  @return Ponteiro para a malha carregada ou NULL em caso de erro
 */
Mesh *gfx_mesh_load(const char *path) {
    TinyObj_Attrib attrib = { 0 };  // Atributos da malha carregada pelo tinyobj_loader
    TinyObj_Shape *shapes = NULL;   // Formas da malha carregadas pelo tinyobj_loader
    TinyObj_Material *materials = NULL; // Materiais da malha carregados pelo tinyobj_loader
    Mesh *mesh = NULL;  // Malha a ser carregada
    size_t num_shapes = 0;  // Número de formas carregadas
    size_t num_materials = 0;  // Número de materiais carregados
    size_t triangle_count = 0;  // Número de triângulos na malha
    size_t vertex_count;    // Número de vértices na malha
    size_t face;    // Índice da face atual
    size_t face_offset = 0; // Deslocamento para acessar os vértices da face atual
    size_t triangle_index = 0;  // Índice do triângulo atual
    size_t vertex_index = 0;    // Índice do vértice atual
    int load_result;    // Resultado do carregamento da malha pelo tinyobj_loader

    if (!path || path[0] == '\0') {
        return NULL;
    }

    tinyobj_attrib_init(&attrib);

    load_result = tinyobj_load_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                                   path, gfx_mesh_file_reader, NULL, TINYOBJ_FLAG_TRIANGULATE);

    if (load_result != TINYOBJ_SUCCESS) {
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return NULL;
    }

    if (gfx_mesh_count_triangles(&attrib, &triangle_count) != 0 || triangle_count > SIZE_MAX / 3U) {
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return NULL;
    }

    vertex_count = triangle_count * 3U;
    if (vertex_count > SIZE_MAX / sizeof(Vec3) || vertex_count > SIZE_MAX / sizeof(Vec2) ||
        triangle_count > SIZE_MAX / sizeof(int)) {
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return NULL;
    }

    mesh = (Mesh *)calloc(1, sizeof(*mesh));
    if (!mesh) {
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        return NULL;
    }

    mesh->positions = (Vec3 *)calloc(vertex_count, sizeof(*mesh->positions));
    mesh->texcoords = (Vec2 *)calloc(vertex_count, sizeof(*mesh->texcoords));
    mesh->normals = (Vec3 *)calloc(vertex_count, sizeof(*mesh->normals));
    mesh->triangle_material_ids = (int *)calloc(triangle_count, sizeof(*mesh->triangle_material_ids));
    mesh->triangle_colors = (Vec3 *)calloc(triangle_count, sizeof(*mesh->triangle_colors));
    mesh->vertex_count = vertex_count;
    mesh->triangle_count = triangle_count;

    if (!mesh->positions || !mesh->texcoords || !mesh->normals || !mesh->triangle_material_ids ||
        !mesh->triangle_colors) {
        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(shapes, num_shapes);
        tinyobj_materials_free(materials, num_materials);
        gfx_mesh_release(mesh);
        return NULL;
    }

    for (face = 0; face < (size_t)attrib.num_face_num_verts; ++face) {
        int face_vertices = attrib.face_num_verts[face];
        int material_id = attrib.material_ids ? attrib.material_ids[face] : -1;
        size_t local_vertex;

        if (face_vertices < 3) {
            face_offset += (size_t)face_vertices;
            continue;
        }

        for (local_vertex = 0; local_vertex + 2U < (size_t)face_vertices; ++local_vertex) {
            TinyObj_Vertex_Index index0 = attrib.faces[face_offset + 0U];
            TinyObj_Vertex_Index index1 = attrib.faces[face_offset + local_vertex + 1U];
            TinyObj_Vertex_Index index2 = attrib.faces[face_offset + local_vertex + 2U];

            mesh->triangle_material_ids[triangle_index] = material_id;
            mesh->triangle_colors[triangle_index] = tinyobj_material_color(materials, num_materials, material_id);

            gfx_mesh_copy_vertex(&attrib, index0,
                                 &mesh->positions[vertex_index + 0U],
                                 &mesh->texcoords[vertex_index + 0U],
                                 &mesh->normals[vertex_index + 0U]);
            gfx_mesh_copy_vertex(&attrib, index1,
                                 &mesh->positions[vertex_index + 1U],
                                 &mesh->texcoords[vertex_index + 1U],
                                 &mesh->normals[vertex_index + 1U]);
            gfx_mesh_copy_vertex(&attrib, index2,
                                 &mesh->positions[vertex_index + 2U],
                                 &mesh->texcoords[vertex_index + 2U],
                                 &mesh->normals[vertex_index + 2U]);

            triangle_index++;
            vertex_index += 3U;
        }

        face_offset += (size_t)face_vertices;
    }

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    return mesh;
}

/** Função para liberar uma malha.
 *  @param mesh Malha a ser liberada
 */
void gfx_mesh_free(Mesh *mesh) {
    gfx_mesh_release(mesh);
}

size_t gfx_mesh_vertex_count(const Mesh *mesh) {
    return mesh ? mesh->vertex_count : 0U;
}

size_t gfx_mesh_triangle_count(const Mesh *mesh) {
    return mesh ? mesh->triangle_count : 0U;
}

const Vec3 *gfx_mesh_positions(const Mesh *mesh) {
    return mesh ? mesh->positions : NULL;
}

const Vec3 *gfx_mesh_triangle_colors(const Mesh *mesh) {
    return mesh ? mesh->triangle_colors : NULL;
}
