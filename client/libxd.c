#include "libxd.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

static int sock_fd = -1;

int xd_connect(void) {
    struct sockaddr_un addr;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("xd_connect: socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, XD_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("xd_connect: connect");
        close(sock_fd);
        sock_fd = -1;
        return -1;
    }
    return 0;
}

void xd_disconnect(void) {
    if (sock_fd >= 0) {
        close(sock_fd);
        sock_fd = -1;
    }
}

/* Write all bytes to socket */
static int send_all(const void *buf, size_t n) {
    size_t sent = 0;
    ssize_t w;
    while (sent < n) {
        w = write(sock_fd, (const char *)buf + sent, n - sent);
        if (w <= 0) {
            perror("xd: write");
            return -1;
        }
        sent += (size_t)w;
    }
    return 0;
}

int xd_draw_pixel(uint16_t x, uint16_t y, xd_color_t color) {
    xd_cmd_draw_pixel_t cmd;
    cmd.type  = XD_CMD_DRAW_PIXEL;
    cmd.x     = x;
    cmd.y     = y;
    cmd.color = color;
    return send_all(&cmd, sizeof(cmd));
}

int xd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, xd_color_t color) {
    xd_cmd_draw_line_t cmd;
    cmd.type  = XD_CMD_DRAW_LINE;
    cmd.x1    = x1;
    cmd.y1    = y1;
    cmd.x2    = x2;
    cmd.y2    = y2;
    cmd.color = color;
    return send_all(&cmd, sizeof(cmd));
}

int xd_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, xd_color_t color) {
    xd_cmd_draw_rect_t cmd;
    cmd.type  = XD_CMD_DRAW_RECT;
    cmd.x     = x;
    cmd.y     = y;
    cmd.w     = w;
    cmd.h     = h;
    cmd.color = color;
    return send_all(&cmd, sizeof(cmd));
}

int xd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, xd_color_t color) {
    xd_cmd_draw_rect_t cmd;
    cmd.type  = XD_CMD_FILL_RECT;
    cmd.x     = x;
    cmd.y     = y;
    cmd.w     = w;
    cmd.h     = h;
    cmd.color = color;
    return send_all(&cmd, sizeof(cmd));
}

int xd_clear(void) {
    xd_cmd_clear_t cmd;
    cmd.type = XD_CMD_CLEAR;
    return send_all(&cmd, sizeof(cmd));
}

int xd_flush(void) {
    xd_cmd_flush_t cmd;
    cmd.type = XD_CMD_FLUSH;
    return send_all(&cmd, sizeof(cmd));
}
