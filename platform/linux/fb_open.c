#include <fcntl.h>      // open, O_RDWR
#include <errno.h>      // errno
#include <stdio.h>      // fprintf, stderr
#include <sys/ioctl.h>  // ioctl — POSIX puro
#include <sys/mman.h>   // mmap, munmap
#include <linux/fb.h>   // struct fb_var_screeninfo — único header de kernel
#include <stdint.h>     // uint32_t
#include <string.h>     // memset, strerror
#include <unistd.h>     // open, close

#include "gfx_platform_linux_fb.h"

GfxLinuxFb gfx_fb_open(const char *path) {
    GfxLinuxFb handle = {0};
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    handle.fd = -1;

    if (!path) {
        fprintf(stderr, "gfx_fb_open: invalid framebuffer path\n");
        return handle;
    }

    handle.fd = open(path, O_RDWR);
    if (handle.fd < 0) {
        fprintf(stderr, "gfx_fb_open: open(%s) failed: %s\n", path, strerror(errno));
        return handle;
    }

    memset(&vinfo, 0, sizeof(vinfo));
    if (ioctl(handle.fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        fprintf(stderr, "gfx_fb_open: FBIOGET_VSCREENINFO failed: %s\n", strerror(errno));
        close(handle.fd);
        handle.fd = -1;
        return handle;
    }

    memset(&finfo, 0, sizeof(finfo));
    if (ioctl(handle.fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        fprintf(stderr, "gfx_fb_open: FBIOGET_FSCREENINFO failed: %s\n", strerror(errno));
        close(handle.fd);
        handle.fd = -1;
        return handle;
    }

    if (vinfo.bits_per_pixel != 32 || (finfo.line_length % sizeof(uint32_t)) != 0) {
        fprintf(stderr, "gfx_fb_open: unsupported framebuffer format (%u bpp, line_length=%u)\n",
                vinfo.bits_per_pixel, finfo.line_length);
        close(handle.fd);
        handle.fd = -1;
        return handle;
    }

    handle.fb.width = vinfo.xres;
    handle.fb.height = vinfo.yres;
    handle.fb.pitch = finfo.line_length;

    {
        size_t size = (size_t)handle.fb.pitch * (size_t)handle.fb.height;
        handle.fb.pixels = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, handle.fd, 0);
        if (handle.fb.pixels == MAP_FAILED) {
            fprintf(stderr, "gfx_fb_open: mmap failed: %s\n", strerror(errno));
            handle.fb.pixels = NULL;
            close(handle.fd);
            handle.fd = -1;
            handle.fb.width = 0;
            handle.fb.height = 0;
            handle.fb.pitch = 0;
        }
    }

    return handle;
}

void gfx_fb_close(GfxLinuxFb *handle) {
    if (!handle) return;

    if (handle->fb.pixels) {
        size_t size = (size_t)handle->fb.pitch * handle->fb.height;
        munmap(handle->fb.pixels, size);
        handle->fb.pixels = NULL;
    }

    if (handle->fd >= 0) {
        close(handle->fd);
        handle->fd = -1;
    }
}
