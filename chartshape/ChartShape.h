/* This file is part of the KDE project

   Copyright 2007 Stefan Nikolaus     <stefan.nikolaus@kdemail.net>
   Copyright 2007-2010 Inge Wallin    <inge@lysator.liu.se>
   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>

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

#ifndef KCHART_SHAPE_H
#define KCHART_SHAPE_H


// Qt
#include <Qt>

// KOffice
#include <KShapeContainer.h>
#include <KFrameShape.h>

// KChart
#include "kchart_export.h"
#include "kchart_global.h"
#include "KChartInterface.h"


class QAbstractItemModel;

class QPointF;
class QSizeF;
class QPen;
class QBrush;
class QColor;
class QString;
class QFont;

class KResourceManager;
class KShapeLoadingContext;
class KShapeSavingContext;
class KOdfStore;
class KXmlWriter;
class KOdfGenericStyles;
class KOdfStylesReader;
class KXmlDocument;

// FIXME: Remove all mentions of KDChart from the public API.
namespace KDChart {
    class Chart;
    class Legend;
    class CartesianAxis;
    class CartesianCoordinatePlane;
    class AbstractCoordinatePlane;
    class AbstractDiagram;
}

// Interface to SimpleTextShape plugin
class KTextShapeData;
#define TextShapeId "TextShapeID"
#define OdfLoadingHelperId "OdfLoadingHelperId"
typedef KTextShapeData TextLabelData;

class DataSet;
class ChartProxyModel;
class Legend;
class PlotArea;
class Surface;
class Axis;
class ThreeDScene;
class CellRegion;
class ChartTableModel;
class Layout;
class TableSource;

extern const char *ODF_CHARTTYPES[ NUM_CHARTTYPES ];

extern const ChartSubtype defaultSubtypes[ NUM_CHARTTYPES ];

extern QString saveOdfFont(KOdfGenericStyles& mainStyles, const QFont& font, const QColor& color);
extern QColor defaultDataSetColor(int dataSetNum);


class CHARTSHAPE_TEST_EXPORT ChartShape
    : public QObject
    , public KChart::ChartInterface // The public interface within KOffice
    , public KFrameShape            // For saving as a frame
    , public KShapeContainer        // The chart shape embeds other shapes.
{
    Q_OBJECT
    Q_INTERFACES(KChart::ChartInterface)

public:
    ChartShape(KResourceManager *documentResourceManager);
    ~ChartShape();

    // Getter methods
    ChartProxyModel *proxyModel() const;

    // Parts of the chart
    KShape        *title() const;
    TextLabelData  *titleData() const;
    KShape        *subTitle() const;
    TextLabelData  *subTitleData() const;
    KShape        *footer() const;
    TextLabelData  *footerData() const;
    Legend         *legend() const;
    PlotArea       *plotArea() const;
    Layout         *layout() const;

    /**
     * Returns a list of all labels in this chart, visible and hidden.
     * Use this method with caution, as it re-creates the list every
     * time you call it.
     */
    QList<KShape*> labels() const;

    void showTitle(bool doShow);
    void showSubTitle(bool doShow);
    void showFooter(bool doShow);

    /**
     * Returns the internal data table if existent, otherwise null.
     */
    QAbstractItemModel *internalModel() const;

    /**
     * Tells the ChartShape what model to use as internal table. When
     * the shape is loaded from ODF, it sets it itself.
     *
     * This method will assume that @a model has already been added to this
     * chart's TableSource.
     */
    void setInternalModel(QAbstractItemModel *model);

    /**
     * Returns a "map" containing all tables that are being used,
     * or may be used, in this chart.
     */
    TableSource *tableSource() const;

    /**
     * Returns true if the chart has no external data sources.
     *
     * This method is used to differentiate between charts that are embedded
     * into e.g. spreadsheets where these sheets provide the data, and
     * applications that do not provide the data, but where the data is stored
     * in an internal model in the chart document.
     *
     * For the first case, this method returns false, for the latter true.
     */
    bool usesInternalModelOnly() const;
    void setUsesInternalModelOnly(bool doesSo);

    ChartType     chartType() const;
    ChartSubtype  chartSubType() const;
    bool          isThreeD() const;

    /***
     * Inherited from koChartInterface
     ***/

    /**
     * Sets the SheetAccessModel to be used by this chart.
     *
     * See kspread/SheetAccessModel.h for details.
     */
    void setSheetAccessModel(QAbstractItemModel* model);

    /**
     * Re-initializes the chart with data from an arbitrary region.
     *
     * @param region             Name of region to use, e.g. "Table1.A1:B3"
     * @param firstRowIsLabel    Whether to interpret the first row as labels
     * @param firstColumnIsLabel Whether to interpret the first column as labels
     * @param dataDirection      orientation of a data set. Qt::Horizontal means a row is
     *                           to be interpreted as one data set, columns with Qt::Vertical.
     *
     * @see ChartProxyModel::init()
     */
    void reset(const QString& region,
                bool firstRowIsLabel,
                bool firstColumnIsLabel,
                Qt::Orientation dataDirection);

    void setChartType(ChartType type);
    void setChartSubType(ChartSubtype subType);
    void setThreeD(bool threeD);

    /// reimplemented
    void paintComponent(QPainter &painter, const KViewConverter &converter);
    void paintDecorations(QPainter &painter, const KViewConverter &converter,
                           const KCanvasBase *canvas);

    /// reimplemented
    bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    bool loadOdfFrameElement(const KXmlElement &element, KShapeLoadingContext &context);
    bool loadOdfData(const KXmlElement &tableElement, KShapeLoadingContext &context);

    bool loadEmbeddedDocument(KOdfStore *store, const KXmlElement &objectElement, const KXmlDocument &manifestDocument);
    bool loadOdfChartElement(const KXmlElement &chartElement, KShapeLoadingContext &context);
    /// reimplemented
    void saveOdf(KShapeSavingContext &context) const;
    void saveOdfData(KXmlWriter &bodyWriter, KOdfGenericStyles &mainStyles) const;

    /**
     * Used by unit tests to disable popping up of message boxes.
     *
     * User interaction is enabled by default.
     */
    static void setEnableUserInteraction(bool enable);

    using KShapeContainer::update;
    /// reimplemented
    void update() const;
    void relayout() const;

    void requestRepaint() const;

    /// the document resource manager we got on construction
    KResourceManager *resourceManager() const;

signals:
    void chartTypeChanged(ChartType);

private:
    class Private;
    Private *const d;
};

#endif
