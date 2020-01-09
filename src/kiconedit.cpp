/*
    kiconedit - a small graphics drawing program for the KDE
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



//Added by qt3to4:
#include <QPixmap>
#include <QtXml/qdom.h>

#include <kmenu.h>
#include <kstandardaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kicon.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include "kiconedit.h"
#include "palettetoolbar.h"

WindowList KIconEdit::windowList;

KIconEdit::KIconEdit(const QImage image, const char *name)
 : KXmlGuiWindow()
{
    setObjectName(QLatin1String(name));
    init();
    img = image;
    img.detach();
    grid->load(&img);
    grid->setModified(true);
}

KIconEdit::KIconEdit(KUrl url, const char *name)
 : KXmlGuiWindow()
{
    setObjectName(QLatin1String(name));
    init();
    icon->open(&img, url);
}

void KIconEdit::init()
{
  setMinimumSize( 600, 540 );

  windowList.append(this);
  setCaption(KGlobal::caption());

  m_paletteToolBar = 0L;
  statusbar = 0L;

  history = new K3CommandHistory( actionCollection(), true );

  gridview = new KGridView(&img, history, this);
  grid = gridview->grid();
  icon = new KIconEditIcon(this, &grid->image());

  setAcceptDrops(true);

  setupActions();
  setupStatusBar();
  setStandardToolBarMenuEnabled( true );

  createGUI();

  connect(this, SIGNAL(newname(const QString &)),
           SLOT( slotUpdateStatusName(const QString &)));

  connect( icon, SIGNAL( saved()), SLOT(slotSaved()));
  connect( icon, SIGNAL( loaded(QImage *)), grid, SLOT(load(QImage *)));
  connect( icon, SIGNAL(opennewwin(const QString &)),
           SLOT(slotNewWin(const QString &)));
  connect(icon, SIGNAL(newname(const QString &)),
           SLOT( slotUpdateStatusName(const QString &)));
  connect(icon, SIGNAL(newmessage(const QString &)),
           SLOT( slotUpdateStatusMessage(const QString &)));
  connect(icon, SIGNAL(addrecent(const QString &)),
           SLOT( addRecent(const QString &)));

  connect( m_paletteToolBar, SIGNAL( newColor(uint)),
     grid, SLOT(setColorSelection(uint)));

  connect( grid, SIGNAL( changed(const QPixmap &) ),
      m_paletteToolBar, SLOT( previewChanged(const QPixmap &) ) );
  connect( grid, SIGNAL( addingcolor(uint) ),
      m_paletteToolBar, SLOT(addColor(uint)));
  connect( grid, SIGNAL( colorschanged(uint, uint*) ),
      m_paletteToolBar, SLOT(addColors(uint, uint*)));

  connect(grid, SIGNAL(sizechanged(int, int)),
           SLOT( slotUpdateStatusSize(int, int)));
  connect(grid, SIGNAL(poschanged(int, int)),
           SLOT( slotUpdateStatusPos(int, int)));
  connect(grid, SIGNAL(scalingchanged(int)),
           SLOT( slotUpdateStatusScaling(int)));
  connect(grid, SIGNAL(scalingchanged(int)),
           SLOT( slotUpdateZoom(int)));
  connect( grid, SIGNAL( addingcolor(uint) ),
           SLOT(slotUpdateStatusColors(uint)));
  connect(grid, SIGNAL(colorschanged(uint, uint*)),
           SLOT( slotUpdateStatusColors(uint, uint*)));
  connect(grid, SIGNAL(newmessage(const QString &)),
           SLOT( slotUpdateStatusMessage(const QString &)));
  connect(grid, SIGNAL(clipboarddata(bool)),
           SLOT( slotUpdatePaste(bool)));
  connect(grid, SIGNAL(colorSelected(uint)),
           m_paletteToolBar, SLOT(currentColorChanged(uint)));
  connect(grid, SIGNAL(modifiedchanged(bool)),
           SLOT( slotUpdateStatusModified(bool)));
  gridview->checkClipboard(); //Not very elegant, but fixes a buglet

  connect(grid, SIGNAL(selecteddata(bool)), SLOT( slotUpdateCopy(bool)));

  kDebug(4640) << "Updating statusbar";
  slotUpdateStatusSize(grid->cols(), grid->rows());
  slotUpdateStatusScaling(grid->scaling());

  slotUpdateZoom( grid->scaling() );

  if(icon->url().length())
    slotUpdateStatusName(icon->url());
  else
    slotUpdateStatusName(i18n("Untitled"));

  slotUpdateCopy(false);

  uint *c = 0, n = 0;
  n = grid->getColors(c);
  slotUpdateStatusColors(n, c);

  setCentralWidget(gridview);

  applyMainWindowSettings( KGlobal::config()->group( "MainWindowSettings" ) );
  updateProperties();

  updateAccels();
  show();
  // FIXME //moveDockWindow( m_paletteToolBar, Qt::DockRight, true, 0 );
}

KIconEdit::~KIconEdit()
{
    windowList.remove(this);

    if (windowList.count() < 1)
    {
        kapp->quit();
    }
}

bool KIconEdit::queryClose()
{
    bool cancel = false;
    if (grid->isModified())
    {
        int r = KMessageBox::warningYesNoCancel(this,
	    i18n("The current file has been modified.\nDo you want to save it?"), QString::null, KStandardGuiItem::save(), KStandardGuiItem::discard());

        switch(r)
        {
            case KMessageBox::Yes:
                if (!icon->save(&grid->image()))
                {
                    cancel = true;
                }
                break;
            case KMessageBox::Cancel:
                cancel = true;
                break;
            case KMessageBox::No:
            default:
                break;
        }
    }

    if(!cancel)
    {
        writeConfig();
    }
    return (!cancel);
}

// this is for exit by request of the session manager
void KIconEdit::saveProperties(KConfigGroup &config )
{
    kDebug(4640) << "KIconEdit::saveProperties";

    config.writePathEntry("Name", icon->url());
}

// this is for instances opened by the session manager
void KIconEdit::readProperties(const KConfigGroup &config)
{
    kDebug(4640) << "KIconEdit::readProperties";

    QString entry = config.readPathEntry("Name", QString()); // no default
    if (entry.isEmpty())  return;
    icon->open(&grid->image(), KUrl( entry ));
}

/*
    this is for normal exits or request from "Options->Save options".
*/
void KIconEdit::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    m_actRecent->saveEntries( KGlobal::config()->group( QString() ) );

    KIconEditProperties::self()->save();

    saveMainWindowSettings( config->group( "MainWindowSettings" ) );
}

