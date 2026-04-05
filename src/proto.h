#ifndef XD_PROTO_H
#define XD_PROTO_H
#include <stdint.h>

#define XD_MAGIC 0x58445331u
#define XD_MSG_FRAME 1
#define XD_FMT_XRGB8888 1

struct xd_msg_header {
    uint32_t magic;
    uint16_t type;
    uint16_t reserved;
    uint32_t size;
};

struct xd_frame_header {
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t reserved;
};

#endif
