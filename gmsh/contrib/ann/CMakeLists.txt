set(ANN_DIR ${GMSH_DIR}/contrib/ANN)

set(SRC
    ${ANN_DIR}/src/ANN.cpp
    ${ANN_DIR}/src/bd_fix_rad_search.cpp
    ${ANN_DIR}/src/bd_pr_search.cpp
    ${ANN_DIR}/src/bd_search.cpp
    ${ANN_DIR}/src/bd_tree.cpp
    ${ANN_DIR}/src/brute.cpp
    ${ANN_DIR}/src/kd_dump.cpp
    ${ANN_DIR}/src/kd_fix_rad_search.cpp
    ${ANN_DIR}/src/kd_pr_search.cpp
    ${ANN_DIR}/src/kd_search.cpp
    ${ANN_DIR}/src/kd_split.cpp
    ${ANN_DIR}/src/kd_tree.cpp
    ${ANN_DIR}/src/kd_util.cpp
    ${ANN_DIR}/src/perf.cpp
)

add_library(ANN OBJECT ${SRC})

target_include_directories(ANN PUBLIC ${ANN_DIR}/include)
