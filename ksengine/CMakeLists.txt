#Needed for compilation on some systems, like Debian Wheezy
add_definitions( -DCL_USE_DEPRECATED_OPENCL_1_1_APIS )
include_directories( ${OPENCL_INCLUDE_DIRS}
                     ${QT4_INCLUDES} )

set(LibKSEngine_SRCS
    dms.cpp
    astrovars.cpp
    convertcoord.cpp
    kscontext.cpp
    kscontextcl.cpp
    kscontexteigen.cpp
    ksbuffer.cpp
    ksbuffercl.cpp
    ksbuffereigen.cpp
)

kde4_add_library(LibKSEngine STATIC ${LibKSEngine_SRCS})
target_link_libraries(LibKSEngine
                      ${OPENCL_LIBRARIES}
                      ${QT_QTCORE_LIBRARY}
                      ${KDE4_KDECORE_LIBS} #Needed only for KDebug
                     )

install ( FILES kernels.cl DESTINATION ${DATA_INSTALL_DIR}/kstars/scripts )

