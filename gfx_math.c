#include "gfx_math.h"

float edge2d(Vec4 a, Vec4 b, Vec4 c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

uint32_t vec3_to_rgba(Vec3 color) {
    uint8_t r = (uint8_t)(fmaxf(0.0f, fminf(1.0f, color.x)) * 255);
    uint8_t g = (uint8_t)(fmaxf(0.0f, fminf(1.0f, color.y)) * 255);
    uint8_t b = (uint8_t)(fmaxf(0.0f, fminf(1.0f, color.z)) * 255);
    return (r << 24) | (g << 16) | (b << 8) | 0xFF; // RGBA com alpha fixo em 255
}
