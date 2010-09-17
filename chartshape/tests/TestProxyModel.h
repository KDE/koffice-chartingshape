/* This file is part of the KDE project

   Copyright 2008, 2010 Johannes Simon <johannes.simon@gmail.com>

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

#ifndef KCHART_TESTPROXYMODEL_H
#define KCHART_TESTPROXYMODEL_H

// Qt
#include <QtTest>

// KChart
#include "ChartProxyModel.h"
#include "ChartTableModel.h"
#include "TableSource.h"

using namespace KChart;

class TestProxyModel : public QObject
{
    Q_OBJECT

public:
    TestProxyModel();

private slots:
    void initTestCase();

    void testWithoutLabels();
    void testFirstRowAsLabel();
    void testFirstColumnAsLabel();
    void testFirstRowAndColumnAsLabels();
    void testRegionOrder();
    void testComplexRegions();

private:
    // m_source must be initialized before m_proxyModel
    TableSource m_source;
    ChartProxyModel m_proxyModel;
    ChartTableModel m_sourceModel;
    Table *m_table;
};
    
#endif // KCHART_TESTPROXYMODEL_H
