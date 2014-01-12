# GEOM cmake build file
PROJECT(GEOM)

SET(GEOM_VERSION_MAJOR 7)
SET(GEOM_VERSION_MINOR 2)
SET(GEOM_VERSION_PATCH 0)
SET(GEOM_VERSION_TWEAK 9)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release") # By default, build in release mode
ENDIF()

SET(BUILD_SHARED_LIBS ON)
SET(LIBRARY_OUTPUT_PATH cmake-build)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(inc)

#################################################################
# Look for OCE or official OpenCASCADE include and lib location #
#################################################################
FIND_PACKAGE(OCE)
IF(OCE_FOUND)
	INCLUDE_DIRECTORIES(${OCE_INCLUDE_DIRS})
	MESSAGE("-- OCE include directory: ${OCE_INCLUDE_DIRS}")
    # Also set as OpenCASCADE include path for doxygen
    SET(OCC_INCLUDE_PATH ${OCE_INCLUDE_DIRS})
ELSE(OCE_FOUND)
	# Find OpenCASCADE library, set include and lib path
	IF(WIN32)
    	# Under Windows, check the CASROOT env variable
    	FIND_PATH(OCC_INCLUDE_PATH Standard_Real.hxx 
              	$ENV{CASROOT}\\inc)
    			SET(OCC_LIB_PATH "$ENV{CASROOT}\\win32\\lib")
    # Find the Standard_Real.hxx header in this path
	ELSE(WIN32) #Unix
    	FIND_PATH(OCC_INCLUDE_PATH Standard_Real.hxx $ENV{CASROOT}/inc $ENV{CASROOT}/include/oce)
    	SET(OCC_LIB_PATH "$ENV{CASROOT}/lib")
    	MESSAGE(${OCC_LIB_PATH})
	ENDIF(WIN32)
	# Tells CMake where are the required headers
	INCLUDE_DIRECTORIES(${OCC_INCLUDE_PATH})
	LINK_DIRECTORIES(${OCC_LIB_PATH})
	MESSAGE("-- OpenCASCADE include directory: ${OCC_INCLUDE_PATH}")
	MESSAGE("-- OpenCASCADE shared libraries directory: ${OCC_LIB_PATH}")
ENDIF(OCE_FOUND)

###################
# Look for lib3ds #
###################
INCLUDE(FindPkgConfig)
IF(USE_EXTERNAL_LIB3DS)
    PKG_CHECK_MODULES(LIB3DS REQUIRED lib3ds)
    INCLUDE_DIRECTORIES(SYSTEM ${LIB3DS_INCLUDE_DIRS})
ENDIF(USE_EXTERNAL_LIB3DS)

############################
# Check 32/64 bit platform #
############################
IF (${CMAKE_SIZEOF_VOID_P} MATCHES "8") # It is 64bit, 32 bit systems returns 4
    ADD_DEFINITIONS(-D_OCC64)
    MESSAGE("-- Build 64bit")
ELSE (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
    MESSAGE("-- Build 32bit")
ENDIF(${CMAKE_SIZEOF_VOID_P} MATCHES "8")

######################################################
# Configure Build settings according to the platform #
######################################################
IF(UNIX)
    # Same settings are used for both MacOSX and Unix/Linux
    ADD_DEFINITIONS(-DHAVE_CONFIG_H -DHAVE_LIMITS_H -DCSFDB -DLIN -DOCC_CONVERT_SIGNALS)
ELSE(UNIX)
  IF(WIN32)
    ######### Windows ###########
    MESSAGE("-- Windows system detected")
    ADD_DEFINITIONS(-DWNT -DWIN32 -D_WINDOWS -DCSFDB)
  ELSE(WIN32)
    MESSAGE("Unknown platform")
  ENDIF(WIN32)
ENDIF(UNIX)

###################################
# Build doxygen API documentation #
###################################
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/doc/src/Doxyfile.in
        ${CMAKE_CURRENT_SOURCE_DIR}/doc/src/Doxyfile @ONLY)
    ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/doc/src/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/src
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
ENDIF(DOXYGEN_FOUND)

