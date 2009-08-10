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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QDialog>
#include <QSettings>
#include "ui_configuration.h"
#include "ui_configwriter.h"

class Configuration : public QDialog
{
Q_OBJECT
public:
    Configuration(QSettings *_settings, QWidget *parent = 0);

    ~Configuration( void );

signals:
	void finished( void );	

private slots:
	void save( void );
	void viewChanged( int item );
	void changeImageDir( void );
	void numberChanged( int number );
    
private:
	void viewWriter( int number );
	QWidget *writerContainer;
	Ui::configwriter writerUi;

	QWidget *configContainer;
	Ui::configuration configUi;
	QSettings *settings;

	int lastnumber;
};

#endif
