#pragma once

#include "../gfx_math.h"
#include <stdint.h>

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

/** Fecha e desmapeia recursos associados ao framebuffer */
void fb_close(Framebuffer *fb);

/**
 * Define um pixel em coordenadas inteiras. x,y devem estar dentro dos limites.
 * @param fb Ponteiro para Framebuffer
 * @param x Coordenada X
 * @param y Coordenada Y
 * @param rgba Cor no formato 0xAARRGGBB (uint32_t)
 */
void fb_set_pixel(Framebuffer *fb, int x, int y, uint32_t rgba);

/** Limpa o framebuffer com uma cor RGBA */
void fb_clear(Framebuffer *fb, uint32_t rgba);
