set(LIBDJINNI_SRC routes.cc utils.cc)
set(LIBDJINNI_EXAMPLE example.cc)

add_library(djinni ${LIBDJINNI_SRC})
add_executable(djinni_ex ${LIBDJINNI_EXAMPLE})
target_link_libraries(djinni_ex djinni)
target_sources(djinni PUBLIC ${LIBDJINNI_SRC}
        PUBLIC FILE_SET HEADERS
        BASE_DIRS . FILES
        djinni/annealers.h
        djinni.h
        djinni/penalties.h
        djinni/routes.h)
set_target_properties(djinni PROPERTIES
        POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
