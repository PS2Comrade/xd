#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../include/xd.h"

/* init and fill with black */
void fb_init(void);

/* point the server at a specific memory location */
void fb_set_buffer(xd_color_t *buffer);

/* see  if we are using a client shared buffer right now */
int fb_is_client_buffer(void);

/* set a single pixel */
void fb_set_pixel(int x, int y, xd_color_t color);

/* bresenhams line */
void fb_draw_line(int x1, int y1, int x2, int y2, xd_color_t color);

/* rectangle outline */
void fb_draw_rect(int x, int y, int w, int h, xd_color_t color);

/* filled rectangle */
void fb_fill_rect(int x, int y, int w, int h, xd_color_t color);

/* clear to black */
void fb_clear(void);

/* dump to ppm file */
int fb_write_ppm(const char *path);

#endif /* FRAMEBUFFER_H */
