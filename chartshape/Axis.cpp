/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Own
#include "Axis.h"

// Qt
#include <QList>
#include <QString>
#include <QTextDocument>

// KOffice
#include <KShapeLoadingContext.h>
#include <KShapeSavingContext.h>
#include <KShapeRegistry.h>
#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KOdfGenericStyles.h>
#include <KOdfXmlNS.h>
#include <KTextShapeData.h>
#include <KOdfStylesReader.h>
#include <KUnit.h>
#include <KOdfStyleStack.h>
#include <KOdfLoadingContext.h>
#include <KCharacterStyle.h>
#include <KOdf.h>
#include <KOdfWorkaround.h>
#include <KTextDocumentLayout.h>

// KDChart
#include <KDChartChart>
#include <KDChartLegend>
#include <KDChartCartesianAxis>
#include <KDChartCartesianCoordinatePlane>
#include <KDChartRadarCoordinatePlane>
#include <KDChartGridAttributes>
#include <KDChartBarDiagram>
#include <KDChartLineDiagram>
#include <KDChartPieDiagram>
#include <KDChartPlotter>
#include <KDChartStockDiagram>
#include <KDChartRingDiagram>
#include <KDChartRadarDiagram>
#include <KDChartBarAttributes>
#include <KDChartPieAttributes>
#include <KDChartThreeDBarAttributes>
#include <KDChartThreeDPieAttributes>
#include <KDChartThreeDLineAttributes>
#include <KDChartBackgroundAttributes>
#include <KDChartRulerAttributes>

// KChart
#include "PlotArea.h"
#include "KDChartModel.h"
#include "DataSet.h"
#include "Legend.h"
#include "KDChartConvertions.h"
#include "ChartProxyModel.h"
#include "TextLabelDummy.h"
#include "Layout.h"
#include "OdfLoadingHelper.h"


class Axis::Private
{
public:
    Private(Axis *axis, AxisDimension dim);
    ~Private();

    void adjustAllDiagrams();
    /// Updates the axis potition in the chart's layout
    /// FIXME: We should instead implement a generic layout position method
    /// and have the layout find out about our position when it changes.
    void updatePosition();

    void registerDiagram(KDChart::AbstractDiagram *diagram);
    void deregisterDiagram(KDChart::AbstractDiagram *diagram);

    KDChart::AbstractDiagram *getDiagramAndCreateIfNeeded(ChartType chartType);
    KDChart::AbstractDiagram *getDiagram(ChartType chartType);
    void deleteDiagram(ChartType chartType);

    void createBarDiagram();
    void createLineDiagram();
    void createAreaDiagram();
    void createCircleDiagram();
    void createRingDiagram();
    void createRadarDiagram();
    void createScatterDiagram();
    void createStockDiagram();
    void createBubbleDiagram();
    void createSurfaceDiagram();
    void createGanttDiagram();
    void applyAttributesToDataSet(DataSet* set, ChartType newCharttype);

    // Pointer to Axis that owns this Private instance
    Axis * const q;

    PlotArea *plotArea;

    const AxisDimension dimension;

    KShape *title;
    TextLabelData *titleData;

    /// FIXME: Unused variable 'id', including id() getter
    QString id;
    QList<DataSet*> dataSets;
    qreal majorInterval;
    int minorIntervalDivisor;
    bool showInnerMinorTicks;
    bool showOuterMinorTicks;
    bool showInnerMajorTicks;
    bool showOuterMajorTicks;
    bool logarithmicScaling;
    bool showMajorGrid;
    bool showMinorGrid;
    bool useAutomaticMajorInterval;
    bool useAutomaticMinorInterval;

    /// Font used for axis labels
    /// TODO: Save to ODF
    QFont font;

    KDChart::CartesianAxis            *const kdAxis;
    KDChart::CartesianCoordinatePlane *kdPlane;
    KDChart::PolarCoordinatePlane     *kdPolarPlane;
    KDChart::RadarCoordinatePlane     *kdRadarPlane;

    KDChart::BarDiagram   *kdBarDiagram;
    KDChart::LineDiagram  *kdLineDiagram;
    KDChart::LineDiagram  *kdAreaDiagram;
    KDChart::PieDiagram   *kdCircleDiagram;
    KDChart::RingDiagram  *kdRingDiagram;
    KDChart::RadarDiagram *kdRadarDiagram;
    KDChart::Plotter      *kdScatterDiagram;
    KDChart::StockDiagram *kdStockDiagram;
    KDChart::Plotter      *kdBubbleDiagram;
    // FIXME BUG: Somehow we need to visualize something for these
    //            missing chart types.  We have some alternatives:
    //            1. Show an empty area
    //            2. Show a text "unsupported chart type"
    //            3. Exchange for something else, e.g. a bar chart.
    //            ... More?
    //
    // NOTE: Whatever we do, we should always store the data so that
    //       it can be saved back into the file for a perfect
    //       roundtrip.
    KDChart::BarDiagram   *kdSurfaceDiagram;
    KDChart::BarDiagram   *kdGanttDiagram;

    ChartType     plotAreaChartType;
    ChartSubtype  plotAreaChartSubType;

    // If KDChart::LineDiagram::centerDataPoints() property is set to true,
    // the data points drawn in a line (i.e., also an area) diagram start at
    // an offset of 0.5, that is, in the middle of a column in the diagram.
    // Set flag to true if at least one dataset is attached to this axis
    // that belongs to a horizontal bar chart
    bool centerDataPoints;

    // TODO: Save to ODF
    int gapBetweenBars;
    // TODO: Save to ODF
    int gapBetweenSets;

    // TODO: Save
    // See ODF v1.2 $19.12 (chart:display-label)
    bool showLabels;

    bool isVisible;
};


Axis::Private::Private(Axis *axis, AxisDimension dim)
    : q(axis)
    , dimension(dim)
    , kdAxis(new KDChart::CartesianAxis)
    , kdPlane(0)
    , kdPolarPlane(0)
    , kdRadarPlane(0)
{
    centerDataPoints = false;

    gapBetweenBars = 0;
    gapBetweenSets = 100;

    isVisible = true;

    useAutomaticMajorInterval = true;
    useAutomaticMinorInterval = true;

    majorInterval = 2;
    minorIntervalDivisor = 1;

    showMajorGrid = false;
    showMinorGrid = false;

    kdBarDiagram     = 0;
    kdLineDiagram    = 0;
    kdAreaDiagram    = 0;
    kdCircleDiagram  = 0;
    kdRingDiagram    = 0;
    kdRadarDiagram   = 0;
    kdScatterDiagram = 0;
    kdStockDiagram   = 0;
    kdBubbleDiagram  = 0;
    kdSurfaceDiagram = 0;
    kdGanttDiagram   = 0;

    title = 0;
    titleData = 0;

    KDChart::RulerAttributes attr = kdAxis->rulerAttributes();
    attr.setShowRulerLine(true);
    kdAxis->setRulerAttributes(attr);
}

Axis::Private::~Private()
{
    Q_ASSERT(plotArea);

    delete kdBarDiagram;
    delete kdAreaDiagram;
    delete kdCircleDiagram;
    delete kdRingDiagram;
    delete kdRadarDiagram;
    delete kdScatterDiagram;
    delete kdStockDiagram;
    delete kdBubbleDiagram;
    delete kdSurfaceDiagram;
    delete kdGanttDiagram;

    delete kdAxis;

    foreach(DataSet *dataSet, dataSets)
        dataSet->setAttachedAxis(0);
}

void Axis::Private::registerDiagram(KDChart::AbstractDiagram *diagram)
{
    KDChartModel *model = new KDChartModel;
    diagram->setModel(model);

    QObject::connect(plotArea->proxyModel(), SIGNAL(columnsInserted(const QModelIndex&, int, int)),
                      model,                  SLOT(slotColumnsInserted(const QModelIndex&, int, int)));

    QObject::connect(diagram, SIGNAL(propertiesChanged()),
                      plotArea, SLOT(plotAreaUpdate()));
    QObject::connect(diagram, SIGNAL(layoutChanged(AbstractDiagram*)),
                      plotArea, SLOT(plotAreaUpdate()));
    QObject::connect(diagram, SIGNAL(modelsChanged()),
                      plotArea, SLOT(plotAreaUpdate()));
    QObject::connect(diagram, SIGNAL(dataHidden()),
                      plotArea, SLOT(plotAreaUpdate()));
}

