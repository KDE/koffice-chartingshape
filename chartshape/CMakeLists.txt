add_subdirectory( kdchart )
add_subdirectory( tests )

include_directories( ${CMAKE_SOURCE_DIR}/chartshape
                     ${CMAKE_SOURCE_DIR}/chartshape/kdchart/include
                     ${CMAKE_SOURCE_DIR}/chartshape/dialogs
                     ${KOFFICELIBS_INCLUDE_DIR}
)

if (KDE4_BUILD_TESTS)
    # only with this definition will the FOO_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

########### Flake Plugin library ###############

SET (chartshape_SRCS
    kchart_global.cpp
    KDChartConvertions.cpp
    Axis.cpp
    DataSet.cpp
    CellRegion.cpp
    CellRegionStringValidator.cpp
    ChartTableModel.cpp
    Legend.cpp
    TextLabelDummy.cpp
    PlotArea.cpp
    TableSource.cpp
    ChartProxyModel.cpp
    KDChartModel.cpp
    Surface.cpp
    ThreeDScene.cpp
    ThreeDTransformation.cpp
    ChartDocument.cpp
    ChartShape.cpp
    ChartTool.cpp
    ChartToolFactory.cpp
    ChartConfigWidget.cpp
    ChartTableView.cpp
    ScreenConversions.cpp
    Layout.cpp
    SingleModelHelper.cpp
    OdfLoadingHelper.cpp
    dialogs/TableEditorDialog.cpp
    dialogs/NewAxisDialog.cpp
    dialogs/AxisScalingDialog.cpp
    dialogs/CellRegionDialog.cpp
    commands/ChartTypeCommand.cpp
    ChartShapeFactory.cpp
)

kde4_add_ui_files(chartshape_SRCS
    ChartConfigWidget.ui
    dialogs/ChartTableEditor.ui
    dialogs/NewAxisDialog.ui
    dialogs/AxisScalingDialog.ui
    dialogs/CellRegionDialog.ui
)

kde4_add_plugin(chartshape ${chartshape_SRCS})
target_link_libraries(chartshape kdchart ${KOMAIN_LIBS})
target_link_libraries(chartshape LINK_INTERFACE_LIBRARIES kdchart ${KOMAIN_LIBS})

########### install files ###############

install(TARGETS chartshape DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES chartshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
