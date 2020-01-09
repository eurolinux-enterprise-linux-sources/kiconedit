/*
    kiconedit - a small graphics drawing program for creating KDE icons
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

#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>

#include <kdebug.h>

#include "kcolorgrid.h"

void KColorArray::remove(int idx)
{
  int oldsize = size();
  if(idx >= (int)size())
  {
    kWarning() << "KColorArray::remove: Index " << idx << " out of range";
    return;
  }
  KColorArray tmp(*this);
  tmp.detach();
  resize(size()-1);
  for(int i = idx; i < oldsize-1; i++)
    at(i) = tmp[i+1];
  //kDebug(4640) << "KColorArray::remove() " << at(idx) << "\t-\tsize: " << size();
}

void KColorArray::append(uint c)
{
  resize(size()+1);
  at(size()-1) = c;
  //kDebug(4640) << "KColorArray::append() " << c << "\t-\tsize: " << size();
}

uint KColorArray::closestMatch(uint color)
{
  //kDebug(4640) << "KColorArray: " << c;
  uint c = color & ~OPAQUE_MASK, d = 0xffffff, t;
  //kDebug(4640) << "KColorArray: " << c;
  //kDebug(4640) << "KColorArray: " << c|OPAQUE_MASK;
  uint cb = c;
  for(uint i = 0; i < size(); i++)
  {
    if (at(i) > cb)
      t = at(i) - cb;
    else
      t = cb - at(i);
    if( t < d )
    {
      d = t;
      c = at(i);
    }
  }
  return c|OPAQUE_MASK;
}

KColorGrid::KColorGrid(QWidget *parent, int space)
 : QWidget(parent, Qt::WResizeNoErase|Qt::WNoAutoErase)
{
  //kDebug(4640) << "KColorGrid - constructor";
  s = space;
  rows = cols = totalwidth = totalheight = 0;
  setCellSize(10);
  setGridState(Plain);
  setGrid(true);
  numcolors.resize(0);
  gridcolors.resize(0);

  //kDebug(4640) << "KColorGrid - constructor - done";
}
/*
void KColorGrid::show()
{
  //updateScrollBars();
  QWidget::show();
}
*/
void KColorGrid::paintEvent(QPaintEvent *e)
{
  //kDebug(4640) << "KColorGrid::paintEvent";

  //updateScrollBars();
  //QWidget::paintEvent(e);

  const QRect urect = e->rect();

  //kDebug(4640) << "Update rect = ( " << //urect.left() << ", " << urect.top() << ", " << urect.width() << ", " << urect.height() << " )";


  int firstcol = getX(urect.x())-1;
  int firstrow = getY(urect.y())-1;
  int lastcol  = getX(urect.right())+1;
  int lastrow  = getY(urect.bottom())+1;

  QMatrix matrix;
  QPixmap pm(urect.width(),urect.height());
  pm.fill( palette().color(backgroundRole()) );
  QPainter p;
  p.begin( &pm );

  firstrow = (firstrow < 0) ? 0 : firstrow;
  firstcol = (firstcol < 0) ? 0 : firstcol;
  lastrow = (lastrow >= rows) ? rows : lastrow;
  lastcol = (lastcol >= cols) ? cols : lastcol;
  //kDebug(4640) << urect.x() << " x " << urect.y() << "  -  row: " << urect.width() << " x " << urect.height();
  //kDebug(4640) << "col: " << firstcol << " -> " << lastcol << "  -  row: " << firstrow << " -> " << lastrow;

/*
  if(this->isA("KDrawGrid"))
    kDebug(4640) << "KDrawGrid\n   firstcol: " << firstcol << "\n   lastcol: " << lastcol << "\n   firstrow: " << firstrow << "\n   lastrow: " << lastrow;
*/
  for(int i = firstrow; i < lastrow; i++)
  {
    //if(this->isA("KDrawGrid"))
    //  kDebug(4640) << "Updating row " << i;
    for(int j = firstcol; j < lastcol; j++)
    {
      matrix.translate( (j*cellsize)-urect.x(), (i*cellsize)-urect.y() );
      p.setWorldMatrix( matrix );
      //p.setClipRect(j*cellsize, i*cellsize, cellsize, cellsize);
      paintCell(&p, i, j);
      //p.setClipping(FALSE);
      matrix.reset();
      p.setWorldMatrix( matrix );
    }
    //kapp->processEvents();
  }

  matrix.translate(-urect.x(),-urect.y());
  p.setWorldMatrix( matrix );
  paintForeground(&p,e);

  p.end();

  bitBlt(this,urect.topLeft(),&pm,QRect(0,0,pm.width(),pm.height()));

}

void KColorGrid::paintForeground(QPainter* , QPaintEvent* )
{
}

/*
void KColorGrid::resizeEvent(QResizeEvent *)
{
  //kDebug(4640) << "resize: width:  " << width() << " - total: " << totalwidth;
  //kDebug(4640) << "resize: height: " << height() << " - total: " << totalheight;
}
*/

QSize KColorGrid::sizeHint() const
{
  return QSize(totalwidth, totalheight);
}

int KColorGrid::getY( int y )
{
  if(y > (totalheight-1))
    y = totalheight;
  if(cellsize == 1)
    return y;
  return (y/cellsize);
}

