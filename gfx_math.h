#pragma once

#include <stdint.h>

/**
 * @file gfx_math.h
 * @brief Tipos matemáticos e estruturas básicas usadas pelo sistema gráfico.
 */

/** Framebuffer mapeado em memória do dispositivo */
typedef struct { 
    uint32_t *pixels;   /**< Ponteiro para pixels mapeados */
    int       fd;       /**< File descriptor do framebuffer */
    uint32_t  width, height, pitch; /**< Largura, altura e pitch (bytes por linha) */
} Framebuffer; // Estrutura que descreve o framebuffer

/** Vetor 2D */
typedef struct {
    float x, y; /**< coordenadas 2D ou UV */
} Vec2; // Tipo para coordenadas 2D

/** Vetor 3D */
typedef struct {
    float x, y, z; /**< coordenadas 3D ou cor RGB (0.0-1.0) */
} Vec3; // Tipo para coordenadas 3D

/** Vetor 4-componentes (por exemplo RGBA ou vetor homogêneo) */
typedef struct {
    float x, y, z, w; /**< coordenadas 3D homogêneas ou cor RGBA (0.0-1.0) */
} Vec4; // Tipo para coordenadas 4D/cores RGBA

/** Matriz 4x4 (armazenada como 4 vetores Vec4) */
typedef struct { Vec4 col[4]; } Mat4; // Matriz de transformação 4x4
