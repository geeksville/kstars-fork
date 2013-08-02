include_directories( ${kstars_SOURCE_DIR}/src )

set(LibKSData_SRCS
    datacomponent.cpp
    datacomposite.cpp
)

kde4_add_library(LibKSData STATIC 
                 ${LibKSData_SRCS} 
                )
target_link_libraries(LibKSData
                      ${QT_QTCORE_LIBRARY}
                      ${KDE4_KDECORE_LIBS} #Needed only for KDebug
                     )