void Axis::Private::deregisterDiagram(KDChart::AbstractDiagram *diagram)
{
    KDChartModel *model = dynamic_cast<KDChartModel*>(diagram->model());
    Q_ASSERT(model);

    QObject::disconnect(plotArea->proxyModel(), SIGNAL(columnsInserted(const QModelIndex&, int, int)),
                         model,                  SLOT(slotColumnsInserted(const QModelIndex&, int, int)));

    QObject::disconnect(diagram, SIGNAL(propertiesChanged()),
                         plotArea, SLOT(plotAreaUpdate()));
    QObject::disconnect(diagram, SIGNAL(layoutChanged(AbstractDiagram*)),
                         plotArea, SLOT(plotAreaUpdate()));
    QObject::disconnect(diagram, SIGNAL(modelsChanged()),
                         plotArea, SLOT(plotAreaUpdate()));
    QObject::disconnect(diagram, SIGNAL(dataHidden()),
                         plotArea, SLOT(plotAreaUpdate()));

    delete model;
}

KDChart::AbstractDiagram *Axis::Private::getDiagramAndCreateIfNeeded(ChartType chartType)
{
    KDChart::AbstractDiagram *diagram = 0;

    switch (chartType) {
    case BarChartType:
        if (!kdBarDiagram)
            createBarDiagram();
        diagram = kdBarDiagram;
        break;
    case LineChartType:
        if (!kdLineDiagram)
            createLineDiagram();
        diagram = kdLineDiagram;
        break;
    case AreaChartType:
        if (!kdAreaDiagram)
            createAreaDiagram();
        diagram = kdAreaDiagram;
        break;
    case CircleChartType:
        if (!kdCircleDiagram)
            createCircleDiagram();
        diagram = kdCircleDiagram;
        break;
    case RingChartType:
        if (!kdRingDiagram)
            createRingDiagram();
        diagram = kdRingDiagram;
        break;
    case RadarChartType:
        if (!kdRadarDiagram)
            createRadarDiagram();
        diagram = kdRadarDiagram;
        break;
    case ScatterChartType:
        if (!kdScatterDiagram)
            createScatterDiagram();
        diagram = kdScatterDiagram;
        break;
    case StockChartType:
        if (!kdStockDiagram)
            createStockDiagram();
        diagram = kdStockDiagram;
        break;
    case BubbleChartType:
        if (!kdBubbleDiagram)
            createBubbleDiagram();
        diagram = kdBubbleDiagram;
        break;
    case SurfaceChartType:
        if (!kdSurfaceDiagram)
            createSurfaceDiagram();
        diagram = kdSurfaceDiagram;
        break;
    case GanttChartType:
        if (!kdGanttDiagram)
            createGanttDiagram();
        diagram = kdGanttDiagram;
        break;
    default:
        ;
    }

    adjustAllDiagrams();

    return diagram;
}

/**
 * Returns currently used internal KDChart diagram for the specified chart type
 */
KDChart::AbstractDiagram *Axis::Private::getDiagram(ChartType chartType)
{
    switch (chartType) {
        case BarChartType:
            return kdBarDiagram;
        case LineChartType:
            return kdLineDiagram;
        case AreaChartType:
            return kdAreaDiagram;
        case CircleChartType:
            return kdCircleDiagram;
        case RingChartType:
            return kdRingDiagram;
        case RadarChartType:
            return kdRadarDiagram;
        case ScatterChartType:
            return kdScatterDiagram;
        case StockChartType:
            return kdStockDiagram;
        case BubbleChartType:
            return kdBubbleDiagram;
        case SurfaceChartType:
            return kdSurfaceDiagram;
        case GanttChartType:
            return kdGanttDiagram;
        case LastChartType:
            return 0;
        // Compiler warning for unhandled chart type is intentional.
    }
    Q_ASSERT(!"Unhandled chart type");
    return 0;
}


void Axis::Private::deleteDiagram(ChartType chartType)
{
    KDChart::AbstractDiagram **diagram = 0;

    switch (chartType) {
    case BarChartType:
        diagram = (KDChart::AbstractDiagram**)&kdBarDiagram;
        break;
    case LineChartType:
        diagram = (KDChart::AbstractDiagram**)&kdLineDiagram;
        break;
    case AreaChartType:
        diagram = (KDChart::AbstractDiagram**)&kdAreaDiagram;
        break;
    case CircleChartType:
        diagram = (KDChart::AbstractDiagram**)&kdCircleDiagram;
        break;
    case RingChartType:
        diagram = (KDChart::AbstractDiagram**)&kdRingDiagram;
        break;
    case RadarChartType:
        diagram = (KDChart::AbstractDiagram**)&kdRadarDiagram;
        break;
    case ScatterChartType:
        diagram = (KDChart::AbstractDiagram**)&kdScatterDiagram;
        break;
    case StockChartType:
        diagram = (KDChart::AbstractDiagram**)&kdStockDiagram;
        break;
    case BubbleChartType:
        diagram = (KDChart::AbstractDiagram**)&kdBubbleDiagram;
        break;
    case SurfaceChartType:
        diagram = (KDChart::AbstractDiagram**)&kdSurfaceDiagram;
        break;
    case GanttChartType:
        diagram = (KDChart::AbstractDiagram**)&kdGanttDiagram;
        break;
    case LastChartType:
        Q_ASSERT("There is no diagram with type LastChartType");
    // Compiler warning for unhandled chart type is intentional.
    }

    Q_ASSERT(diagram);
    Q_ASSERT(*diagram);

    deregisterDiagram(*diagram);
    delete *diagram;

    *diagram = 0;

    adjustAllDiagrams();
}


void Axis::Private::createBarDiagram()
{
    Q_ASSERT(kdBarDiagram == 0);

    kdBarDiagram = new KDChart::BarDiagram(plotArea->kdChart(), kdPlane);
    registerDiagram(kdBarDiagram);
    // By 'vertical', KDChart means the orientation of a chart's bars,
    // not the orientation of the x axis.
    kdBarDiagram->setOrientation(plotArea->isVertical() ? Qt::Horizontal : Qt::Vertical);
    kdBarDiagram->setPen(QPen(Qt::black, 0.0));

    if (plotAreaChartSubType == StackedChartSubtype)
        kdBarDiagram->setType(KDChart::BarDiagram::Stacked);
    else if (plotAreaChartSubType == PercentChartSubtype) {
        kdBarDiagram->setType(KDChart::BarDiagram::Percent);
        kdBarDiagram->setUnitSuffix("%", kdBarDiagram->orientation());
    }

    if (isVisible)
        kdBarDiagram->addAxis(kdAxis);
    kdPlane->addDiagram(kdBarDiagram);

    Q_ASSERT(plotArea);
    foreach (Axis *axis, plotArea->axes())
    {
        if (axis->isVisible() && axis->dimension() == XAxisDimension)
            kdBarDiagram->addAxis(axis->kdAxis());
    }

    // Set default bar diagram attributes
    q->setGapBetweenBars(0);
    q->setGapBetweenSets(100);

    // Propagate existing settings
    KDChart::ThreeDBarAttributes attributes(kdBarDiagram->threeDBarAttributes());
    attributes.setEnabled(plotArea->isThreeD());
    kdBarDiagram->setThreeDBarAttributes(attributes);

    plotArea->parent()->legend()->kdLegend()->addDiagram(kdBarDiagram);
}

