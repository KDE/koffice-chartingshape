/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

// Local
#include "ChartShape.h"

// KChart
#include "KDChartAbstractCoordinatePlane.h"
#include "KDChartBackgroundAttributes.h"
#include "KDChartBarDiagram.h"
#include "KDChartCartesianAxis.h"
#include "KDChartCartesianCoordinatePlane.h"
#include "KDChartChart.h"
#include "KDChartFrameAttributes.h"
#include "KDChartGridAttributes.h"
#include "KDChartLegend.h"
#include "KDChartLineDiagram.h"

// KOffice
#include <KoViewConverter.h>

// KDE
#include <kdebug.h>
#include <klocale.h>

// Qt
#include <QAbstractItemModel>
#include <QPainter>

using namespace KChart;
using namespace KDChart;

class ChartShape::Private
{
public:
    Chart* chart;
    AbstractDiagram* diagram;
    QAbstractItemModel* model;
};


ChartShape::ChartShape()
    : d( new Private )
{
    kDebug() << k_funcinfo << endl;
    d->chart = new Chart();
    d->diagram = new BarDiagram( d->chart, static_cast<CartesianCoordinatePlane*>(d->chart->coordinatePlane() ) );
    d->model = 0;

    AbstractCartesianDiagram* diagram = static_cast<AbstractCartesianDiagram*>(d->diagram);
    CartesianAxis *xAxis = new CartesianAxis( diagram );
    CartesianAxis *yAxis = new CartesianAxis( diagram );
    CartesianAxis *axisTop = new CartesianAxis( diagram );
    CartesianAxis *axisRight = new CartesianAxis( diagram );
    xAxis->setPosition( CartesianAxis::Bottom );
    yAxis->setPosition( CartesianAxis::Left );
    axisTop->setPosition( CartesianAxis::Top );
    axisRight->setPosition( CartesianAxis::Right );

    diagram->addAxis( xAxis );
    diagram->addAxis( yAxis );
    diagram->addAxis( axisTop );
    diagram->addAxis( axisRight );

    d->chart->coordinatePlane()->replaceDiagram( d->diagram );
qDebug() << d->chart->coordinatePlane()->diagram()->metaObject()->className();

    // diagram->coordinatePlane returns an abstract plane one.
    // if we want to specify the orientation we need to cast
    // as follow
    CartesianCoordinatePlane* plane = static_cast <CartesianCoordinatePlane*>
		    ( d->diagram->coordinatePlane() );

    /* Configure grid steps and pen */

    // Vertical
    GridAttributes gv ( plane->gridAttributes( Qt::Vertical ) );

    // Configure a grid pen
    // I know it is horrible
    // just for demo'ing
    QPen gridPen(  Qt::gray );
    gridPen.setWidth( 0 );
    gv.setGridPen(  gridPen );

    // Configure a sub-grid pen
    QPen subGridPen( Qt::darkGray );
    subGridPen.setWidth( 0 );
    subGridPen.setStyle( Qt::DotLine );
    gv.setSubGridPen(  subGridPen );

    // Display a blue zero line
    gv.setZeroLinePen( QPen( Qt::blue ) );

    // change step and substep width
    // or any of those.
    gv.setGridStepWidth( 1.0 );
    gv.setGridSubStepWidth( 0.5 );
    gv.setGridVisible(  true );
    gv.setSubGridVisible( true );

    // Horizontal

    GridAttributes gh = plane->gridAttributes( Qt::Horizontal );
    gh.setGridPen( gridPen );
    gh.setGridStepWidth(  0.5 );
    gh.setSubGridPen(  subGridPen );
    gh.setGridSubStepWidth( 0.1 );

    plane->setGridAttributes( Qt::Vertical,  gv );
    plane->setGridAttributes( Qt::Horizontal,  gh );
}

ChartShape::~ChartShape()
{
    delete d->model;
    delete d->diagram;
    delete d->chart;
    delete d;
}

Chart* ChartShape::chart() const
{
    return d->chart;
}

void ChartShape::setModel( QAbstractItemModel* model )
{
    kDebug() << k_funcinfo << "BEGIN";
    d->model = model;
    d->chart->coordinatePlane()->takeDiagram( d->diagram );
    d->diagram->setModel( model );
    d->diagram->update();
    d->chart->coordinatePlane()->replaceDiagram( d->diagram );

    for ( int iColumn = 0; iColumn<d->diagram->model()->columnCount(); ++iColumn )
    {
        QPen pen(d->diagram->pen( iColumn ));
        pen.setColor( Qt::black );
        pen.setWidth(4);
        d->diagram->setPen( iColumn, pen );
    }

    KDChart::FrameAttributes faChart( d->chart->frameAttributes() );
    faChart.setPen( QPen(QColor(0x60,0x60,0xb0), 8) );
    d->chart->setFrameAttributes( faChart );

    BackgroundAttributes baChart( d->chart->backgroundAttributes() );
    baChart.setVisible( true );
    baChart.setBrush( QColor(0xd0,0xd0,0xff) );
    d->chart->setBackgroundAttributes( baChart );

    // Set up the legend
    Legend* m_legend;
    m_legend = new Legend( d->diagram, d->chart );
    m_legend->setPosition( Position::South );
    m_legend->setAlignment( Qt::AlignRight );
    m_legend->setShowLines( false );
    m_legend->setTitleText( i18n( "Legend" ) );
    m_legend->setOrientation( Qt::Horizontal );
    d->chart->addLegend( m_legend );

    kDebug() << k_funcinfo <<" END";
}

void ChartShape::paint( QPainter& painter, const KoViewConverter& converter )
{
    const QRectF paintRect = QRectF( QPointF( 0.0, 0.0 ), size() );

    applyConversion( painter, converter );
    painter.setClipRect( paintRect, Qt::IntersectClip );

    kDebug() << "Painting chart into " << paintRect << endl;
    // painting chart contents
    d->chart->paint( &painter, paintRect.toRect() );
}

void ChartShape::saveOdf( KoShapeSavingContext & context ) const
{
}

bool ChartShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context ) {
    return false; // TODO
}
