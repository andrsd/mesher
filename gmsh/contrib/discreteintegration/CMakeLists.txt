set(DISCRETEINTEGRATION_DIR ${GMSH_DIR}/contrib/DiscreteIntegration)

set(SRC
    ${DISCRETEINTEGRATION_DIR}/Integration3D.cpp
    ${DISCRETEINTEGRATION_DIR}/recurCut.cpp
)

add_library(DiscreteIntegration OBJECT ${SRC})

target_include_directories(DiscreteIntegration
    PUBLIC
        ${DISCRETEINTEGRATION_DIR}
    PRIVATE
        ${GMSH_DIR}/src/common
        ${GMSH_DIR}/src/geo
        ${GMSH_DIR}/src/numeric
        ${GMSH_DIR}/src/post
        ${CMAKE_BINARY_DIR}/gmsh
)