void Axis::Private::createLineDiagram()
{
    Q_ASSERT(kdLineDiagram == 0);

    kdLineDiagram = new KDChart::LineDiagram(plotArea->kdChart(), kdPlane);
    registerDiagram(kdLineDiagram);

    if (plotAreaChartSubType == StackedChartSubtype)
        kdLineDiagram->setType(KDChart::LineDiagram::Stacked);
    else if (plotAreaChartSubType == PercentChartSubtype)
        kdLineDiagram->setType(KDChart::LineDiagram::Percent);

    if (isVisible)
        kdLineDiagram->addAxis(kdAxis);
    kdPlane->addDiagram(kdLineDiagram);

    Q_ASSERT(plotArea);
    foreach (Axis *axis, plotArea->axes()) {
        if (axis->dimension() == XAxisDimension)
            if (axis->isVisible())
                kdLineDiagram->addAxis(axis->kdAxis());
    }

    // Propagate existing settings
    KDChart::ThreeDLineAttributes attributes(kdLineDiagram->threeDLineAttributes());
    attributes.setEnabled(plotArea->isThreeD());
    kdLineDiagram->setThreeDLineAttributes(attributes);

    KDChart::LineAttributes lineAttr = kdLineDiagram->lineAttributes();
    lineAttr.setMissingValuesPolicy(KDChart::LineAttributes::MissingValuesHideSegments);
    kdLineDiagram->setLineAttributes(lineAttr);

    plotArea->parent()->legend()->kdLegend()->addDiagram(kdLineDiagram);
}

void Axis::Private::createAreaDiagram()
{
    Q_ASSERT(kdAreaDiagram == 0);

    kdAreaDiagram = new KDChart::LineDiagram(plotArea->kdChart(), kdPlane);
    registerDiagram(kdAreaDiagram);
    KDChart::LineAttributes attr = kdAreaDiagram->lineAttributes();
    // Draw the area under the lines. This makes this diagram an area chart.
    attr.setDisplayArea(true);
    kdAreaDiagram->setLineAttributes(attr);
    kdAreaDiagram->setPen(QPen(Qt::black, 0.0));
    // KD Chart by default draws the first data set as last line in a normal
    // line diagram, we however want the first series to appear in front.
    kdAreaDiagram->setReverseDatasetOrder(true);

    if (plotAreaChartSubType == StackedChartSubtype)
        kdAreaDiagram->setType(KDChart::LineDiagram::Stacked);
    else if (plotAreaChartSubType == PercentChartSubtype)
    {
        kdAreaDiagram->setType(KDChart::LineDiagram::Percent);
        kdAreaDiagram->setUnitSuffix("%", kdBarDiagram->orientation());
    }

    if (isVisible)
        kdAreaDiagram->addAxis(kdAxis);
    kdPlane->addDiagram(kdAreaDiagram);

    Q_ASSERT(plotArea);
    foreach (Axis *axis, plotArea->axes()) {
        if (axis->dimension() == XAxisDimension)
            if (axis->isVisible())
                kdAreaDiagram->addAxis(axis->kdAxis());
    }

    // Propagate existing settings
    KDChart::ThreeDLineAttributes attributes(kdAreaDiagram->threeDLineAttributes());
    attributes.setEnabled(plotArea->isThreeD());
    kdAreaDiagram->setThreeDLineAttributes(attributes);

    plotArea->parent()->legend()->kdLegend()->addDiagram(kdAreaDiagram);
}

void Axis::Private::createCircleDiagram()
{
    Q_ASSERT(kdCircleDiagram == 0);

    kdCircleDiagram = new KDChart::PieDiagram(plotArea->kdChart(), kdPolarPlane);
    registerDiagram(kdCircleDiagram);
    KDChartModel *model = dynamic_cast<KDChartModel*>(kdCircleDiagram->model());
    Q_ASSERT(model);
    model->setDataDirection(Qt::Horizontal);

    plotArea->parent()->legend()->kdLegend()->addDiagram(kdCircleDiagram);
    kdPolarPlane->addDiagram(kdCircleDiagram);

    // Propagate existing settings
    KDChart::ThreeDPieAttributes attributes(kdCircleDiagram->threeDPieAttributes());
    attributes.setEnabled(plotArea->isThreeD());
    kdCircleDiagram->setThreeDPieAttributes(attributes);

    // Initialize with default values that are specified in PlotArea
    // Note: KDChart takes an int here, though ODF defines the offset to be a double.
    kdPolarPlane->setStartPosition((int)plotArea->pieAngleOffset());
}

void Axis::Private::createRingDiagram()
{
    Q_ASSERT(kdRingDiagram == 0);

    kdRingDiagram = new KDChart::RingDiagram(plotArea->kdChart(), kdPolarPlane);
    registerDiagram(kdRingDiagram);
    KDChartModel *model = dynamic_cast<KDChartModel*>(kdRingDiagram->model());
    Q_ASSERT(model);
    model->setDataDirection(Qt::Horizontal);

    plotArea->parent()->legend()->kdLegend()->addDiagram(kdRingDiagram);
    kdPolarPlane->addDiagram(kdRingDiagram);

    // Propagate existing settings
    KDChart::ThreeDPieAttributes attributes(kdRingDiagram->threeDPieAttributes());
    attributes.setEnabled(plotArea->isThreeD());
    kdRingDiagram->setThreeDPieAttributes(attributes);

    // Initialize with default values that are specified in PlotArea
    // Note: KDChart takes an int here, though ODF defines the offset to be a double.
    kdPolarPlane->setStartPosition((int)plotArea->pieAngleOffset());
}

void Axis::Private::createRadarDiagram()
{
    Q_ASSERT(kdRadarDiagram == 0);

    //kdRadarDiagramModel->setDataDimensions(2);
    //kdRadarDiagramModel->setDataDirection(Qt::Horizontal);

    kdRadarDiagram = new KDChart::RadarDiagram(plotArea->kdChart(), kdRadarPlane);
    registerDiagram(kdRadarDiagram);
    kdRadarDiagram->setCloseDatasets(true);

#if 0  // Stacked and Percent not supported by KDChart.
    if (plotAreaChartSubType == StackedChartSubtype)
        kdRadarDiagram->setType(KDChart::PolarDiagram::Stacked);
    else if (plotAreaChartSubType == PercentChartSubtype)
        kdRadarDiagram->setType(KDChart::PolarDiagram::Percent);
#endif
    plotArea->parent()->legend()->kdLegend()->addDiagram(kdRadarDiagram);
    kdRadarPlane->addDiagram(kdRadarDiagram);
}

void Axis::Private::createScatterDiagram()
{
    Q_ASSERT(kdScatterDiagram == 0);
    Q_ASSERT(plotArea);

    kdScatterDiagram = new KDChart::Plotter(plotArea->kdChart(), kdPlane);
    registerDiagram(kdScatterDiagram);

    KDChartModel *model = dynamic_cast<KDChartModel*>(kdScatterDiagram->model());
    Q_ASSERT(model);
    model->setDataDimensions(2);

    kdScatterDiagram->setPen(Qt::NoPen);

    if (isVisible)
        kdScatterDiagram->addAxis(kdAxis);
    kdPlane->addDiagram(kdScatterDiagram);


    foreach (Axis *axis, plotArea->axes()) {
        if (axis->dimension() == XAxisDimension)
            if (axis->isVisible())
                kdScatterDiagram->addAxis(axis->kdAxis());
    }

    // Propagate existing settings
    KDChart::ThreeDLineAttributes attributes(kdScatterDiagram->threeDLineAttributes());
    attributes.setEnabled(plotArea->isThreeD());
    kdScatterDiagram->setThreeDLineAttributes(attributes);

    plotArea->parent()->legend()->kdLegend()->addDiagram(kdScatterDiagram);
}

void Axis::Private::createStockDiagram()
{
    Q_ASSERT(kdStockDiagram == 0);

    kdStockDiagram = new KDChart::StockDiagram(plotArea->kdChart(), kdPlane);
    registerDiagram(kdStockDiagram);

#if 0  // Stacked and Percent not supported by KDChart.
    if (plotAreaChartSubType == StackedChartSubtype)
        kdStockDiagram->setType(KDChart::StockDiagram::Stacked);
    else if (plotAreaChartSubType == PercentChartSubtype)
        kdStockDiagram->setType(KDChart::StockDiagram::Percent);
#endif

    if (isVisible)
        kdStockDiagram->addAxis(kdAxis);
    kdPlane->addDiagram(kdStockDiagram);

    Q_ASSERT(plotArea);
    foreach (Axis *axis, plotArea->axes()) {
        if (axis->dimension() == XAxisDimension)
            if (axis->isVisible())
                kdStockDiagram->addAxis(axis->kdAxis());
    }

    plotArea->parent()->legend()->kdLegend()->addDiagram(kdStockDiagram);

}

