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

#include <stdlib.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <kvbox.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <QVBoxLayout>
//Added by qt3to4:
#include <QComboBox>
#include <QPixmap>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>

#include <klocale.h>
#include <kdialogbuttonbox.h>
#include <kcolorbutton.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kurlrequester.h>
#include <kpagewidgetmodel.h>

#include "kiconconfig.h"
#include "kiconedit.h"

#ifndef PICS_INCLUDED
#define PICS_INCLUDED
#include "pics/logo.xpm"
#endif

// little helper:
static inline QPixmap loadIcon( const char * name )
{
  return KIconLoader::global()
    ->loadIcon( QString::fromLatin1(name), KIconLoader::NoGroup, KIconLoader::SizeMedium );
}

KTemplateEditDlg::KTemplateEditDlg(QWidget *parent)
    : KDialog(parent )
{
    setCaption( i18n( "Icon Template" ) );
    setButtons( Ok|Cancel );
    setModal( true );
  QFrame *frame = new QFrame;
  setMainWidget( frame );
  QVBoxLayout *ml = new QVBoxLayout(frame);
  Q3GroupBox *grp = new Q3GroupBox(i18n("Template"), frame);
  grp->setColumnLayout(0, Qt::Vertical);
  grp->layout()->setSpacing(spacingHint());
  grp->layout()->setMargin(marginHint());
  QGridLayout *l = new QGridLayout();
  grp->layout()->addItem( l );

  ln_name = new QLineEdit( grp );
  connect( ln_name, SIGNAL( textChanged( const QString & ) ),
      SLOT( slotTextChanged() ) );
  QLabel* lb_name = new QLabel( ln_name, i18n( "Description:" ), grp );

  ln_path = new KUrlRequester(grp);
  connect( ln_path, SIGNAL( textChanged( const QString & ) ),
      SLOT( slotTextChanged() ) );
  QLabel* lb_path = new QLabel( ln_path, i18n( "Path:" ), grp );

  l->addWidget(lb_name, 0, 0);
  l->addWidget(ln_name, 0, 1);
  l->addWidget(lb_path, 1, 0);
  l->addWidget(ln_path, 1, 1);

  ml->addWidget( grp, 1);

  slotTextChanged();

  setMinimumSize( 400, 150 );
}

void KTemplateEditDlg::slotTextChanged()
{
  QString name = ln_name->text(), path = ln_path->url().path();
  enableButtonOk(!name.isEmpty() && !path.isEmpty());
}

void KTemplateEditDlg::setName(const QString & name)
{
  ln_name->setText(name);
}

QString KTemplateEditDlg::name()
{
  return ln_name->text();
}

void KTemplateEditDlg::setPath(const QString & path)
{
  ln_path->setPath(path);
}

QString KTemplateEditDlg::path()
{
  return ln_path->url().path();
}

KTemplateConfig::KTemplateConfig(QWidget *parent) : QWidget(parent)
{
  kDebug(4640) << "KTemplateConfig constructor";

  btadd = btremove = btedit = 0L;

  Q3GroupBox* grp = new Q3GroupBox( i18n( "Templates" ), this );
  grp->setColumnLayout( 0, Qt::Horizontal );

  templates = new KIconListBox( grp );
  connect( templates, SIGNAL( highlighted( int ) ),
      SLOT( checkSelection( int ) ) );
  connect( templates, SIGNAL(doubleClicked( Q3ListBoxItem * ) ),
      SLOT( edit() ) );

  QVBoxLayout* ml = new QVBoxLayout( this );
  ml->addWidget( grp );

  QVBoxLayout* l = new  QVBoxLayout( );
  grp->layout()->addItem( l );
  l->setSpacing( KDialog::spacingHint() );
  l->addWidget( templates );

  for( int i = 0; i <  KIconTemplateContainer::self()->count(); i++ )
    templates->insertItem( new KIconListBoxItem( *KIconTemplateContainer::self()->at( i ) ) ) ;

  KDialogButtonBox *bbox = new KDialogButtonBox( grp );

  btadd = bbox->addButton( i18n( "&Add..." ), QDialogButtonBox::ActionRole );
  connect( btadd, SIGNAL( clicked() ), SLOT( add() ) );

  btedit = bbox->addButton( i18n( "&Edit..." ), QDialogButtonBox::ActionRole );
  connect( btedit, SIGNAL( clicked() ), SLOT( edit() ) );
  btedit->setEnabled( false );

  btremove = bbox->addButton( i18n( "&Remove" ), QDialogButtonBox::ActionRole );
  connect( btremove, SIGNAL( clicked() ), SLOT( remove() ) );
  btremove->setEnabled( false );

  bbox->layout();
  l->addWidget( bbox );
}

