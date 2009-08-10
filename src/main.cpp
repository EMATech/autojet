/*
    Copyright (C) 2008-2009 Raphaël Doursenaud <rdoursenaud@free.fr>
    Copyright (C) 2006 David Weisgerber <weisgerber@mytum.de>

    This file is part of autojet.

    autojet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    autojet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with autojet.  If not, see <http://www.gnu.org/licenses/>.

The grass withers and the flowers fall, but the word of our God stands for ever.
Isaiah 40, 8
*/

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QLocale>
#include "autojet.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

    QString locale = QLocale::system().name();

    QTranslator qtTranslator;
    QTranslator autojetTranslator;

    qtTranslator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    autojetTranslator.load(QString("autojet_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&autojetTranslator);

    autojet * mw = new autojet();
    mw->show();
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