void Axis::Private::createBubbleDiagram()
{
    Q_ASSERT(kdBubbleDiagram == 0);
    Q_ASSERT(plotArea);

    kdBubbleDiagram = new KDChart::Plotter(plotArea->kdChart(), kdPlane);
    registerDiagram(kdBubbleDiagram);

    KDChartModel *model = dynamic_cast<KDChartModel*>(kdBubbleDiagram->model());
    Q_ASSERT(model);
    model->setDataDimensions(2);

    kdPlane->addDiagram(kdBubbleDiagram);

    foreach (Axis *axis, plotArea->axes()) {
        //if (axis->dimension() == XAxisDimension)
            if (axis->isVisible())
                kdBubbleDiagram->addAxis(axis->kdAxis());
    }

     // disable the connecting line
    KDChart::LineAttributes la = kdBubbleDiagram->lineAttributes();
    la.setVisible(false);
    kdBubbleDiagram->setLineAttributes(la);

    plotArea->parent()->legend()->kdLegend()->addDiagram(kdBubbleDiagram);
}

void Axis::Private::createSurfaceDiagram()
{
    Q_ASSERT(!kdSurfaceDiagram);

    // This is a so far a by KDChart unsupported chart type.
    // Fall back to bar diagram for now.
    kdSurfaceDiagram = new KDChart::BarDiagram(plotArea->kdChart(), kdPlane);
    registerDiagram(kdSurfaceDiagram);
    plotArea->parent()->legend()->kdLegend()->addDiagram(kdSurfaceDiagram);
    kdPlane->addDiagram(kdSurfaceDiagram);
}

void Axis::Private::createGanttDiagram()
{
    // This is a so far a by KDChart unsupported chart type (through KDGantt got merged into KDChart with 2.3)
    Q_ASSERT(!kdGanttDiagram);

    // This is a so far a by KDChart unsupported chart type.
    // Fall back to bar diagram for now.
    kdGanttDiagram = new KDChart::BarDiagram(plotArea->kdChart(), kdPlane);
    registerDiagram(kdGanttDiagram);
    plotArea->parent()->legend()->kdLegend()->addDiagram(kdGanttDiagram);
    kdPlane->addDiagram(kdGanttDiagram);
}

/**
 * Automatically adjusts the diagram so that all currently displayed
 * diagram types fit together.
 */
void Axis::Private::adjustAllDiagrams()
{
    // If at least one dataset is attached that belongs to a
    // horizontal bar chart, set centerDataPoints to true.
    centerDataPoints = kdBarDiagram != 0;
    if (kdLineDiagram)
        kdLineDiagram->setCenterDataPoints(centerDataPoints);
    if (kdAreaDiagram)
        kdAreaDiagram->setCenterDataPoints(centerDataPoints);
}


// ================================================================
//                             class Axis


Axis::Axis(PlotArea *parent, AxisDimension dimension)
    : d(new Private(this, dimension))
{
    Q_ASSERT(parent);

    parent->addAxis(this);

    d->plotArea = parent;
    KDChart::BackgroundAttributes batt(d->kdAxis->backgroundAttributes());
    batt.setBrush(QBrush(Qt::white));
    d->kdAxis->setBackgroundAttributes(batt);
    d->kdPlane = parent->kdCartesianPlane(this);
    d->kdPolarPlane = parent->kdPolarPlane();
    d->kdRadarPlane = parent->kdRadarPlane();

    d->plotAreaChartType    = d->plotArea->chartType();
    d->plotAreaChartSubType = d->plotArea->chartSubType();

    KShapeFactoryBase *textShapeFactory = KShapeRegistry::instance()->value(TextShapeId);
    if (textShapeFactory)
        d->title = textShapeFactory->createDefaultShape(parent->parent()->resourceManager());
    if (d->title) {
        d->titleData = qobject_cast<TextLabelData*>(d->title->userData());
        if (d->titleData == 0) {
            d->titleData = new TextLabelData;
            d->title->setUserData(d->titleData);
        }

        QFont font = d->titleData->document()->defaultFont();
        font.setPointSizeF(9);
        d->titleData->document()->setDefaultFont(font);
    }
    else {
        d->title = new TextLabelDummy;
        d->titleData = new TextLabelData;
        KTextDocumentLayout *documentLayout = new KTextDocumentLayout(d->titleData->document());
        d->titleData->document()->setDocumentLayout(documentLayout);
        d->title->setUserData(d->titleData);
    }
    d->title->setSize(QSizeF(CM_TO_POINT(3), CM_TO_POINT(0.75)));

    d->plotArea->parent()->addShape(d->title);
    d->plotArea->parent()->setClipped(d->title, true);
    d->plotArea->parent()->setInheritsTransform(d->title, true);

    connect(d->plotArea, SIGNAL(gapBetweenBarsChanged(int)),
             this,        SLOT(setGapBetweenBars(int)));
    connect(d->plotArea, SIGNAL(gapBetweenSetsChanged(int)),
             this,        SLOT(setGapBetweenSets(int)));
    connect(d->plotArea, SIGNAL(pieAngleOffsetChanged(qreal)),
             this,        SLOT(setPieAngleOffset(qreal)));

    d->updatePosition();
}

Axis::~Axis()
{
    Q_ASSERT(d->plotArea);
    d->plotArea->parent()->KShapeContainer::removeShape(d->title);

    Q_ASSERT(d->title);
    if (d->title)
        delete d->title;

   delete d;
}

PlotArea* Axis::plotArea() const
{
    return d->plotArea;
}

KShape *Axis::title() const
{
    return d->title;
}

QString Axis::titleText() const
{
    return d->titleData->document()->toPlainText();
}

bool Axis::showLabels() const
{
    return d->showLabels;
}

QString Axis::id() const
{
    return d->id;
}

AxisDimension Axis::dimension() const
{
    return d->dimension;
}

QList<DataSet*> Axis::dataSets() const
{
    return d->dataSets;
}

bool Axis::attachDataSet(DataSet *dataSet)
{
    Q_ASSERT(!d->dataSets.contains(dataSet));
    if (d->dataSets.contains(dataSet))
        return false;

    d->dataSets.append(dataSet);

    if (dimension() == YAxisDimension) {
        dataSet->setAttachedAxis(this);

        ChartType chartType = dataSet->chartType();
        if (chartType == LastChartType)
            chartType = d->plotAreaChartType;

        KDChart::AbstractDiagram *diagram = d->getDiagramAndCreateIfNeeded(chartType);
        Q_ASSERT(diagram);
        KDChartModel *model = dynamic_cast<KDChartModel*>(diagram->model());
        Q_ASSERT(model);

        model->addDataSet(dataSet);

        layoutPlanes();
        requestRepaint();
    }

    return true;
}

bool Axis::detachDataSet(DataSet *dataSet, bool silent)
{
    Q_ASSERT(d->dataSets.contains(dataSet));
    if (!d->dataSets.contains(dataSet))
        return false;
    d->dataSets.removeAll(dataSet);

    if (dimension() == YAxisDimension) {
        ChartType chartType = dataSet->chartType();
        if (chartType == LastChartType)
            chartType = d->plotAreaChartType;

        KDChart::AbstractDiagram *oldDiagram = d->getDiagram(chartType);
        Q_ASSERT(oldDiagram);
        KDChartModel *oldModel = dynamic_cast<KDChartModel*>(oldDiagram->model());
        Q_ASSERT(oldModel);

        const int rowCount = oldModel->dataDirection() == Qt::Vertical
                                 ? oldModel->columnCount() : oldModel->rowCount();
        // If there's only as many rows as needed for *one*
        // dataset, that means that the dataset we're removing is
        // the last one in the model --> delete model
        if (rowCount == oldModel->dataDimensions())
            d->deleteDiagram(chartType);
        else
            oldModel->removeDataSet(dataSet, silent);

        dataSet->setKdChartModel(0);
        dataSet->setAttachedAxis(0);

        if (!silent) {
            layoutPlanes();
            requestRepaint();
        }
    }

    return true;
}

