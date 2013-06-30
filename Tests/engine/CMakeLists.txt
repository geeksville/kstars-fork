find_package(Qt4 REQUIRED)
set( TEST_LIBRARIES
    ${QT_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    KStarsLib
   )
include_directories( ${kstars_SOURCE_DIR} ${EIGEN2_INCLUDE_DIR} )

set( KSEngineTests_SRCS testconvertcoord.cpp testastrovars.cpp )
QT4_AUTOMOC( ${KSEngineTests_SRCS} )

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
