/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>
   Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
     Contact: Suresh Chande suresh.chande@nokia.com

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
#include "TestLoadingBase.h"

// Qt
#include <QTextDocument>

// KDE
#include <qtest_kde.h>

// KOffice
#include <KoStore.h>
#include <KoOdfReadStore.h>
#include <KoTextShapeData.h>

// KChart
#include "ChartShape.h"
#include "ChartProxyModel.h"
#include "ChartDocument.h"
#include "PlotArea.h"
#include "Legend.h"
#include "DataSet.h"
#include "TableSource.h"
#include "Axis.h"

// KD Chart
#include <KDChartAbstractDiagram>
#include <KDChartLegend>

using namespace KChart;

TestLoadingBase::TestLoadingBase()
    : QObject()
{
    // No message boxes please.
    ChartShape::setEnableUserInteraction( false );
    m_chart = new ChartShape( 0 );
}

void TestLoadingBase::initTestCase()
{
    ChartDocument document( m_chart );
    KoStore *store = KoStore::createStore( QString( KDESRCDIR ) + "/doc", KoStore::Read );
    QVERIFY( store->enterDirectory( "doc" ) );
    QString errorMsg;
    KoOdfReadStore odfReadStore( store );
    bool success = odfReadStore.loadAndParse( errorMsg );
    if ( !success )
        qDebug() << "Error in odfReadStore.loadAndParse(): " << errorMsg;
    QVERIFY( success );
    QVERIFY( document.loadOdf( odfReadStore ) );
}

void TestLoadingBase::testElementIsVisible( KoShape *element, bool shouldBeVisible )
{
    QVERIFY( element );
    QCOMPARE( element->isVisible(), shouldBeVisible );
}

void TestLoadingBase::testLegendElements( QStringList labels )
{
    QVERIFY( m_chart->legend() );
    QVERIFY( m_chart->legend()->kdLegend() );
    QCOMPARE( m_chart->legend()->kdLegend()->datasetCount(),
              (unsigned int)labels.count() );

    QList<KDChart::AbstractDiagram*> diagrams = m_chart->legend()->kdLegend()->diagrams();
    foreach( KDChart::AbstractDiagram *diagram, diagrams ) {
        QVERIFY( diagram );
        QStringList diagramLabels = diagram->datasetLabels();
        foreach( QString diagramLabel, diagramLabels ) {
            QVERIFY( !labels.isEmpty() );
            QCOMPARE( diagramLabel, labels.takeFirst() );
        }
    }
    QVERIFY( labels.isEmpty() );
}

void TestLoadingBase::testDataSetCellRegions( int dataSetNr,
                                              CellRegion yDataRegion,
                                              CellRegion labelDataRegion,
                                              CellRegion categoryDataRegion,
                                              CellRegion xDataRegion,
                                              CellRegion customDataRegion )
{
    QVERIFY( m_chart->proxyModel() );
    QList<DataSet*> dataSets = m_chart->proxyModel()->dataSets();
    QVERIFY( dataSetNr >= 0 );
    QVERIFY( dataSets.count() > dataSetNr );
    DataSet *dataSet = dataSets[dataSetNr];
    QVERIFY( dataSet );

    int dataSetSize = 0;
    dataSetSize = qMax( dataSetSize, yDataRegion.cellCount() );
    dataSetSize = qMax( dataSetSize, categoryDataRegion.cellCount() );
    dataSetSize = qMax( dataSetSize, xDataRegion.cellCount() );
    dataSetSize = qMax( dataSetSize, customDataRegion.cellCount() );
    QCOMPARE( dataSet->size(), dataSetSize );

    QCOMPARE( dataSet->xDataRegion(),        xDataRegion );
    QCOMPARE( dataSet->yDataRegion(),        yDataRegion );
    QCOMPARE( dataSet->labelDataRegion(),    labelDataRegion );
    QCOMPARE( dataSet->categoryDataRegion(), categoryDataRegion );
    QCOMPARE( dataSet->customDataRegion(),   customDataRegion );
}

void TestLoadingBase::testHasOnlyInternalTable()
{
    QVERIFY( m_chart->usesInternalModelOnly() );
    QVERIFY( internalTable() );
}

void TestLoadingBase::testInternalTableSize( int rowCount, int colCount )
{
    QAbstractItemModel *model = m_chart->internalModel();
    QVERIFY( model );
    QVERIFY( m_chart->tableSource()->get( model ) );
    QCOMPARE( model->rowCount(), rowCount );
    QCOMPARE( model->columnCount(), colCount );
}

void TestLoadingBase::testTitleText( const QString &text )
{
    QVERIFY( m_chart->title() );
    KoTextShapeData *data = dynamic_cast<KoTextShapeData*>( m_chart->title()->userData() );
    QVERIFY( data );
    QVERIFY( data->document() );
    QCOMPARE( data->document()->toPlainText(), text );
}

void TestLoadingBase::testSubTitleText( const QString &text )
{
    QVERIFY( m_chart->subTitle() );
    KoTextShapeData *data = dynamic_cast<KoTextShapeData*>( m_chart->subTitle()->userData() );
    QVERIFY( data );
    QVERIFY( data->document() );
    QCOMPARE( data->document()->toPlainText(), text );
}

void TestLoadingBase::testFooterText( const QString &text )
{
    QVERIFY( m_chart->footer() );
    KoTextShapeData *data = dynamic_cast<KoTextShapeData*>( m_chart->footer()->userData() );
    QVERIFY( data );
    QVERIFY( data->document() );
    QCOMPARE( data->document()->toPlainText(), text );
}

void TestLoadingBase::testAxisTitle( Axis *axis, const QString &text )
{
    QVERIFY( axis );
    QVERIFY( axis->title() );
    KoTextShapeData *data = dynamic_cast<KoTextShapeData*>( axis->title()->userData() );
    QVERIFY( data );
    QVERIFY( data->document() );
    QCOMPARE( data->document()->toPlainText(), text );
}

Table *TestLoadingBase::internalTable()
{
    QAbstractItemModel *internalModel = m_chart->internalModel();
    if ( !internalModel )
        return 0;
    return m_chart->tableSource()->get( internalModel );
}

namespace QTest {
    template<>
    char *toString( const KChart::CellRegion &region ) {
        return qstrdup( region.toString().toAscii().data() );
    }
}