void Axis::clearDataSets()
{
    QList<DataSet*> list = d->dataSets;
    foreach(DataSet *dataSet, list)
        detachDataSet(dataSet, true);
}

qreal Axis::majorInterval() const
{
    return d->majorInterval;
}

void Axis::setMajorInterval(qreal interval)
{
    // Don't overwrite if automatic interval is being requested (for
    // interval = 0)
    if (interval != 0.0) {
        d->majorInterval = interval;
        d->useAutomaticMajorInterval = false;
    } else
        d->useAutomaticMajorInterval = true;

    // KDChart
    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes(orientation());
    attributes.setGridStepWidth(interval);
    d->kdPlane->setGridAttributes(orientation(), attributes);

    attributes = d->kdPolarPlane->gridAttributes(true);
    attributes.setGridStepWidth(interval);
    d->kdPolarPlane->setGridAttributes(true, attributes);

    // FIXME: Hide minor tick marks more appropriately
    if (!d->showMinorGrid && interval != 0.0)
        setMinorInterval(interval);

    requestRepaint();
}

qreal Axis::minorInterval() const
{
    return (d->majorInterval / (qreal)d->minorIntervalDivisor);
}

void Axis::setMinorInterval(qreal interval)
{
    if (interval == 0.0)
        setMinorIntervalDivisor(0);
    else
        setMinorIntervalDivisor(int(qRound(d->majorInterval / interval)));
}

int Axis::minorIntervalDivisor() const
{
    return d->minorIntervalDivisor;
}

void Axis::setMinorIntervalDivisor(int divisor)
{
    // A divisor of 0.0 means automatic minor interval calculation
    if (divisor != 0) {
        d->minorIntervalDivisor = divisor;
        d->useAutomaticMinorInterval = false;
    } else
        d->useAutomaticMinorInterval = true;

    // KDChart
    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes(orientation());
    attributes.setGridSubStepWidth((divisor != 0) ? (d->majorInterval / divisor) : 0.0);
    d->kdPlane->setGridAttributes(orientation(), attributes);

    attributes = d->kdPolarPlane->gridAttributes(true);
    attributes.setGridSubStepWidth((divisor != 0) ? (d->majorInterval / divisor) : 0.0);
    d->kdPolarPlane->setGridAttributes(true, attributes);

    requestRepaint();
}

bool Axis::useAutomaticMajorInterval() const
{
    return d->useAutomaticMajorInterval;
}

bool Axis::useAutomaticMinorInterval() const
{
    return d->useAutomaticMinorInterval;
}

void Axis::setUseAutomaticMajorInterval(bool automatic)
{
    d->useAutomaticMajorInterval = automatic;
    // A value of 0.0 will activate automatic intervals,
    // but not change d->majorInterval
    setMajorInterval(automatic ? 0.0 : majorInterval());
}

void Axis::setUseAutomaticMinorInterval(bool automatic)
{
    d->useAutomaticMinorInterval = automatic;
    // A value of 0.0 will activate automatic intervals,
    // but not change d->minorIntervalDivisor
    setMinorInterval(automatic ? 0.0 : minorInterval());
}

bool Axis::showInnerMinorTicks() const
{
    return d->showInnerMinorTicks;
}

bool Axis::showOuterMinorTicks() const
{
    return d->showOuterMinorTicks;
}

bool Axis::showInnerMajorTicks() const
{
    return d->showInnerMinorTicks;
}

bool Axis::showOuterMajorTicks() const
{
    return d->showOuterMajorTicks;
}

void Axis::setScalingLogarithmic(bool logarithmicScaling)
{
    d->logarithmicScaling = logarithmicScaling;

    if (dimension() != YAxisDimension)
        return;

    d->kdPlane->setAxesCalcModeY(d->logarithmicScaling
                                  ? KDChart::AbstractCoordinatePlane::Logarithmic
                                  : KDChart::AbstractCoordinatePlane::Linear);
    d->kdPlane->layoutPlanes();

    requestRepaint();
}

bool Axis::scalingIsLogarithmic() const
{
    return d->logarithmicScaling;
}

bool Axis::showMajorGrid() const
{
    return d->showMajorGrid;
}

void Axis::setShowMajorGrid(bool showGrid)
{
    d->showMajorGrid = showGrid;

    // KDChart
    KDChart::GridAttributes  attributes = d->kdPlane->gridAttributes(orientation());
    attributes.setGridVisible(d->showMajorGrid);
    d->kdPlane->setGridAttributes(orientation(), attributes);

    attributes = d->kdPolarPlane->gridAttributes(true);
    attributes.setGridVisible(d->showMajorGrid);
    d->kdPolarPlane->setGridAttributes(true, attributes);

    requestRepaint();
}

bool Axis::showMinorGrid() const
{
    return d->showMinorGrid;
}

void Axis::setShowMinorGrid(bool showGrid)
{
    d->showMinorGrid = showGrid;

    // KDChart
    KDChart::GridAttributes  attributes = d->kdPlane->gridAttributes(orientation());
    attributes.setSubGridVisible(d->showMinorGrid);
    d->kdPlane->setGridAttributes(orientation(), attributes);

    attributes = d->kdPolarPlane->gridAttributes(true);
    attributes.setSubGridVisible(d->showMinorGrid);
    d->kdPolarPlane->setGridAttributes(true, attributes);

    requestRepaint();
}

void Axis::setTitleText(const QString &text)
{
    d->titleData->document()->setPlainText(text);
}

void Axis::setShowLabels(bool show)
{
    d->showLabels = show;

    KDChart::TextAttributes textAttr = d->kdAxis->textAttributes();
    textAttr.setVisible(show);
    d->kdAxis->setTextAttributes(textAttr);
}

Qt::Orientation Axis::orientation()
{
    bool chartIsVertical = d->plotArea->isVertical();
    bool horizontal = d->dimension == (chartIsVertical ? YAxisDimension
                                                       : XAxisDimension);
    return horizontal ? Qt::Horizontal : Qt::Vertical;
}

