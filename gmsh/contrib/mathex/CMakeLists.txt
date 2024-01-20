set(MATHEX_DIR ${GMSH_DIR}/contrib/MathEx)

set(SRC
    ${MATHEX_DIR}/mathex.cpp
)

add_library(MathEx OBJECT ${SRC})

target_include_directories(MathEx PUBLIC ${MATHEX_DIR})
