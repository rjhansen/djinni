set(LIBDJINNI_EXAMPLE example.cc)

if (MSVC)
    add_compile_options(/EHsc)
    add_compile_options(/DUSE_BOUNDS_CHECKING)
endif (MSVC)

add_executable(djinni_example ${LIBDJINNI_EXAMPLE})