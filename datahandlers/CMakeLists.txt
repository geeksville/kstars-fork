include_directories( ${kstars_SOURCE_DIR}/src )

set(LibKSDataHandlers_SRCS
        catalogentrydata.cpp
        catalogdata.cpp
        ksparser.cpp
        catalogdb.cpp
)

kde4_add_library(LibKSDataHandlers STATIC ${LibKSDataHandlers_SRCS})

