cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED On)
set(CMAKE_C_EXTENSIONS Off)
set(CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/rjhansen/djinni")
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

project(djinni
    VERSION 2.5
    DESCRIPTION "implements Ohlmann-Thomas compressed annealing"
    LANGUAGES CXX C
    HOMEPAGE_URL CMAKE_PROJECT_HOMEPAGE_URL)

add_subdirectory(src)

if (UNIX)
    include(GNUInstallDirs)
    install(TARGETS djinni LIBRARY)
    install(FILES
        src/djinni.h
        DESTINATION include)
    install(FILES
        src/djinni/annealers.h
        src/djinni/penalties.h
        src/djinni/routes.h
        DESTINATION include/djinni)
endif(UNIX)