bool Axis::loadOdf(const KXmlElement &axisElement, KShapeLoadingContext &context)
{
    KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
    OdfLoadingHelper *helper = (OdfLoadingHelper*)context.sharedData(OdfLoadingHelperId);

    d->title->setVisible(false);

    QPen gridPen(Qt::NoPen);
    QPen subGridPen(Qt::NoPen);

    d->showMajorGrid = false;
    d->showMinorGrid = false;

    d->showInnerMinorTicks = false;
    d->showOuterMinorTicks = false;
    d->showInnerMajorTicks = false;
    d->showOuterMajorTicks = true;

    // Use automatic interval calculation by default
    setMajorInterval(0.0);
    setMinorInterval(0.0);

    if (!axisElement.isNull()) {
        KXmlElement n;
        forEachElement (n, axisElement) {
            if (n.namespaceURI() != KOdfXmlNS::chart)
                continue;
            if (n.localName() == "title") {
                if (n.hasAttributeNS(KOdfXmlNS::svg, "x")
                     && n.hasAttributeNS(KOdfXmlNS::svg, "y"))
                {
                    const qreal x = KUnit::parseValue(n.attributeNS(KOdfXmlNS::svg, "x"));
                    const qreal y = KUnit::parseValue(n.attributeNS(KOdfXmlNS::svg, "y"));
                    d->title->setPosition(QPointF(x, y));
                }

                if (n.hasAttributeNS(KOdfXmlNS::svg, "width")
                     && n.hasAttributeNS(KOdfXmlNS::svg, "height"))
                {
                    const qreal width = KUnit::parseValue(n.attributeNS(KOdfXmlNS::svg, "width"));
                    const qreal height = KUnit::parseValue(n.attributeNS(KOdfXmlNS::svg, "height"));
                    d->title->setSize(QSizeF(width, height));
                }

                if (n.hasAttributeNS(KOdfXmlNS::chart, "style-name")) {
                    styleStack.clear();
                    context.odfLoadingContext().fillStyleStack(n, KOdfXmlNS::chart, "style-name", "chart");
                    styleStack.setTypeProperties("text");

                    if (styleStack.hasProperty(KOdfXmlNS::fo, "font-size")) {
                        const qreal fontSize = KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "font-size"));
                        QFont font = d->titleData->document()->defaultFont();
                        font.setPointSizeF(fontSize);
                        d->titleData->document()->setDefaultFont(font);
                    }
                }

                const KXmlElement textElement = KoXml::namedItemNS(n, KOdfXmlNS::text, "p");
                if (!textElement.isNull()) {
                    d->title->setVisible(true);
                    setTitleText(textElement.text());
                }
                else {
                    qWarning() << "Error: Axis' <chart:title> element contains no <text:p>";
                }
            }
            else if (n.localName() == "grid") {
                bool major = false;
                if (n.hasAttributeNS(KOdfXmlNS::chart, "class")) {
                    const QString className = n.attributeNS(KOdfXmlNS::chart, "class");
                    if (className == "major")
                        major = true;
                } else {
                    qWarning() << "Error: Axis' <chart:grid> element contains no valid class. It must be either \"major\" or \"minor\".";
                    continue;
                }

                if (major) {
                    d->showMajorGrid = true;
                } else {
                    d->showMinorGrid = true;
                }

                if (n.hasAttributeNS(KOdfXmlNS::chart, "style-name")) {
                    styleStack.clear();
                    context.odfLoadingContext().fillStyleStack(n, KOdfXmlNS::style, "style-name", "chart");
                    styleStack.setTypeProperties("graphic");
                    if (styleStack.hasProperty(KOdfXmlNS::svg, "stroke-color")) {
                        const QString strokeColor = styleStack.property(KOdfXmlNS::svg, "stroke-color");
                        //d->showMajorGrid = true;
                        if (major)
                            gridPen = QPen(QColor(strokeColor));
                        else
                            subGridPen = QPen(QColor(strokeColor));
                    }
                }
            }
            else if (n.localName() == "categories") {
                if (n.hasAttributeNS(KOdfXmlNS::table, "cell-range-address")) {
                    const CellRegion region = CellRegion(helper->tableSource, n.attributeNS(KOdfXmlNS::table, "cell-range-address"));
                    helper->categoryRegionSpecifiedInXAxis = true;
                    plotArea()->proxyModel()->setCategoryDataRegion(region);
                }
            }
        }

        if (axisElement.hasAttributeNS(KOdfXmlNS::chart, "axis-name")) {
            const QString name = axisElement.attributeNS(KOdfXmlNS::chart, "axis-name", QString());
            //setTitleText(name);
        }

        // NOTE: chart:dimension already handled by PlotArea before and passed
        // explicitly in the constructor.
    }

    if (axisElement.hasAttributeNS(KOdfXmlNS::chart, "style-name")) {
        styleStack.clear();
        context.odfLoadingContext().fillStyleStack(axisElement, KOdfXmlNS::chart, "style-name", "chart");
        styleStack.setTypeProperties("text");

        KCharacterStyle charStyle;
        charStyle.loadOdf(context);
        setFont(charStyle.font());

        styleStack.setTypeProperties("chart");

        if (styleStack.hasProperty(KOdfXmlNS::chart, "logarithmic")
             && styleStack.property(KOdfXmlNS::chart, "logarithmic") == "true")
        {
            setScalingLogarithmic(true);
        }

        if (styleStack.hasProperty(KOdfXmlNS::chart, "interval-major"))
            setMajorInterval(KUnit::parseValue(styleStack.property(KOdfXmlNS::chart, "interval-major")));
        if (styleStack.hasProperty(KOdfXmlNS::chart, "interval-minor-divisor"))
            setMinorIntervalDivisor(KUnit::parseValue(styleStack.property(KOdfXmlNS::chart, "interval-minor-divisor")));
        if (styleStack.hasProperty(KOdfXmlNS::chart, "display-label"))
            setShowLabels(styleStack.property(KOdfXmlNS::chart, "display-label") != "false");
        if (styleStack.hasProperty(KOdfXmlNS::chart, "visible"))
            setVisible(styleStack.property(KOdfXmlNS::chart, "visible")  != "false");
        if (styleStack.hasProperty(KOdfXmlNS::chart, "minimum")) {
            const qreal minimum = styleStack.property(KOdfXmlNS::chart, "minimum").toDouble();
            const qreal maximum = orientation() == Qt::Vertical
                                    ? d->kdPlane->verticalRange().second
                                    : d->kdPlane->horizontalRange().second;
            if (orientation() == Qt::Vertical)
                d->kdPlane->setVerticalRange(qMakePair(minimum, maximum));
            else
                d->kdPlane->setHorizontalRange(qMakePair(minimum, maximum));
        }
        if (styleStack.hasProperty(KOdfXmlNS::chart, "maximum")) {
            const qreal minimum = orientation() == Qt::Vertical
                                    ? d->kdPlane->verticalRange().first
                                    : d->kdPlane->horizontalRange().first;
            const qreal maximum = styleStack.property(KOdfXmlNS::chart, "maximum").toDouble();
            if (orientation() == Qt::Vertical)
                d->kdPlane->setVerticalRange(qMakePair(minimum, maximum));
            else
                d->kdPlane->setHorizontalRange(qMakePair(minimum, maximum));
        }
    } else {
        setShowLabels(KOdfWorkaround::fixMissingStyle_DisplayLabel(axisElement, context));
    }

    KDChart::GridAttributes gridAttr = d->kdPlane->gridAttributes(orientation());
    gridAttr.setGridVisible(d->showMajorGrid);
    gridAttr.setSubGridVisible(d->showMinorGrid);
    if (gridPen.style() != Qt::NoPen)
        gridAttr.setGridPen(gridPen);
    if (subGridPen.style() != Qt::NoPen)
        gridAttr.setSubGridPen(subGridPen);
    d->kdPlane->setGridAttributes(orientation(), gridAttr);

    gridAttr = d->kdPolarPlane->gridAttributes(orientation());
    gridAttr.setGridVisible(d->showMajorGrid);
    gridAttr.setSubGridVisible(d->showMinorGrid);
    if (gridPen.style() != Qt::NoPen)
        gridAttr.setGridPen(gridPen);
    if (subGridPen.style() != Qt::NoPen)
        gridAttr.setSubGridPen(subGridPen);
//     if (plotArea()->chartType() == RadarChartType)
//         d->kdPolarPlane->setGridAttributes(false, gridAttr);
//     else
    d->kdPolarPlane->setGridAttributes(true, gridAttr);

    gridAttr = d->kdRadarPlane->globalGridAttributes();
    gridAttr.setGridVisible(d->showMajorGrid);
    gridAttr.setSubGridVisible(d->showMinorGrid);
    if (gridPen.style() != Qt::NoPen)
        gridAttr.setGridPen(gridPen);
    if (subGridPen.style() != Qt::NoPen)
        gridAttr.setSubGridPen(subGridPen);
    d->kdRadarPlane->setGlobalGridAttributes(gridAttr);
    KDChart::TextAttributes ta(d->kdRadarPlane->textAttributes());
    ta.setVisible(helper->categoryRegionSpecifiedInXAxis);
    ta.setFont(font());
    ta.setFontSize(50);
    d->kdRadarPlane->setTextAttributes(ta);

    // Style of axis is still in styleStack
    if (!loadOdfChartSubtypeProperties(axisElement, context))
        return false;

    requestRepaint();

    return true;
}

