#pragma once

#include "gfx_math.h"

/**
 * @brief Fachada pública e dispatcher dos backends gráficos.
 * @author Gabryel-lima
 * @date 2026-05-02
 * @file include/gfx.h
 */

/** Tipo opaco para uma malha 3D. */
typedef struct Mesh Mesh;
/** Tipo opaco para um material de render. */
typedef struct Material Material;

/** Tabela de callbacks implementada por um backend gráfico. */
typedef struct GfxBackend {
    void (*begin_frame)(void *ctx);                        /**< Inicia um frame */
    void (*end_frame)  (void *ctx);                        /**< Finaliza um frame */
    void (*draw_mesh)  (void *ctx, Mesh *m, Mat4 transform, Material *mat); /**< Desenha uma malha */
    void (*set_camera) (void *ctx, Vec3 pos, Vec3 target, float fov);       /**< Configura a câmera */
} GfxBackend;

/** Contexto opaco que guarda o backend ativo e seu estado específico. */
typedef struct GfxContext {
    GfxBackend backend;      /**< Implementação do backend ativo */
    void       *backend_ctx; /**< Estado interno do backend ativo */
} GfxContext;

/**
 * Desenha uma malha usando o backend ativo.
 * @param g Ponteiro para GfxContext
 * @param m Ponteiro para Mesh a ser desenhada
 * @param t Matriz de transformação (transform)
 * @param mat Material a ser usado
 */
void gfx_draw_mesh(GfxContext *g, Mesh *m, Mat4 t, Material *mat);

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

/** Libera recursos associados ao contexto gráfico, quando houver.
 * @param g Ponteiro para GfxContext
 */
void gfx_cleanup(GfxContext *g);

/** Retorna o backend stub usado pelos exemplos e smoke tests. */
GfxBackend gfx_get_stub_backend(void);
