#pragma once

#include <math.h>

#include "framebuffer.h"
#include "../gfx_math.h"

/**
 * @file include/rasterizer.h
 * @brief API pública mínima para rasterização de triângulos.
 */

/**
 * Rasteriza um único triângulo no framebuffer e z-buffer fornecidos.
 * @param fb Ponteiro para o framebuffer
 * @param zbuf Ponteiro para o z-buffer
 * @param p0, p1, p2 Vértices do triângulo (coordenadas homogêneas)
 * @param c0, c1, c2 Cores dos vértices (RGB)
 */
void rasterize_triangle(Framebuffer *fb, float *zbuf,
                        Vec4 p0, Vec4 p1, Vec4 p2,
                        Vec3 c0, Vec3 c1, Vec3 c2);

// /**
//  * Rasteriza um quadrado no framebuffer e z-buffer fornecidos.
//  * @param fb Ponteiro para o framebuffer
//  * @param zbuf Ponteiro para o z-buffer
//  * @param p0, p1, p2, p3 Vértices do quadrado (coordenadas homogêneas)
//  * @param c0, c1, c2, c3 Cores dos vértices (RGB)
//  */
// void rasterize_square(Framebuffer *fb, float *zbuf,
//                         Vec4 p0, Vec4 p1, Vec4 p2, Vec4 p3,
//                         Vec3 c0, Vec3 c1, Vec3 c2, Vec3 c3);