##########
# TKXBO #
##########
FILE(GLOB XBooleanOperations_source_files src/XBooleanOperations/*.cxx)
FILE(GLOB XBOP_source_files src/XBOP/*.cxx)
FILE(GLOB XBOPTColStd_source_files src/XBOPTColStd/*.cxx)
FILE(GLOB XBOPTools_source_files src/XBOPTools/*.cxx)
FILE(GLOB XIntTools_source_files src/XIntTools/*.cxx)

FILE(GLOB Derived_XBooleanOperations_source_files drv/XBooleanOperations/*.cxx)
FILE(GLOB Derived_XBOP_source_files drv/XBOP/*.cxx)
FILE(GLOB Derived_XBOPTColStd_source_files drv/XBOPTColStd/*.cxx)
FILE(GLOB Derived_XBOPTools_source_files drv/XBOPTools/*.cxx)
FILE(GLOB Derived_XIntTools_source_files drv/XIntTools/*.cxx)

INCLUDE_DIRECTORIES(inc drv/XBooleanOperations drv/XBOP drv/XBOPTColStd drv/XBOPTools drv/XIntTools)
ADD_LIBRARY(TKXBO SHARED ${XBooleanOperations_source_files} ${XBOP_source_files} ${XBOPTColStd_source_files} ${XBOPTools_source_files} ${XIntTools_source_files} ${Derived_XBooleanOperations_source_files} ${Derived_XBOP_source_files} ${Derived_XBOPTColStd_source_files} ${Derived_XBOPTools_source_files} ${Derived_XIntTools_source_files})
TARGET_LINK_LIBRARIES(TKXBO TKernel TKBRep TKMath TKG3d TKG2d TKGeomAlgo TKGeomBase TKTopAlgo TKService TKBool TKMesh TKLCAF TKCAF TKCDF TKAdvTools TKXSBase TKPrim TKFillet TKOffset TKShHealing)

##########
# TKGeom #
##########
FILE(GLOB Archimede_source_files src/Archimede/*.cxx)
FILE(GLOB BlockFix_source_files src/BlockFix/*.cxx)
FILE(GLOB GEOM_source_files src/GEOM/*.cxx)
FILE(GLOB GEOMAlgo_source_files src/GEOMAlgo/*.cxx)
FILE(GLOB GEOMImpl_source_files src/GEOMImpl/*.cxx)
FILE(GLOB NMTDS_source_files src/NMTDS/*.cxx)
FILE(GLOB NMTTools_source_files src/NMTTools/*.cxx)
FILE(GLOB ShHealOper_source_files src/ShHealOper/*.cxx)
FILE(GLOB Sketcher_source_files src/Sketcher/*.cxx)

INCLUDE_DIRECTORIES(inc)
ADD_LIBRARY(TKGeom SHARED ${Archimede_source_files} ${BlockFix_source_files} ${GEOM_source_files} ${GEOMAlgo_source_files} ${GEOMImpl_source_files} ${NMTDS_source_files} ${NMTTools_source_files} ${ShHealOper_source_files} ${Sketcher_source_files})
TARGET_LINK_LIBRARIES(TKGeom TKXBO TKernel TKBRep TKMath TKG3d TKG2d TKGeomAlgo TKGeomBase TKTopAlgo TKService TKBool TKMesh TKLCAF TKCAF TKCDF TKAdvTools TKXSBase TKPrim TKFillet TKOffset TKShHealing)

###############
# TKExchange* #
###############
ADD_LIBRARY(TKExchangeBREP  SHARED src/ExchangeBREP/ExchangeBREP_Export.cxx src/ExchangeBREP/ExchangeBREP_Import.cxx)
TARGET_LINK_LIBRARIES(TKExchangeBREP TKernel TKBRep TKMath)

ADD_LIBRARY(TKExchangeCSFDB SHARED src/ExchangeCSFDB/ExchangeCSFDB_Export.cxx src/ExchangeCSFDB/ExchangeCSFDB_Import.cxx)
TARGET_LINK_LIBRARIES(TKExchangeCSFDB TKernel TKBRep TKMath TKPShape PTKernel TKShapeSchema)

ADD_LIBRARY(TKExchangeIGES  SHARED src/ExchangeIGES/ExchangeIGES_Export.cxx src/ExchangeIGES/ExchangeIGES_Import.cxx)
TARGET_LINK_LIBRARIES(TKExchangeIGES TKernel TKBRep TKMath TKIGES TKXSBase TKLCAF TKCAF)

ADD_LIBRARY(TKExchangeSTEP  SHARED src/ExchangeSTEP/ExchangeSTEP_Export.cxx src/ExchangeSTEP/ExchangeSTEP_Import.cxx)
TARGET_LINK_LIBRARIES(TKExchangeSTEP TKernel TKBRep TKMath TKSTEP TKXSBase TKLCAF TKSTEPBase TKCAF)

ADD_LIBRARY(TKExchangeSTL   SHARED src/ExchangeSTL/ExchangeSTL_Export.cxx)
TARGET_LINK_LIBRARIES(TKExchangeSTL TKernel TKSTL)

ADD_LIBRARY(TKExchangeVRML  SHARED src/ExchangeVRML/ExchangeVRML_Export.cxx)
TARGET_LINK_LIBRARIES(TKExchangeVRML TKernel TKVRML)

ADD_LIBRARY(TKExchangeOBJ   SHARED src/ExchangeOBJ/ExchangeOBJ_Export.cxx)
TARGET_LINK_LIBRARIES(TKExchangeOBJ TKernel TKBRep TKMath TKMesh TKV3d TKXSBase)

IF(USE_EXTERNAL_LIB3DS)
    ADD_LIBRARY(TKExchange3DS SHARED src/Exchange3DS/Exchange3DS_Export.cxx src/Exchange3DS/Exchange3DS_Import.cxx)
    TARGET_LINK_LIBRARIES(TKExchange3DS TKernel TKBRep TKMath TKMesh TKV3d TKTopAlgo ${LIB3DS_LIBRARIES})
ELSE(USE_EXTERNAL_LIB3DS)
    FILE(GLOB Exchange3DS_source_files src/Lib3DS/*.c)
    INCLUDE_DIRECTORIES(inc)
    ADD_LIBRARY(TKExchange3DS SHARED ${Exchange3DS_source_files} src/Exchange3DS/Exchange3DS_Export.cxx src/Exchange3DS/Exchange3DS_Import.cxx)
    TARGET_LINK_LIBRARIES(TKExchange3DS TKernel TKBRep TKMath TKMesh TKV3d TKTopAlgo)
ENDIF(USE_EXTERNAL_LIB3DS)

#############
# SET FLAGS #
#############
IF(MINGW)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthreads")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mthreads -Wl,--export-all-symbols")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -mthreads -Wl,--export-all-symbols")
ENDIF(MINGW)

###########
# INSTALL #
###########
IF(UNIX)
    # Setup install paths
    IF(NOT LIB_INSTALL_DIR)
        SET(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
    ENDIF()
    IF(NOT INCLUDE_INSTALL_DIR)
        SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/geom)
    ENDIF()
    # Set soname version
    SET_TARGET_PROPERTIES(TKXBO TKGeom TKExchangeBREP TKExchangeCSFDB TKExchangeIGES TKExchangeSTEP TKExchangeSTL TKExchangeVRML TKExchangeOBJ TKExchange3DS
			PROPERTIES SOVERSION ${GEOM_VERSION_MAJOR}.${GEOM_VERSION_MINOR}.${GEOM_VERSION_PATCH}.${GEOM_VERSION_TWEAK})
    # Libraries are installed by default in /usr/local/lib
    INSTALL(TARGETS TKXBO TKGeom TKExchangeBREP TKExchangeCSFDB TKExchangeIGES TKExchangeSTEP TKExchangeSTL TKExchangeVRML TKExchangeOBJ TKExchange3DS
			DESTINATION ${LIB_INSTALL_DIR})
    # Headers are installed by default in /usr/local/include
    INSTALL(DIRECTORY inc/
            DESTINATION ${INCLUDE_INSTALL_DIR}
            FILES_MATCHING PATTERN "*.h*"
            PATTERN "*.svn*" EXCLUDE)
ENDIF(UNIX)

########################
# Packaging with CPack #
########################
INCLUDE(InstallRequiredSystemLibraries)

# Define version
SET(CPACK_PACKAGE_VERSION_MAJOR ${GEOM_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${GEOM_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${GEOM_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION_TWEAK ${GEOM_VERSION_TWEAK})

# Include license file
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)

# package name
set(CPACK_PACKAGE_NAME GEOM )

INCLUDE(CPack)
