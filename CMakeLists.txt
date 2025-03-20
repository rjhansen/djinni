cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/rjhansen/djinni")

project(djinni
    VERSION 2.5
    DESCRIPTION "implements Ohlmann-Thomas compressed annealing"
    LANGUAGES CXX
    HOMEPAGE_URL CMAKE_PROJECT_HOMEPAGE_URL)

add_subdirectory(src)

if (UNIX)
    include(GNUInstallDirs)
    install(FILES
        src/djinni.h
        DESTINATION include)
    install(FILES
        src/djinni/annealers.h
        src/djinni/penalties.h
        src/djinni/routes.h
        DESTINATION include/djinni)
endif (UNIX)
