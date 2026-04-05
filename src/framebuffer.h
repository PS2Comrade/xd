#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../include/xd.h"

/* Initialize the framebuffer (fills with black) */
void fb_init(void);

/* Set a single pixel */
void fb_set_pixel(int x, int y, xd_color_t color);

/* Draw a line using Bresenham's algorithm */
void fb_draw_line(int x1, int y1, int x2, int y2, xd_color_t color);

/* Draw rectangle outline */
void fb_draw_rect(int x, int y, int w, int h, xd_color_t color);

/* Fill a rectangle */
void fb_fill_rect(int x, int y, int w, int h, xd_color_t color);

/* Clear the framebuffer to black */
void fb_clear(void);

/* Write framebuffer contents to a PPM file */
int fb_write_ppm(const char *path);

#endif /* FRAMEBUFFER_H */
