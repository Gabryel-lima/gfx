#pragma once

#include <stdint.h>

/**
 * @file gfx_math.h
 * @brief Tipos matemáticos e estruturas básicas usadas pelo sistema gráfico.
 */

/** Framebuffer mapeado em memória do dispositivo */
typedef struct Framebuffer {
    uint32_t *pixels;   /**< Ponteiro para pixels mapeados */
    int       fd;       /**< File descriptor do framebuffer */
    uint32_t  width, height, pitch; /**< Largura, altura e pitch (bytes por linha) */
} Framebuffer; // Estrutura que descreve o framebuffer

/** Vetor 2D float */
typedef struct Vec2 {
    float x, y; /**< coordenadas 2D ou UV */
} Vec2; // Tipo para coordenadas 2D

/** Vetor 3D float */
typedef struct Vec3 {
    float x, y, z; /**< coordenadas 3D ou cor RGB (0.0-1.0) */
} Vec3; // Tipo para coordenadas 3D

/** Vetor 4-componentes float (por exemplo RGBA ou vetor homogêneo) */
typedef struct Vec4 {
    float x, y, z, w; /**< coordenadas 3D homogêneas ou cor RGBA (0.0-1.0) */
} Vec4; // Tipo para coordenadas 4D/cores RGBA

/** Matriz 4x4 (armazenada como 4 vetores Vec4) */
typedef struct Mat4 { Vec4 col[4]; } Mat4; // Matriz de transformação 4x4

/** Calcula o valor da aresta em 2D para rasterização 
 * @param a Primeiro vértice do triângulo
 * @param b Segundo vértice do triângulo
 * @param c Ponto a ser testado
 * @return O valor da aresta (positivo se c estiver à esquerda de a->b, negativo se à direita, zero se colinear)
*/
float edge2d(Vec4 a, Vec4 b, Vec4 c);

/** Função utilitária para converter cor Vec3 (RGB) para uint32_t RGBA 
 * @param color Cor em formato Vec3 (valores entre 0.0 e 1.0)
 * @return Cor convertida para formato uint32_t RGBA (0xRRGGBBAA)
*/
uint32_t vec3_to_rgba(Vec3 color);
