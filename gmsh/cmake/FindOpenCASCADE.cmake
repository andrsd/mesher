set(OCC_MINIMAL_VERSION "6.9.1")
if(WIN32)
    if(HAVE_64BIT_SIZE_T)
        set(OCC_SYS_NAME win64)
    else()
        set(OCC_SYS_NAME win32)
    endif()
else()
    set(OCC_SYS_NAME ${CMAKE_SYSTEM_NAME})
endif()
find_path(OCC_INC "Standard_Version.hxx" HINTS ENV CASROOT PATH_SUFFIXES
    inc include include/oce opencascade include/opencascade
    occt include/occt)
if(OCC_INC)
    file(STRINGS ${OCC_INC}/Standard_Version.hxx
        OCC_MAJOR REGEX "#define OCC_VERSION_MAJOR.*")
    file(STRINGS ${OCC_INC}/Standard_Version.hxx
        OCC_MINOR REGEX "#define OCC_VERSION_MINOR.*")
    file(STRINGS ${OCC_INC}/Standard_Version.hxx
        OCC_MAINT REGEX "#define OCC_VERSION_MAINTENANCE.*")
    if(OCC_MAJOR AND OCC_MINOR AND OCC_MAINT)
        string(REGEX MATCH "[0-9]+" OCC_MAJOR "${OCC_MAJOR}")
        string(REGEX MATCH "[0-9]+" OCC_MINOR "${OCC_MINOR}")
        string(REGEX MATCH "[0-9]+" OCC_MAINT "${OCC_MAINT}")
        set(OCC_VERSION "${OCC_MAJOR}.${OCC_MINOR}.${OCC_MAINT}")
        # message(STATUS "Found OpenCASCADE version ${OCC_VERSION} in ${OCC_INC}")
    endif()
endif()
if(OCC_VERSION AND OCC_VERSION VERSION_LESS ${OCC_MINIMAL_VERSION})
    message(WARNING "Gmsh requires OpenCASCADE >= ${OCC_MINIMAL_VERSION}. "
        "Use CMAKE_PREFIX_PATH or the CASROOT environment variable "
        "to explicitly specify the installation path of OpenCASCADE")
elseif(OCC_INC)
    if(OCC_VERSION AND OCC_VERSION VERSION_GREATER_EQUAL "7.8.0")
        set(OCC_LIBS_REQUIRED
            # subset of DataExchange
            TKDESTEP TKDEIGES TKXSBase
            # ModelingAlgorithms
            TKOffset TKFeat TKFillet TKBool TKMesh TKHLR TKBO TKPrim TKShHealing
            TKTopAlgo TKGeomAlgo
            # ModelingData
            TKBRep TKGeomBase TKG3d TKG2d
            # FoundationClasses
            TKMath TKernel)
    else()
        set(OCC_LIBS_REQUIRED
            # subset of DataExchange
            TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKIGES TKXSBase
            # ModelingAlgorithms
            TKOffset TKFeat TKFillet TKBool TKMesh TKHLR TKBO TKPrim TKShHealing
            TKTopAlgo TKGeomAlgo
            # ModelingData
            TKBRep TKGeomBase TKG3d TKG2d
            # FoundationClasses
            TKMath TKernel)
    endif()
    if(ENABLE_OCC_TBB)
        list(APPEND OCC_LIBS_REQUIRED tbb tbbmalloc)
    endif()
    list(LENGTH OCC_LIBS_REQUIRED NUM_OCC_LIBS_REQUIRED)
    if(OCC_LIBS)
        message(STATUS "OCC libraries specified explicitly: " ${OCC_LIBS})
        list(LENGTH OCC_LIBS_REQUIRED NUM_OCC_LIBS)
    else()
        set(OCC_LIBS)
        foreach(OCC ${OCC_LIBS_REQUIRED})
            find_library(OCC_LIB ${OCC} HINTS ENV CASROOT PATH_SUFFIXES
                lib ${OCC_SYS_NAME}/vc8/lib ${OCC_SYS_NAME}/vc9/lib
                ${OCC_SYS_NAME}/vc10/lib ${OCC_SYS_NAME}/vc11/lib
                ${OCC_SYS_NAME}/vc12/lib ${OCC_SYS_NAME}/vc14/lib
                ${OCC_SYS_NAME}/gcc/lib ${OCC_SYS_NAME}/gcc/bin
                ${OCC_SYS_NAME}/lib)
            if(OCC_LIB)
                list(APPEND OCC_LIBS ${OCC_LIB})
            else()
                message(STATUS "OCC lib " ${OCC} " not Found")
            endif()
            unset(OCC_LIB CACHE)
        endforeach()
        list(LENGTH OCC_LIBS NUM_OCC_LIBS)
    endif()