KTemplateConfig::~KTemplateConfig()
{
}

void KTemplateConfig::saveSettings()
{
  kDebug(4640) << "KTemplateConfig::saveSettings";

  KIconTemplateContainer::self()->clear();

  for(int i = 0; i < (int)templates->count(); i++)
  {
      KIconTemplateContainer::self()->append(templates->iconTemplate(i));
  }
  KIconTemplateContainer::self()->save();
  kDebug(4640) << "KTemplateConfig::saveSettings - done";

}

void KTemplateConfig::checkSelection(int)
{
  kDebug(4640) << "KTemplateConfig::checkSelection";
  if(templates->currentItem() != -1)
  {
    if(btremove) btremove->setEnabled(true);
    if(btedit) btedit->setEnabled(true);
  }
  else
  {
    if(btremove) btremove->setEnabled(false);
    if(btedit) btedit->setEnabled(false);
  }
  kDebug(4640) << "KTemplateConfig::checkSelection - done";
}

void KTemplateConfig::remove()
{
  templates->removeItem(templates->currentItem());
}

void KTemplateConfig::add()
{
  KTemplateEditDlg dlg(this);
  if(dlg.exec())
  {
    KIconTemplate it;
    it.path = dlg.path();
    it.title = dlg.name();
    templates->insertItem(new KIconListBoxItem(it));
  }
}

void KTemplateConfig::edit()
{
  KTemplateEditDlg dlg(this);
  dlg.setPath(templates->path(templates->currentItem()));
  dlg.setName(templates->text(templates->currentItem()));
  templates->item(templates->currentItem());
  if(dlg.exec())
  {
    //Edit the entry
    KIconTemplate &it=templates->iconTemplate(templates->currentItem());
    it.path = dlg.path();
    it.title = dlg.name();
    static_cast<KIconListBoxItem*>(templates->item(templates->currentItem()))->reloadIcon();
    templates->update();
  }
}

