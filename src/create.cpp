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

#include "create.h"

Create::Create(QWidget *parent)
 : QDialog(parent)
{
	createUi.setupUi( this );

	setWindowTitle( tr("Create Image") );
	
	createUi.imageName->setFocus();

	connect(createUi.buttonBox, SIGNAL(accepted()), this, SLOT(save()));
	QString test = createUi.imageName->text();
}


Create::~Create()
{
}

void Create::save()
{
	emit finished();
}
