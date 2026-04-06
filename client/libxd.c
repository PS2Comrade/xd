#define _GNU_SOURCE
#include "libxd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int sock_fd = -1;
static xd_color_t *shm_fb = NULL;
static size_t shm_size = 0;

/* pass a memfd to the server over a unix socket via SCM_RIGHTS */
static int send_fd(int sock, int fd) {
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	char buf[CMSG_SPACE(sizeof(int))];
	struct iovec io = {.iov_base = "", .iov_len = 1};

	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = buf;
	msg.msg_controllen = sizeof(buf);

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));

	*((int *)CMSG_DATA(cmsg)) = fd;

	if (sendmsg(sock, &msg, 0) < 0) {
		perror("sendmsg");
		return -1;
	}
	return 0;
}

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

	/* memfd as the shared framebuffer, server will mmap it too */
	shm_size = XD_WIDTH * XD_HEIGHT * sizeof(xd_color_t);
	int fd = memfd_create("xd-fb", 0);
	if (fd < 0) {
		perror("memfd_create");
		return -1;
	}

	if (ftruncate(fd, shm_size) < 0) {
		perror("ftruncate");
		close(fd);
		return -1;
	}

	shm_fb = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shm_fb == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return -1;
	}

	/* tell the server about the buffer and hand over the fd */
	xd_cmd_new_buffer_t cmd;
	cmd.type = XD_CMD_NEW_BUFFER;
	cmd.size = (uint32_t)shm_size;
	if (write(sock_fd, &cmd, sizeof(cmd)) != sizeof(cmd)) {
		perror("write cmd_new_buffer");
		return -1;
	}

	if (send_fd(sock_fd, fd) < 0) {
		close(fd);
		return -1;
	}

	/* server has its own ref now */
	close(fd);
	return 0;
}

void xd_disconnect(void) {
	if (shm_fb) {
		munmap(shm_fb, shm_size);
		shm_fb = NULL;
	}
	if (sock_fd >= 0) {
		close(sock_fd);
		sock_fd = -1;
	}
}

xd_color_t *xd_get_fb(void) { return shm_fb; }

/* drain all bytes to the socket */
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
	if (shm_fb && x < XD_WIDTH && y < XD_HEIGHT) {
		shm_fb[y * XD_WIDTH + x] = color;
		return 0;
	}
	return -1;
}

int xd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
				 xd_color_t color) {
	if (!shm_fb)
		return -1;

	/* same bresenham as the server side */
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	while (1) {
		xd_draw_pixel(x1, y1, color);
		if (x1 == x2 && y1 == y2)
			break;
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}
	return 0;
}

int xd_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
				 xd_color_t color) {
	if (!shm_fb)
		return -1;
	/* promote to int so x+w and y+h don't wrap on uint16_t */
	int x2 = (int)x + (int)w - 1;
	int y2 = (int)y + (int)h - 1;
	xd_draw_line(x, y, x2, y, color);
	xd_draw_line(x, y2, x2, y2, color);
	xd_draw_line(x, y, x, y2, color);
	xd_draw_line(x2, y, x2, y2, color);
	return 0;
}

int xd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
				 xd_color_t color) {
	if (!shm_fb)
		return -1;
	/* same int promotion trick as xd_draw_rect */
	int x_end = (int)x + (int)w;
	int y_end = (int)y + (int)h;
	for (int cy = y; cy < y_end; cy++)
		for (int cx = x; cx < x_end; cx++)
			xd_draw_pixel(cx, cy, color);
	return 0;
}

int xd_clear(void) {
	if (!shm_fb)
		return -1;
	/* server's current_fb is the same shm, so just tell it to clear */
	xd_cmd_clear_t cmd = {.type = XD_CMD_CLEAR};
	return send_all(&cmd, sizeof(cmd));
}

int xd_flush(void) {
	xd_cmd_flush_t cmd;
	cmd.type = XD_CMD_FLUSH;
	return send_all(&cmd, sizeof(cmd));
}
