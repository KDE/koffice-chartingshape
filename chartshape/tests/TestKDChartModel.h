/* This file is part of the KDE project

   Copyright 2009 Johannes Simon <johannes.simon@gmail.com>

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

#ifndef KCHART_TESTKDCHARTMODEL_H
#define KCHART_TESTKDCHARTMODEL_H

#include <QtTest>
#include <QStandardItemModel>

#include "KDChartModel.h"
#include "ModelObserver.h"
#include "TableSource.h"

using namespace KChart;

class TestKDChartModel : public QObject
{
    Q_OBJECT

public:
    TestKDChartModel();

private slots:
    void init();
    void cleanup();
    void initTestCase();

    void testDataSetInsertion();
    void testDataSetInsertionAndRemoval();
    void testData();

private:
    KDChartModel *m_model;
    ModelObserver *m_testModel;

    // These are all only dummies, but we need them for valid CellRegions
    QStandardItemModel m_itemModel;
    TableSource m_source;
    Table *m_table;
};

#endif // KCHART_TESTKDCHARTMODEL_H
