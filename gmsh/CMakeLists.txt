cmake_minimum_required(VERSION 3.16)

project(libgmsh
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

# GMSH modules
set(HAVE_MESH TRUE)
set(HAVE_PARSER TRUE)
set(HAVE_PLUGINS TRUE)
set(HAVE_POST TRUE)
set(HAVE_SOLVER TRUE)

# GMSH deps
check_include_file(dlfcn.h DLFCN_H)
if(DLFCN_H)
    set(HAVE_DLOPEN TRUE)
endif()

find_program(BISON bison REQUIRED)
find_program(FLEX flex REQUIRED)

find_package(BLAS)

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
set(HAVE_EIGEN FALSE)

find_package(OpenCASCADE 6.9.1 REQUIRED)
set(HAVE_OCC TRUE)

set(HAVE_ALGLIB TRUE)
set(HAVE_ANN TRUE)
set(HAVE_BAMG TRUE)
set(HAVE_BLAS TRUE)
set(HAVE_BLOSSOM TRUE)
set(HAVE_DINTEGRATION TRUE)
set(HAVE_DOMHEX TRUE)
set(HAVE_GMM TRUE)
set(HAVE_HXT TRUE)
set(HAVE_KBIPACK FALSE)
set(HAVE_LAPACK TRUE)
set(HAVE_LIBCGNS FALSE)
set(HAVE_MATHEX TRUE)
set(HAVE_MED FALSE)
set(HAVE_METIS TRUE)
set(HAVE_MMG FALSE)
set(HAVE_NETGEN TRUE)
set(HAVE_OPTHOM FALSE)
set(HAVE_QUADTRI TRUE)
set(HAVE_TETGENBR FALSE)
set(HAVE_VOROPP TRUE)

# gmsh 3rd party packages
add_subdirectory(contrib/alglib)
add_subdirectory(contrib/ann)
add_subdirectory(contrib/bamg)
add_subdirectory(contrib/blossom)
add_subdirectory(contrib/discreteintegration)
add_subdirectory(contrib/domhex)
add_subdirectory(contrib/gmm)
add_subdirectory(contrib/hxt)
add_subdirectory(contrib/kbipack)
add_subdirectory(contrib/mathex)
add_subdirectory(contrib/metis)
add_subdirectory(contrib/netgen)
add_subdirectory(contrib/quadtri)
add_subdirectory(contrib/voro++)

#

file(GLOB COMMON_SRC CONFIGURE_DEPENDS
    ${GMSH_DIR}/src/common/*.cpp
    ${GMSH_DIR}/src/common/*.c
)

file(GLOB GEO_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/geo/*.cpp)
list(REMOVE_ITEM GEO_SRCS ${GMSH_DIR}/src/geo/gmshEdgeDiscretize.cpp)

file(GLOB GRAPHICS_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/graphics/*.cpp)

file(GLOB MESH_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/mesh/*.cpp)

file(GLOB NUMERIC_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/numeric/*.cpp)
list(REMOVE_ITEM NUMERIC_SRCS ${GMSH_DIR}/src/numeric/hausdorffDistance.cpp)

set(PARSER_SRCS
    ${GMSH_DIR}/src/parser/FunctionManager.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Gmsh.tab.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Gmsh.yy.cpp
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Gmsh.yy.cpp
    COMMAND ${FLEX} -P gmsh_yy -o ${CMAKE_CURRENT_BINARY_DIR}/Gmsh.yy.cpp ${GMSH_DIR}/src/parser/Gmsh.l
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Gmsh.tab.cpp
    COMMAND ${BISON} -p gmsh_yy --output ${CMAKE_CURRENT_BINARY_DIR}/Gmsh.tab.cpp -d ${GMSH_DIR}/src/parser/Gmsh.y
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB PLUGIN_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/plugin/*.cpp)

file(GLOB POST_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/post/*.cpp)

file(GLOB SOLVER_SRCS CONFIGURE_DEPENDS ${GMSH_DIR}/src/solver/*.cpp)

add_library(${PROJECT_NAME}
    SHARED
        ${COMMON_SRC}
        ${GEO_SRCS}
        ${GRAPHICS_SRCS}
        ${MESH_SRCS}
        ${NUMERIC_SRCS}
        ${PARSER_SRCS}
        ${PLUGIN_SRCS}
        ${POST_SRCS}
        ${SOLVER_SRCS}
)

if(NOT MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME gmsh)
endif()

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
    PUBLIC
        ALGLIB
        ANN
        bamg
        blossom
        DiscreteIntegration
        domhex
        gmm
        hxt
        kbipack
        MathEx
        metis
        Netgen
        QuadTri
        voropp
        ${OCC_LIBRARIES}
        BLAS::BLAS
        OpenGL::GL
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
