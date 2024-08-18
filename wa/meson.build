wa_src = files(
    'src/wa_log.c',
    'src/wa_wayland.c',
    'src/wa_wayland_input.c'
)

wa_private_include = include_directories('include/')

# Find Wayland dependencies
wayland_client = dependency('wayland-client', required: true)
wayland_egl = dependency('wayland-egl', required: true)

# Find OpenGL and EGL dependencies
egl = dependency('egl', required: true)
gl = dependency('gl', required: true)
glew = dependency('glew', required: true)

# Find xkbcommon
xkbcommon = dependency('xkbcommon', required: true)

wayland_protocols = dependency('wayland-protocols', required: true)

wa_deps = [wayland_client, wayland_egl, egl, gl, glew, xkbcommon]

subdir('wayland-protocols/')

wa_deps += client_protos

libwa = library('wa', wa_src,
    include_directories: [
        wa_include,
        wa_private_include
    ],
    dependencies: wa_deps
)
