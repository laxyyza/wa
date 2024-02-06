cmake_minimum_required(VERSION 3.27.0)

project("WA" LANGUAGES C)

include_directories(include/)

add_library(wa SHARED)
target_sources(wa PRIVATE
    wa/src/wa_wayland.c
    wa/src/wa_wayland_input.c
    wa/src/wa_log.c

    wa/src/xdg-decoration.c
    wa/src/xdg-shell.c
)
target_include_directories(wa PRIVATE wa/include)
target_link_libraries(wa PRIVATE
    wayland-client
    wayland-egl
    EGL
    GL
    GLEW
    xkbcommon
)

add_executable(wa_test)
target_sources(wa_test PRIVATE 
    sandbox/main.c
)
target_link_libraries(wa_test PRIVATE 
    wa
)
