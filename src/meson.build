djinni_example = executable('djinni_example', 'example.cc',
    dependencies: djinni_dep,
    cpp_args: test_cpp_args,
    install: false,
)
test('example', djinni_example, workdir: meson.current_source_dir(), timeout: 60)
