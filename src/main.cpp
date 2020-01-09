/*
    KDE Draw - a small graphics drawing program for the KDE.
    Copyright (C) 1998  Thomas Tanghus (tanghus@kde.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// A lot of this code is lifted from KMail. Thanks, guys!

#include <stdlib.h>

#include <klocale.h>
#include <kconfig.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdeversion.h>
#include <kdebug.h>

#include "kiconedit.h"
#include "properties.h"

static const char description[] =
	I18N_NOOP("KDE Icon Editor");

int main(int argc, char **argv)
{
  KAboutData aboutData( "kiconedit", 0, ki18n("KIconEdit"),
    KDE_VERSION_STRING, ki18n(description), KAboutData::License_GPL,
    ki18n("(c) 1998, Thomas Tanghus"));

  aboutData.addAuthor(ki18n("Thomas Tanghus"),KLocalizedString(), "tanghus@kde.org");
  aboutData.addAuthor(ki18n("John Califf"),KLocalizedString(), "jcaliff@compuzone.net");
  aboutData.addAuthor(ki18n("Laurent Montel"),KLocalizedString(), "montel@kde.org");
  aboutData.addAuthor(ki18n("Aaron J. Seigo"),KLocalizedString(), "aseigo@olympusproject.org");
  aboutData.addCredit( ki18n("Nadeem Hasan"), ki18n("Rewrote UI to use XMLGUI\n"
      "Lots of fixes and cleanup"), "nhasan@nadmm.com" );
  aboutData.addCredit( ki18n("Adrian Page"), ki18n("Bug fixes and GUI tidy up"),
      "Adrian.Page@tesco.net" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add("+[file(s)]", ki18n("Icon file(s) to open"));
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication a;

//  setSignalHandler(signalHandler);

  if (a.isSessionRestored())
  {
    RESTORE(KIconEdit);
  }
  else
  {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for(int i = 0; i < args->count(); i++)
    {
        KIconEdit *ki = new KIconEdit(args->url(i), "kiconedit");
        Q_CHECK_PTR(ki);
    }

    if (args->count() == 0)
    {
        KIconEdit *ki = new KIconEdit;
        Q_CHECK_PTR(ki);
    }
    args->clear();
  }

  int rc = a.exec();
  delete KIconEditProperties::self();
  return rc;
}

