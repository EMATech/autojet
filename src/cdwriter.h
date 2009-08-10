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

#ifndef CDWRITER_H
#define CDWRITER_H

class CDWriter;

#include <QFrame>
#include <QSettings>
#include <QProcess>
#include "ui_minimum.h"
#include "ui_maximum.h"
#include "autojet.h"

class CDWriter : public QFrame
{
	Q_OBJECT
public:
	CDWriter( autojet *parent, QSettings *settings, int number);

	~CDWriter( void );
	bool isIdle() { return idle; }

public slots:
	void eject( void );
	void create( void );
	void blank( void );
	void burn( void );
	void imagefile( QString file );
	void minimize( void );
	void maximize( void );

protected:
	void mouseReleaseEvent( QMouseEvent * event );

signals:
	void clicked( void );

private slots:
	void finished( int exitCode, QProcess::ExitStatus exitStatus );
	void finishedInfo( int, QProcess::ExitStatus exitStatus );
	void readLine( void );
	void readLineInfo( void );
	void select( void );

private:
	void append( QString message );
	void setIdle( bool isIdle );
	int calculateCueSize( QString filename );
	void setStatus( QString in );
	
	QProcess *process, *procModel;
	QString device;
	QStringList filelist;
	QString driveropts;
	QString model;
	QString imagedir;

	int mode;
	bool cuefile;
	bool wasburning;
	bool waseject;
	bool idle;

	autojet *aj;

	QWidget *minContainer;
	Ui::minContainer uiMin;

	QWidget *maxContainer;
	Ui::maxContainer uiMax;
	
	QColor bgColor;
	QColor processColor;
	QColor finishedColor;
	QColor errorColor;
};

#endif
