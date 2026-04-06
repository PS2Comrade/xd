#include "protocol.h"
#include "../include/xd.h"
#include "framebuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

/* ppm counter, rolls over at 10000 */
static unsigned int flush_count = 0;

/* grab a memfd from the client that backs the shared framebuffer */
static int recv_fd(int sock) {
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	char buf[CMSG_SPACE(sizeof(int))];
	char dummy;
	struct iovec io = {.iov_base = &dummy, .iov_len = 1};

	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = buf;
	msg.msg_controllen = sizeof(buf);

	if (recvmsg(sock, &msg, 0) < 0) {
		perror("recvmsg");
		return -1;
	}

	cmsg = CMSG_FIRSTHDR(&msg);
	if (!cmsg || cmsg->cmsg_type != SCM_RIGHTS) {
		return -1;
	}

	return *((int *)CMSG_DATA(cmsg));
}

/* read exactly n bytes, sockets can be stingy */
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
	void *shm_ptr = NULL;
	size_t shm_size = 0;

	printf("[server] client connected (fd=%d)\n", fd);

	/* read command bytes until the client disconnects */
	while (read_exact(fd, &type, 1) == 0) {
		switch (type) {
		case XD_CMD_NEW_BUFFER: {
			xd_cmd_new_buffer_t cmd;
			cmd.type = type;
			if (read_exact(fd, (char *)&cmd + 1, sizeof(cmd) - 1) < 0)
				goto disconnect;

			/* mmap the clients memfd so we both see the same memory */
			int client_fd = recv_fd(fd);
			if (client_fd < 0) {
				fprintf(stderr, "[server] failed to receive fd\n");
				goto disconnect;
			}

			shm_size = cmd.size;
			shm_ptr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED,
						   client_fd, 0);
			close(client_fd);

			if (shm_ptr == MAP_FAILED) {
				perror("mmap");
				goto disconnect;
			}

			fb_set_buffer(shm_ptr);
			printf("[server] mapped client buffer (%zu bytes)\n", shm_size);
			break;
		}
		case XD_CMD_FLUSH: {
			char path[64];
			snprintf(path, sizeof(path), "/tmp/xd_frame_%04u.ppm", flush_count);
			flush_count = (flush_count + 1) % 10000;
			if (fb_write_ppm(path) == 0)
				printf("[server] flushed framebuffer -> %s\n", path);
			break;
		}
		case XD_CMD_CLEAR:
			fb_clear();
			printf("[server] framebuffer cleared\n");
			break;
		default:
			fprintf(stderr,
					"[server] unknown command 0x%02x, dropping client\n", type);
			goto disconnect;
		}
	}

disconnect:
	/* unmap client shm and fall back to static buffer so no dangling pointers
	 */
	if (shm_ptr && fb_is_client_buffer()) {
		munmap(shm_ptr, shm_size);
		fb_set_buffer(NULL);
	}
	printf("[server] client disconnected (fd=%d)\n", fd);
	close(fd);
}
