/* This file is part of the KDE project
   Copyright (C) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "KChartParamsIface.h"

#include "kchart_params.h"

#include <dcopclient.h>

KChartParamsIface::KChartParamsIface( KChartParams *params_ )
    : DCOPObject()
{
    params = params_;
}

QString KChartParamsIface::chartType()const
{
    return params->chartTypeToString(params->chartType());
}

void KChartParamsIface::setChartType( const QString &chart )
{
    params->setChartType(params->stringToChartType( chart ));
}

QString KChartParamsIface::barChartSubType() const
{
    return params->barChartSubTypeToString(params->barChartSubType());
}

bool KChartParamsIface::threeDBars()
{
    return params->threeDBars();
}

void KChartParamsIface::setThreeDBars( bool threeDBars )
{
    params->setThreeDBars(threeDBars);
}

void KChartParamsIface::setThreeDBarsShadowColors( bool shadow )
{
    params->setThreeDBarsShadowColors(shadow);
}

bool KChartParamsIface::threeDBarsShadowColors() const
{
    return params->threeDBarsShadowColors();
}

void KChartParamsIface::setThreeDBarAngle( uint angle )
{
    params->setThreeDBarAngle(angle);
}

uint KChartParamsIface::threeDBarAngle() const
{
    return params->threeDBarAngle();
}

void KChartParamsIface::setThreeDBarDepth( double depth )
{
    params->setThreeDBarDepth(depth);
}

double KChartParamsIface::threeDBarDepth() const
{
    return params->threeDBarDepth();
}


bool KChartParamsIface::threeDPies()
{
    return params->threeDPies();
}

void KChartParamsIface::setThreeDPieHeight( int pixels )
{
    params->setThreeDPieHeight(pixels);
}

int KChartParamsIface::threeDPieHeight()
{
    return params->threeDPieHeight();
}

void KChartParamsIface::setPieStart( int degrees )
{
    params->setPieStart(degrees);
}

int KChartParamsIface::pieStart()
{
    return params->pieStart();
}

void KChartParamsIface::setExplode( bool explode )
{
    params->setExplode(explode);
}

bool KChartParamsIface::explode() const
{
    return params->explode();
}

void KChartParamsIface::setExplodeFactor( double factor )
{
    params->setExplodeFactor(factor);
}

double KChartParamsIface::explodeFactor() const
{
    return params->explodeFactor();
}

void KChartParamsIface::setThreeDPies( bool threeDPies )
{
    params->setThreeDPies(threeDPies);
}

void KChartParamsIface::hideLegend()
{
    params->setLegendPosition(KDChartParams::NoLegend);
}

void KChartParamsIface::setLegendPosition(const QString & pos)
{
    params->setLegendPosition(params->stringToLegendPosition(pos));
}

QString KChartParamsIface::legendPostion() const
{
    return params->legendPositionToString(params->legendPosition());
}

void KChartParamsIface::setLegendTitleText( const QString& text )
{
    params->setLegendTitleText( text );
}

void KChartParamsIface::setLegendSpacing( uint space )
{
    params->setLegendSpacing(space );
}


uint KChartParamsIface::legendSpacing()
{
    return params->legendSpacing();
}

bool KChartParamsIface::lineMarker() const
{
    return params->lineMarker();
}

void KChartParamsIface::setLineMarker( bool b)
{
    params->setLineMarker(b);
}

void KChartParamsIface::setLineWidth( uint width )
{
    params->setLineWidth( width );
}

uint KChartParamsIface::lineWidth() const
{
    return params->lineWidth();
}

bool KChartParamsIface::showGrid()
{
    return params->showGrid();
}

QString KChartParamsIface::header1Text() const
{
    return params->header1Text();
}

void KChartParamsIface::setHeader1Text( const QString& text )
{
    params->setHeader1Text(text);
}

void KChartParamsIface::setHeader2Text( const QString& text )
{
    params->setHeader2Text(text);
}

QString KChartParamsIface::header2Text() const
{
    return params->header2Text();
}


void KChartParamsIface::setOutlineDataLineWidth( uint width )
{
    params->setOutlineDataLineWidth( width );
}

uint KChartParamsIface::outlineDataLineWidth() const
{
    return params->outlineDataLineWidth();
}
