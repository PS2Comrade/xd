#include "framebuffer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* In-memory RGBA framebuffer: 800x600 */
static xd_color_t fb[XD_HEIGHT][XD_WIDTH];

void fb_init(void) {
    fb_clear();
}

void fb_set_pixel(int x, int y, xd_color_t color) {
    if (x < 0 || x >= XD_WIDTH || y < 0 || y >= XD_HEIGHT)
        return;
    fb[y][x] = color;
}

void fb_draw_line(int x1, int y1, int x2, int y2, xd_color_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;

    while (1) {
        fb_set_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2)
            break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

void fb_draw_rect(int x, int y, int w, int h, xd_color_t color) {
    fb_draw_line(x,         y,         x + w - 1, y,         color); /* top */
    fb_draw_line(x,         y + h - 1, x + w - 1, y + h - 1, color); /* bottom */
    fb_draw_line(x,         y,         x,         y + h - 1, color); /* left */
    fb_draw_line(x + w - 1, y,         x + w - 1, y + h - 1, color); /* right */
}

void fb_fill_rect(int x, int y, int w, int h, xd_color_t color) {
    int cx, cy;
    int x2 = x + w;
    int y2 = y + h;
    for (cy = y; cy < y2; cy++)
        for (cx = x; cx < x2; cx++)
            fb_set_pixel(cx, cy, color);
}

void fb_clear(void) {
    memset(fb, 0, sizeof(fb));
}

int fb_write_ppm(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) {
        perror("fopen");
        return -1;
    }

    fprintf(f, "P6\n%d %d\n255\n", XD_WIDTH, XD_HEIGHT);

    int x, y;
    for (y = 0; y < XD_HEIGHT; y++) {
        for (x = 0; x < XD_WIDTH; x++) {
            fputc(fb[y][x].r, f);
            fputc(fb[y][x].g, f);
            fputc(fb[y][x].b, f);
        }
    }

    fclose(f);
    return 0;
}
