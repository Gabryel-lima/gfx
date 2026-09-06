#pragma once

#include <stddef.h>

#include "gfx_math.h"

/**
 * @brief Fachada pública e dispatcher dos backends gráficos.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file include/gfx.h
 */

/** Tipo opaco para uma malha 3D carregada de arquivo (definido em core/gfx_mesh.c). */
typedef struct Mesh Mesh;

/** Tabela de callbacks implementada por um backend gráfico.
 * `cleanup` é opcional: um backend sem estado a liberar pode deixá-lo NULL.
*/
typedef struct GfxBackend {
    void (*begin_frame)(void *ctx);                        /**< Inicia um frame */
    void (*end_frame)  (void *ctx);                        /**< Finaliza um frame */
    void (*draw_mesh)  (void *ctx, Mesh *m, Mat4 transform); /**< Desenha uma malha */
    void (*set_camera) (void *ctx, Vec3 pos, Vec3 target, float fov);       /**< Configura a câmera */
    void (*cleanup)    (void *ctx);                        /**< Libera recursos do backend, se houver */
} GfxBackend;

/** Contexto opaco que guarda o backend ativo e seu estado específico. */
typedef struct GfxContext {
    GfxBackend backend;      /**< Implementação do backend ativo */
    void       *backend_ctx; /**< Estado interno do backend ativo */
} GfxContext;

/**
 * Inicializa um GfxContext com um backend e seu estado.
 * @param g Ponteiro para o GfxContext a inicializar
 * @param backend Backend a ser usado (ex.: gfx_get_stub_backend())
 * @param backend_ctx Estado específico do backend (pode ser NULL)
 */
void gfx_context_init(GfxContext *g, GfxBackend backend, void *backend_ctx);

/**
 * Desenha uma malha usando o backend ativo.
 * @param g Ponteiro para GfxContext
 * @param m Ponteiro para Mesh a ser desenhada
 * @param t Matriz de transformação (transform)
 */
void gfx_draw_mesh(GfxContext *g, Mesh *m, Mat4 t);

/**
 * Atualiza a câmera do backend ativo.
 * @param g Ponteiro para GfxContext
 * @param pos Posição da câmera
 * @param target Alvo da câmera
 * @param fov Campo de visão (Field of View)
 */
void gfx_set_camera(GfxContext *g, Vec3 pos, Vec3 target, float fov);

/** Inicia um frame no backend ativo.
 * @param g Ponteiro para GfxContext
 */
void gfx_begin_frame(GfxContext *g);

/** Finaliza um frame no backend ativo.
 * @param g Ponteiro para GfxContext
 */
void gfx_end_frame(GfxContext *g);

/** Libera recursos associados ao backend ativo, quando houver.
 * @param g Ponteiro para GfxContext
 * @note Chama `g->backend.cleanup`, se o backend tiver registrado um.
 */
void gfx_cleanup(GfxContext *g);

/** Retorna o backend stub usado pelos exemplos e smoke tests. */
GfxBackend gfx_get_stub_backend(void);

/** Carrega uma malha a partir de um arquivo OBJ.
 * @param path Caminho para o arquivo da malha
 * @return Ponteiro para Mesh carregada ou NULL em caso de erro.
 * @note Implementação em core/gfx_mesh.c; não depende de GL/X11.
*/
Mesh *gfx_mesh_load(const char *path);

/** Libera uma `Mesh` previamente carregada com `gfx_mesh_load`.
 * @param m Ponteiro para a Mesh a ser liberada
*/
void gfx_mesh_free(Mesh *m);

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

/** Retorna o array contíguo de normais por vértice.
 * @param mesh Ponteiro para a malha.
 * @return Ponteiro para a primeira normal ou NULL em caso de erro.
 * @note O array é paralelo ao de posições: uma normal por vértice.
 * @note Nunca vem zerado por falta de dados no arquivo. Se o OBJ não trouxer
 *       diretivas `vn`, cada vértice recebe a normal geométrica do próprio
 *       triângulo — sombreamento facetado em vez de superfície preta. Normais
 *       presentes no arquivo são preservadas como estão.
 * @note Os dados são não-possuidos pelo chamador e permanecem válidos até `gfx_mesh_free`.
 */
const Vec3 *gfx_mesh_normals(const Mesh *mesh);

/** Retorna o array contíguo de coordenadas de textura por vértice.
 * @param mesh Ponteiro para a malha.
 * @return Ponteiro para o primeiro UV ou NULL em caso de erro.
 * @note O array é paralelo ao de posições: um UV por vértice. Vértices de um
 *       OBJ sem diretivas `vt` ficam com (0,0).
 * @note Os dados são não-possuidos pelo chamador e permanecem válidos até `gfx_mesh_free`.
 */
const Vec2 *gfx_mesh_texcoords(const Mesh *mesh);
