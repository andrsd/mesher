cmake_minimum_required(VERSION 3.16)

project(gmsh
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")


set(GMSH_DIR ${CMAKE_SOURCE_DIR}/contrib/gmsh)
set(GMSH_MAJOR_VERSION 4)
set(GMSH_MINOR_VERSION 12)
set(GMSH_PATCH_VERSION 0)
if(NOT GMSH_EXTRA_VERSION)
    set(GMSH_EXTRA_VERSION "")
endif()
set(GMSH_EXTRA_VERSION_TEXI "${GMSH_EXTRA_VERSION}")
set(GMSH_EXTRA_VERSION_ORIG ${GMSH_EXTRA_VERSION})

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

if(NOT DATE)
    set(DATE "unknown")
endif()
set(GMSH_DATE "${DATE}")

if(NOT GMSH_HOST)
    execute_process(COMMAND hostname OUTPUT_VARIABLE HOST
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT HOST)
        set(HOST "unknown")
    endif()
    set(GMSH_HOST "${HOST}")
endif()

if(NOT GMSH_PACKAGER)
    execute_process(COMMAND whoami OUTPUT_VARIABLE PACKAGER
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT PACKAGER)
        set(PACKAGER "unknown")
    endif()
    string(REPLACE "\\" " " PACKAGER ${PACKAGER})
    set(GMSH_PACKAGER "${PACKAGER}")
endif()

# check the size of size_t
check_type_size("size_t" SIZEOF_SIZE_T)
if(SIZEOF_SIZE_T EQUAL 8)
#    set_config_option(HAVE_64BIT_SIZE_T "64Bit")
    set(HAVE_64BIT_SIZE_T TRUE)
endif()

# append 32/64 to the build name on Linux and Windows
if(NOT APPLE)
    if(HAVE_64BIT_SIZE_T)
        set(GMSH_OS "${GMSH_OS}64")
    else()
        set(GMSH_OS "${GMSH_OS}32")
    endif()
endif()

if(ENABLE_BUILD_DYNAMIC)
    set(GMSH_OS "${GMSH_OS}-sdk")
endif()


check_include_file(dlfcn.h DLFCN_H)
if(DLFCN_H)
    set(HAVE_DLOPEN TRUE)
endif()

find_package(ZLIB)
if(ZLIB_FOUND)
    set(HAVE_LIBZ TRUE)
endif()

find_package(OpenGL REQUIRED)
set(HAVE_OPENGL TRUE)
#set(HAVE_OSMESA TRUE)

find_package(Eigen3 QUIET)
if (Eigen3_FOUND)
    set(HAVE_EIGEN TRUE)
endif()

find_package(OpenCASCADE 6.9.1 REQUIRED)
set(HAVE_OCC TRUE)

set(HAVE_MESH TRUE)
set(HAVE_POST TRUE)

#

file(GLOB COMMON_SRC CONFIGURE_DEPENDS ${GMSH_DIR}/src/common/*.cpp)
list(REMOVE_ITEM COMMON_SRC ${GMSH_DIR}/src/common/gmsh.cpp)

file(GLOB GEO_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/geo/*.cpp)
list(REMOVE_ITEM GEO_SRCS ${GMSH_DIR}/src/geo/gmshEdgeDiscretize.cpp)

file(GLOB MESH_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/mesh/*.cpp)

file(GLOB NUMERIC_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/numeric/*.cpp)
list(REMOVE_ITEM NUMERIC_SRCS ${GMSH_DIR}/src/numeric/hausdorffDistance.cpp)

file(GLOB PARSER_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/parser/*.cpp)

file(GLOB PLUGIN_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/plugin/*.cpp)

file(GLOB POST_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/post/*.cpp)

file(GLOB SOLVER_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/solver/*.cpp)

add_library(${PROJECT_NAME}
    OBJECT
        ${COMMON_SRC}
        ${GEO_SRCS}
        ${MESH_SRCS}
        ${NUMERIC_SRCS}
        ${PARSER_SRCS}
        ${PLUGIN_SRCS}
        ${POST_SRCS}
        ${SOLVER_SRCS}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${PROJECT_BINARY_DIR}
        ${GMSH_DIR}/src/common
        ${GMSH_DIR}/src/geo
        ${GMSH_DIR}/src/graphics
        ${GMSH_DIR}/src/mesh
        ${GMSH_DIR}/src/numeric
        ${GMSH_DIR}/src/parser
        ${GMSH_DIR}/src/plugin
        ${GMSH_DIR}/src/post
        ${GMSH_DIR}/src/solver
        ${GMSH_DIR}/api
        ${OCC_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        ${OCC_LIBRARIES}
)

if(HAVE_DLOPEN)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
endif()
if(HAVE_EIGEN)
    target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)
endif()
if(HAVE_LIBZ)
    target_include_directories(${PROJECT_NAME} PUBLIC ${ZLIB_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES})
endif()


configure_file(${GMSH_DIR}/src/common/GmshConfig.h.in ${PROJECT_BINARY_DIR}/GmshConfig.h)
configure_file(${GMSH_DIR}/src/common/GmshVersion.h.in ${PROJECT_BINARY_DIR}/GmshVersion.h)
