#include "internal/rasterizer.h"

void rasterize_triangle(Framebuffer *fb, float *zbuf,
                        Vec4 p0, Vec4 p1, Vec4 p2,
                        Vec3 c0, Vec3 c1, Vec3 c2) {
    int minx = (int)fminf(fminf(p0.x, p1.x), p2.x);
    int maxx = (int)fmaxf(fmaxf(p0.x, p1.x), p2.x);
    int miny = (int)fminf(fminf(p0.y, p1.y), p2.y);
    int maxy = (int)fmaxf(fmaxf(p0.y, p1.y), p2.y);

    float area = edge2d(p0, p1, p2);   // área do triângulo inteiro

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            Vec4 p = {x + 0.5f, y + 0.5f, 0, 0};  // centro do pixel
            float w0 = edge2d(p1, p2, p) / area;
            float w1 = edge2d(p2, p0, p) / area;
            float w2 = edge2d(p0, p1, p) / area;

            if (w0 < 0 || w1 < 0 || w2 < 0) continue;

            // Interpolação de profundidade para z-buffer
            float z = w0 * p0.z + w1 * p1.z + w2 * p2.z;
            int idx = y * fb->width + x;
            if (z >= zbuf[idx]) continue;   // depth test
            zbuf[idx] = z;

            // Interpolação de cor
            Vec3 color = {
                w0*c0.x + w1*c1.x + w2*c2.x,
                w0*c0.y + w1*c1.y + w2*c2.y,
                w0*c0.z + w1*c1.z + w2*c2.z,
            };
            fb_set_pixel(fb, x, y, vec3_to_rgba(color));
        }
    }
}

// void rasterize_square(Framebuffer *fb, float *zbuf,
//                         Vec4 p0, Vec4 p1, Vec4 p2, Vec4 p3,
//                         Vec3 c0, Vec3 c1, Vec3 c2, Vec3 c3) {
//     // Divide o quadrado em dois triângulos: (p0,p1,p2) e (p0,p2,p3)
//     rasterize_triangle(fb, zbuf, p0, p1, p2, c0, c1, c2);
//     rasterize_triangle(fb, zbuf, p0, p2, p3, c0, c2, c3);
// }
