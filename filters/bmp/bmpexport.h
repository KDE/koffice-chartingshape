/* This file is part of the KDE project
   Copyright (C) 2005 Laurent Montel <montel@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef __BMPEXPORT_H__
#define __BMPEXPORT_H__

#include "imageexport.h"

class BmpExport : public ImageExport
{
    Q_OBJECT

public:
    BmpExport(KoFilter *parent, const char *name, const QStringList&);
    virtual ~BmpExport();
    virtual bool saveImage( QString fileName);
    virtual void extraImageAttribute();
    virtual const char * exportFormat();
};

#endif // __PNGEXPORT_H__
