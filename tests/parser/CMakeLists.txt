include_directories( ${kstars_SOURCE_DIR} )

FIND_PACKAGE(Qt4 REQUIRED)
SET( TEST_LIBRARIES 
    ${QT_QTTEST_LIBRARY} 
    ${QT_QTCORE_LIBRARY} 
    KStarsLib
)

SET( KSParserTests_SRCS testcsvparser.cpp testfwparser.cpp )
QT4_AUTOMOC( ${KSParserTests_SRCS} )

ADD_EXECUTABLE( testcsvparser testcsvparser.cpp )
TARGET_LINK_LIBRARIES( testcsvparser ${TEST_LIBRARIES} )
ADD_TEST( NAME CSVParserTest COMMAND testcsvparser )

ADD_EXECUTABLE( testfwparser testfwparser.cpp )
TARGET_LINK_LIBRARIES( testfwparser ${TEST_LIBRARIES} )
ADD_TEST( NAME FixedWidthParserTest COMMAND testfwparser )

