/*
    KDE Draw - a small graphics drawing program for the KDE
    Copyright (C) 1998  Thomas Tanghus (tanghus@kde.org)
    Copyright (C) 2002  Nadeem Hasan ( nhasan@kde.org )

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGroupBox>
#include <klocale.h>
#include <knuminput.h>

#include "kresize.h"

KResizeWidget::KResizeWidget( QWidget* parent, const QSize& size ) : QWidget( parent )
{
  QHBoxLayout* genLayout = new QHBoxLayout;
  QGroupBox *group = new QGroupBox(i18n( "Size" ));
  
  m_width = new KIntSpinBox( 1, 200, 1, 1,group );
  m_width->setValue( size.width() );

  QLabel* label = new QLabel( "X" );

  m_height = new KIntSpinBox( 1, 200, 1, 1,group);
  m_height->setValue( size.height() );

  QHBoxLayout *hbox = new QHBoxLayout;
  
  hbox->addWidget(m_width);
  hbox->addWidget(label);
  hbox->addWidget(m_height);
  hbox->setSpacing( 0 );
  group->setLayout(hbox);
  genLayout->addWidget(group);
  setLayout(genLayout);
  setMinimumSize( 200, 100 );
  genLayout->setMargin(0);
}

KResizeWidget::~KResizeWidget()
{
}

const QSize KResizeWidget::getSize()
{
  return QSize( m_width->value(), m_height->value() );
}

KResizeDialog::KResizeDialog( QWidget* parent, const QSize size )
    : KDialog( parent )
{
    setCaption(  i18n( "Select Size" ) );
    setButtons( Ok|Cancel );
    setModal( true );
  m_resize = new KResizeWidget( this, size );

  setMainWidget( m_resize );
}

KResizeDialog::~KResizeDialog()
{
}

const QSize KResizeDialog::getSize()
{
  return m_resize->getSize();
}

#include "kresize.moc"
