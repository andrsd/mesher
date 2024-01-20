set(GMM_DIR ${GMSH_DIR}/contrib/gmm)

add_library(gmm)

target_include_directories(gmm PUBLIC ${GMM_DIR})
