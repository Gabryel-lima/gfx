#include <fcntl.h>      // open, O_RDWR
#include <errno.h>      // errno
#include <stdio.h>      // fprintf, stderr
#include <sys/ioctl.h>  // ioctl — POSIX puro
#include <sys/mman.h>   // mmap, munmap
#include <linux/fb.h>   // struct fb_var_screeninfo — único header de kernel
#include <stdint.h>     // uint32_t
#include <string.h>     // memset, strerror
#include <unistd.h>     // open, close

#include "internal/framebuffer.h"

Framebuffer gfx_fb_open(const char *path) {
    Framebuffer fb = {0};
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    fb.fd = -1;

    if (!path) {
        fprintf(stderr, "gfx_fb_open: invalid framebuffer path\n");
        return fb;
    }

    fb.fd = open(path, O_RDWR);
    if (fb.fd < 0) {
        fprintf(stderr, "gfx_fb_open: open(%s) failed: %s\n", path, strerror(errno));
        return fb;
    }

    memset(&vinfo, 0, sizeof(vinfo));
    if (ioctl(fb.fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        fprintf(stderr, "gfx_fb_open: FBIOGET_VSCREENINFO failed: %s\n", strerror(errno));
        close(fb.fd);
        fb.fd = -1;
        return fb;
    }

    memset(&finfo, 0, sizeof(finfo));
    if (ioctl(fb.fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        fprintf(stderr, "gfx_fb_open: FBIOGET_FSCREENINFO failed: %s\n", strerror(errno));
        close(fb.fd);
        fb.fd = -1;
        return fb;
    }

    if (vinfo.bits_per_pixel != 32 || (finfo.line_length % sizeof(uint32_t)) != 0) {
        fprintf(stderr, "gfx_fb_open: unsupported framebuffer format (%u bpp, line_length=%u)\n",
                vinfo.bits_per_pixel, finfo.line_length);
        close(fb.fd);
        fb.fd = -1;
        return fb;
    }

    fb.width = vinfo.xres;
    fb.height = vinfo.yres;
    fb.pitch = finfo.line_length;

    {
        size_t size = (size_t)fb.pitch * (size_t)fb.height;
        fb.pixels = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fb.fd, 0);
        if (fb.pixels == MAP_FAILED) {
            fprintf(stderr, "gfx_fb_open: mmap failed: %s\n", strerror(errno));
            fb.pixels = NULL;
            close(fb.fd);
            fb.fd = -1;
            fb.width = 0;
            fb.height = 0;
            fb.pitch = 0;
        }
    }

    return fb;
}

/** Define a cor de um pixel no framebuffer.
 *  @param fb Ponteiro para o framebuffer.
 *  @param x Coordenada x do pixel.
 *  @param y Coordenada y do pixel.
 *  @param rgba Cor do pixel no formato RGBA.
 */
void gfx_fb_set_pixel(Framebuffer *fb, int x, int y, uint32_t rgba) {
    size_t stride;

    if (!fb || !fb->pixels || x < 0 || y < 0) {
        return;
    }

    if (fb->width == 0 || fb->height == 0 || (unsigned int)x >= fb->width || (unsigned int)y >= fb->height) {
        return;
    }

    if ((fb->pitch % sizeof(uint32_t)) != 0) {
        return;
    }

    stride = (size_t)fb->pitch / sizeof(uint32_t);
    if (stride < (size_t)fb->width) {
        return;
    }

    fb->pixels[(size_t)y * stride + (size_t)x] = rgba;
}
