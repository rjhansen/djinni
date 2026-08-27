project('djinni', 'cpp',
    version: '2.5.2',
    license: 'ISC',
    meson_version: '>=1.0.0',
)

cpp = meson.get_compiler('cpp')

# Deliberately not using Meson's built-in cpp_std project option: Meson
# validates that value at project() parse time against a hardcoded list of
# standards it believes the detected compiler supports, and rejects
# anything not on that list before any of our own logic below ever runs.
# Some MSVC toolchains (e.g. the one on GitHub's windows-2022 runners as of
# this writing) don't advertise 'c++23' in that list, only up through
# 'c++20' plus 'c++latest'/'vc++latest' -- even though passing
# /std:c++latest directly works fine and gets us C++23 (and then some).
# Passing the flag straight through, rather than going via cpp_std, skips
# that validation and its false negatives entirely.
if cpp.get_argument_syntax() == 'msvc'
    add_project_arguments('/std:c++latest', '/EHsc', '/DUSE_BOUNDS_CHECKING',
        language: 'cpp')
else
    add_project_arguments('-std=c++23', language: 'cpp')
endif

if get_option('faster')
    if cpp.get_argument_syntax() == 'msvc'
        add_project_arguments('/DDJINNI_FASTER', language: 'cpp')
    else
        add_project_arguments('-DDJINNI_FASTER', language: 'cpp')
    endif
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
