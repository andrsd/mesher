set(BLOSSOM_DIR ${GMSH_DIR}/contrib/blossom)

set(SRC
#   ${BLOSSOM_DIR}/concorde97/BIGGUY/bg_test.c
    ${BLOSSOM_DIR}/concorde97/BIGGUY/bigguy.c
    ${BLOSSOM_DIR}/concorde97/CUT/connect.c
    ${BLOSSOM_DIR}/concorde97/CUT/cut_st.c
#   ${BLOSSOM_DIR}/concorde97/CUT/mc_main.c
    ${BLOSSOM_DIR}/concorde97/CUT/mincut.c
    ${BLOSSOM_DIR}/concorde97/CUT/segments.c
    ${BLOSSOM_DIR}/concorde97/CUT/shrink.c
    ${BLOSSOM_DIR}/concorde97/EDGEGEN/edgegen.c
#   ${BLOSSOM_DIR}/concorde97/EDGEGEN/eg_main.c
    ${BLOSSOM_DIR}/concorde97/EDGEGEN/xnear.c
#   ${BLOSSOM_DIR}/concorde97/FMATCH/fm_main.c
    ${BLOSSOM_DIR}/concorde97/FMATCH/fmatch.c
#   ${BLOSSOM_DIR}/concorde97/KDTREE/kd_main.c
    ${BLOSSOM_DIR}/concorde97/KDTREE/kdbuild.c
    ${BLOSSOM_DIR}/concorde97/KDTREE/kdnear.c
    ${BLOSSOM_DIR}/concorde97/KDTREE/kdspan.c
    ${BLOSSOM_DIR}/concorde97/KDTREE/kdtwoopt.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ary.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_bt0.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_bt1.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_bt2.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_bt3.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_bt4.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_bt5.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_btc.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_btd.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_btr.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll0.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll1.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll2.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll3.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll4.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll5.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll6.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll7.c
    ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll8.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_ll9.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_llA.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_llB.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_llC.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_llD.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_sg1.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_sg2.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_sg3.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_sp1.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_sp2.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_try.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_tw2.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/flip_two.c
    ${BLOSSOM_DIR}/concorde97/LINKERN/linkern.c
#   ${BLOSSOM_DIR}/concorde97/LINKERN/lk_main.c
#   ${BLOSSOM_DIR}/concorde97/LP/lpcplex.c
    ${BLOSSOM_DIR}/concorde97/LP/lpsolve.c
    ${BLOSSOM_DIR}/concorde97/TSP/bcontrol.c
    ${BLOSSOM_DIR}/concorde97/TSP/branch.c
    ${BLOSSOM_DIR}/concorde97/TSP/cliqhash.c
    ${BLOSSOM_DIR}/concorde97/TSP/cliqwork.c
#   ${BLOSSOM_DIR}/concorde97/TSP/concorde.c
    ${BLOSSOM_DIR}/concorde97/TSP/control.c
    ${BLOSSOM_DIR}/concorde97/TSP/cutcall.c
    ${BLOSSOM_DIR}/concorde97/TSP/cutpool.c
    ${BLOSSOM_DIR}/concorde97/TSP/edgemap.c
    ${BLOSSOM_DIR}/concorde97/TSP/ex_price.c
    ${BLOSSOM_DIR}/concorde97/TSP/generate.c
#   ${BLOSSOM_DIR}/concorde97/TSP/poolcat.c
    ${BLOSSOM_DIR}/concorde97/TSP/prob_io.c
    ${BLOSSOM_DIR}/concorde97/TSP/qsparse.c
    ${BLOSSOM_DIR}/concorde97/TSP/teething.c
    ${BLOSSOM_DIR}/concorde97/TSP/tighten.c
    ${BLOSSOM_DIR}/concorde97/TSP/tsp_lp.c
    ${BLOSSOM_DIR}/concorde97/TSP/xtour.c
    ${BLOSSOM_DIR}/concorde97/UTIL/allocrus.c
    ${BLOSSOM_DIR}/concorde97/UTIL/bgetopt.c
    ${BLOSSOM_DIR}/concorde97/UTIL/dheaps_i.c
    ${BLOSSOM_DIR}/concorde97/UTIL/edg2cyc.c
    ${BLOSSOM_DIR}/concorde97/UTIL/edgelen.c
    ${BLOSSOM_DIR}/concorde97/UTIL/fastread.c
    ${BLOSSOM_DIR}/concorde97/UTIL/genhash.c
    ${BLOSSOM_DIR}/concorde97/UTIL/getdata.c
    ${BLOSSOM_DIR}/concorde97/UTIL/priority.c
    ${BLOSSOM_DIR}/concorde97/UTIL/safe_io.c
    ${BLOSSOM_DIR}/concorde97/UTIL/sortrus.c
    ${BLOSSOM_DIR}/concorde97/UTIL/urandom.c
    ${BLOSSOM_DIR}/concorde97/UTIL/util.c
    ${BLOSSOM_DIR}/concorde97/UTIL/zeit.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xallcuts.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xblobs.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xblock.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xblossom.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xcclean.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xclique.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xcuthash.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xcutload.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xcuts.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xcututil.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xflow.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xgomhu.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xgraph.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xnecklac.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xnewkids.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xourallo.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xpqnew.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xshrink.c
    ${BLOSSOM_DIR}/concorde97/XSTUFF/Xstuff.c
#   ${BLOSSOM_DIR}/concorde97/XSTUFF/Xtest.c
    ${BLOSSOM_DIR}/MATCH/match.c
    ${BLOSSOM_DIR}/MATCH/matprice.c
)

add_library(blossom OBJECT ${SRC})

target_include_directories(blossom
    PUBLIC
        ${BLOSSOM_DIR}/concorde97
        ${BLOSSOM_DIR}/concorde97/INCLUDE
)
