# xd

<img width="500" height="250" alt="Logo for xd" src="https://github.com/user-attachments/assets/3cfa7fc8-5a19-42d6-998e-445b2aad1c49" />

> "the screen is just a shared whiteboard where we meet to paint."

### what is this?
a tiny display server prototype built in c. it's not fancy, but it's fast.

### the approach
we moved away from sending every pixel over a socket (too slow). now, the app and the server share a piece of memory (ram). the app paints, the server looks. zero-copy, 100x faster, wayland-style.

### how to play
1. `make`
2. `./xd_server`
3. in another terminal: `./xd_demo`

check `/tmp/xd_frame_0000.ppm` to see the masterpiece.
