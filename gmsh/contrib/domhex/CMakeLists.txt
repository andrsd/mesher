set(DOMHEX_DIR ${GMSH_DIR}/contrib/domhex)

set(SRC
    ${DOMHEX_DIR}/directions3D.cpp
    ${DOMHEX_DIR}/simple3D.cpp
    ${DOMHEX_DIR}/pointInsertion.cpp
    ${DOMHEX_DIR}/pointInsertionRTreeTools.cpp
    ${DOMHEX_DIR}/yamakawa.cpp
    ${DOMHEX_DIR}/ThinLayer.cpp
    ${DOMHEX_DIR}/surfaceFiller.cpp
    ${DOMHEX_DIR}/BGMBase.cpp
    ${DOMHEX_DIR}/BackgroundMesh2D.cpp
    ${DOMHEX_DIR}/BackgroundMesh3D.cpp
    ${DOMHEX_DIR}/BackgroundMeshManager.cpp
)

add_library(domhex OBJECT ${SRC})

target_include_directories(domhex
    PUBLIC
       ${DOMHEX_DIR}
    PRIVATE
        ${GMSH_DIR}/src/common
        ${GMSH_DIR}/src/geo
        ${GMSH_DIR}/src/mesh
        ${GMSH_DIR}/src/numeric
        ${GMSH_DIR}/src/post
        ${GMSH_DIR}/src/solver
        ${CMAKE_BINARY_DIR}/gmsh
)

target_link_libraries(domhex PRIVATE ANN)
