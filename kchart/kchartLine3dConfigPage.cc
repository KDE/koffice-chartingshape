/* This file is part of the KDE project
   Copyright (C) 2000,2001,2002,2003,2004 Laurent Montel <montel@kde.org>

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

#include "kchartLine3dConfigPage.h"
#include "kchartLine3dConfigPage.moc"

#include <kapplication.h>
#include <klocale.h>
#include <kdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

#include "kchart_params.h"

namespace KChart
{

KChartLine3dConfigPage::KChartLine3dConfigPage( KChartParams* params,
                                                          QWidget* parent ) :
    QWidget( parent ),_params( params )
{
  QGridLayout* layout = new QGridLayout(this, 2, 2,KDialog::marginHint(), KDialog::spacingHint() );

  QButtonGroup* gb = new QButtonGroup( i18n("3D Line Parameters"), this );
  QGridLayout *grid1 = new QGridLayout(gb,3,2,KDialog::marginHint(), KDialog::spacingHint());
  layout->addWidget(gb,0,0);


  line3d=new QCheckBox(i18n("3D line"),gb);
  grid1->addWidget(line3d,0,0);

  connect(line3d, SIGNAL(toggled ( bool )),this,SLOT(slotChange3DParameter(bool)));

  QLabel *tmpLabel = new QLabel( i18n( "Line width:" ), gb );
  grid1->addWidget(tmpLabel,1,0);

  lineWidth=new KIntNumInput(0, gb, 10);
  grid1->addWidget(lineWidth,1,1);

  drawShadowColor=new QCheckBox(i18n("Draw shadow color"),gb);
  grid1->addWidget(drawShadowColor,2,0);

  tmpLabel = new QLabel( i18n( "Rotation around the X-axis in degrees:" ), gb );
  tmpLabel->resize( tmpLabel->sizeHint() );
  grid1->addWidget(tmpLabel,3,0);

  angle3dX=new KIntNumInput(0, gb, 10);
  grid1->addWidget(angle3dX,3,1);
  angle3dX->setRange(0, 90, 1);

  tmpLabel = new QLabel( i18n( "Rotation around the Y-axis in degrees:" ), gb );
  tmpLabel->resize( tmpLabel->sizeHint() );
  grid1->addWidget(tmpLabel,4,0);

  angle3dY=new KIntNumInput(0, gb, 10);
  grid1->addWidget(angle3dY,4,1);
  angle3dY->setRange(0, 90, 1);


  tmpLabel = new QLabel( i18n( "Depth:" ), gb );
  tmpLabel->resize( tmpLabel->sizeHint() );
  grid1->addWidget(tmpLabel,5,0);

  depth=new KDoubleNumInput(0, gb);
  depth->resize(100,depth->sizeHint().height());
  grid1->addWidget(depth,5,1);
  depth->setRange(0,40, 0.1);

  gb->setAlignment(Qt::AlignLeft);
  grid1->addColSpacing(0,depth->width());
  grid1->addColSpacing(0,angle3dX->width());
  grid1->setColStretch(0,1);
  grid1->activate();
  //it's not good but I don't know how
  //to reduce space
  layout->addColSpacing(1,300);
}

void KChartLine3dConfigPage::slotChange3DParameter(bool b)
{
    angle3dX->setEnabled(b);
    angle3dY->setEnabled(b);
    depth->setEnabled(b);
    drawShadowColor->setEnabled(b);
    lineWidth->setEnabled(!b);
}

void KChartLine3dConfigPage::init()
{
    bool state=_params->threeDLines();
    line3d->setChecked(state);
    angle3dX->setValue( _params->threeDLineXRotation() );
    angle3dY->setValue( _params->threeDLineYRotation() );
    depth->setValue( _params->threeDLineDepth() );
    drawShadowColor->setChecked(_params->threeDShadowColors());
    lineWidth->setValue(_params->lineWidth());
    slotChange3DParameter(state);
}

void KChartLine3dConfigPage::apply()
{
    _params->setThreeDLines(line3d->isChecked());
    _params->setThreeDLineXRotation( angle3dX->value() );
    _params->setThreeDLineYRotation( angle3dY->value() );
    _params->setThreeDLineDepth( static_cast<int>( depth->value() ) );
    _params->setThreeDShadowColors( drawShadowColor->isChecked());
    _params->setLineWidth( lineWidth->value() );
}

}  //KChart namespace
