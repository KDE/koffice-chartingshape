/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
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


#ifndef KCHART_CHARTTABLEMODEL_H
#define KCHART_CHARTTABLEMODEL_H


// Qt
#include <QVector>
#include <QStandardItemModel>

// Interface
#include <KChartModel.h>

// KChart
#include "ChartShape.h"


class QString;

// FIXME: Should it inherit QAbstractTableModel instead?

/**
 * @brief The ChartTableModel class is used to store a data table inside a chart document.  It's main difference to a normal table model is that it can load and store to ODF.
 */
class CHARTSHAPE_TEST_EXPORT ChartTableModel : public QStandardItemModel, public KChart::ChartModel
{
    Q_OBJECT
    Q_INTERFACES(KChart::ChartModel)

public:
    ChartTableModel(QObject *parent = 0);
    ~ChartTableModel();

    // KChart::ChartModel interface
    virtual QHash<QString, QVector<QRect> > cellRegion() const;
    virtual bool setCellRegion(const QString& regionName);
    virtual bool isCellRegionValid(const QString& regionName) const;

    // Load and save the contained data in an ODF DOM tree.
    // FIXME: Move saving/loading logic to helper class like TableLoaderAndSaver,
    // this class is outdated and should be removed.
    bool loadOdf(const KXmlElement &tableElement,
                  KShapeLoadingContext &context);
    bool saveOdf(KXmlWriter &bodyWriter, KOdfGenericStyles &mainStyles) const;
};

#endif // KCHART_TABLEMODEL_H
