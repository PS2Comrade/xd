#include "protocol.h"
#include "framebuffer.h"
#include "../include/xd.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>

/* PPM output counter so each flush creates a unique file */
static int flush_count = 0;

/* receive a file descriptor over a unix socket */
static int recv_fd(int sock) {
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    char buf[CMSG_SPACE(sizeof(int))];
    char dummy;
    struct iovec io = { .iov_base = &dummy, .iov_len = 1 };

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
    void *shm_ptr = NULL;
    size_t shm_size = 0;

    printf("[server] client connected (fd=%d)\n", fd);

    while (read_exact(fd, &type, 1) == 0) {
        switch (type) {
            case XD_CMD_NEW_BUFFER: {
                xd_cmd_new_buffer_t cmd;
                cmd.type = type;
                if (read_exact(fd, (char *)&cmd + 1, sizeof(cmd) - 1) < 0)
                    goto disconnect;

                int client_fd = recv_fd(fd);
                if (client_fd < 0) {
                    fprintf(stderr, "[server] failed to receive fd\n");
                    goto disconnect;
                }

                shm_size = cmd.size;
                shm_ptr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, client_fd, 0);
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
                snprintf(path, sizeof(path), "/tmp/xd_frame_%04d.ppm",
                                 flush_count++);
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
    if (shm_ptr) {
        munmap(shm_ptr, shm_size);
        fb_set_buffer(NULL); /* reset to static buffer or handle safety */
    }
    printf("[server] client disconnected (fd=%d)\n", fd);
    close(fd);
}
