/*
    KDE Draw - a small graphics drawing program for the KDE
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

#include <qlayout.h>
#include <qpainter.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3StrList>
#include <QHBoxLayout>
#include <Q3ValueList>
#include <QVBoxLayout>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kapplication.h>

#include "knew.h"

KIconTemplateContainer* KIconTemplateContainer::instance = 0;

void createStandardTemplates(KIconTemplateContainer *list)
{
    KIconLoader *kil = KIconLoader::global();

    KIconTemplate it;
    it.path = kil->iconPath("standard", KIconLoader::User);
    it.title = i18n("Standard File");
    list->append(it);

    it.path = kil->iconPath("source", KIconLoader::User);
    it.title = i18n("Source File");
    list->append(it);

    it.path = kil->iconPath("compressed", KIconLoader::User);
    it.title = i18n("Compressed File");
    list->append(it);

    it.path = kil->iconPath("folder", KIconLoader::User);
    it.title = i18n("Standard Folder");
    list->append(it);

    it.path = kil->iconPath("package", KIconLoader::User);
    it.title = i18n("Standard Package");
    list->append(it);

    it.path = kil->iconPath("mini-folder", KIconLoader::User);
    it.title = i18n("Mini Folder");
    list->append(it);

    it.path = kil->iconPath("mini-package", KIconLoader::User);
    it.title = i18n("Mini Package");
    list->append(it);
}

void KIconTemplateContainer::save()
{
  KConfigGroup k(KGlobal::config(), "Templates");

  QStringList names;
  for (Q3ValueListIterator<KIconTemplate> iter = begin(); iter != end(); iter++)
  {
      names.append((*iter).title);
  }

  k.writeEntry("Names",  names);

  for(unsigned int i = 0; i < names.count(); i++)
  {
    k.writePathEntry(names[i], (*this)[i].path);
  }
}

KIconTemplateContainer::KIconTemplateContainer() : Q3ValueList<KIconTemplate>()
{
  QStringList names;
  KConfigGroup k(KGlobal::config(), "Templates");
  k.readEntry("Names", names);
  for(int i = 0; i < (int)names.count(); i++)
  {
    KIconTemplate it;
    it.path = k.readPathEntry(names.at(i), QString());
    it.title = names.at(i);
    // FIXME // kDebug(4640) << "Template: " << names.at(i) << "\n" << path.data();
    append(it);
  }

  if(count() == 0)
  {
    createStandardTemplates(this);
  }
}

KIconTemplateContainer::~KIconTemplateContainer()
{
  clear();
}

KIconListBoxItem::KIconListBoxItem( KIconTemplate t )
   : Q3ListBoxItem(), icontemplate(t)
{
  //kDebug(4640) << "KIconListBoxItem - " << t->path.data() << ", " << t->title.data();

  pm.load(t.path);
  setText( t.title );
}

void KIconListBoxItem::reloadIcon()
{
  pm.load(icontemplate.path);
  setText( icontemplate.title );
}



void KIconListBoxItem::paint( QPainter *p )
{
  p->drawPixmap( 3, 0, pm );
  QFontMetrics fm = p->fontMetrics();
  int yPos;                       // vertical text position
  if ( pm.height() < fm.height() )
    yPos = fm.ascent() + fm.leading()/2;
  else
    yPos = pm.height()/2 - fm.height()/2 + fm.ascent();
  p->drawText( pm.width() + 5, yPos, text() );
}

int KIconListBoxItem::height(const Q3ListBox *lb ) const
{
  return qMax( pm.height(), lb->fontMetrics().lineSpacing() + 1 );
}

int KIconListBoxItem::width(const Q3ListBox *lb ) const
{
  return pm.width() + lb->fontMetrics().width( text() ) + 6;
}

NewSelect::NewSelect(QWidget *parent) : QWidget( parent )
{
  wiz = (K3Wizard*) parent;
  grp = new Q3ButtonGroup( this );
  connect( grp, SIGNAL( clicked( int ) ), SLOT( buttonClicked( int ) ) );
  grp->setExclusive( true );

  QVBoxLayout* ml = new QVBoxLayout( this );
  ml->addWidget( grp, 1 );
  //ml->addWidget(grp, 10, AlignLeft);
  QVBoxLayout* l = new QVBoxLayout( grp );
  l->setSpacing( 10 );

  rbscratch = new QRadioButton( i18n( "Create from scratch" ), grp );
  l->addWidget( rbscratch, 1 );
  //l->addWidget(rbscratch, 5, AlignLeft);

  rbtempl = new QRadioButton( i18n( "Create from template" ), grp );
  l->addWidget( rbtempl, 1 );
  //l->addWidget(rbtempl, 5, AlignLeft);

  //grp->setMinimumSize(grp->childrenRect().size());

  grp->setButton( 0 );
}

NewSelect::~NewSelect()
{
}

void NewSelect::buttonClicked(int id)
{
  //kDebug(4640) << "Button: " << id;

  emit iconopenstyle(id);
}

NewFromTemplate::NewFromTemplate( QWidget* parent )
  : QWidget( parent )
{
  wiz = (K3Wizard*) parent;

  QVBoxLayout* ml = new QVBoxLayout(this);

  grp = new Q3GroupBox( i18n( "Templates" ), this );
  ml->addWidget( grp, 1 );
  //ml->addWidget(grp, 10, AlignLeft);

  QHBoxLayout* l = new QHBoxLayout( grp );
  l->setSpacing( 15 );

  templates = new KIconListBox( grp );
  connect( templates, SIGNAL( highlighted( int ) ), SLOT( checkSelection( int ) ) );
  l->addWidget( templates );

  for( int i = 0; i < (int) KIconTemplateContainer::self()->count(); i++ )
    templates->insertItem( new KIconListBoxItem( *KIconTemplateContainer::self()->at( i ) ) );
}

NewFromTemplate::~NewFromTemplate()
{
}

void NewFromTemplate::checkSelection( int )
{
  //kDebug(4640) << "checkSelection(int) " << templates->currentItem();
  if( templates->currentItem() != -1 )
    wiz->finishButton()->setEnabled( true );
  else
    wiz->finishButton()->setEnabled( false );
}

KNewIcon::KNewIcon( QWidget* parent )
  : K3Wizard( parent, 0, true )
{
  //kDebug(4640) << "KNewIcon";
  setWindowTitle( i18n( "Create New Icon" ) );
  resize( 400, 250 );

  openstyle = 0;

  finishButton()->setEnabled( true );
  cancelButton()->setEnabled( true );
  nextButton()->setEnabled( false );

  select = new NewSelect( this );
  connect( select, SIGNAL( iconopenstyle( int ) ), SLOT( iconOpenStyle( int ) ) );

  scratch = new KResizeWidget( this, QSize( 32, 32 ) );
  // this doesn't accept default valid size, besides spin buttons won't allow
  // an invalid size to be set by the user - forces user to change valid default
  // size to create the new icon object -
  connect( scratch, SIGNAL( validSize( bool ) ), SLOT( checkPage( bool ) ) );
  connect(this, SIGNAL(selected(const QString &)), this, SLOT(checkPage(const QString &)));
  templ = new NewFromTemplate(this);
  templ->hide();

  addPage(select, i18n("Select Icon Type"));
  addPage(scratch, i18n("Create From Scratch"));
  act = scratch;
  //addPage(templ, i18n("Create From Template"));
}

KNewIcon::~KNewIcon()
{
  delete select;
  delete scratch;
  delete templ;
}

void KNewIcon::okClicked()
{
  if(openstyle == Blank)
    emit newicon(scratch->getSize());
  else
    emit newicon(templ->path());
  hide();
  setResult(1);
  accept();
}

void KNewIcon::cancelClicked()
{
  hide();
  setResult(0);
  reject();
}

void KNewIcon::iconOpenStyle(int style)
{
  openstyle = style;

  if( act )
    removePage( act );

  if(openstyle == Blank)
  {
    act = scratch;
    setNextEnabled( act, true );
    addPage( scratch, i18n( "Create From Scratch" ) );
  }
  else if( openstyle == Template )
  {
    act = templ;
    setNextEnabled( act, true );
    addPage( templ, i18n( "Create From Template" ) );
  }
}

void KNewIcon::checkPage( bool b)
{
    //kDebug(4640) << "checkPage(int) " << openstyle << " " << p;
    if(openstyle == Blank)
        finishButton()->setEnabled(true);
    else if( !b )
        finishButton()->setEnabled(false);
    else
        templ->checkSelection(0);
}

void KNewIcon::checkPage(const QString &)
{
  if(currentPage() == select || openstyle == Blank)
    finishButton()->setEnabled(true);
  else
    finishButton()->setEnabled(false);
}
#include "knew.moc"





