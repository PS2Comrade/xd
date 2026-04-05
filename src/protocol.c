#include "protocol.h"
#include "framebuffer.h"
#include "../include/xd.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* PPM output counter so each flush creates a unique file */
static int flush_count = 0;

/* Read exactly n bytes from fd, returns 0 on success, -1 on disconnect/error */
static int read_exact(int fd, void *buf, size_t n) {
    size_t total = 0;
    ssize_t got;
    while (total < n) {
        got = read(fd, (char *)buf + total, n - total);
        if (got <= 0)
            return -1;
        total += (size_t)got;
    }
    return 0;
}

void protocol_handle_client(int fd) {
    uint8_t type;

    printf("[server] client connected (fd=%d)\n", fd);

    while (read_exact(fd, &type, 1) == 0) {
        switch (type) {
            case XD_CMD_DRAW_PIXEL: {
                xd_cmd_draw_pixel_t cmd;
                cmd.type = type;
                if (read_exact(fd, (char *)&cmd + 1, sizeof(cmd) - 1) < 0)
                    goto disconnect;
                fb_set_pixel(cmd.x, cmd.y, cmd.color);
                break;
            }
            case XD_CMD_DRAW_LINE: {
                xd_cmd_draw_line_t cmd;
                cmd.type = type;
                if (read_exact(fd, (char *)&cmd + 1, sizeof(cmd) - 1) < 0)
                    goto disconnect;
                fb_draw_line(cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.color);
                break;
            }
            case XD_CMD_DRAW_RECT:
            case XD_CMD_FILL_RECT: {
                xd_cmd_draw_rect_t cmd;
                cmd.type = type;
                if (read_exact(fd, (char *)&cmd + 1, sizeof(cmd) - 1) < 0)
                    goto disconnect;
                if (type == XD_CMD_FILL_RECT)
                    fb_fill_rect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.color);
                else
                    fb_draw_rect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.color);
                break;
            }
            case XD_CMD_FLUSH: {
                char path[64];
                snprintf(path, sizeof(path), "/tmp/xd_frame_%04d.ppm", flush_count++);
                if (fb_write_ppm(path) == 0)
                    printf("[server] flushed framebuffer -> %s\n", path);
                break;
            }
            case XD_CMD_CLEAR:
                fb_clear();
                printf("[server] framebuffer cleared\n");
                break;
            default:
                fprintf(stderr, "[server] unknown command 0x%02x, dropping client\n", type);
                goto disconnect;
        }
    }

disconnect:
    printf("[server] client disconnected (fd=%d)\n", fd);
    close(fd);
}
