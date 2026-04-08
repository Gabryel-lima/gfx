#pragma once

#include <math.h>

#include "framebuffer.h"
#include "gfx_math.h"

/**
 * @brief Rasterização software de triângulos para o caminho CPU.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file include/rasterizer.h
*/

/**
 * Rasteriza um único triângulo no framebuffer e no z-buffer fornecidos.
 * @param fb Ponteiro para o framebuffer
 * @param zbuf Ponteiro para o z-buffer
 * @param p0, p1, p2 Vértices do triângulo (coordenadas homogêneas)
 * @param c0, c1, c2 Cores dos vértices (RGB)
*/
void gfx_rasterize_triangle(Framebuffer *fb, float *zbuf,
                            Vec4 p0, Vec4 p1, Vec4 p2,
                            Vec3 c0, Vec3 c1, Vec3 c2);
