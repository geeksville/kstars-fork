find_package(Qt4 REQUIRED)
set(TEST_LIBRARIES
    ${QT_QTCORE_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    LibKSCl
   )
include_directories( ${kstars_SOURCE_DIR}
                     ${OPENCL_INCLUDE_DIRS}
                     ${EIGEN3_INCLUDE_DIR} )

set( KSClTests_SRCS testksclcontext.cpp )
QT4_AUTOMOC( ${KSClTests_SRCS} )

#
# Test context creation
#

add_executable( testksclcontext testksclcontext.cpp )
target_link_libraries( testksclcontext ${TEST_LIBRARIES} )
add_test( NAME KSClContextTest COMMAND testksclcontext )

