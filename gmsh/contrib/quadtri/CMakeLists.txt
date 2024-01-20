set(QUADTRI_DIR ${GMSH_DIR}/contrib/QuadTri)

set(SRC
    ${QUADTRI_DIR}/QuadTriExtruded2D.cpp
    ${QUADTRI_DIR}/QuadTriExtruded3D.cpp
    ${QUADTRI_DIR}/QuadTriTransfinite3D.cpp
    ${QUADTRI_DIR}/QuadTriUtils.cpp
)

add_library(QuadTri OBJECT ${SRC})

target_include_directories(
    QuadTri
    PUBLIC
        ${QUADTRI_DIR}
    PRIVATE
        ${GMSH_DIR}/src/common
        ${GMSH_DIR}/src/geo
        ${GMSH_DIR}/src/mesh
        ${GMSH_DIR}/src/numeric
        ${CMAKE_BINARY_DIR}/gmsh
)