bool Axis::loadOdfChartSubtypeProperties(const KXmlElement &axisElement,
                                          KShapeLoadingContext &context)
{
    Q_UNUSED(axisElement);
    KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("chart");

    // Load these attributes regardless of the actual chart type. They'll have
    // no effect if their respective chart type is not in use.
    // However, they'll be saved back to ODF that way.
    if (styleStack.hasProperty(KOdfXmlNS::chart, "gap-width"))
        setGapBetweenSets(KUnit::parseValue(styleStack.property(KOdfXmlNS::chart, "gap-width")));
    if (styleStack.hasProperty(KOdfXmlNS::chart, "overlap"))
        // The minus is intended!
        setGapBetweenBars(-KUnit::parseValue(styleStack.property(KOdfXmlNS::chart, "overlap")));

    return true;
}

void Axis::saveOdf(KShapeSavingContext &context)
{
    KXmlWriter &bodyWriter = context.xmlWriter();
    KOdfGenericStyles &mainStyles = context.mainStyles();
    bodyWriter.startElement("chart:axis");

    KOdfGenericStyle axisStyle(KOdfGenericStyle::ParagraphAutoStyle, "chart");
    axisStyle.addProperty("chart:display-label", "true");

    const QString styleName = mainStyles.insert(axisStyle, "ch");
    bodyWriter.addAttribute("chart:style-name", styleName);

    // TODO scale: logarithmic/linear
    // TODO visibility

    if (dimension() == XAxisDimension)
        bodyWriter.addAttribute("chart:dimension", "x");
    else if (dimension() == YAxisDimension)
        bodyWriter.addAttribute("chart:dimension", "y");

    QString name;
    switch(dimension()) {
    case XAxisDimension:
        name = "x";
        break;
    case YAxisDimension:
        name = "y";
        break;
    case ZAxisDimension:
        name = "z";
        break;
    }
    int i = 1;
    foreach (Axis *axis, d->plotArea->axes()) {
        if (axis == this)
            break;
        if (axis->dimension() == dimension())
            i++;
    }
    if (i == 1)
        name = "primary-" + name;
    else if (i == 2)
        name = "secondary-" + name;
    // Usually, there's not more than two axes of the same dimension.
    // But use a fallback name here nevertheless.
    else
        name = QString::number(i) + '-' + name;
    bodyWriter.addAttribute("chart:name", name);

    bodyWriter.startElement("chart:title");

    bodyWriter.addAttributePt("svg:x", d->title->position().x());
    bodyWriter.addAttributePt("svg:y", d->title->position().y());
    bodyWriter.addAttributePt("svg:width", d->title->size().width());
    bodyWriter.addAttributePt("svg:height", d->title->size().height());
    if (plotArea()->proxyModel()->categoryDataRegion().isValid())
    {
        bodyWriter.startElement("chart:categories");
          bodyWriter.addAttribute("table:cell-range-address", plotArea()->proxyModel()->categoryDataRegion().toString());
        bodyWriter.endElement();
    }

    bodyWriter.startElement("text:p");
    bodyWriter.addTextNode(d->titleData->document()->toPlainText());
    bodyWriter.endElement(); // text:p
    bodyWriter.endElement(); // chart:title

    if (showMajorGrid())
        saveOdfGrid(context, OdfMajorGrid);
    if (showMinorGrid())
        saveOdfGrid(context, OdfMinorGrid);

    bodyWriter.endElement(); // chart:axis
}

void Axis::saveOdfGrid(KShapeSavingContext &context, OdfGridClass gridClass)
{
    KXmlWriter &bodyWriter = context.xmlWriter();
    KOdfGenericStyles &mainStyles = context.mainStyles();

    KOdfGenericStyle gridStyle(KOdfGenericStyle::GraphicAutoStyle, "chart");

    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes(orientation());
    QPen gridPen = (gridClass == OdfMinorGrid ? attributes.subGridPen() : attributes.gridPen());
    KOdf::saveOdfStrokeStyle(gridStyle, mainStyles, gridPen);

    bodyWriter.startElement("chart:grid");
    bodyWriter.addAttribute("chart:class", gridClass == OdfMinorGrid ? "minor" : "major");

    bodyWriter.addAttribute("chart:style-name", mainStyles.insert(gridStyle, "ch"));
    bodyWriter.endElement(); // chart:grid
}

void Axis::update() const
{
    if (d->kdBarDiagram) {
        d->kdBarDiagram->doItemsLayout();
        d->kdBarDiagram->update();
    }

    if (d->kdLineDiagram) {
        d->kdLineDiagram->doItemsLayout();
        d->kdLineDiagram->update();
    }

    if (d->kdStockDiagram) {
        d->kdStockDiagram->doItemsLayout();
        d->kdStockDiagram->update();
    }

    d->plotArea->parent()->requestRepaint();
}

KDChart::CartesianAxis *Axis::kdAxis() const
{
    return d->kdAxis;
}

KDChart::AbstractCoordinatePlane *Axis::kdPlane() const
{
    return d->kdPlane;
}

void Axis::plotAreaChartTypeChanged(ChartType newChartType)
{
    if (dimension() != YAxisDimension)
        return;

    // Return if there's nothing to do
    if (newChartType == d->plotAreaChartType)
        return;

    if (d->dataSets.isEmpty()) {
        d->plotAreaChartType = newChartType;
        return;
    }

    ChartType oldChartType = d->plotAreaChartType;

    KDChart::AbstractDiagram *newDiagram = d->getDiagramAndCreateIfNeeded(newChartType);

    KDChartModel *newModel = dynamic_cast<KDChartModel*>(newDiagram->model());
    // FIXME: This causes a crash on unimplemented types. We should
    //        handle that in some other way.
    Q_ASSERT(newModel);


    if (   (isPolar(newChartType) && !isPolar(d->plotAreaChartType))
         || (!isPolar(newChartType) && isPolar(d->plotAreaChartType)))
    {
        foreach (DataSet *dataSet, d->dataSets) {
            if (dataSet->chartType() != LastChartType) {
                dataSet->setChartType(LastChartType);
                dataSet->setChartSubType(NoChartSubtype);
            }
        }
    }

    KDChart::AbstractDiagram *oldDiagram = d->getDiagram(oldChartType);
    Q_ASSERT(oldDiagram);
    // We need to know the old model so that we can remove the data sets
    // from the old model that we added to the new model.
    KDChartModel *oldModel = dynamic_cast<KDChartModel*>(oldDiagram->model());
    Q_ASSERT(oldModel);

    foreach (DataSet *dataSet, d->dataSets) {
        if (dataSet->chartType() != LastChartType)
            continue;

// FIXME: What does this do? Only the user may set a data set's pen through
// a proper UI, in any other case the pen falls back to a default
// which depends on the chart type, so setting it here will break the default
// for other chart types.
#if 0
        Qt::PenStyle newPenStyle = newDiagram->pen().style();
        QPen newPen = dataSet->pen();
        newPen.setStyle(newPenStyle);
        dataSet->setPen( newPen);
#endif
        newModel->addDataSet(dataSet);
        const int dataSetCount = oldModel->dataDirection() == Qt::Vertical
                                 ? oldModel->columnCount() : oldModel->rowCount();
        if (dataSetCount == oldModel->dataDimensions())
            // We need to call this method so set it sets d->kd[TYPE]Diagram to NULL
            d->deleteDiagram(oldChartType);
        else
            oldModel->removeDataSet(dataSet);
    }

    d->plotAreaChartType = newChartType;

    layoutPlanes();

    requestRepaint();
}

