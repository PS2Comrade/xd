project('x-display_server', 'c',
  version : '0.0.1',
  default_options : ['warning_level=2', 'c_std=c11']
)

inc = include_directories('include')

# server 
xd_server = executable('xd_server',
  sources : [
    'src/server.c',
    'src/framebuffer.c',
    'src/protocol.c',
  ],
  include_directories : inc,
  install : true,
)

# client 
xd_demo = executable('xd_demo',
  sources : [
    'client/demo.c',
    'client/libxd.c',
  ],
  include_directories : inc,
  install : true,
)
