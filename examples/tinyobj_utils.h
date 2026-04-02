#pragma once

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tinyobj_loader.h"

#ifndef TINYOBJ_UTILS_PATH_MAX
#ifdef PATH_MAX
#define TINYOBJ_UTILS_PATH_MAX PATH_MAX
#else
#define TINYOBJ_UTILS_PATH_MAX 4096
#endif
#endif

/**
 * @brief Helpers locais do exemplo tinyobj_demo para caminho, IO e resumo.
 * @author Gabryel-lima
 * @date 2026-05-02
 * @file examples/tinyobj_utils.h
 */

/** Imprime um resumo do modelo carregado 
 *  @param filename Nome do arquivo do modelo
 *  @param attrib Atributos do modelo
 *  @param shapes Array de formas analisadas
 *  @param num_shapes Tamanho do array `shapes`
 *  @param materials Array de materiais analisados
 *  @param num_materials Tamanho do array `materials`
 *  @param min_v Limite mínimo da caixa de limites do modelo
 *  @param max_v Limite máximo da caixa de limites do modelo
*/
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

/** Obtém o caminho do executável atual 
 *  @param buffer Buffer para armazenar o caminho do executável
 *  @param size Tamanho do buffer
 *  @return 0 em caso de sucesso, -1 em caso de erro
*/
static inline int tinyobj_get_executable_path(char *buffer, size_t size) {
    if (!buffer || size == 0) {
        return -1;
    }

#if defined(__linux__)
    ssize_t length = readlink("/proc/self/exe", buffer, size - 1);
    if (length < 0 || (size_t)length >= size) {
        return -1;
    }

    buffer[length] = '\0';
    return 0;
#else
    (void)buffer;
    (void)size;
    return -1;
#endif
}

/** Remove o nome do arquivo de um caminho, deixando apenas o diretório
 *  @param path Caminho completo do arquivo
 *  @return 0 em caso de sucesso, -1 em caso de erro
 */
static inline int tinyobj_strip_basename(char *path) {
    char *slash;

    if (!path) {
        return -1;
    }

    slash = strrchr(path, '/');
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

/** Copia um caminho de arquivo para outro buffer
 *  @param dest Buffer de destino
 *  @param size Tamanho do buffer de destino
 *  @param source Caminho de origem
 *  @return 0 em caso de sucesso, -1 em caso de erro
 */
static inline int tinyobj_copy_path(char *dest, size_t size, const char *source) {
    size_t length;

    if (!dest || !source || size == 0) {
        return -1;
    }

    length = strlen(source);
    if (length + 1 > size) {
        return -1;
    }

    memcpy(dest, source, length + 1);
    return 0;
}

/** Junta dois caminhos de arquivo em um único caminho
 *  @param dest Buffer de destino
 *  @param size Tamanho do buffer de destino
 *  @param left Caminho à esquerda
 *  @param right Caminho à direita
 *  @return 0 em caso de sucesso, -1 em caso de erro
 */
static inline int tinyobj_join_path(char *dest, size_t size, const char *left, const char *right) {
    size_t left_len;
    size_t right_len;
    size_t copy_len;
    int needs_slash;

    if (!dest || !left || !right || size == 0) {
        return -1;
    }

    left_len = strlen(left);
    right_len = strlen(right);
    needs_slash = (left_len > 0 && left[left_len - 1] != '/') ? 1 : 0;
    copy_len = left_len + (size_t)needs_slash + right_len + 1;
    if (copy_len > size) {
        return -1;
    }

    memcpy(dest, left, left_len);
    if (needs_slash) {
        dest[left_len] = '/';
        memcpy(dest + left_len + 1, right, right_len + 1);
    } else {
        memcpy(dest + left_len, right, right_len + 1);
    }

    return 0;
}

/** Resolve um caminho relativo em relação a um caminho base
 *  @param base_path Caminho base
 *  @param path Caminho relativo
 *  @param out Buffer de saída para o caminho resolvido
 *  @param size Tamanho do buffer de saída
 *  @return 0 em caso de sucesso, -1 em caso de erro
 */
static inline int tinyobj_resolve_path(const char *base_path, const char *path,
                                       char *out, size_t size) {
    char base_dir[TINYOBJ_UTILS_PATH_MAX];
    const char *slash;

    if (!path || !out || size == 0) {
        return -1;
    }

    if (!base_path || path[0] == '/') {
        return tinyobj_copy_path(out, size, path);
    }

    slash = strrchr(base_path, '/');
    if (!slash) {
        return tinyobj_copy_path(out, size, path);
    }

    if ((size_t)(slash - base_path) + 2 > sizeof(base_dir)) {
        return -1;
    }

    memcpy(base_dir, base_path, (size_t)(slash - base_path) + 1);
    base_dir[(size_t)(slash - base_path) + 1] = '\0';
    return tinyobj_join_path(out, size, base_dir, path);
}

/** Lê o conteúdo de um arquivo para um buffer
 *  @param filename Nome do arquivo
 *  @param buf Buffer de saída para o conteúdo do arquivo
 *  @param len Tamanho do buffer de saída
 *  @return 0 em caso de sucesso, -1 em caso de erro
 */
static inline int tinyobj_read_file(const char *filename, char **buf, size_t *len) {
    FILE *f;
    long n;
    size_t read_count;
    char *data;

    if (!buf || !len) {
        return -1;
    }

    *buf = NULL;
    *len = 0;

    if (!filename) {
        return -1;
    }

    f = fopen(filename, "rb");
    if (!f) {
        return -1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }

    n = ftell(f);
    if (n < 0) {
        fclose(f);
        return -1;
    }

    rewind(f);

    data = (char *)malloc((size_t)n + 1);
    if (!data) {
        fclose(f);
        return -1;
    }

    read_count = fread(data, 1, (size_t)n, f);
    if (read_count != (size_t)n && ferror(f)) {
        free(data);
        fclose(f);
        return -1;
    }

    data[read_count] = '\0';
    fclose(f);
    *buf = data;
    *len = read_count + 1;
    return 0;
}

/** Leitor de arquivos padrão para TinyOBJ
 *  @param ctx Contexto do leitor de arquivos (não utilizado)
 *  @param filename Nome do arquivo
 *  @param is_mtl Indica se o arquivo é um MTL
 *  @param obj_filename Nome do arquivo OBJ associado
 *  @param buf Buffer de saída para o conteúdo do arquivo
 *  @param len Tamanho do buffer de saída
 */
static inline void tinyobj_default_file_reader(void *ctx, const char *filename,
                                               int is_mtl, const char *obj_filename,
                                               char **buf, size_t *len) {
    char resolved[TINYOBJ_UTILS_PATH_MAX];
    const char *path = filename;

    (void)ctx;

    if (buf) {
        *buf = NULL;
    }
    if (len) {
        *len = 0;
    }

    if (!filename || !buf || !len) {
        return;
    }

    if (is_mtl && obj_filename && filename[0] != '/') {
        if (tinyobj_resolve_path(obj_filename, filename, resolved, sizeof(resolved)) == 0) {
            path = resolved;
        }
    }

    if (tinyobj_read_file(path, buf, len) != 0) {
        *buf = NULL;
        *len = 0;
    }
}