int KColorGrid::getX( int x )
{
  if( x > totalwidth-1)
    x = totalwidth;
  if(cellsize == 1)
    return x;
  return (x/cellsize);
}

const QRect KColorGrid::viewRect()
{
  //kDebug(4640) << "viewRect";
  const QRect r(0, 0, width(), height());
  //kDebug(4640) << "viewRect - " << x << " x " << y << " - " << w << " x " << h;
  return r;
}

void KColorGrid::setNumRows(int n)
{
  //kDebug(4640) << "setNumRows";
  if(n < 0 || n == rows)
    return;

  rows = n;

  gridcolors.resize(n*numCols());
  //QTableView::setNumRows(n);
  totalheight = (n * cellsize) + 1;
  resize(totalwidth, totalheight);
  //kDebug(4640) << "setNumRows() - gridcolors: " << gridcolors.size() << "  size: " << numCols()*numRows();
}

void KColorGrid::setNumCols(int n)
{
  //kDebug(4640) << "setNumCols";
  if(n < 0)
    return;
  int on = numCols();
  KColorArray gc(gridcolors);
  gc.detach();
  //kDebug(4640) << "gc size: " << gc.size() << " numrows: " << numRows();
  gridcolors.resize(n*numRows());
  cols = n;

  totalwidth = (n * cellsize) + 1;
  resize(totalwidth, totalheight);
  //kDebug(4640) << "numRows: " << numRows();
  //kDebug(4640) << "gridcolor: " << gridcolors.size() << "  grid: " << numRows()*numCols();
  if(numRows() == 0)
    return;

  for(int i = 0; i < numRows(); i++)
  {
    for(int j = 0; j < n; j++)
    {
      //kDebug(4640) << "row " << i << " , col " << j;
      if(j < on ) //If there's something to read here -- i.e. we're within the original grid
      {
        //kDebug(4640) << (i*numCols())+j << " " << (i*on)+j;
        gridcolors.at((i*numCols())+j) = gc.at((i*on)+j);
      }
      else //Initialize to something..
      {
        if (gc.size()) //Have some pixels originally..
            gridcolors.at((i*numCols())+j) = gc.at(0);
        else
            gridcolors.at((i*numCols())+j) = 0; //Picks something #### Update numcolors?
      }
    }
  }

  //kDebug(4640) << "setNumCols() - gridcolors: " << gridcolors.size() << "  size: " << numCols()*numRows();
}

void KColorGrid::fill( uint color)
{
  gridcolors.fill(color);
  numcolors.resize(1);
  numcolors.at(0) = color;
  emit colorschanged(numcolors.size(), numcolors.data());
}

void KColorGrid::setColor( int colNum, uint col, bool update )
{
  //kDebug(4640) << "KColorGrid::setColor";
  uint oldcolor = gridcolors[colNum];
  gridcolors[colNum] = col;

  if(!update)
    return;

  //kDebug(4640) << "KColorGrid::setColor - before adding";
  if(!numcolors.contains(col))
  {
    //kDebug(4640) << "KColorGrid::setColor() - adding " << //  col << " - " << qRed(col) << " " << qGreen(col) << " " << qBlue(col);
    numcolors.append(col);
    //kDebug(4640) << "KColorGrid::setColor() - adding done " << numcolors.size()-1;
    //numcolors++;
    emit addingcolor(col);
  }

  //kDebug(4640) << "KColorGrid::setColor - before removing";
  if(!gridcolors.contains(oldcolor))
  {
    int idx = numcolors.find(oldcolor);
    if(idx != -1)
    {
      //kDebug(4640) << "KColorGrid::setColor() - removing " << //  oldcolor << " - " << qRed(oldcolor) << " " << qGreen(oldcolor) << " " << qBlue(oldcolor);
      numcolors.remove(idx);
      //kDebug(4640) << "KColorGrid::setColor() - removing done";
      emit colorschanged(numcolors.size(), numcolors.data());
    }
    //numcolors--;
  }

  //kDebug(4640) << "KColorGrid::setColor - before updateCell";
  repaint((colNum%numCols())*cellsize,(colNum/numCols())*cellsize,  cellsize, cellsize);
  //updateCell( colNum/numCols(), colNum%numCols(), false );
  //kDebug(4640) << "KColorGrid::setColor - after updateCell";
}

void KColorGrid::updateCell( int row, int col, bool  )
{
    //kDebug(4640) << "updateCell - before repaint";
  QMatrix matrix;
  QPainter p;
  p.begin( this );
  matrix.translate( (col*cellsize), (row*cellsize) );
  p.setWorldMatrix( matrix );
  paintCell(&p, row, col);
  p.end();

}

void KColorGrid::updateColors()
{
  numcolors.resize(0);
  for(int i = 0; i < (int)gridcolors.size(); i++)
  {
    uint col = gridcolors.at(i);
    if(!numcolors.contains(col))
      numcolors.append(col);
  }
  emit colorschanged(numcolors.size(), numcolors.data());
}

void KColorGrid::setCellSize( int s )
{
  cellsize = s;
  totalwidth = (numCols() * s) + 1;
  totalheight = (numRows() * s) + 1;
  resize(totalwidth, totalheight);
  if ( isVisible() )
    repaint(viewRect());
}
#include "kcolorgrid.moc"
