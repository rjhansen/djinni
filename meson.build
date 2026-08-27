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
    # -Werror=sign-conversion: a 2026-08 review pass eliminated every
    # int/uint32_t mismatch this warning found (49 sites, all real --
    # see git history around that date). Making it a hard error keeps it
    # that way instead of letting a future change silently reintroduce
    # the mismatch class that produced this codebase's actual
    # out-of-bounds/underflow bugs. Scoped to this one warning, not a
    # general -Werror policy. No MSVC equivalent here: its closest
    # warnings (C4245/C4365) have different semantics and aren't enabled
    # by this project's warning level, so MSVC builds rely on the other
    # platforms' CI legs to catch this class of regression instead.
    add_project_arguments('-std=c++23', '-Wsign-conversion',
        '-Werror=sign-conversion', language: 'cpp')
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

# Extra flags for the test programs (the four tests/test_*.cc executables
# plus djinni_example, which doubles as the 'example' test) specifically:
# debug info plus speed optimization, regardless of --buildtype. Neither
# buildtype in common use here gives both -- the unset default (debug)
# gives -g -O0, and --buildtype=release (used by build.sh/build.ps1 and
# CI's main job) gives -O3 with no debug info. Appended as target-level
# cpp_args rather than project-wide args or a buildtype change, so it
# only affects these programs and -O2 reliably wins over any buildtype-
# supplied -O flag (GCC/Clang take the last -O on the command line, and
# target args land after buildtype-injected ones). No MSVC form: the
# request was scoped to clang/g++.
test_cpp_args = cpp.get_argument_syntax() == 'msvc' ? [] : ['-g', '-O2']

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
