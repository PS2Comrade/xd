CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -I./include
LDFLAGS =

SRV_SRCS = src/server.c src/framebuffer.c src/protocol.c
CLI_SRCS = client/demo.c client/libxd.c

SRV_BIN  = xd_server
CLI_BIN  = xd_demo

.PHONY: all clean

all: $(SRV_BIN) $(CLI_BIN)

$(SRV_BIN): $(SRV_SRCS) src/framebuffer.h src/protocol.h include/xd.h
	$(CC) $(CFLAGS) -o $@ $(SRV_SRCS) $(LDFLAGS)

$(CLI_BIN): $(CLI_SRCS) client/libxd.h include/xd.h
	$(CC) $(CFLAGS) -o $@ $(CLI_SRCS) $(LDFLAGS)

clean:
	rm -f $(SRV_BIN) $(CLI_BIN) /tmp/xd_frame_*.ppm /tmp/xd.sock