KBackgroundConfig::KBackgroundConfig( QWidget* parent )
  : QWidget( parent )
{
  kDebug(4640) << "KBackgroundConfig - constructor";

  lb_ex = 0L;

  KIconEditProperties *props = KIconEditProperties::self();

  pixpath = props->bgPixmap();
  pix.load(pixpath);
  if(pix.isNull())
  {
    kDebug(4640) << "BGPIX: " << pixpath << " not valid!";
    QPixmap pmlogo((const char**)logo);
    pix = pmlogo;
  }

  QVBoxLayout *mainLayout = new QVBoxLayout( this );

  Q3GroupBox *grp1 = new Q3GroupBox( i18n( "Select Background" ), this );
  grp1->setColumnLayout(0, Qt::Vertical );
  grp1->layout()->setSpacing( KDialog::spacingHint() );
  grp1->layout()->setMargin( KDialog::marginHint() );
  mainLayout->addWidget( grp1 );

  QGridLayout *grp1Layout = new QGridLayout( );
  grp1->layout()->addItem( grp1Layout );

  Q3ButtonGroup* btngrp = new Q3ButtonGroup( grp1 );
  btngrp->setExclusive( true );
#ifdef __GNUC__
#warning Find how to port missing Q3ButtonGroup::setFrameStyle
#endif  
  //btngrp->setFrameStyle( QFrame::NoFrame );
  connect( btngrp, SIGNAL( clicked( int ) ), SLOT( slotBackgroundMode( int ) ) );
  grp1Layout->addWidget( btngrp, 0, 0 );

  QVBoxLayout *bgl = new QVBoxLayout( btngrp );
  bgl->setSpacing( 5 );

  QRadioButton *rbc = new QRadioButton( i18n( "Use co&lor" ), btngrp );
  btngrp->insert( rbc, 0 );
  bgl->addWidget( rbc );

  QRadioButton *rbp = new QRadioButton( i18n( "Use pix&map" ), btngrp );
  btngrp->insert( rbp, 1 );
  bgl->addWidget( rbp );

  bgl->addStretch( 1 );

  KVBox *bbox = new KVBox( grp1 );
  grp1Layout->addWidget( bbox, 0, 1 );

  btcolor = new KColorButton(props->bgColor(), bbox) ;
  connect(btcolor, SIGNAL(changed(const QColor &)),
      SLOT( selectColor(const QColor &)));

  btpix = new QPushButton(i18n( "Choose..." ), bbox);
  connect( btpix, SIGNAL( clicked() ), SLOT( selectPixmap() ) );

  Q3GroupBox *grp2 = new Q3GroupBox( i18n( "Preview" ), this );
  mainLayout->addWidget( grp2, 1 );

  QBoxLayout *l2 = new QVBoxLayout( grp2 );
  l2->setSpacing( 15 );

  l2->addSpacing( 10 );

  lb_ex = new QLabel( grp2 );
  lb_ex->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  l2->addWidget( lb_ex );

/*
  l1->addWidget( btngrp, 0, AlignLeft );
  l1->addLayout( l1r );
*/
  bgmode = props->bgMode();

  if( bgmode == Qt::FixedPixmap )
  {
    btngrp->setButton( 1 );
    btcolor->setEnabled( false );
    lb_ex->setBackgroundPixmap( pix );
  }
  else
  {
    btngrp->setButton( 0 );
    btpix->setEnabled( false );
    lb_ex->setBackgroundColor(btcolor->color());
  }
}

KBackgroundConfig::~KBackgroundConfig()
{
}

void KBackgroundConfig::slotBackgroundMode(int id)
{
  if(id == 0)
  {
    bgmode = Qt::FixedColor;
    btpix->setEnabled(false);
    btcolor->setEnabled(true);
    if(lb_ex)
      lb_ex->setBackgroundColor(btcolor->color());
  }
  else
  {
    bgmode = Qt::FixedPixmap;
    btpix->setEnabled(true);
    btcolor->setEnabled(false);
    if(lb_ex)
      lb_ex->setBackgroundPixmap(pix);
  }
}

void KBackgroundConfig::saveSettings()
{
  kDebug(4640) << "KBackgroundConfig::saveSettings";
  KIconEditProperties *props = KIconEditProperties::self();
  props->setBgMode( bgmode );
  props->setBgPixmap( pixpath );
  props->setBgColor( btcolor->color() );
  kDebug(4640) << "KBackgroundConfig::saveSettings - done";
}

void KBackgroundConfig::selectColor(const QColor & newColor)
{
  lb_ex->setBackgroundColor(newColor);
}

void KBackgroundConfig::selectPixmap()
{
  //  KUrl url = KFileDialog::getOpenUrl("/", "*.xpm");
  KUrl url = KFileDialog::getImageOpenUrl(KUrl(QLatin1String("/")));

  if( url.isEmpty() )
    return;

  if( !url.isLocalFile() )
  {
    KMessageBox::sorry( 0L, i18n( "Only local files are supported yet." ) );
    return;
  }

  QPixmap p(url.path());

  if( !p.isNull() )
  {
    lb_ex->setBackgroundPixmap( p );
    pixpath = url.path();
  }
}

