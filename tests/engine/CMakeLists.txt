find_package(Qt4 REQUIRED)
set( TEST_LIBRARIES
    ${QT_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    LibKSEngine
    KStarsLib
   )
include_directories( ${kstars_SOURCE_DIR}
                     ${OPENCL_INCLUDE_DIRS}
                     ${EIGEN3_INCLUDE_DIR} )

set( EngineTests_SRCS 
    testconvertcoord.cpp 
    testastrovars.cpp 
    testclconvert.cpp
    testksclcontext.cpp )
QT4_AUTOMOC( ${EngineTests_SRCS} )

#
# Test coordinate conversions
#

add_executable( testconvertcoord testconvertcoord.cpp )
target_link_libraries( testconvertcoord ${TEST_LIBRARIES} )
add_test( NAME ConvertCoordTest COMMAND testconvertcoord )

#
# Test misc calculations
#

add_executable( testastrovars testastrovars.cpp )
target_link_libraries( testastrovars ${TEST_LIBRARIES} )
add_test( NAME AstroVarsTest COMMAND testastrovars )

#
# Test context creation
#

add_executable( testksclcontext testksclcontext.cpp )
target_link_libraries( testksclcontext ${TEST_LIBRARIES} )
add_test( NAME KSClContextTest COMMAND testksclcontext )

#
# Test using CL to process points
#

add_executable( testclconvert testclconvert.cpp )
target_link_libraries( testclconvert ${TEST_LIBRARIES} )
add_test( NAME ClConvertTest COMMAND testclconvert )

