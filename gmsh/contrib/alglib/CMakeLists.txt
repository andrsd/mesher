set(ALGLIB_DIR ${GMSH_DIR}/contrib/ALGLIB)

set(SRC
    ${ALGLIB_DIR}/ap.cpp
    ${ALGLIB_DIR}/alglibinternal.cpp
    ${ALGLIB_DIR}/alglibmisc.cpp
    ${ALGLIB_DIR}/linalg.cpp
    ${ALGLIB_DIR}/optimization.cpp
    ${ALGLIB_DIR}/solvers.cpp
)

add_library(ALGLIB OBJECT ${SRC})

target_include_directories(ALGLIB PUBLIC ${ALGLIB_DIR})
