cmake_minimum_required(VERSION 3.16)

project(mesher
    VERSION 0.0
    LANGUAGES C CXX)

set(MESHER_COPYRIGHT
    "Copyright © 2024-present, David Andrš, All Rights Reserved"
)
mark_as_advanced(FORCE MESHER_COPYRIGHT)

math(EXPR MESHER_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})
math(EXPR MESHER_MINOR_VERSION ${PROJECT_VERSION_MINOR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2.0...<6.3.0 REQUIRED
    COMPONENTS
        Core
        Gui
        Widgets
        Network
        OpenGLWidgets
        Svg
        SvgWidgets
)

add_compile_definitions(GL_SILENCE_DEPRECATION)

add_subdirectory(widgets)
add_subdirectory(gmsh)
add_subdirectory(src)
