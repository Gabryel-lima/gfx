#pragma once

#include "../../include/gfx.h"

/**
 * @brief Abstração opaca para uma janela Linux com contexto GLX e ciclo de apresentação.
 * @author Gabryel-lima
 * @date 2026-04-08
 * @file src/internal/platform_window.h
 */

/** Declaração opaca para a estrutura de janela, definida em platform_window.c. */
typedef struct PlatformWindow PlatformWindow;

/** Cria uma janela nativa Linux com contexto GLX 
 *  já associado. 
 *  @param title Título da janela.
 *  @param width Largura da janela em pixels.
 *  @param height Altura da janela em pixels.
 *  @return Ponteiro para a janela criada, ou NULL 
 *  em caso de falha.
*/
PlatformWindow *gfx_platform_window_create(const char *title,
                                           unsigned int width,
                                           unsigned int height);

/** Libera uma janela nativa Linux e os recursos 
 *  associados. 
 *  @param window Ponteiro para a janela a ser destruída.
 *  @note A janela deve ter sido criada 
 *  previamente com `gfx_platform_window_create`. 
 *  Após a chamada, o ponteiro não deve ser mais usado.
*/
void gfx_platform_window_destroy(PlatformWindow *window);

/** Processa a fila de eventos pendentes da janela. 
 *  Retorna diferente de zero ao receber 
 *  pedido de fechamento. 
 *  @param window Ponteiro para a janela que deve 
 *  processar os eventos.
 *  @return Zero se a janela deve continuar aberta, 
 *  ou diferente de zero se a janela deve ser fechada.
*/
int gfx_platform_window_pump_events(PlatformWindow *window);

/** Informa se a janela recebeu pedido de fechamento. 
 *  @param window Ponteiro para a janela que deve 
 *  verificar o estado de fechamento.
 *  @return Zero se a janela deve continuar aberta, 
 *  ou diferente de zero se a janela deve ser fechada.
*/
int gfx_platform_window_should_close(const PlatformWindow *window);

/** Ajusta a cor usada em `gfx_begin_frame`. 
 *  @param window Ponteiro para a janela que deve 
 *  ter a cor de limpeza ajustada.
 *  @param red Componente vermelho da cor de limpeza.
 *  @param green Componente verde da cor de limpeza.
 *  @param blue Componente azul da cor de limpeza.
 *  @param alpha Componente alfa da cor de limpeza.
*/
void gfx_platform_window_set_clear_color(PlatformWindow *window,
                                         float red,
                                         float green,
                                         float blue,
                                         float alpha);

/** Retorna o contexto público que despacha para a janela. 
 *  @param window Ponteiro para a janela que deve fornecer o contexto.
 *  @return Ponteiro para o contexto público associado à janela, ou NULL em caso de erro.
 *  @note O ponteiro retornado pertence à janela e só é válido enquanto ela existir.
*/
GfxContext *gfx_platform_window_context(PlatformWindow *window);
