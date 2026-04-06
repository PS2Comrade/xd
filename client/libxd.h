#ifndef LIBXD_H
#define LIBXD_H

#include "../include/xd.h"

/* connect to server, 0 on success */
int xd_connect(void);

/* disconnect */
void xd_disconnect(void);

/* get the shared framebuffer pointer */
xd_color_t *xd_get_fb(void);

/* draw a pixel */
int xd_draw_pixel(uint16_t x, uint16_t y, xd_color_t color);

/* draw a line */
int xd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, xd_color_t color);

/* rectangle outline */
int xd_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, xd_color_t color);

/* filled rectangle */
int xd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, xd_color_t color);

/* clear framebuffer */
int xd_clear(void);

/* tell server to write ppm */
int xd_flush(void);

#endif /* LIBXD_H */
