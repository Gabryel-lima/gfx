#pragma once

#include "gfx_raster.h"

/**
 * @brief Acesso ao framebuffer Linux (/dev/fb0). Extra específico de plataforma,
 *        não faz parte do núcleo portátil (veja gfx_raster.h).
 * @author Gabryel-lima
 * @date 2026-04-08
 * @file include/gfx_platform_linux_fb.h
 */

/** Framebuffer aberto via /dev/fb0, com o descritor de arquivo necessário
 * para fechar o dispositivo. O campo `fb` é o Framebuffer portátil comum
 * ao resto do gfx (gfx_raster.h); `fd` só existe aqui, no header de
 * plataforma, porque é um conceito específico do Linux/POSIX.
*/
typedef struct GfxLinuxFb {
    Framebuffer fb; /**< Framebuffer portátil (pixels/width/height/pitch) */
    int         fd; /**< File descriptor do dispositivo; -1 se inválido */
} GfxLinuxFb;

/**
 * Abre o dispositivo de framebuffer Linux (por exemplo, "/dev/fb0").
 * @param path Caminho para o dispositivo de framebuffer
 * @return `GfxLinuxFb` com `fd == -1` e `fb.pixels == NULL` em caso de erro.
*/
GfxLinuxFb gfx_fb_open(const char *path);

/** Fecha e desmapeia os recursos associados ao framebuffer.
 * @param handle Ponteiro para o GfxLinuxFb a ser fechado
 * @note Após chamar esta função, `handle->fb.pixels` não deve ser usado sem reabri-lo.
*/
void gfx_fb_close(GfxLinuxFb *handle);
