// ps2comrade libxd.c 
// 5 april 2026


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "../include/xd_proto.h"
#include "libxd.h"

struct xd_context {
    int sock;
    uint32_t next_surface_id;
    xd_surface_t* last_surface;
};

struct xd_surface {
    xd_context_t* ctx;
    uint32_t id;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    int shm_fd;
    uint32_t* pixels;
    size_t size;
    xd_input_callback_t input_cb;
    void* input_cb_data;
};

int send_fd(int sock, int fd, void* msg, size_t msg_len) {
    struct msghdr msgh = {0};
    struct iovec iov[1];
    char control[CMSG_SPACE(sizeof(int))] = {0};

    iov[0].iov_base = msg;
    iov[0].iov_len = msg_len;
    msgh.msg_iov = iov;
    msgh.msg_iovlen = 1;

    if (fd != -1) {
        msgh.msg_control = control;
        msgh.msg_controllen = sizeof(control);
        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msgh);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));
    }

    return sendmsg(sock, &msgh, 0);
}

xd_context_t* xd_connect(void) {
    xd_context_t* ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return NULL;
    
    ctx->sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ctx->sock < 0) {
        free(ctx);
        return NULL;
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, XD_SOCKET_PATH);

    if (connect(ctx->sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(ctx->sock);
        free(ctx);
        return NULL;
    }

    xd_msg_header_t msg = { .type = XD_MSG_HELLO, .length = sizeof(msg) };
    write(ctx->sock, &msg, sizeof(msg));
    ctx->next_surface_id = 1;

    return ctx;
}

void xd_disconnect(xd_context_t* ctx) {
    if (!ctx) return;
    close(ctx->sock);
    free(ctx);
}

xd_surface_t* xd_create_surface(xd_context_t* ctx, uint32_t width, uint32_t height) {
    xd_surface_t* surf = calloc(1, sizeof(*surf));
    surf->ctx = ctx;
    surf->id = ctx->next_surface_id++;
    surf->width = width;
    surf->height = height;
    surf->stride = width * 4;
    surf->size = surf->stride * height;

    xd_msg_create_surface_t msg_create = {
        .header = { .type = XD_MSG_CREATE_SURFACE, .length = sizeof(msg_create) },
        .width = width,
        .height = height
    };
    write(ctx->sock, &msg_create, sizeof(msg_create));

    surf->shm_fd = memfd_create("xd_surface", MFD_CLOEXEC);
    if (surf->shm_fd < 0) {
        free(surf);
        return NULL;
    }
    if (ftruncate(surf->shm_fd, surf->size) < 0) {
        close(surf->shm_fd);
        free(surf);
        return NULL;
    }
    surf->pixels = mmap(NULL, surf->size, PROT_READ | PROT_WRITE, MAP_SHARED, surf->shm_fd, 0);

    xd_msg_attach_shm_t msg_attach = {
        .header = { .type = XD_MSG_ATTACH_SHM, .length = sizeof(msg_attach) },
        .surface_id = surf->id,
        .width = width,
        .height = height,
        .stride = surf->stride,
        .format = 0
    };
    send_fd(ctx->sock, surf->shm_fd, &msg_attach, sizeof(msg_attach));

    ctx->last_surface = surf;
    return surf;
}

uint32_t* xd_surface_get_pixels(xd_surface_t* surface) {
    return surface->pixels;
}

void xd_surface_commit(xd_surface_t* surface) {
    xd_msg_commit_t msg = {
        .header = { .type = XD_MSG_COMMIT, .length = sizeof(msg) },
        .surface_id = surface->id
    };
    write(surface->ctx->sock, &msg, sizeof(msg));
}

void xd_set_input_callback(xd_surface_t* surface, xd_input_callback_t cb, void* data) {
    surface->input_cb = cb;
    surface->input_cb_data = data;
}

void xd_destroy_surface(xd_surface_t* surface) {
    if (!surface) return;
    if (surface->ctx->last_surface == surface) {
        surface->ctx->last_surface = NULL;
    }
    munmap(surface->pixels, surface->size);
    close(surface->shm_fd);
    free(surface);
}

void xd_poll_events(xd_context_t* ctx) {
    char buf[1024];
    ssize_t n = recv(ctx->sock, buf, sizeof(buf), MSG_DONTWAIT);
    if (n > 0) {
        size_t offset = 0;
        while (offset < (size_t)n) {
            xd_msg_header_t* header = (xd_msg_header_t*)(buf + offset);
            if (header->type == XD_MSG_INPUT_EVENT) {
                xd_msg_input_event_t* msg = (xd_msg_input_event_t*)(buf + offset);
                if (ctx->last_surface && ctx->last_surface->id == msg->surface_id) {
                    if (ctx->last_surface->input_cb) {
                        ctx->last_surface->input_cb(ctx->last_surface, msg->type, msg->x, msg->y, msg->button, msg->state, ctx->last_surface->input_cb_data);
                    }
                }
            }
            offset += header->length;
        }
    }
}

//son 
