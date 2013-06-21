find_package(Qt4 REQUIRED)
set( TEST_LIBRARIES
    ${QT_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    KStarsLib
   )
include_directories( ${kstars_SOURCE_DIR} ${EIGEN2_INCLUDE_DIR} )

set( KSEngineTests_SRCS convertCoord.cpp )
QT4_AUTOMOC( ${KSEngineTests_SRCS} )

#
# Test coordinate conversions
#

add_executable( testConvertCoord convertCoord.cpp )
target_link_libraries( testConvertCoord ${TEST_LIBRARIES} )
add_test( NAME ConvertCoordTest COMMAND testConvertCoord )
