set(HXT_DIR ${GMSH_DIR}/contrib/hxt)

file(GLOB COMBINE_SRC CONFIGURE_DEPENDS ${HXT_DIR}/Combine/src/*.cpp)
set(CORE_SRC
    ${HXT_DIR}/core/src/hxt_bbox.c
    ${HXT_DIR}/core/src/hxt_mesh.c
    ${HXT_DIR}/core/src/hxt_message.c
    ${HXT_DIR}/core/src/hxt_sort.c
)

set(REPARAM_SRC
    ${HXT_DIR}/reparam/src/hxt_curvature.c
    ${HXT_DIR}/reparam/src/hxt_edge.c
    ${HXT_DIR}/reparam/src/hxt_linear_system.c
    ${HXT_DIR}/reparam/src/hxt_linear_system_lu.c
    ${HXT_DIR}/reparam/src/hxt_linear_system_petsc.c
    ${HXT_DIR}/reparam/src/hxt_mean_values.c
    ${HXT_DIR}/reparam/src/hxt_non_linear_solver.c
)

set(PREDICATES_SRC
    ${HXT_DIR}/predicates/src/predicates.c
)

set(TETBR_SRC
    ${HXT_DIR}/tetBR/src/hxt_boundary_recovery.cxx
)

set(TETMESH_SRC
    ${HXT_DIR}/tetMesh/src/hxt_edgeRemoval.c
    ${HXT_DIR}/tetMesh/src/hxt_smoothing.c
    ${HXT_DIR}/tetMesh/src/HXTSPR.c
    ${HXT_DIR}/tetMesh/src/hxt_tetColor.c
    ${HXT_DIR}/tetMesh/src/hxt_tetDelaunay.c
    ${HXT_DIR}/tetMesh/src/hxt_tetDelaunayReshape.c
    ${HXT_DIR}/tetMesh/src/hxt_tetFlag.c
    ${HXT_DIR}/tetMesh/src/hxt_tetMesh.c
    ${HXT_DIR}/tetMesh/src/hxt_tetNodalSize.c
    ${HXT_DIR}/tetMesh/src/hxt_tetOpti.c
    ${HXT_DIR}/tetMesh/src/hxt_tetQuality.c
    ${HXT_DIR}/tetMesh/src/hxt_tetRefine.c
    ${HXT_DIR}/tetMesh/src/hxt_tetRepair.c
    ${HXT_DIR}/tetMesh/src/hxt_tetSync.c
    ${HXT_DIR}/tetMesh/src/hxt_tetUtils.c
    ${HXT_DIR}/tetMesh/src/hxt_vertices.c
)

add_library(hxt
    OBJECT
        ${COMBINE_SRC}
        ${CORE_SRC}
        ${REPARAM_SRC}
        ${PREDICATES_SRC}
        ${TETBR_SRC}
        ${TETMESH_SRC}
)

target_include_directories(
    hxt
    PUBLIC
        ${HXT_DIR}/Combine/src
        ${HXT_DIR}/core/include
        ${HXT_DIR}/predicates/include
        ${HXT_DIR}/reparam/include
        ${HXT_DIR}/tetBR/include
        ${HXT_DIR}/tetMesh/include
        ${HXT_DIR}/tetMesh/src
)
