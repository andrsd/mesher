set(BAMG_DIR ${GMSH_DIR}/contrib/bamg)

set(SRC
    ${BAMG_DIR}/Mesh2d.cpp
    ${BAMG_DIR}/bamg-gmsh.cpp
    ${BAMG_DIR}/bamglib/Mesh2.cpp
    ${BAMG_DIR}/bamglib/MeshGeom.cpp
    ${BAMG_DIR}/bamglib/MeshRead.cpp
    ${BAMG_DIR}/bamglib/Metric.cpp
    ${BAMG_DIR}/bamglib/R2.cpp
    ${BAMG_DIR}/bamglib/MeshDraw.cpp
    ${BAMG_DIR}/bamglib/MeshQuad.cpp
    ${BAMG_DIR}/bamglib/MeshWrite.cpp
    ${BAMG_DIR}/bamglib/Meshio.cpp
    ${BAMG_DIR}/bamglib/QuadTree.cpp
    ${BAMG_DIR}/bamglib/SetOfE4.cpp
)

add_library(bamg OBJECT ${SRC})

target_include_directories(bamg
    PUBLIC
        ${BAMG_DIR}
        ${BAMG_DIR}/bamglib
    PRIVATE
        ${GMSH_DIR}/src/common
        ${GMSH_DIR}/src/geo
        ${GMSH_DIR}/src/numeric
        ${CMAKE_BINARY_DIR}/gmsh
)
