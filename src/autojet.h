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

/* Colour code for the background of the progress bar when burning has finished:
	R: 23
	G: 178
	B: 109
*/

/* Colour code for the background of the progress bar when burning finished with
	errors:
	R: 249
	G: 0
	B: 0
*/

#ifndef AUTOJET_H
#define AUTOJET_H

class autojet;

#ifndef AUTOJET_LOG
#define AUTOJET_LOG "autojet.log"
#endif

#include "ui_mainwindow.h"
#include "cdwriter.h"
#include "configuration.h"
#include "create.h"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFile>
#include <QVector>
#include <QSettings>
#include <QTextEdit>

class autojet: public QMainWindow
{
    Q_OBJECT

public:
	autojet( QWidget *parent = 0, Qt::WFlags flags = 0 );
	~autojet( void ) {}

	void addLog( QString message );

protected:
	void resizeEvent ( QResizeEvent * );
	void keyReleaseEvent ( QKeyEvent * event );

private slots:
	void buildView( void );
	void halt( void );
	void help( void );
	void eject( void );
	void create( void );
	void blank( void );
	void imagefile( void );
	void burn( void );
	void showConfig( void );
	void saveNotes(void );
	void changeView( void );

private:
	bool cuefile;

	Ui::MainWindow ui;
	QFile *logFile;
	QVector<CDWriter *> burnersList;
	QSettings *settings;
	Configuration *configDialog;
	Create *createDialog;
	
	int maximized; // Maximized burner item

	QString imagedir;
};


#endif
