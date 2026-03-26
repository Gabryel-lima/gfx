#pragma once

#include <stdint.h>

#include "../gfx_math.h"

/**
 * @file include/framebuffer.h
 * @brief API pública mínima para manipulação de framebuffer mapeado.
 */

/**
 * Abre o dispositivo de framebuffer da plataforma (por exemplo "/dev/fb0").
 * @param path Caminho para o dispositivo de framebuffer
 * @return Estrutura `Framebuffer`. Em caso de erro, `fd` será -1.
 */
Framebuffer fb_open(const char *path);

/** Fecha e desmapeia recursos associados ao framebuffer 
 * @param fb Ponteiro para Framebuffer a ser fechado
 * @return void
 * @note Após chamar esta função, o ponteiro `fb` não deve ser usado novamente sem reabri-lo.
*/
void fb_close(Framebuffer *fb);

/**
 * Define um pixel em coordenadas inteiras. x,y devem estar dentro dos limites.
 * @param fb Ponteiro para Framebuffer
 * @param x Coordenada X
 * @param y Coordenada Y
 * @param rgba Cor no formato 0xAARRGGBB (uint32_t)
 */
void fb_set_pixel(Framebuffer *fb, int x, int y, uint32_t rgba);

/** Limpa o framebuffer com uma cor RGBA 
 * @param fb Ponteiro para Framebuffer
 * @param rgba Cor de limpeza no formato 0xAARRGGBB (uint32_t)
 * @return void
*/
void fb_clear(Framebuffer *fb, uint32_t rgba);
