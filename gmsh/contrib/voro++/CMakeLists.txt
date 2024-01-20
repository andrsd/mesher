set(VOROPP_DIR ${GMSH_DIR}/contrib/voro++)

set(SRC
    ${VOROPP_DIR}/src/voro++.cc
)

set_property(SOURCE ${VOROPP_DIR}/src/voro++.hh PROPERTY SKIP_AUTOGEN ON)

add_library(voropp OBJECT ${SRC})

target_include_directories(
    voropp
    PUBLIC
        ${VOROPP_DIR}/src
)