KMiscConfig::KMiscConfig(QWidget *parent) : QWidget(parent)
{
  kDebug(4640) << "KMiscConfig - constructor";

  KIconEditProperties* props = KIconEditProperties::self();

  QBoxLayout *ml = new QVBoxLayout( this );
  ml->setSpacing( 5 );

  QCheckBox *cbp = new QCheckBox( i18n( "Paste &transparent pixels" ), this );
  connect( cbp, SIGNAL( toggled( bool ) ), SLOT( pasteMode( bool ) ) );
  ml->addWidget(cbp);

  QCheckBox *cbr = new QCheckBox( i18n( "Show &rulers" ), this );
  connect( cbr, SIGNAL( toggled( bool ) ), SLOT( showRulers( bool ) ) );
  ml->addWidget(cbr);

  Q3ButtonGroup* btngrp = new Q3ButtonGroup( i18n( "Transparency Display" ), this);
  btngrp->setExclusive( true );
  connect( btngrp, SIGNAL( clicked( int ) ), SLOT( slotTransparencyDisplayType( int ) ) );
  ml->addWidget( btngrp );

  QVBoxLayout *tgl = new QVBoxLayout( btngrp );
  tgl->setMargin( KDialog::marginHint() );
  tgl->setSpacing( KDialog::spacingHint() );
  tgl->insertSpacing(0, 10);

  QHBoxLayout *hl = new QHBoxLayout();
  tgl->addItem( hl );

  QRadioButton *solidColorRButton = new QRadioButton( i18n( "&Solid color:" ), btngrp );
  btngrp->insert( solidColorRButton, 0 );
  hl->addWidget( solidColorRButton );

  m_solidColorButton = new KColorButton(props->transparencySolidColor(), btngrp);
  btngrp->insert( m_solidColorButton, 2 );
  hl->addWidget(m_solidColorButton);
  //connect(btcolor, SIGNAL(changed(const QColor &)),
    //  SLOT( selectColor(const QColor &)));

  QRadioButton *checkerboardRButton = new QRadioButton( i18n( "Checker&board" ), btngrp );
  btngrp->insert( checkerboardRButton, 1 );
  tgl->addWidget( checkerboardRButton );

  QGridLayout *grid = new QGridLayout();
  tgl->addItem( grid );
  grid->setSpacing(40);
  grid->setColumnStretch(1, 1);
  grid->setColumnStretch(2, 1);

  m_checkerboardSizeCombo = new QComboBox(btngrp);
  m_checkerboardSizeCombo->addItem( i18n( "Small" ) );
  m_checkerboardSizeCombo->addItem( i18n( "Medium" ) );
  m_checkerboardSizeCombo->addItem( i18n( "Large" ) );
  m_checkerboardSizeCombo->setCurrentIndex(props->checkerboardSize());

  QLabel *label = new QLabel(m_checkerboardSizeCombo, i18n("Si&ze:"), btngrp);

  grid->addWidget(label, 0, 1);
  grid->addWidget(m_checkerboardSizeCombo, 0, 2);

  m_checkerboardColor1Button = new KColorButton(props->checkerboardColor1(), btngrp);
  label = new QLabel(m_checkerboardColor1Button, i18n("Color &1:"), btngrp);

  grid->addWidget(label, 1, 1);
  grid->addWidget(m_checkerboardColor1Button, 1, 2);

  m_checkerboardColor2Button = new KColorButton(props->checkerboardColor2(), btngrp);
  label = new QLabel(m_checkerboardColor2Button, i18n("Color &2:"), btngrp);

  grid->addWidget(label, 2, 1);
  grid->addWidget(m_checkerboardColor2Button, 2, 2);

  if(props->transparencyDisplayType() == KIconEditGrid::TRD_CHECKERBOARD)
  {
    checkerboardRButton->setChecked(true);
    m_checkerboardColor1Button->setEnabled(true);
    m_checkerboardColor2Button->setEnabled(true);
    m_checkerboardSizeCombo->setEnabled(true);

    solidColorRButton->setChecked(false);
    m_solidColorButton->setEnabled(false);
  }
  else
  {
    checkerboardRButton->setChecked(false);
    m_checkerboardColor1Button->setEnabled(false);
    m_checkerboardColor2Button->setEnabled(false);
    m_checkerboardSizeCombo->setEnabled(false);

    solidColorRButton->setChecked(true);
    m_solidColorButton->setEnabled(true);
  }

  ml->addStretch(1);

  cbp->setChecked( props->pasteTransparent() );
  cbr->setChecked( props->showRulers() );
}

