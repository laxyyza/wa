sandbox_src = files(
    'main.c'
)

executable('wa_test', sandbox_src,
    include_directories: [wa_include],
    link_with: [libwa],
    dependencies: [gl, glew]
)
