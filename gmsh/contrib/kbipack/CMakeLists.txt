set(KBIPACK_DIR ${GMSH_DIR}/contrib/kbipack)

set(SRC
    ${KBIPACK_DIR}/gmp_normal_form.cpp
    ${KBIPACK_DIR}/gmp_matrix_io.cpp
    ${KBIPACK_DIR}/gmp_matrix.cpp
    ${KBIPACK_DIR}/gmp_blas.cpp
    ${KBIPACK_DIR}/mpz.cpp
)

add_library(kbipack OBJECT ${SRC})

target_include_directories(
    kbipack
    PUBLIC
        ${KBIPACK_DIR}
    PRIVATE
        ${GMSH_DIR}/src/common
        ${CMAKE_BINARY_DIR}/gmsh
)
