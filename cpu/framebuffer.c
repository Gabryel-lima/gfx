#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#include "internal/framebuffer.h"

void gfx_fb_close(Framebuffer *fb) {
	if (!fb) return;

	if (fb->pixels) {
		size_t size = (size_t)fb->pitch * fb->height;
		munmap(fb->pixels, size);
		fb->pixels = NULL;
	}

	if (fb->fd >= 0) {
		close(fb->fd);
		fb->fd = -1;
	}
}

void gfx_fb_clear(Framebuffer *fb, uint32_t rgba) {
	size_t stride;
	size_t count;

	if (!fb || !fb->pixels || fb->width == 0 || fb->height == 0) return;
	if ((fb->pitch % sizeof(uint32_t)) != 0) return;

	stride = (size_t)fb->pitch / sizeof(uint32_t);
	if (stride < (size_t)fb->width) return;

	count = stride * (size_t)fb->height;
	for (size_t i = 0; i < count; ++i) fb->pixels[i] = rgba;
}
