cmake_minimum_required(VERSION 3.5)

project(mandelbrot LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        canvas_widget.cpp
        canvas_widget.h
        renderer.cpp
        renderer.h
        )

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mandelbrot
            ${PROJECT_SOURCES}
            )
else ()
    if (ANDROID)
        add_library(mandelbrot SHARED
                ${PROJECT_SOURCES}
                )
    else ()
        add_executable(mandelbrot
                ${PROJECT_SOURCES}
                )
    endif ()
endif ()

target_link_libraries(mandelbrot PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
