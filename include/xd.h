#ifndef XD_H
#define XD_H

#include <stdint.h>

/* Socket path */
#define XD_SOCKET_PATH "/tmp/xd.sock"

/* Framebuffer dimensions */
#define XD_WIDTH  800
#define XD_HEIGHT 600

/* Command types */
#define XD_CMD_DRAW_PIXEL 0x01
#define XD_CMD_DRAW_LINE  0x02
#define XD_CMD_DRAW_RECT  0x03
#define XD_CMD_FILL_RECT  0x04
#define XD_CMD_FLUSH      0x05
#define XD_CMD_CLEAR      0x06

/* RGBA color */
typedef struct {
    uint8_t r, g, b, a;
} xd_color_t;

/* Fixed-size command structs - sent over socket as raw bytes */

typedef struct {
    uint8_t type;       /* XD_CMD_DRAW_PIXEL */
    uint16_t x, y;
    xd_color_t color;
} __attribute__((packed)) xd_cmd_draw_pixel_t;

typedef struct {
    uint8_t type;       /* XD_CMD_DRAW_LINE */
    uint16_t x1, y1, x2, y2;
    xd_color_t color;
} __attribute__((packed)) xd_cmd_draw_line_t;

typedef struct {
    uint8_t type;       /* XD_CMD_DRAW_RECT or XD_CMD_FILL_RECT */
    uint16_t x, y, w, h;
    xd_color_t color;
} __attribute__((packed)) xd_cmd_draw_rect_t;

typedef struct {
    uint8_t type;       /* XD_CMD_FLUSH */
} __attribute__((packed)) xd_cmd_flush_t;

typedef struct {
    uint8_t type;       /* XD_CMD_CLEAR */
} __attribute__((packed)) xd_cmd_clear_t;

/* Helper macros for colors */
#define XD_COLOR(r, g, b)    ((xd_color_t){(r), (g), (b), 255})
#define XD_RGBA(r, g, b, a)  ((xd_color_t){(r), (g), (b), (a)})

#define XD_BLACK   XD_COLOR(0,   0,   0)
#define XD_WHITE   XD_COLOR(255, 255, 255)
#define XD_RED     XD_COLOR(255, 0,   0)
#define XD_GREEN   XD_COLOR(0,   255, 0)
#define XD_BLUE    XD_COLOR(0,   0,   255)
#define XD_YELLOW  XD_COLOR(255, 255, 0)
#define XD_CYAN    XD_COLOR(0,   255, 255)
#define XD_MAGENTA XD_COLOR(255, 0,   255)

#endif /* XD_H */
