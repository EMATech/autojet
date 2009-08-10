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

#include "autojet.h"

#include <QDir>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>

autojet::autojet( QWidget *parent, Qt::WFlags flags ) :
		QMainWindow( parent, flags )
{
	ui.setupUi( this );
	
	logFile = new QFile( QDir::home().absolutePath()+"/"+AUTOJET_LOG );
	logFile->open( QIODevice::WriteOnly | QIODevice::Append );

	//Fullscreen
	//setWindowState( Qt::WindowFullScreen );

	connect( ui.actionQuit, SIGNAL( activated() ),
		this, SLOT( close() ) );

	connect( ui.actionShut_Down, SIGNAL( activated() ),
		this, SLOT( halt() ) );

	settings = new QSettings( "ematech", "autojet" );

	settings->beginGroup( "general" );
	imagedir = settings->value("imagedir", "/" ).toString();
	ui.notes->setPlainText( settings->value( "notes", tr("Notes") ).toString() );
	settings->endGroup();

	configDialog = new Configuration( settings, this );
	connect( configDialog, SIGNAL( finished() ), this, SLOT( buildView() ) );
	
	createDialog = new Create( this );
	connect( createDialog, SIGNAL( finished() ), this, SLOT( buildView() ) );

	connect( ui.actionConfigure, SIGNAL( activated() ),
		 this, SLOT( showConfig() ) );

	connect( ui.actionEject, SIGNAL( activated() ),
		 this, SLOT( eject() ) );
	connect( ui.actionCreate_Image, SIGNAL( activated() ),
		 this, SLOT( create() ) );
	connect( ui.actionBlank, SIGNAL( activated() ),
		 this, SLOT( blank() ) );
	connect( ui.actionSet_Image, SIGNAL( activated() ),
		 this, SLOT( imagefile() ) );
	connect( ui.actionBurn, SIGNAL( activated() ),
		 this, SLOT( burn() ) );
	connect( ui.actionHelp, SIGNAL( activated() ),
		 this, SLOT( help() ) );
	connect( ui.notes, SIGNAL( textChanged() ),
		 this, SLOT( saveNotes() ) );
	buildView();
}

void autojet::resizeEvent ( QResizeEvent * )
{
	//area->setGeometry( 0, 0, this->width(), this->height() - 103 );
}

void autojet::showConfig( void )
{
	configDialog->show();
}

void autojet::buildView( void )
{
	// Cleanup burnersList
	burnersList.clear();
	
	// Retrieve number of burners from settings
	settings->beginGroup( "general" );
	int numBurners = settings->value( "numburners", 0 ).toInt();
	settings->endGroup();

	// Create CDWriter objects and populate GUI
	for( int i = 1; i <= numBurners; i++ )
	{
		CDWriter *cur = new CDWriter( this, settings, i );
		burnersList.append( cur );
		ui.burnersLayout->addWidget( cur );

		connect( cur, SIGNAL( clicked() ), this, SLOT( changeView() ) );
	}

	// Maximizes the first burner in the list
	if ( burnersList.size() > 0 )
	{
		burnersList.at( 0 )->maximize();
	}
	maximized = 0;
}

void autojet::changeView( void )
{
	CDWriter *obj = (CDWriter*) sender();
	
	for (int i = 0; i < burnersList.size(); ++i)
	{
		if ( obj == burnersList.at( i ) )
		{
			burnersList.at( i )->maximize();
			maximized = i ;
		} else
		{
			burnersList.at( i )->minimize();
		}
	}
}

void autojet::halt( void )
{
	//TODO: Find a better way
	//TODO: Catch return code?! (W)
	system("kdesu -c 'shutdown -h -P 0'");
}

void autojet::help( void )
{
	QMessageBox::about( this, tr("EMA Tech. - AutoJet : Multiple CD/DVD Burner"),
			    tr("<html><p><strong>EMA Tech. - AutoJet version 0.1</strong></p>\
				<p>Copyright (C) 2008 Rapha&euml;l Doursenaud<br/>\
				Copyright (C) 2005-2007 David Weisgerber</p>\
				<p>Autojet is based on David Weisgerber's TurboJet 2</p> \
				<p>AutoJet comes with ABSOLUTELY NO WARRANTY.<br/>\
				This is free software, and you are welcome to redistribute it under certain conditions.</p>\
				<p>Website and support: <a href=\"http://autojet.ematech.fr\">EMA Tech.</a></p>\
				</html>"));
}

void autojet::eject( void )
{
	int i = 0;
	while( i < burnersList.count() )
	{
		if ( burnersList.at(i)->isIdle() ) burnersList.at(i)->eject();
		i++;
	}
}

void autojet::create( void )
{

 for (int i = 0; i < burnersList.size(); ++i)
	{
		if ( !burnersList.at( i )->isIdle() )
		{
			QMessageBox::warning( this, tr("Please wait..."),
							tr("All CD Writers must be idle.\n"
							"Please wait for them to settle down.") );
			return;
		}
	}

	createDialog->show();
}

void autojet::blank( void )
{
	int i = 0;
	while( i < burnersList.count() )
	{
		if ( burnersList.at(i)->isIdle() ) burnersList.at(i)->blank();
		i++;
	}
}

void autojet::burn( void )
{
	int i = 0;
	while( i < burnersList.count() )
	{
		if ( burnersList.at(i)->isIdle() ) burnersList.at(i)->burn();
		i++;
	}
}

void autojet::imagefile( void )
{
	QString isoFilter = tr( "ISO Images (*.iso)" );
	QString cueFilter = tr( "Cue Sheets (*.cue *.toc)");
	QString *outFilter = new QString();
	QString file = QFileDialog::getOpenFileName( this,
			tr("Select Image to burn"),
			imagedir, isoFilter + "\n" + cueFilter, 0 );
	if ( file.isNull() )
	{
		return;
	}
	int i = 0;
	while( i < burnersList.count() )
	{
		if ( burnersList.at(i)->isIdle() )
		{
			if ( *outFilter == cueFilter )
				cuefile = true;
			else
				cuefile = false;

			burnersList.at(i)->imagefile( file );
		}
		i++;
	}
}

void autojet::addLog( QString in )
{
	logFile->write( in.toUtf8() );
	logFile->flush();
}

void autojet::keyReleaseEvent ( QKeyEvent * event )
{
	if ( ui.notes->hasFocus() )
	{
		return;
	}
	
	switch( event->key() )
	{
		case Qt::Key_Down:
			maximized++;
			break;
		case Qt::Key_Up:
			maximized--;
			break;
		case Qt::Key_0:
			maximized = 9;
			break;
		default:
		{
			if ( event->key() > Qt::Key_0 && event->key() <= Qt::Key_9 )
			{
				maximized = event->key() - Qt::Key_1;
			}
		}
	}

	if ( maximized >= burnersList.size() )
	{
		maximized = 0;
	}

	if ( maximized < 0 )
	{
		maximized = burnersList.size() - 1;
	}

	for (int i = 0; i < burnersList.size(); ++i)
	{
		if ( maximized == i )
		{
			burnersList.at( i )->maximize();
		} else
		{
			burnersList.at( i )->minimize();
		}
	}
}

void autojet::saveNotes( void )
{
	settings->beginGroup( "general" );
	settings->setValue( "notes", ui.notes->toPlainText() );
	settings->endGroup();
}