void Axis::plotAreaChartSubTypeChanged(ChartSubtype subType)
{
    d->plotAreaChartSubType = subType;
    if (d->kdBarDiagram) {
        d->kdBarDiagram->setUnitSuffix("", d->kdBarDiagram->orientation());
    }
    switch (d->plotAreaChartType) {
    case BarChartType:
        if (d->kdBarDiagram) {
            KDChart::BarDiagram::BarType type;
            switch (subType) {
            case StackedChartSubtype:
                type = KDChart::BarDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::BarDiagram::Percent;
                d->kdBarDiagram->setUnitSuffix("%", d->kdBarDiagram->orientation());
                break;
            default:
                type = KDChart::BarDiagram::Normal;
            }
            d->kdBarDiagram->setType(type);

        }
        break;
    case LineChartType:
        if (d->kdLineDiagram) {
            KDChart::LineDiagram::LineType type;
            switch (subType) {
            case StackedChartSubtype:
                type = KDChart::LineDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::LineDiagram::Percent;
                d->kdLineDiagram->setUnitSuffix("%", Qt::Vertical);
                break;
            default:
                type = KDChart::LineDiagram::Normal;
            }
            d->kdLineDiagram->setType(type);
        }
        break;
    case AreaChartType:
        if (d->kdAreaDiagram) {
            KDChart::LineDiagram::LineType type;
            switch (subType) {
            case StackedChartSubtype:
                type = KDChart::LineDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::LineDiagram::Percent;
                d->kdAreaDiagram->setUnitSuffix("%", Qt::Vertical);
                break;
            default:
                type = KDChart::LineDiagram::Normal;
            }
            d->kdAreaDiagram->setType(type);
        }
        break;
    case RadarChartType:
#if 0 // FIXME: Stacked and Percent not supported by KDChart
        if (d->kdRadarDiagram) {
            KDChart::PolarDiagram::PolarType type;
            switch (subType) {
            case StackedChartSubtype:
                type = KDChart::PolarDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::PolarDiagram::Percent; break;
            default:
                type = KDChart::PolarDiagram::Normal;
            }
            d->kdRadarDiagram->setType(type);
        }
#endif
        break;
    case StockChartType:
        if (d->kdStockDiagram) {
            KDChart::StockDiagram::Type type;
            switch (subType) {
#if 0
            case StackedChartSubtype:
                type = KDChart::StockDiagram::Candlestick; break;
            case PercentChartSubtype:
                type = KDChart::StockDiagram::OpenHighLowClose; break;
#endif
            default:
                type = KDChart::StockDiagram::HighLowClose;
            }
            d->kdStockDiagram->setType(type);
        }
        break;
    default:;
        // FIXME: Implement more chart types
    }
}

void Axis::plotAreaIsVerticalChanged()
{
    d->updatePosition();
}

void Axis::Private::updatePosition()
{
    // Is the first x or y axis?
    bool first = (dimension == XAxisDimension) ? plotArea->xAxis() == q
                                               : plotArea->yAxis() == q;

    Position position;
    if (q->orientation() == Qt::Horizontal)
        position = first ? BottomPosition : TopPosition;
    else
        position = first ? StartPosition : EndPosition;

    if (position == StartPosition)
        title->rotate(-90 - title->rotation());
    else if (position == EndPosition)
        title->rotate(90 - title->rotation());

    // KDChart
    kdAxis->setPosition(PositionToKDChartAxisPosition(position));
    Layout *layout = plotArea->parent()->layout();
    layout->setPosition(title, position);
    layout->layout();

    q->requestRepaint();
}

void Axis::registerKdAxis(KDChart::CartesianAxis *axis)
{
    if (d->kdBarDiagram)
        d->kdBarDiagram->addAxis(axis);
    if (d->kdLineDiagram)
        d->kdLineDiagram->addAxis(axis);
    if (d->kdAreaDiagram)
        d->kdAreaDiagram->addAxis(axis);
    if (d->kdScatterDiagram)
        d->kdScatterDiagram->addAxis(axis);
    if (d->kdStockDiagram)
        d->kdStockDiagram->addAxis(axis);
    if (d->kdBubbleDiagram)
        d->kdBubbleDiagram->addAxis(axis);
    // FIXME: Add all diagrams here
}

void Axis::deregisterKdAxis(KDChart::CartesianAxis *axis)
{
    if (d->kdBarDiagram)
        d->kdBarDiagram->takeAxis(axis);
    if (d->kdLineDiagram)
        d->kdLineDiagram->takeAxis(axis);
    if (d->kdAreaDiagram)
        d->kdAreaDiagram->takeAxis(axis);
    if (d->kdScatterDiagram)
        d->kdScatterDiagram->takeAxis(axis);
    if (d->kdStockDiagram)
        d->kdStockDiagram->takeAxis(axis);
    if (d->kdBubbleDiagram)
        d->kdBubbleDiagram->takeAxis(axis);
    // FIXME: Add all diagrams here
}

void Axis::setThreeD(bool threeD)
{
    // FIXME: Setting KD Chart attributes does not belong here. They should be
    // determined dynamically somehow.
    // KDChart
    if (d->kdBarDiagram) {
        KDChart::ThreeDBarAttributes attributes(d->kdBarDiagram->threeDBarAttributes());
        attributes.setEnabled(threeD);
        attributes.setDepth(15.0);
        d->kdBarDiagram->setThreeDBarAttributes(attributes);
    }

    if (d->kdLineDiagram) {
        KDChart::ThreeDLineAttributes attributes(d->kdLineDiagram->threeDLineAttributes());
        attributes.setEnabled(threeD);
        attributes.setDepth(15.0);
        d->kdLineDiagram->setThreeDLineAttributes(attributes);
    }

    if (d->kdAreaDiagram) {
        KDChart::ThreeDLineAttributes attributes(d->kdAreaDiagram->threeDLineAttributes());
        attributes.setEnabled(threeD);
        attributes.setDepth(15.0);
        d->kdAreaDiagram->setThreeDLineAttributes(attributes);
    }

    if (d->kdCircleDiagram) {
        KDChart::ThreeDPieAttributes attributes(d->kdCircleDiagram->threeDPieAttributes());
        attributes.setEnabled(threeD);
        attributes.setDepth(15.0);
        d->kdCircleDiagram->setThreeDPieAttributes(attributes);
    }

    if (d->kdRingDiagram) {
        KDChart::ThreeDPieAttributes attributes(d->kdRingDiagram->threeDPieAttributes());
        attributes.setEnabled(threeD);
        attributes.setDepth(15.0);
        d->kdRingDiagram->setThreeDPieAttributes(attributes);
    }

    // The following types don't support 3D, at least not in KDChart:
    // scatter, radar, stock, bubble, surface, gantt

    requestRepaint();
}

void Axis::requestRepaint() const
{
    d->plotArea->requestRepaint();
}

void Axis::layoutPlanes()
{
    d->kdPlane->layoutPlanes();
    d->kdPolarPlane->layoutPlanes();
    d->kdRadarPlane->layoutPlanes();
}

void Axis::setGapBetweenBars(int percent)
{
    // This method is also used to override KDChart's default attributes.
    // Do not just return and do nothing if value doesn't differ from stored one.
    d->gapBetweenBars = percent;

    if (d->kdBarDiagram) {
        KDChart::BarAttributes attributes = d->kdBarDiagram->barAttributes();
        attributes.setBarGapFactor((float)percent / 100.0);
        d->kdBarDiagram->setBarAttributes(attributes);
    }

    requestRepaint();
}

void Axis::setGapBetweenSets(int percent)
{
    // This method is also used to override KDChart's default attributes.
    // Do not just return and do nothing if value doesn't differ from stored one.
    d->gapBetweenSets = percent;

    if (d->kdBarDiagram) {
        KDChart::BarAttributes attributes = d->kdBarDiagram->barAttributes();
        attributes.setGroupGapFactor((float)percent / 100.0);
        d->kdBarDiagram->setBarAttributes(attributes);
    }

    requestRepaint();
}

void Axis::setPieAngleOffset(qreal angle)
{
    // only set if we already have a diagram else the value will be picked up on creating the diagram
    if (d->kdPolarPlane->diagram()) {
        // KDChart takes an int here, though ODF defines it to be a double.
        d->kdPolarPlane->setStartPosition((int)angle);

        requestRepaint();
    }
}

QFont Axis::font() const
{
    return d->font;
}

void Axis::setFont(const QFont &font)
{
    // Save the font for later retrieval
    d->font = font;

    // Set the KDChart axis to use this font as well.
    KDChart::TextAttributes attr = d->kdAxis->textAttributes();
    attr.setFont(font);
    d->kdAxis->setTextAttributes(attr);
}

bool Axis::isVisible() const
{
    return d->isVisible;
}

void Axis::setVisible(bool visible)
{
    d->isVisible = visible;

    if (visible)
        registerKdAxis(d->kdAxis);
    else
        deregisterKdAxis(d->kdAxis);
}

#include "Axis.moc"
