/*
    KDE Icon Editor - a small graphics drawing program for the KDE
    Copyright (C) 1998  Thomas Tanghus (tanghus@kde.org)

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

#include <kconfig.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include "properties.h"

KIconEditProperties* KIconEditProperties::m_self = 0;

KIconEditProperties* KIconEditProperties::self()
{
    if (!m_self)
        m_self = new KIconEditProperties();
    return m_self;
}

KIconEditProperties::KIconEditProperties() : QObject()
{
    KConfigGroup config = KGlobal::config()->group( "Appearance" );

    m_bgMode = (Qt::BackgroundMode)config.readEntry( "BackgroundMode", int(Qt::FixedPixmap));
    QColor aux(Qt::gray);
    m_bgColor = config.readEntry( "BackgroundColor", aux);
    m_bgPixmap = config.readPathEntry("BackgroundPixmap", QString());

    config = KGlobal::config()->group( "Grid");
    m_pasteTransparent = config.readEntry( "PasteTransparent", false );
    m_showGrid = config.readEntry( "ShowGrid", true );
    m_gridScale = config.readEntry( "GridScaling", 10 );
    m_showRulers = config.readEntry( "ShowRulers", true );

    if(config.readEntry( "TransparencyDisplayType", "Checkerboard" ) == "Checkerboard")
    {
      m_transparencyDisplayType = KIconEditGrid::TRD_CHECKERBOARD;
    }
    else
    {
      m_transparencyDisplayType = KIconEditGrid::TRD_SOLIDCOLOR;
    }

    QColor checkColor1(255, 255, 255);
    QColor checkColor2(127, 127, 127);

    m_checkerboardColor1 = config.readEntry( "CheckerboardColor1", checkColor1);
    m_checkerboardColor2 = config.readEntry( "CheckerboardColor2", checkColor2);

    QString checkerboardSize = config.readEntry( "CheckerboardSize", "Medium" );

    if(checkerboardSize == "Small")
    {
      m_checkerboardSize = KIconEditGrid::CHK_SMALL;
    }
    else
    if(checkerboardSize == "Medium")
    {
      m_checkerboardSize = KIconEditGrid::CHK_MEDIUM;
    }
    else
    {
      m_checkerboardSize = KIconEditGrid::CHK_LARGE;
    }

    QColor solidColor(255, 255, 255);
    m_transparencySolidColor = config.readEntry( "TransparencySolidColor", solidColor);
}

KIconEditProperties::~KIconEditProperties()
{
  kDebug(4640) << "KIconEditProperties: Deleting properties";
  m_self = 0;
}

void KIconEditProperties::save()
{
    KConfigGroup config = KGlobal::config()->group( "Appearance" );

    config.writeEntry("BackgroundMode", (int)m_bgMode );
    config.writeEntry("BackgroundColor", m_bgColor );
    config.writePathEntry("BackgroundPixmap", m_bgPixmap );

    config = KGlobal::config()->group( "Grid");
    config.writeEntry("PasteTransparent", m_pasteTransparent );
    config.writeEntry("ShowGrid", m_showGrid );
    config.writeEntry("GridScaling", m_gridScale );
    config.writeEntry("ShowRulers", m_showRulers );

    QString transparencyDisplayType;

    switch(m_transparencyDisplayType)
    {
      case KIconEditGrid::TRD_SOLIDCOLOR:
        transparencyDisplayType = "SolidColor";
        break;
      case KIconEditGrid::TRD_CHECKERBOARD:
      default:
        transparencyDisplayType = "Checkerboard";
        break;
    }

    config.writeEntry( "TransparencyDisplayType", transparencyDisplayType );
    config.writeEntry( "CheckerboardColor1", m_checkerboardColor1 );
    config.writeEntry( "CheckerboardColor2", m_checkerboardColor2 );

    QString checkerboardSize;

    switch(m_checkerboardSize)
    {
      case KIconEditGrid::CHK_SMALL:
        checkerboardSize = "Small";
        break;
      case KIconEditGrid::CHK_MEDIUM:
        checkerboardSize = "Medium";
        break;
      case KIconEditGrid::CHK_LARGE:
      default:
        checkerboardSize = "Large";
        break;
    }

    config.writeEntry( "CheckerboardSize", checkerboardSize );
    config.writeEntry( "TransparencySolidColor", m_transparencySolidColor );

    config.sync();
}

