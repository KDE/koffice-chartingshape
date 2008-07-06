/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>

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
#include "TableModel.h"

// Qt
#include <QDebug>

// KOffice
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>

#include <cmath>

namespace KChart {

class TableModel::Private
{
public:
    Private();
    ~Private();
};

TableModel::Private::Private()
{
}

TableModel::Private::~Private()
{
}

TableModel::TableModel( QObject *parent /* = 0 */)
    : KoChart::ChartModel()
    , QStandardItemModel( parent )
    , d( new Private )
{
}

TableModel::~TableModel()
{
    delete d;
}

int rangeCharToInt( char c )
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 1) : -1;
}

int rangeStringToInt( const QString &string )
{
    int result = 0;
    const int size = string.size();
    for ( int i = 0; i < size; i++ )
    {
        qDebug() << "---" << rangeCharToInt( string[i].toAscii() ) * pow( 10, ( size - i - 1 ) );
        result += rangeCharToInt( string[i].toAscii() ) * pow( 10, ( size - i - 1 ) );
    }
    qDebug() << "+++++ result=" << result;
    return result;
}

QString rangeIntToString( int i )
{
    QString tmp = QString::number( i );
    for( int j = 0; j < tmp.size(); j++ )
    {
        tmp[j] = 'A' + tmp[j].toAscii() - '1';
    }
    qDebug() << "tmp=" << tmp;
    return tmp;
}

// reimplemented
QVector<QRect> TableModel::stringToRegion( const QString &string ) const
{
    const bool isPoint = !string.contains( ":" );
    qDebug() << "TableModel::stringToRegion():" << string;
    QString s = string;
    QStringList regionStrings = isPoint ? s.split( "." ) : s.remove( QChar(':') ).split( "." );
    QPoint topLeftPoint;
    QPoint bottomRightPoint;
    
    if ( isPoint && regionStrings.size() < 2 || !isPoint && regionStrings.size() < 3 )
    {
        qWarning() << "1) TableModel::stringToRegion(): Invalid region string \"" << string << "\"";
        return QVector<QRect>();
    }
    
    const QString tableName = regionStrings[0];
    
    const QString topLeft = regionStrings[1];
    QStringList l = topLeft.split( "$" );
    if ( l.size() < 3 )
    {
        qDebug() << topLeft;
        qWarning() << "2) TableModel::stringToRegion(): Invalid region string \"" << string << "\"";
        return QVector<QRect>();
    }
    int column = rangeStringToInt( l[1] );
    int row = l[2].toInt() - 1;
    topLeftPoint = QPoint( column, row );
    
    if ( isPoint )
    {
        qDebug() << "Returning" << QVector<QRect>( 1, QRect( topLeftPoint, QSize( 1, 1 ) ) );
        return QVector<QRect>( 1, QRect( topLeftPoint, QSize( 1, 1 ) ) );
    }
    
    const QString bottomRight = regionStrings[2];
    l = bottomRight.split( "$" );
    if ( l.size() < 3 )
    {
        qWarning() << "TableModel::stringToRegion(): Invalid region string \"" << string << "\"";
        return QVector<QRect>();
    }
    column = rangeStringToInt( l[1] );
    row = l[2].toInt() - 1;
    bottomRightPoint = QPoint( column, row );
    
    qDebug() << "///" << QRect( topLeftPoint, bottomRightPoint );
    
    return QVector<QRect>( 1, QRect( topLeftPoint, bottomRightPoint ) );
}

QString TableModel::regionToString( const QVector<QRect> &region ) const
{
    QString result( "local-table" );
    return result;
}    


void TableModel::loadOdf( const KoXmlElement &tableElement, const KoOdfStylesReader &stylesReader )
{
    setRowCount( 0 );
    setColumnCount( 0 );
    
    KoXmlElement n = tableElement.firstChild().toElement();
    for( ; !n.isNull(); n = n.nextSibling().toElement() )
    {
        qDebug() << n.localName();
        if ( n.namespaceURI() != KoXmlNS::table )
            continue;
        if ( n.localName() == "table-rows" || n.localName() == "table-header-rows" )
        {
            const bool isHeader = n.localName() == "table-header-rows";
            int row = 0;
            KoXmlElement _n = n.firstChild().toElement();
            for ( ; !_n.isNull(); _n = _n.nextSibling().toElement() )
            {
                if ( _n.namespaceURI() != KoXmlNS::table )
                    continue;
                if ( _n.localName() == "table-row" )
                {
                    int column = 0;
                    if ( !isHeader )
                        setRowCount( rowCount() + 1 );
                    KoXmlElement __n = _n.firstChild().toElement();
                    for ( ; !__n.isNull(); __n = __n.nextSibling().toElement() )
                    {
                        if ( __n.namespaceURI() != KoXmlNS::table )
                            continue;
                        if ( __n.localName() == "table-cell" )
                        {
                            if ( isHeader )
                                setColumnCount( columnCount() + 1 );
                            const QString valueType = __n.attributeNS( KoXmlNS::office, "value-type" );
                            QString valueString;
                            const KoXmlElement valueElement = __n.namedItemNS( KoXmlNS::text, "p" ).toElement();
                            if ( valueElement.isNull() || !valueElement.isElement() )
                            {
                                qWarning() << "TableModel::loadOdf(): Cell contains no valid <text:p> element, cannnot load cell data.";
                                continue;
                            }
                            QVariant value;
                            valueString = valueElement.text();
                            if ( valueType == "float" )
                                value = valueString.toDouble();
                            else if ( valueType == "boolean" )
                                value = (bool)valueString.toInt();
                            else // if ( valueType == "string" )
                                value = valueString;
                            if ( isHeader )
                                setHeaderData( column, Qt::Horizontal, value );
                            else
                                setData( index( row, column ), value );
                            qDebug() << isHeader;
                            qDebug() << "Setting data in" << row << column << "to" << value;
                            column++;
                        }
                    }
                }
                row++;
            }
        }
    }
    
    qDebug() << "BEFORE" << rowCount() << columnCount();
    reset();
    qDebug() << "AFTER" << rowCount() << columnCount();
    
    qDebug() << "++++++++ " << rowCount() << columnCount();
}

bool TableModel::saveOdf( KoXmlWriter &bodyWriter, KoGenStyles &mainStyles ) const
{
    return true;
}

}

#include "TableModel.moc"
