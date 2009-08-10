/*
    Copyright (C) 2008-2009 Raphaël Doursenaud <rdoursenaud@free.fr>

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
*/

#ifndef CREATE_H
#define CREATE_H

#include <QDialog>
#include "ui_create.h"

class Create : public QDialog
{
Q_OBJECT
public:
    Create(QWidget *parent = 0);

    ~Create();

signals:
	void finished();	

private slots:
	void save();

private:
	Ui::CreateImage createUi;
};

#endif