QSize KIconEdit::sizeHint() const
{
    if(gridview)
        return gridview->sizeHint();
    else
        return QSize(-1, -1);
}

void KIconEdit::setupActions()
{
  kDebug(4640) << "setupActions";

  QAction *action;
  KToggleAction *toolAction;
  KShortcut cut;

  // File Menu
  action = actionCollection()->addAction("file_new_window");
  action->setIcon(KIcon("window-new"));
  action->setText(i18n("New &Window"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotNewWin()));
  action->setShortcuts(cut);
  action->setWhatsThis(i18n("New window\n\nOpens a new icon editor window."));

  action = KStandardAction::openNew(this, SLOT(slotNew()), actionCollection());
  action->setWhatsThis(i18n("New\n\nCreate a new icon, either from a"
      " template or by specifying the size"));

  action = KStandardAction::open(this, SLOT(slotOpen()), actionCollection());
  action->setWhatsThis(i18n("Open\n\nOpen an existing icon"));

  m_actRecent = KStandardAction::openRecent(this,
      SLOT(slotOpenRecent(const KUrl&)), actionCollection());
  m_actRecent->setMaxItems(15); // FIXME should be configurable!
  m_actRecent->loadEntries(KGlobal::config()->group( QString() ) );

  action = KStandardAction::save(this, SLOT(slotSave()), actionCollection());
  action->setWhatsThis(i18n("Save\n\nSave the current icon"));

  KStandardAction::saveAs(this, SLOT(slotSaveAs()), actionCollection());

  action = KStandardAction::print(this, SLOT(slotPrint()), actionCollection());
  action->setWhatsThis(i18n("Print\n\nOpens a print dialog to let you print"
      " the current icon."));

  KStandardAction::close(this, SLOT(slotClose()), actionCollection());

  // Edit Menu

  m_actCut = KStandardAction::cut(this, SLOT(slotCut()), actionCollection());
  m_actCut->setWhatsThis(i18n("Cut\n\nCut the current selection out of the"
      " icon.\n\n(Tip: You can make both rectangular and circular selections)"));

  m_actCopy = KStandardAction::copy(this, SLOT(slotCopy()), actionCollection());
  m_actCopy->setWhatsThis(i18n("Copy\n\nCopy the current selection out of the"
      " icon.\n\n(Tip: You can make both rectangular and circular selections)"));

  m_actPaste = KStandardAction::paste(this, SLOT(slotPaste()), actionCollection());
  m_actPaste->setWhatsThis(i18n("Paste\n\n"
       "Paste the contents of the clipboard into the current icon.\n\n"
       "If the contents are larger than the current icon you can paste them"
       " in a new window.\n\n"
       "(Tip: Select \"Paste transparent pixels\" in the configuration dialog"
       " if you also want to paste transparency.)"));

  m_actPasteNew = actionCollection()->addAction( "edit_paste_as_new" );
  m_actPasteNew->setText( i18n( "Paste as &New" ) );
  connect(m_actPasteNew, SIGNAL(triggered(bool) ), grid, SLOT( editPasteAsNew() ));
  m_actPasteNew->setShortcuts(cut);

  KStandardAction::clear(this, SLOT(slotClear()), actionCollection());
  KStandardAction::selectAll(this, SLOT(slotSelectAll()), actionCollection());

  action = actionCollection()->addAction("edit_resize");
  action->setIcon(KIcon("transform-scale"));
  action->setText(i18n("Resi&ze..."));
  connect(action, SIGNAL(triggered(bool)), grid, SLOT(editResize()));
  action->setShortcuts(cut);
  action->setWhatsThis(i18n("Resize\n\nSmoothly resizes the icon while"
      " trying to preserve the contents"));

  action = actionCollection()->addAction("edit_grayscale");
  action->setIcon(KIcon("grayscale"));
  action->setText(i18n("&GrayScale"));
  connect(action, SIGNAL(triggered(bool)), grid, SLOT(grayScale()));
  action->setShortcuts(cut);
  action->setWhatsThis(i18n("Gray scale\n\nGray scale the current icon.\n"
      "(Warning: The result is likely to contain colors not in the icon"
      " palette"));

  // View Menu
  m_actZoomIn = KStandardAction::zoomIn(this, SLOT(slotZoomIn()),
      actionCollection());
  m_actZoomIn->setWhatsThis(i18n("Zoom in\n\nZoom in by one."));

  m_actZoomOut = KStandardAction::zoomOut(this, SLOT(slotZoomOut()),
      actionCollection());
  m_actZoomOut->setWhatsThis(i18n("Zoom out\n\nZoom out by one."));

  KActionMenu *actMenu = actionCollection()->add<KActionMenu>( "view_zoom" );
  actMenu->setIcon( KIcon("zoom-original") );
  actMenu->setText( i18n( "&Zoom" ) );

  // xgettext:no-c-format
  action = actionCollection()->addAction( "view_zoom_1" );
  action->setText( i18n( "100%" ) );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotZoom1() ));
  action->setShortcuts(cut);
  actMenu->addAction( action );
  // xgettext:no-c-format
  action = actionCollection()->addAction( "view_zoom_2" );
  action->setText( i18n( "200%" ) );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotZoom2() ));
  action->setShortcuts(cut);
  actMenu->addAction( action );
  // xgettext:no-c-format
  action = actionCollection()->addAction( "view_zoom_5" );
  action->setText( i18n( "500%" ) );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotZoom5() ));
  action->setShortcuts(cut);
  actMenu->addAction( action );
  // xgettext:no-c-format
  action = actionCollection()->addAction( "view_zoom_10" );
  action->setText( i18n( "1000%" ) );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotZoom10() ));
  action->setShortcuts(cut);
  actMenu->addAction( action );

  // Settings Menu
  KStandardAction::keyBindings(this, SLOT(slotConfigureKeys()),
      actionCollection());
  KStandardAction::preferences(this, SLOT(slotConfigureSettings()),
      actionCollection());

  createStandardStatusBarAction();

  QActionGroup *toolActionsGrp;
  toolActionsGrp = new QActionGroup((QAction*)this);

  KToggleAction *toggle;

  toggle = actionCollection()->add<KToggleAction>( "options_show_grid" );
  toggle->setIcon( KIcon("show-grid") );
  toggle->setText( i18n( "Show &Grid" ) );
  connect(toggle, SIGNAL(triggered(bool) ), SLOT( slotShowGrid() ));
  toggle->setShortcuts(cut);
  toggle->setCheckedState(KGuiItem(i18n("Hide &Grid")));
  toggle->setWhatsThis( i18n( "Show grid\n\nToggles the grid in the icon"
      " edit grid on/off" ) );
  toggle->setChecked( KIconEditProperties::self()->showGrid() );

  // Tools Menu
  toolAction = actionCollection()->add<KToggleAction>( "tool_find_pixel" );
  toolAction->setIcon( KIcon("color-picker") );
  toolAction->setText( i18n("Color Picker") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolPointer()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Color Picker\n\nThe color of the pixel clicked"
      " on will be the current draw color"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_freehand" );
  toolAction->setIcon( KIcon("draw-freehand") );
  toolAction->setText( i18n("Freehand") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolFreehand()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Free hand\n\nDraw non-linear lines"));

  toolAction->setChecked( true );
  grid->setTool(KIconEditGrid::Freehand);

  toolAction = actionCollection()->add<KToggleAction>( "tool_rectangle" );
  toolAction->setIcon( KIcon("draw-rectangle") );
  toolAction->setText( i18n("Rectangle") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolRectangle()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Rectangle\n\nDraw a rectangle"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_filled_rectangle" );
  toolAction->setIcon( KIcon("draw-rectangle-filled") );
  toolAction->setText( i18n("Filled Rectangle") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolFilledRectangle()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Filled rectangle\n\nDraw a filled rectangle"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_circle" );
  toolAction->setIcon( KIcon("draw-circle") );
  toolAction->setText( i18n("Circle") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolCircle()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Circle\n\nDraw a circle"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_filled_circle" );
  toolAction->setIcon( KIcon("draw-circle-filled") );
  toolAction->setText( i18n("Filled Circle") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolFilledCircle()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Filled circle\n\nDraw a filled circle"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_ellipse" );
  toolAction->setIcon( KIcon("draw-ellipse") );
  toolAction->setText( i18n("Ellipse") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolEllipse()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Ellipse\n\nDraw an ellipse"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_filled_ellipse" );
  toolAction->setIcon( KIcon("draw-ellipse-filled") );
  toolAction->setText( i18n("Filled Ellipse") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolFilledEllipse()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Filled ellipse\n\nDraw a filled ellipse"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_spray" );
  toolAction->setIcon( KIcon("draw-airbrush") );
  toolAction->setText( i18n("Spray") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolSpray()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Spray\n\nDraw scattered pixels in the"
      " current color"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_flood_fill" );
  toolAction->setIcon( KIcon("fill-color") );
  toolAction->setText( i18n("Flood Fill") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolFlood()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Flood fill\n\nFill adjoining pixels with"
      " the same color with the current color"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_line" );
  toolAction->setIcon( KIcon("draw-line") );
  toolAction->setText( i18n("Line") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolLine()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Line\n\nDraw a straight line vertically,"
      " horizontally or at 45 deg. angles"));

  toolAction = actionCollection()->add<KToggleAction>( "tool_eraser" );
  toolAction->setIcon( KIcon("draw-eraser") );
  toolAction->setText( i18n("Eraser (Transparent)") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolEraser()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Erase\n\nErase pixels. Set the pixels to"
      " be transparent\n\n(Tip: If you want to draw transparency with a"
      " different tool, first click on \"Erase\" then on the tool you want"
      " to use)"));

  toolAction = actionCollection()->add<KToggleAction>( "edit_select_rectangle" );
  toolAction->setIcon( KIcon("select-rectangular") );
  toolAction->setText( i18n("Rectangular Selection") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolSelectRect()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Select\n\nSelect a rectangular section"
      " of the icon using the mouse."));

  toolAction = actionCollection()->add<KToggleAction>( "edit_select_circle" );
  toolAction->setIcon( KIcon("select-circle") );
  toolAction->setText( i18n("Circular Selection") );
  connect(toolAction, SIGNAL(triggered(bool) ), SLOT(slotToolSelectCircle()));
  toolAction->setShortcuts(cut);
  toolAction->setActionGroup(toolActionsGrp);
  toolAction->setWhatsThis(i18n("Select\n\nSelect a circular section of the"
      " icon using the mouse."));
}

