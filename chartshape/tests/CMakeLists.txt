set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_DIRECTORY} )

include_directories( ${CMAKE_SOURCE_DIR}/chartshape
                     ${CMAKE_SOURCE_DIR}/chartshape/kdchart/include
                     ${KOFFICELIBS_INCLUDE_DIR} )

########### next target ###############
set( TestProxyModel_test_SRCS TestProxyModel.cpp )
set( TestDataSet_test_SRCS TestDataSet.cpp )
set( TestKDChartModel_test_SRCS TestKDChartModel.cpp ModelObserver.cpp )
set( TestTableSource_test_SRCS TestTableSource.cpp ../TableSource.cpp )
set( TestCellRegion_test_SRCS TestCellRegion.cpp ../CellRegion.cpp ../TableSource.cpp )
kde4_add_unit_test( TestProxyModel TESTNAME kchart-TestProxyModel ${TestProxyModel_test_SRCS} )
kde4_add_unit_test( TestDataSet TESTNAME kchart-TestDataSet ${TestDataSet_test_SRCS} )
kde4_add_unit_test( TestKDChartModel TESTNAME kchart-TestKDChartModel ${TestKDChartModel_test_SRCS} )
kde4_add_unit_test( TestTableSource TESTNAME kchart-TestTableSource ${TestTableSource_test_SRCS} )
kde4_add_unit_test( TestCellRegion TESTNAME kchart-TestCellRegion ${TestCellRegion_test_SRCS} )
target_link_libraries( TestProxyModel ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} chartshapelib )
target_link_libraries( TestDataSet ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} chartshapelib )
target_link_libraries( TestKDChartModel ${QT_QTTEST_LIBRARY} chartshapelib )
target_link_libraries( TestTableSource ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} )
target_link_libraries( TestCellRegion ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} )

add_subdirectory( odf )

