#include "libxd.h"
#include <stdio.h>

int main(void) {
	if (xd_connect() < 0) {
		fprintf(stderr, "demo: could not connect to xd server at %s\n",
				XD_SOCKET_PATH);
		return 1;
	}

	printf("demo: connected to xd server\n");

	/* start fresh */
	xd_clear();

	/* dark blue background */
	xd_fill_rect(0, 0, XD_WIDTH, XD_HEIGHT, XD_COLOR(20, 20, 60));

	/* white border */
	xd_draw_rect(0, 0, XD_WIDTH, XD_HEIGHT, XD_WHITE);

	/* diagonal cross, because every demo needs an X */
	xd_draw_line(0, 0, XD_WIDTH - 1, XD_HEIGHT - 1, XD_CYAN);
	xd_draw_line(XD_WIDTH - 1, 0, 0, XD_HEIGHT - 1, XD_CYAN);

	/* 8x6 grid of colored rectangles */
	int cols = 8, rows = 6;
	int cw = XD_WIDTH / cols;
	int ch = XD_HEIGHT / rows;
	xd_color_t palette[] = {
		XD_RED,	 XD_GREEN,	 XD_BLUE,  XD_YELLOW,
		XD_CYAN, XD_MAGENTA, XD_WHITE, XD_COLOR(128, 64, 0)};
	int num_colors = (int)(sizeof(palette) / sizeof(palette[0]));

	int r, c;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			xd_color_t col = palette[(r * cols + c) % num_colors];
			/* Draw outline rectangle for each cell */
			xd_draw_rect(c * cw + 4, r * ch + 4, cw - 8, ch - 8, col);
		}
	}

	/* rgb bars in the center */
	xd_fill_rect(300, 200, 200, 40, XD_RED);
	xd_fill_rect(300, 250, 200, 40, XD_GREEN);
	xd_fill_rect(300, 300, 200, 40, XD_BLUE);

	/* 200 yellow pixels */
	int i;
	xd_color_t spark = XD_YELLOW;
	for (i = 0; i < 200; i++) {
		uint16_t px = (uint16_t)((i * 137) % XD_WIDTH);
		uint16_t py = (uint16_t)((i * 97) % XD_HEIGHT);
		xd_draw_pixel(px, py, spark);
	}

	/* server writes ppm to /tmp */
	xd_flush();
	printf("demo: flushed frame -> /tmp/xd_frame_0000.ppm\n");

	xd_disconnect();
	printf("demo: done\n");
	return 0;
}
