# xd

<img width="500" height="250" alt="Logo for xd" src="https://github.com/user-attachments/assets/3cfa7fc8-5a19-42d6-998e-445b2aad1c49" />

> "the screen is just a shared whiteboard where we meet to paint."

### what is this?
a minimal display server prototype in c. apps and the server share memory via memfd + mmap — zero-copy rendering, no pixel-pushing over sockets. built to explore the fundamentals of display server architecture, the wayland way.

### the approach
we moved away from sending every pixel over a socket (too slow). now, the app and the server share a piece of memory (ram). the app paints, the server looks. zero-copy, 100x faster, wayland-style.

### how to run
1. `meson setup build && meson compile -C build`
2. `./build/xd_server`
3. in another terminal: `./build/xd_demo`

check `/tmp/xd_frame_0000.ppm` to see the masterpiece.
