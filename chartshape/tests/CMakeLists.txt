set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_DIRECTORY} )
add_definitions(-DCOMPILING_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/chartshape
                     ${CMAKE_SOURCE_DIR}/chartshape/kdchart/include
                     ${KOFFICELIBS_INCLUDE_DIR} )

########### next target ###############
set(TestProxyModel_test_SRCS
    TestProxyModel.cpp
)
kde4_add_unit_test( TestProxyModel TESTNAME kchart-TestProxyModel ${TestProxyModel_test_SRCS} )
target_link_libraries( TestProxyModel ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} chartshape)

########### next target ###############
set(TestDataSet_test_SRCS
    TestDataSet.cpp
#   ../ChartTableModel.cpp
#   ../TableSource.cpp
#   ../CellRegion.cpp
#   ../ChartProxyModel.cpp
#   ../DataSet.cpp
#   ../KDChartModel.cpp
#   ../Axis.cpp
#   ../PlotArea.cpp
#   ../kchart_global.cpp
#   ../ChartShape.cpp
#   ../Legend.cpp
#   ../Layout.cpp
#   ../Surface.cpp
#   ../KDChartConvertions.cpp
#   ../TextLabelDummy.cpp
#   ../ThreeDScene.cpp
#   ../ScreenConversions.cpp
#   ../ChartDocument.cpp
)
kde4_add_unit_test( TestDataSet TESTNAME kchart-TestDataSet ${TestDataSet_test_SRCS} )
target_link_libraries( TestDataSet ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} kdchart ${KOTEXT_LIBS} chartshape)

########### next target ###############
set(TestKDChartModel_test_SRCS
    TestKDChartModel.cpp
    ModelObserver.cpp
)
kde4_add_unit_test( TestKDChartModel TESTNAME kchart-TestKDChartModel ${TestKDChartModel_test_SRCS} )
target_link_libraries( TestKDChartModel ${QT_QTTEST_LIBRARY} chartshape)

########### next target ###############
set(TestTableSource_test_SRCS
    TestTableSource.cpp
    ../TableSource.cpp
)
kde4_add_unit_test( TestTableSource TESTNAME kchart-TestTableSource ${TestTableSource_test_SRCS} )
target_link_libraries(TestTableSource ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} )

########### next target ###############
set(TestCellRegion_test_SRCS
    TestCellRegion.cpp
    ../CellRegion.cpp
    ../TableSource.cpp
)
kde4_add_unit_test( TestCellRegion TESTNAME kchart-TestCellRegion ${TestCellRegion_test_SRCS} )
target_link_libraries( TestCellRegion ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} )

add_subdirectory( odf )

