/* -*- Mode: C++ -*-
   $Id$
   KDChart - a multi-platform charting engine
*/

/****************************************************************************
** Copyright (C) 2001-2002 Klar�lvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KDChart library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KDChart licenses may use this file in
** accordance with the KDChart Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.klaralvdalens-datakonsult.se/Public/products/ for
**   information about KDChart Commercial License Agreements.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
**********************************************************************/
#ifndef __KDCHARTVECTORTABLE_H__
#define __KDCHARTVECTORTABLE_H__

#include <qvaluevector.h>
#include <qshared.h>
#include <qtable.h>

#include <KDChartData.h>
#include <KDChartTableBase.h>

class KDChartVectorTablePrivate : public QShared
{
public:
    KDChartVectorTablePrivate() : QShared() {
        row_count = 0;
        col_count = 0;
    }

    KDChartVectorTablePrivate( uint _rows, uint _cols ) : QShared() {
        matrix.resize( _rows * _cols, KDChartData() );
        col_count = _cols;
        row_count = _rows;
    }

    KDChartVectorTablePrivate( const KDChartVectorTablePrivate& _t ) :
        QShared(),
        matrix( _t.matrix ),
        col_count( _t.col_count ),
        row_count( _t.row_count ) {}

    ~KDChartVectorTablePrivate() {}

    void expand( uint _rows, uint _cols ) {
        // Save the old table
        QValueVector<KDChartData> save( matrix );
        
        // Delete old data, then resize
        matrix.resize( 0 );
        matrix.resize( _rows * _cols, KDChartData() );
        
        // Copy over the old data
        for( uint row = 0; row < row_count; row++ )
            for( uint col = 0; col < col_count; col++ )
                matrix[ row * _cols + col ] = save[ row * col_count + col ];
        
        // set the new counts
        col_count = _cols;
        row_count = _rows;
    }
    
    KDChartData& cell( uint _row, uint _col ) {
        Q_ASSERT( _row < row_count && _col < col_count );
        return matrix[ static_cast < int > ( _row * col_count + _col ) ];
    }
    const KDChartData& cell( uint _row, uint _col ) const {
        Q_ASSERT( _row < row_count && _col < col_count );
        return matrix[ static_cast < int > ( _row * col_count + _col ) ];
    }
    void setCell( uint _row, uint _col, const KDChartData& _element ) {
        Q_ASSERT( _row < row_count && _col < col_count );
        matrix[ static_cast < int > ( _row * col_count + _col ) ] = _element;
    }

    void clearCell( uint _row, uint _col ) {
        Q_ASSERT( _row < row_count && _col < col_count );
        matrix[ static_cast < int > ( _row * col_count + _col ) ].clearValue();
    }

    void clearAllCells() {
        for ( uint r = 0; r < row_count; ++r )
            for ( uint c = 0; c < col_count; ++c )
                matrix[ static_cast < int > ( r * col_count + c ) ].clearValue();
    }

    QValueVector<KDChartData> matrix;

    uint col_count;
    uint row_count;
};


class KDChartVectorTableData : public KDChartTableDataBase
{
private:
    typedef KDChartVectorTablePrivate Priv;
    uint _usedRows, _usedCols;

public:
    /**
    * Typedefs
    */
    typedef QValueVector<KDChartData>::iterator Iterator;
    typedef QValueVector<KDChartData>::const_iterator ConstIterator;

    typedef QValueVector<int>::iterator RowIterator;
    typedef QValueVector<int>::const_iterator ConstRowIterator;

    typedef QValueVector<int>::iterator ColIterator;
    typedef QValueVector<int>::const_iterator ConstColIterator;

    /**
    * API
    */
    KDChartVectorTableData() {
        sh = new Priv;
        _usedCols = 0;
        _usedRows = 0;
    }
    KDChartVectorTableData( uint _rows, uint _cols ) {
        sh = new Priv( _rows, _cols );
        _usedRows = _rows;
        _usedCols = _cols;
    }

    KDChartVectorTableData( const KDChartVectorTableData& _t ) :
      KDChartTableDataBase( _t ) {
        _usedRows = _t._usedRows;
        _usedCols = _t._usedCols;
        sh = _t.sh;
        sh->ref();
    }

    virtual ~KDChartVectorTableData() {
        if ( sh->deref() )
            delete sh;
    }

    KDChartVectorTableData& operator=( const KDChartVectorTableData& t ) {
        if ( &t == this )
            return * this;
        _usedRows = t._usedRows;
        _usedCols = t._usedCols;
        t.sh->ref();
        if ( sh->deref() )
            delete sh;
        sh = t.sh;
        return *this;
    }

    Iterator begin() {
        return sh->matrix.begin();
    }

    ConstIterator begin() const {
        return sh->matrix.begin();
    }

    Iterator end() {
        return sh->matrix.end();
    }

    ConstIterator end() const {
        return sh->matrix.end();
    }

    bool isEmpty() const {
        return ( sh->col_count == 0 && sh->row_count == 0 );
    }

    uint cols() const {
        return sh->col_count;
    }

    uint rows() const {
        return sh->row_count;
    }

    KDChartData& cell( uint _row, uint _col ) {
        detach();
        return sh->cell( _row, _col );
    }

    const KDChartData& cell( uint _row, uint _col ) const {
        return sh->cell( _row, _col );
    }

    void setCell( uint _row, uint _col, const KDChartData& _element ) {
        detach();
        sh->setCell( _row, _col, _element );
    }

    void clearCell( uint _row, uint _col ) {
        detach();
        sh->clearCell( _row, _col );
    }

    void clearAllCells() {
        detach();
        sh->clearAllCells();
    }

    void expand( uint _rows, uint _cols ) {
        detach();
        sh->expand( _rows, _cols );
        _usedRows = _rows;
        _usedCols = _cols;
    }

    void setUsedRows( uint _rows ) {
        Q_ASSERT( _rows <= rows() );
        _usedRows = _rows;
    }

    uint usedRows() const {
        return _usedRows;
    }

    void setUsedCols( uint _cols ) {
        Q_ASSERT( _cols <= cols() );
        _usedCols = _cols;
    }

    uint usedCols() const {
        return _usedCols;
    }

private:
    /**
    * Helpers
    */
    void detach() {
        if ( sh->count > 1 ) {
            sh->deref();
            sh = new Priv( *sh );
        }
    }

    /**
     * Variables
     */
    Priv* sh;
};

#endif
// __KDCHARTLISTTABLE_H__
