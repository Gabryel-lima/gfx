#include <fcntl.h>      // open, O_RDWR
#include <sys/ioctl.h>  // ioctl — POSIX puro
#include <sys/mman.h>   // mmap, munmap
#include <linux/fb.h>   // struct fb_var_screeninfo — único header de kernel
#include <stdint.h>     // uint32_t
#include <unistd.h>     // open, close

#include "internal/framebuffer.h"

Framebuffer gfx_fb_open(const char *path) {
    Framebuffer fb = {0};
    fb.fd = open(path, O_RDWR);

    struct fb_var_screeninfo vinfo;
    ioctl(fb.fd, FBIOGET_VSCREENINFO, &vinfo);

    fb.width  = vinfo.xres;
    fb.height = vinfo.yres;
    fb.pitch  = vinfo.xres * (vinfo.bits_per_pixel / 8);

    size_t size = fb.pitch * fb.height;
    fb.pixels = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fb.fd, 0);
    return fb;
}

// Escrever um pixel: endereçamento direto
void gfx_fb_set_pixel(Framebuffer *fb, int x, int y, uint32_t rgba) {
    fb->pixels[y * fb->width + x] = rgba;
}
