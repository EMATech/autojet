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

#include "configuration.h"
#include <QFileDialog>

Configuration::Configuration(QSettings *_settings, QWidget *parent)
 : QDialog(parent)
{
	settings = _settings;
	writerContainer = new QWidget( this );
	writerUi.setupUi( writerContainer );
	writerContainer->hide();

	configContainer = new QWidget( this );
	configUi.setupUi( configContainer );

	lastnumber = 0;

	setWindowTitle( tr("Configuration") );

	connect( writerUi.okButton, SIGNAL( pressed() ), this, SLOT( save() ) );
	connect( configUi.okButton, SIGNAL( pressed() ), this, SLOT( save() ) );
	connect( writerUi.listWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( viewChanged( int ) ) );
	connect( configUi.listWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( viewChanged( int ) ) );
	connect( configUi.imageButton, SIGNAL( pressed() ), this, SLOT( changeImageDir() ) );
	connect( configUi.spinBox, SIGNAL( valueChanged ( int ) ), this, SLOT( numberChanged( int ) ) );

	settings->beginGroup( "general" );
	
	configUi.image->setText( settings->value("imagedir", "/" ).toString() );
	configUi.spinBox->setValue( settings->value( "numburners", 0).toInt() );

	settings->endGroup();
}


Configuration::~Configuration( void )
{
}

void Configuration::save( void )
{
	settings->beginGroup( "general" );
	settings->setValue( "imagedir", configUi.image->text() );
	settings->setValue( "numburners", configUi.spinBox->value() );
	settings->endGroup();

	if ( writerUi.listWidget->currentRow()  > 0 )
	{
		settings->beginGroup( QString::number( writerUi.listWidget->currentRow() ) );
		settings->setValue( "device", writerUi.devicefile->text() );
		settings->setValue( "speed", writerUi.speed->value() );
		settings->setValue( "eject", writerUi.eject->isChecked() );
		settings->setValue( "simulation", writerUi.simulation->isChecked() );
		settings->setValue( "options", writerUi.driveropts->text() );
		settings->endGroup();
	}
	
	hide();

	emit finished();
}

void Configuration::viewChanged( int item )
{
	if ( item == lastnumber )
		return;
	
	if ( 0 == item )
	{
		writerContainer->hide();
		configContainer->show();
	} else
	{
		writerContainer->show();
		configContainer->hide();
	}

	if ( item > 0 )
	{
		viewWriter( item );
	}

	lastnumber = item;

	writerUi.listWidget->setCurrentRow( item );
	configUi.listWidget->setCurrentRow( item );
}

void Configuration::viewWriter( int number )
{
	if ( lastnumber > 0 )
	{
		settings->beginGroup( QString::number( lastnumber ) );
		settings->setValue( "device", writerUi.devicefile->text() );
		settings->setValue( "speed", writerUi.speed->value() );
		settings->setValue( "eject", writerUi.eject->isChecked() );
		settings->setValue( "simulation", writerUi.simulation->isChecked() );
		settings->setValue( "options", writerUi.driveropts->text() );
		settings->endGroup();
	}
		
	settings->beginGroup( QString::number( number ) );
	writerUi.devicefile->setText( settings->value( "device", "/dev/scd" ).toString() );
	writerUi.speed->setValue( settings->value( "speed", 0 ).toInt() );
	writerUi.eject->setChecked( settings->value( "eject", true ).toBool() );
	writerUi.simulation->setChecked( settings->value( "simulation", false ).toBool() );
	writerUi.driveropts->setText( settings->value( "options", "burnfree" ).toString() );
	settings->endGroup();
}

void Configuration::changeImageDir( void )
{
	configUi.image->setText( QFileDialog::getExistingDirectory( this, tr("Select Image Directory") ) );
}

void Configuration::numberChanged( int number )
{
	writerUi.listWidget->clear();
	configUi.listWidget->clear();
	writerUi.listWidget->addItem( tr("General") );
	configUi.listWidget->addItem( tr("General") );
		
	for( int i = 1; i <= number; i++ )
	{
		writerUi.listWidget->addItem( tr("CD Writer #") + QString::number( i ) );
		configUi.listWidget->addItem( tr("CD Writer #") + QString::number( i ) );
	}

	writerUi.listWidget->setCurrentRow( 0 );
	configUi.listWidget->setCurrentRow( 0 );
}
