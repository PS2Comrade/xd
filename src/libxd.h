// comradeps2 - libxd.h 
// 5 april 2026

#ifndef LIBXD_H
#define LIBXD_H

#include <stdint.h>

typedef struct xd_context xd_context_t;
typedef struct xd_surface xd_surface_t;

// input callback
typedef void (*xd_input_callback_t)(xd_surface_t* surface, int type, int x, int y, int button, int state, void* data);

xd_context_t* xd_connect(void);
void xd_disconnect(xd_context_t* ctx);

xd_surface_t* xd_create_surface(xd_context_t* ctx, uint32_t width, uint32_t height);
void xd_destroy_surface(xd_surface_t* surface);

// get the pixel buffer to draw into
uint32_t* xd_surface_get_pixels(xd_surface_t* surface);

// draw the drawn buffer to the server
void xd_surface_commit(xd_surface_t* surface);

// listen for input devices
void xd_set_input_callback(xd_surface_t* surface, xd_input_callback_t cb, void* data);

// son this does polling to look for any change.
void xd_poll_events(xd_context_t* ctx);

// son i wanna kms cuz ts pmo - by ps2

#endif // LIBXD_H
