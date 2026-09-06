#pragma once

#include "gfx.h"

/**
 * @brief Abstração opaca para uma janela Linux com contexto GLX e ciclo de apresentação.
 * Extra opcional específico de plataforma (Linux + X11 + OpenGL via dlopen);
 * não faz parte do núcleo do gfx (veja gfx.h e gfx_raster.h).
 * @author Gabryel-lima
 * @date 2026-04-08
 * @file include/gfx_platform_gl_window.h
 */

/** Declaração opaca para a estrutura de janela, definida em platform_window.c. */
typedef struct PlatformWindow PlatformWindow;

/** Luz direcional usada pelo sombreamento Phong do backend de janela.
 *
 *  É direcional, não pontual: os raios chegam paralelos e não há atenuação
 *  com a distância. É o modelo certo para uma fonte distante (o sol) e o que
 *  evita ter de escolher constantes de atenuação sem uma escala de cena
 *  definida.
 *
 *  @param direction Direção *da superfície para a luz*, não o contrário. Não
 *         precisa vir normalizada; o shader normaliza.
 *  @param color Cor e intensidade da luz; componentes acima de 1.0 estouram.
 *  @param ambient Termo ambiente, aplicado à cor da malha independentemente da
 *         orientação. É o que impede que faces sem luz fiquem pretas.
 *  @param specular_strength Peso do brilho especular, tipicamente 0.0 a 1.0.
 *  @param shininess Expoente especular: valores altos dão um brilho pequeno e
 *         concentrado, valores baixos dão um brilho amplo e suave.
 */
typedef struct GfxLight {
    Vec3  direction;
    Vec3  color;
    Vec3  ambient;
    float specular_strength;
    float shininess;
} GfxLight;

/** Luz padrão aplicada a uma janela recém-criada.
 *  @return Uma luz branca vinda de cima e da frente, com ambiente suave.
 *  @note Serve como ponto de partida sensato: qualquer malha carregada
 *  aparece iluminada sem que o chamador precise configurar nada.
 */
GfxLight gfx_platform_window_default_light(void);

/** Ajusta a luz usada no sombreamento das malhas.
 *  @param window Ponteiro para a janela.
 *  @param light Nova configuração de luz.
 *  @note Vale para todas as malhas desenhadas a partir do próximo frame.
 */
void gfx_platform_window_set_light(PlatformWindow *window, GfxLight light);

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

/** Lê a cor de um pixel já desenhado no frame corrente.
 *  @param window Ponteiro para a janela.
 *  @param x Coluna, com origem no canto inferior esquerdo (convenção do OpenGL).
 *  @param y Linha, com origem no canto inferior esquerdo.
 *  @param out_rgb Recebe os três componentes em [0,1].
 *  @return Zero em caso de sucesso, diferente de zero em caso de erro.
 *  @note Precisa ser chamada entre `gfx_begin_frame` e `gfx_end_frame`: lê o
 *  back buffer, que `gfx_end_frame` troca. Existe para tornar a saída do
 *  backend verificável sem alguém olhando a tela — é o que permite checar o
 *  sombreamento sob Xvfb.
 */
int gfx_platform_window_read_pixel(PlatformWindow *window,
                                   unsigned int x,
                                   unsigned int y,
                                   float out_rgb[3]);

/** Retorna o contexto público que despacha para a janela. 
 *  @param window Ponteiro para a janela que deve fornecer o contexto.
 *  @return Ponteiro para o contexto público associado à janela, ou NULL em caso de erro.
 *  @note O ponteiro retornado pertence à janela e só é válido enquanto ela existir.
*/
GfxContext *gfx_platform_window_context(PlatformWindow *window);