void KIconEdit::updateAccels()
{
    // FIXME //actionCollection()->readShortcutSettings();
    actionCollection()->readSettings();
}

QWidget *KIconEdit::createContainer( QWidget *parent, int index,
        const QDomElement &element, QAction* &containerAction )
{
    if ( element.attribute( "name" ) == "paletteToolBar" )
    {
        m_paletteToolBar = new PaletteToolBar( this );
        m_paletteToolBar->setObjectName( "paletteToolBar" );
        // FIXME //m_paletteToolBar->setText( i18n( "Palette Toolbar" ) );
        return m_paletteToolBar;
    }

    return KXMLGUIBuilder::createContainer( parent, index, element, containerAction );
}

void KIconEdit::setupStatusBar()
{
    statusbar = statusBar();

    QString str = i18n("Statusbar\n\nThe statusbar gives information on"
        " the status of the current icon. The fields are:\n\n"
        "\t- Application messages\n\t- Cursor position\n\t- Size\n\t- Zoom factor\n"
        "\t- Number of colors");
    statusbar->setWhatsThis( str);

    statusbar->insertFixedItem("99999,99999", 0);
    statusbar->insertFixedItem("99999 x 99999", 1);
    statusbar->insertFixedItem(" 1:999", 2);
    str = i18n("Colors: %1", 9999999);
    statusbar->insertFixedItem(str, 3);
    statusbar->insertItem("", 4);

    statusbar->changeItem( "", 0);
    statusbar->changeItem( "", 1);
    statusbar->changeItem( "", 2);
    statusbar->changeItem( "", 3);
}

void KIconEdit::addRecent(const QString & path)
{
  m_actRecent->addUrl(KUrl( path ));
}

#include "kiconedit.moc"
