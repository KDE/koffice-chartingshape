/* This file is part of the KDE project
   Copyright (C) 1999 Kalle Dalheimer <kalle@kde.org>

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

#include "kchart_shell.h"
#include "kchart_part.h"
#include "kchart_factory.h"
#include <kstddirs.h>
#include <klocale.h>

KChartShell::KChartShell( const char* name )
    : KoMainWindow( KChartFactory::global(), name )
{
}

KChartShell::~KChartShell()
{
}

QString KChartShell::nativeFormatName() const
{
  return i18n("KChart");
}

#include "kchart_shell.moc"
