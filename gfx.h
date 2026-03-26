#pragma once

#include "gfx_math.h"

/**
 * @file gfx.h
 * @brief Interface gráfica de alto nível (API pública) para desenho de malhas.
 */

/** Encaminhamento (forward) para `Mesh` */
typedef struct Mesh Mesh; // Estrutura que representa uma malha 3D
/** Encaminhamento (forward) para `Material` */
typedef struct Material Material; // Estrutura que descreve material/atributos de render

/** Backend gráfico: conjunto de funções que um backend deve implementar 
 * @param begin_frame Função para iniciar um frame de renderização
 * @param end_frame Função para finalizar um frame de renderização
 * @param draw_mesh Função para desenhar uma malha com transformação e material
 * @param set_camera Função para configurar a câmera (posição, alvo, FOV)
*/
typedef struct GfxBackend {
    void (*begin_frame)(void *ctx);                        /**< Inicia frame */
    void (*end_frame)  (void *ctx);                        /**< Finaliza frame */
    void (*draw_mesh)  (void *ctx, Mesh *m, Mat4 transform, Material *mat); /**< Desenha uma malha */
    void (*set_camera) (void *ctx, Vec3 pos, Vec3 target, float fov);       /**< Configura câmera */
} GfxBackend;

/** Contexto gráfico principal 
 * @param backend Backend gráfico a ser usado (CPU/GPU)
 * @param backend_ctx Contexto específico do backend (CPUCtx ou GPUCtx)
*/
typedef struct GfxContext {
    GfxBackend backend;      /**< Implementação de backend (CPU/GPU) */
    void       *backend_ctx; /**< Contexto específico do backend (CPUCtx ou GPUCtx) */
} GfxContext;

/**
 * Desenha uma malha usando o contexto gráfico.
 * @param g Ponteiro para GfxContext
 * @param m Ponteiro para Mesh a ser desenhada
 * @param t Matriz de transformação (transform)
 * @param mat Material a ser usado
 */
void gfx_draw_mesh(GfxContext *g, Mesh *m, Mat4 t, Material *mat);

/**
 * Configura a câmera do contexto gráfico.
 * @param g Ponteiro para GfxContext
 * @param pos Posição da câmera
 * @param target Alvo da câmera
 * @param fov Campo de visão (Field of View)
 */
void gfx_set_camera(GfxContext *g, Vec3 pos, Vec3 target, float fov);

/** Inicia um frame de renderização.
 * @param g Ponteiro para GfxContext
 */
void gfx_begin_frame(GfxContext *g);

/** Finaliza um frame de renderização.
 * @param g Ponteiro para GfxContext
 */
void gfx_end_frame(GfxContext *g);

/** Limpa os recursos do contexto gráfico.
 * @param g Ponteiro para GfxContext
 */
void gfx_cleanup(GfxContext *g);

/** Retorna um backend de stub usado nos exemplos */
GfxBackend gfx_get_stub_backend(void);
