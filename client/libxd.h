#ifndef LIBXD_H
#define LIBXD_H

#include "../include/xd.h"

/* Connect to the xd server. Returns 0 on success, -1 on error. */
int xd_connect(void);

/* Disconnect from the server */
void xd_disconnect(void);

/* Draw a single pixel */
int xd_draw_pixel(uint16_t x, uint16_t y, xd_color_t color);

/* Draw a line from (x1,y1) to (x2,y2) */
int xd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, xd_color_t color);

/* Draw rectangle outline */
int xd_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, xd_color_t color);

/* Draw filled rectangle */
int xd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, xd_color_t color);

/* Clear the framebuffer */
int xd_clear(void);

/* Flush framebuffer to PPM file on the server */
int xd_flush(void);

#endif /* LIBXD_H */
