#pragma once

#include <stddef.h>

#include "../../include/gfx.h"
#include "mesh.h"

/**
 * @brief Suporte esboçado para carregamento e liberação de malhas.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file src/internal/mesh.h
*/

/** Carrega uma malha a partir de um arquivo.
 * @param path Caminho para o arquivo da malha
 * @return Ponteiro para Mesh carregada ou NULL em caso de erro.
 * @note Implementação real em gpu/mesh.c.
*/
Mesh *gfx_mesh_load(const char *path);

/** Retorna a quantidade de vértices armazenada na malha.
 * @param mesh Ponteiro para a malha.
 * @return Número de vértices ou zero se a malha for inválida.
 * @note O valor descreve o array contíguo retornado por `gfx_mesh_positions`.
 */
size_t gfx_mesh_vertex_count(const Mesh *mesh);

/** Retorna a quantidade de triângulos armazenada na malha.
 * @param mesh Ponteiro para a malha.
 * @return Número de triângulos ou zero se a malha for inválida.
 * @note Cada triângulo corresponde a 3 vértices no array de posições e a 1 cor no array de cores.
 */
size_t gfx_mesh_triangle_count(const Mesh *mesh);

/** Retorna o array contíguo de posições da malha.
 * @param mesh Ponteiro para a malha.
 * @return Ponteiro para o primeiro vértice ou NULL em caso de erro.
 * @note Os dados são não-possuidos pelo chamador e permanecem válidos até `gfx_mesh_free`.
 */
const Vec3 *gfx_mesh_positions(const Mesh *mesh);

/** Retorna a cor difusa calculada por triângulo.
 * @param mesh Ponteiro para a malha.
 * @return Ponteiro para a primeira cor de triângulo ou NULL em caso de erro.
 * @note O array é paralelo aos triângulos da malha e contém uma cor por triângulo, não por vértice.
 * @note Os dados são não-possuidos pelo chamador e permanecem válidos até `gfx_mesh_free`.
 */
const Vec3 *gfx_mesh_triangle_colors(const Mesh *mesh);

/** Libera recursos associados a uma `Mesh`.
 * @param m Ponteiro para a Mesh a ser liberada
 * @return void
 * @note Implementação real em gpu/mesh.c.
*/
void gfx_mesh_free(Mesh *m);