endif()

# additional OCC libraries to handle reading of STEP/IGES attributes. Oh my...
if(ENABLE_OCC_CAF)
    find_package(Freetype)
    if(FREETYPE_FOUND)
        if(OCC_VERSION AND OCC_VERSION VERSION_GREATER_EQUAL "7.8.0")
            set(OCC_CAF_LIBS_REQUIRED
                TKXCAF TKLCAF TKVCAF TKCAF TKV3d TKService TKCDF)
        else()
            set(OCC_CAF_LIBS_REQUIRED
                TKXDESTEP TKXDEIGES TKXCAF TKLCAF TKVCAF TKCAF TKV3d TKService TKCDF)
        endif()
        list(LENGTH OCC_CAF_LIBS_REQUIRED NUM_OCC_CAF_LIBS_REQUIRED)
        set(OCC_CAF_LIBS)
        foreach(OCC ${OCC_CAF_LIBS_REQUIRED})
            find_library(OCC_CAF_LIB ${OCC} HINTS ENV CASROOT PATH_SUFFIXES
                lib ${OCC_SYS_NAME}/vc8/lib ${OCC_SYS_NAME}/vc9/lib
                ${OCC_SYS_NAME}/vc10/lib ${OCC_SYS_NAME}/vc11/lib
                ${OCC_SYS_NAME}/vc12/lib ${OCC_SYS_NAME}/vc14/lib
                ${OCC_SYS_NAME}/gcc/lib ${OCC_SYS_NAME}/gcc/bin
                ${OCC_SYS_NAME}/lib)
            if(OCC_CAF_LIB)
                list(APPEND OCC_CAF_LIBS ${OCC_CAF_LIB})
            else()
                message(STATUS "OCC CAF lib " ${OCC} " not Found")
            endif()
            unset(OCC_CAF_LIB CACHE)
        endforeach()
        list(LENGTH OCC_CAF_LIBS NUM_OCC_CAF_LIBS)
    endif()
endif()

if(NUM_OCC_LIBS EQUAL NUM_OCC_LIBS_REQUIRED)
    # append OCC CAF libraries first...
    if(NUM_OCC_CAF_LIBS EQUAL NUM_OCC_CAF_LIBS_REQUIRED)
        # FIXME: set_config_option(HAVE_OCC_CAF "OpenCASCADE-CAF")
        # set(HAVE_OCC_CAF TRUE)
        list(APPEND OCC_LIBRARIES ${OCC_CAF_LIBS} ${FREETYPE_LIBRARIES})
        list(APPEND OCC_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
         if(WIN32)
             list(APPEND OCC_LIBRARIES "windowscodecs")
             list(APPEND OCC_LIBRARIES "ole32")
         endif()
    endif()
    # then append OCC libraries
    list(APPEND OCC_LIBRARIES ${OCC_LIBS})
    list(APPEND OCC_INCLUDE_DIRS ${OCC_INC})
    # if(WIN32 AND NOT MSVC)
    #     add_definitions(-DOCC_CONVERT_SIGNALS)
    # endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    OpenCASCADE
    REQUIRED_VARS OCC_LIBRARIES OCC_INCLUDE_DIRS
    VERSION_VAR OCC_VERSION
    HANDLE_COMPONENTS
)
