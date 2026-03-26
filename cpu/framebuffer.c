#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#include "../include/framebuffer.h"

void fb_close(Framebuffer *fb) {
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

void fb_clear(Framebuffer *fb, uint32_t rgba) {
	if (!fb || !fb->pixels) return;
	size_t count = (size_t)fb->width * fb->height;
	for (size_t i = 0; i < count; ++i) fb->pixels[i] = rgba;
}