KMiscConfig::~KMiscConfig()
{

}

void KMiscConfig::saveSettings()
{
  kDebug(4640) << "KMiscConfig::saveSettings";
  KIconEditProperties* props =  KIconEditProperties::self();
  props->setPasteTransparent( pastemode );
  props->setShowRulers( showrulers );
  if(m_solidColorButton->isEnabled())
  {
    props->setTransparencyDisplayType(KIconEditGrid::TRD_SOLIDCOLOR);
    props->setTransparencySolidColor(m_solidColorButton->color());
  }
  else
  {
    props->setTransparencyDisplayType(KIconEditGrid::TRD_CHECKERBOARD);
    props->setCheckerboardColor1(m_checkerboardColor1Button->color());
    props->setCheckerboardColor2(m_checkerboardColor2Button->color());
    props->setCheckerboardSize((KIconEditGrid::CheckerboardSize)m_checkerboardSizeCombo->currentIndex());
  }
}

void KMiscConfig::pasteMode(bool mode)
{
  pastemode = mode;
}

void KMiscConfig::showRulers(bool mode)
{
  showrulers = mode;
}

void KMiscConfig::slotTransparencyDisplayType(int id)
{
  if(id == 0)
  {
    m_checkerboardColor1Button->setEnabled(false);
    m_checkerboardColor2Button->setEnabled(false);
    m_checkerboardSizeCombo->setEnabled(false);

    m_solidColorButton->setEnabled(true);
  }
  else
  if(id == 1)
  {
    m_checkerboardColor1Button->setEnabled(true);
    m_checkerboardColor2Button->setEnabled(true);
    m_checkerboardSizeCombo->setEnabled(true);

    m_solidColorButton->setEnabled(false);
  }
}

KIconConfig::KIconConfig(QWidget *parent)
  : KPageDialog( parent)
{
  setHelp(QString::null);
  setButtons( KDialog::Help|KDialog::Ok|KDialog::Apply|KDialog::Cancel );
  setDefaultButton( KDialog::Ok );
  setModal( true );
  setCaption( i18n("Configure") );
  setFaceType( KPageDialog::List );
  //KWin::setIcons(winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),IconSize(KIconLoader::Small)));
  connect(this, SIGNAL(finished()), this, SLOT(finis()));

  KVBox*page = new KVBox(this);
  KPageWidgetItem *pageItem = new KPageWidgetItem( page, i18n("Icon Templates") );
  pageItem->setIcon( KIcon("preferences-desktop-icons") );
  addPage( pageItem );
  temps = new KTemplateConfig(page);

  page = new KVBox( this );
  pageItem = new KPageWidgetItem( page, i18n("Background") );
  pageItem->setIcon( KIcon(loadIcon("preferences-other")) );
  addPage( pageItem );
  backs = new KBackgroundConfig(page);

  page = new KVBox( this );
  pageItem = new KPageWidgetItem( page,i18n("Icon Grid")  );
  pageItem->setIcon( KIcon(loadIcon("kiconedit")) );
  addPage( pageItem );
  misc = new KMiscConfig(page);

  QSize min(300, 400);

  if (300 < sizeHint().width()) { min.setWidth(sizeHint().width()); }
  if (400 < sizeHint().height()) { min.setHeight(sizeHint().height()); }

  resize(min);
  connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
  connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

KIconConfig::~KIconConfig()
{
  //delete dict;
}

void KIconConfig::slotApply()
{
  kDebug(4640) << "KIconEditConfig::saveSettings";

  temps->saveSettings();
  backs->saveSettings();
  misc->saveSettings();

  for (KIconEdit* window = KIconEdit::windowList.first();
         window;
         window = KIconEdit::windowList.next())
  {
    window->updateProperties();
  }
}

void KIconConfig::slotOk()
{
    slotApply();
    accept();
}

void KIconConfig::finis()
{
    delayedDestruct();
}

#include "kiconconfig.moc"
