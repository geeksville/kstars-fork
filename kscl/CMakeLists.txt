#Needed for compilation on some systems, like Debian Wheezy
add_definitions( -DCL_USE_DEPRECATED_OPENCL_1_1_APIS )
include_directories( ${OPENCL_INCLUDE_DIRS} )

set(LibKSCl_SRCS
    ksclcontext.cpp
    ksclcommandqueue.cpp
    ksclbuffer.cpp
    ksclkernel.cpp
    ksclevent.cpp
)

kde4_add_library(LibKSCl STATIC ${LibKSCl_SRCS})
target_link_libraries(LibKSCl
                      ${OPENCL_LIBRARIES}
                      ${KDE4_KDECORE_LIBS} #Needed only for KDebug
                     )

