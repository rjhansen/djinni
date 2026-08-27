djinni_example = executable('djinni_example', 'example.cc',
    dependencies: djinni_dep,
    install: false,
)
test('example', djinni_example, workdir: meson.current_source_dir(), timeout: 60)
