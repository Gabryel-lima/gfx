#include "gfx_math.h"

#include <stdint.h>

/* Provide simple scalar implementations to avoid linker errors when libm
 * isn't linked. These are small and portable. */
float fminf(float a, float b) { return a < b ? a : b; }
float fmaxf(float a, float b) { return a > b ? a : b; }

/* Vec3 helpers */
Vec3 vec3_min(Vec3 a, Vec3 b) {
    Vec3 r = { fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z) };
    return r;
}
Vec3 vec3_max(Vec3 a, Vec3 b) {
    Vec3 r = { fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z) };
    return r;
}
Vec3 vec3_clamp(Vec3 v, float lo, float hi) {
    Vec3 r = { fmaxf(lo, fminf(hi, v.x)), fmaxf(lo, fminf(hi, v.y)), fmaxf(lo, fminf(hi, v.z)) };
    return r;
}

float edge2d(Vec4 a, Vec4 b, Vec4 c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

uint32_t vec3_to_rgba(Vec3 color) {
    Vec3 c = vec3_clamp(color, 0.0f, 1.0f);
    uint8_t r = (uint8_t)(c.x * 255);
    uint8_t g = (uint8_t)(c.y * 255);
    uint8_t b = (uint8_t)(c.z * 255);
    return (r << 24) | (g << 16) | (b << 8) | 0xFF; // RGBA com alpha fixo em 255
}
