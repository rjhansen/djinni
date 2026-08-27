project('djinni', 'cpp',
    version: '2.5.2',
    license: 'ISC',
    default_options: [
        'cpp_std=c++23',
    ],
    meson_version: '>=1.0.0',
)

cpp = meson.get_compiler('cpp')

if cpp.get_argument_syntax() == 'msvc'
    add_project_arguments('/EHsc', '/DUSE_BOUNDS_CHECKING', language: 'cpp')
endif

djinni_dep = declare_dependency(
    include_directories: include_directories('src'),
)
dumas_dataset = meson.project_source_root() / 'src' / 'Dumas-1.set'

subdir('src')
subdir('tests')

install_headers('src/djinni.h')
install_subdir('src/djinni', install_dir: get_option('includedir'))

pkg = import('pkgconfig')
pkg.generate(
    name: 'djinni',
    description: 'implements Ohlmann-Thomas compressed annealing',
    version: meson.project_version(),
)
