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

#include <QDateTime>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QMouseEvent>
#include <QFileDialog>
#include "cdwriter.h"
#include "autojet.h"

CDWriter::CDWriter( autojet *parent, QSettings *settings, int number)
{
	// Set parent?
	aj = parent;

	// Create containers
	minContainer = new QWidget( this );
	uiMin.setupUi( minContainer );
	
	maxContainer = new QWidget( this );
	uiMax.setupUi( maxContainer );

	// Connect signals
	connect( uiMax.eject, SIGNAL( pressed() ), this, SLOT( eject() ) );
	connect( uiMax.create, SIGNAL( pressed() ), this, SLOT( create() ) );
	connect( uiMax.blank, SIGNAL( pressed() ), this, SLOT( blank() ) );
	connect( uiMax.burn, SIGNAL( pressed() ), this, SLOT( burn() ) );
	connect( uiMax.image, SIGNAL( pressed() ), this, SLOT( select() ) );

	// Set item colors
	bgColor = minContainer->palette().color( minContainer->backgroundRole() );
	processColor = QColor( 255, 246, 0 );
	finishedColor = QColor( 23, 178, 109 );
	errorColor = QColor( 249, 0, 0 );

	// Distinguish impair items by darker colors
	if( number % 2 ) {
		bgColor = bgColor.dark( 110 );
		processColor = processColor.dark( 110 );
		finishedColor = finishedColor.dark( 110 );
		errorColor = errorColor.dark( 110 );
		QPalette p=minContainer->palette();
		p.setColor(minContainer->backgroundRole(), bgColor);
		minContainer->setPalette(p);
	}

	// Define item number
	uiMin.number->setText( QString::number( number ) );
	uiMax.number->setText( QString::number( number ) );

	/*
	0 means no file selected
	1 means ISO image selected
	2 means DVD-ISO
	3 means Cue-Sheet using cdrdao
	*/

	mode = 0;
	cuefile = false;
	wasburning = false;
	waseject = false;

	settings->beginGroup( QString::number( number ) );
	device = settings->value( "device", "/" ).toString();
	uiMax.speedBox->setValue( settings->value( "speed", 0 ).toInt() );
	driveropts = settings->value( "options", "burnfree" ).toString();
	uiMax.simulation->setChecked( settings->value( "simulation", false ).toBool() );
	uiMax.ejectBox->setChecked( settings->value( "eject", true ).toBool() );
	settings->endGroup();

	settings->beginGroup( "general" );
	imagedir = settings->value("imagedir", "/" ).toString();
	settings->endGroup();

	procModel = new QProcess();
	connect( procModel, SIGNAL( finished( int, QProcess::ExitStatus ) ),
		 this, SLOT( finishedInfo( int, QProcess::ExitStatus ) ) );
	connect( procModel, SIGNAL( readyReadStandardOutput() ), this, SLOT( readLineInfo() ) );
	connect( procModel, SIGNAL( readyReadStandardError() ), this, SLOT( readLineInfo() ) );
	procModel->start( "cdrecord", QStringList() << "dev=" + device <<
				"-prcap" );

	minimize();
}

CDWriter::~CDWriter( void )
{
}

void CDWriter::maximize( void )
{
	setMinimumSize( maxContainer->minimumSize() );
	setMaximumSize( maxContainer->maximumSize() );
	minContainer->hide();
	maxContainer->show();
}

void CDWriter::minimize( void )
{
	setMinimumSize( minContainer->minimumSize() );
	setMaximumSize( minContainer->maximumSize() );
	maxContainer->hide();
	minContainer->show();
}

void CDWriter::mouseReleaseEvent( QMouseEvent * event )
{
	if ( event->button() == Qt::LeftButton )
	{
		emit clicked();
	}
}

// Ejects and loads the tray
void CDWriter::eject( void )
{
	if (!isIdle()) return;

	process = new QProcess( this );

	connect( process, SIGNAL( finished( int, QProcess::ExitStatus ) ),
		 this, SLOT( finished( int, QProcess::ExitStatus ) ) );

	setIdle( false );
	
	process->start( "eject", QStringList() << "-T" << device );
}

void CDWriter::create( void )
{
	if (!isIdle()) return;
	
	process = new QProcess( this );
	
	// TODO write routine
}

// TODO implement "blank=all" alternative
// TODO implement dvd+rw blanking (dvd+rw-format -force /dev/dvd)
// TODO merge with burn? (Duplicate code)
void CDWriter::blank( void )
{
	if ( mode == 0 || !isIdle() ) return;
	
	QString program;
	QStringList arguments;
	process = new QProcess( this );

	setIdle( false );

	program = "cdrecord";
		
	arguments << "dev=" + device
		<< "blank=fast"
		<< "-v" << "-v";

	append( "Starting: " + program + " " + arguments.join( " " ) );

	connect( process, SIGNAL( finished( int, QProcess::ExitStatus ) ),
		 this, SLOT( finished( int, QProcess::ExitStatus ) ) );
	connect( process, SIGNAL( readyReadStandardOutput() ), this, SLOT( readLine() ) );
	connect( process, SIGNAL( readyReadStandardError() ), this, SLOT( readLine() ) );

	aj->addLog( "[" + QDateTime::currentDateTime().toString( "yyyyMMdd hh:mm" ) + "] " +
		  filelist.join( "," ) + "\n" );

	uiMax.idleBox->setText( tr("Blanking") );
	wasburning = true;

	process->start( program, arguments );

	QPalette p=uiMax.idleBox->palette();
	p.setColor(uiMax.idleBox->backgroundRole(), processColor);
	uiMax.idleBox->setPalette(p);

	p=minContainer->palette();
	p.setColor(minContainer->backgroundRole(), processColor);
	minContainer->setPalette(p);
}

// TODO Autodetect Image type (ISO or CueSheet)
void CDWriter::imagefile( QString file )
{
	if (!isIdle()) return;
	
	QFileInfo getsize( file );
	
	filelist.clear();
	filelist.append( file );
	
	uiMax.size->setText( "ISO: "+QString::number( getsize.size() / 1024 / 1024 )+ " MB" );
	append( tr( "ISO Image selected: ") + file );
	
	uiMax.tracks->clear();
	uiMax.tracks->append( tr("ISO Image: ") + file.section('/', -1) );

	uiMin.track->setText( file.section('/', -1) );
	uiMax.track->setText( uiMin.track->text() );
	
	mode = 1;
}

// TODO : Merge with imagefile and do autodetect
/*
void CDWriter::dvdfile( QString file )
{
	if (!isIdle()) return;
	
	QFileInfo getsize( file );
	
	filelist.clear();
	filelist.append( file );
	
	uiMax.size->setText( "DVD: "+QString::number( getsize.size() / 1024 / 1024 )+ " MB" );
	append( tr( "DVD-ISO Image selected: ") + file );
	
	uiMax.tracks->clear();
	uiMax.tracks->append( tr("DVD Image: ") + file );

	uiMin.track->setText( file );
	uiMax.track->setText( uiMin.track->text() );
	
	mode = 2;
}

void CDWriter::cuefile( QString file )
{
	if (!isIdle()) return;
	
	filelist.clear();
	filelist.append( file );
	
	uiMax.size->setText( "CUE/TOC: " + QString::number( calculateCueSize(file) /1024 /1024) + " MB"  );
	append( tr( "Cue Sheet selected: ") + file );
	
	uiMax.tracks->clear();
	uiMax.tracks->append( tr("Cue Sheet: ") + file );

	uiMin.track->setText( file );
	uiMax.track->setText( uiMin.track->text() );
	
	mode = 3;
}

int CDWriter::calculateCueSize( QString filename )
{
	int size = 0;
	QFile f( filename );
	if ( !f.open( QIODevice::ReadOnly ) )
		return 0;
	
	QFileInfo fd( filename );
	QString dir = fd.dir().absolutePath();
	
	QTextStream in(&f);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		
		if ( line.indexOf( "FILE" ) == -1 )
			continue;
		
		int marker1 = line.indexOf( "\"" );
		int marker2 = line.indexOf( "\"", marker1 + 1);
		
		QFileInfo binFile( dir + "/" + line.mid( marker1 + 1, marker2 - marker1 -1 ) );
		
		size += binFile.size();
	}
		
	return size;
}
*/

void CDWriter::burn( void )
{
	if ( mode == 0 || !isIdle() ) return;

	QString program;
	QStringList arguments;
	process = new QProcess( this );

	setIdle( false );

	/* We are using cdrecord to burn CD ISO images  */
	if ( mode == 1 )
	{
		program = "cdrecord";
		
		arguments << "dev=" + device
			<< "speed=" + QString::number( uiMax.speedBox->value() )
// TODO:
// Speeds can be obtained by doing "cdrecord -prcap"
// we need to extract and parse a bunch of lines like :
//   Write speed # 0:  8467 kB/s CLV/PCAV (CD  48x, DVD  6x)
// to get only the :
//   CD  48x, DVD  6x
// part and store the 48 and 6 values for future use
// This needs to be done at drive initialisation NOT here!
			<< "driveropts=" + driveropts
			<< "-v" << "-v"
			<< "-dao";
		
		if ( uiMax.simulation->isChecked() ) arguments << "--dummy";

		arguments << "--data";

		arguments << filelist;
	}

	/* We are using growisofs to burn DVD ISO images */
	if ( mode == 2 )
	{
		program = "growisofs";
		
		arguments << "-dvd-compat";
		if ( uiMax.speedBox->value() > 0 ) arguments << "-speed=" + QString::number( uiMax.speedBox->value() );
		arguments << "-use-the-force-luke=tty";
		if ( uiMax.simulation->isChecked() ) arguments << "-use-the-force-luke=dummy";
		arguments << "-Z";
		arguments << device + "=" + filelist.first();
	}

	/* We are using cdrdao to burn CueSheets */
	if ( mode == 3 )
	{
		program = "cdrdao";
		
		if ( uiMax.simulation->isChecked() ) arguments << "simulate";
		else arguments << "write";
		arguments << "--device" << device;
		if ( uiMax.speedBox->value() > 0 )
		{
			arguments << "--speed" << QString::number( uiMax.speedBox->value() );
		}
		arguments << filelist.first();
	}

	append( "Starting: " + program + " " + arguments.join( " " ) );

	connect( process, SIGNAL( finished( int, QProcess::ExitStatus ) ),
		 this, SLOT( finished( int, QProcess::ExitStatus ) ) );
	connect( process, SIGNAL( readyReadStandardOutput() ), this, SLOT( readLine() ) );
	connect( process, SIGNAL( readyReadStandardError() ), this, SLOT( readLine() ) );

	aj->addLog( "[" + QDateTime::currentDateTime().toString( "yyyyMMdd hh:mm" ) + "] " +
		  filelist.join( "," ) + "\n" );

	//uiMax.idleBox->setPaletteBackgroundColor ( QColor( 255, 246, 0 ) );
	uiMax.idleBox->setText( tr("Burning") );
	wasburning = true;

	process->start( program, arguments );

	QPalette p=uiMax.idleBox->palette();
	p.setColor(uiMax.idleBox->backgroundRole(), processColor);
	uiMax.idleBox->setPalette(p);

	p=minContainer->palette();
	p.setColor(minContainer->backgroundRole(), processColor);
	minContainer->setPalette(p);
}

void CDWriter::finished( int exitCode, QProcess::ExitStatus )
{
	setIdle( true );

	if ( wasburning && waseject ) {
		wasburning = false;
		waseject = false;
	}else if ( wasburning )
	{
		append( tr("Exited with status ") + QString::number( exitCode ) );
		
		if ( exitCode == 0 )
		{
			QPalette p=uiMax.idleBox->palette();
			p.setColor(uiMax.idleBox->backgroundRole(), finishedColor);
			uiMax.idleBox->setPalette(p);

			p=minContainer->palette();
			p.setColor(minContainer->backgroundRole(), finishedColor);
			minContainer->setPalette(p);
			
			uiMax.idleBox->setText( "Success" );
		} else
		{
			QPalette p=uiMax.idleBox->palette();
			p.setColor( uiMax.idleBox->backgroundRole(), errorColor);
			uiMax.idleBox->setPalette(p);

			p=minContainer->palette();
			p.setColor( minContainer->backgroundRole(), errorColor);
			minContainer->setPalette(p);
			
			uiMax.idleBox->setText( "Error" );
		}	
	} else
	{
		QPalette p=minContainer->palette();
		p.setColor(minContainer->backgroundRole(), bgColor);
		minContainer->setPalette(p);
		uiMax.idleBox->setPalette( palette() );

		uiMax.idleBox->setText( "Idle" );
	}
	
	delete process;
	
	if ( uiMax.ejectBox->isChecked() && wasburning )
	{
		eject();
		waseject = true;
	} else {
		wasburning = false;
	}
}

void CDWriter::append( QString string )
{
	uiMax.output->append( string );
}

void CDWriter::readLineInfo( void )
{
	QString temp = procModel->readAllStandardOutput();
	QString _model = temp;

	if ( _model.indexOf( "Vendor_info" ) != -1 )
	{
		_model = _model.right( _model.length() - ( _model.indexOf( "Vendor_info    : '" ) + 18 ) );
		_model = _model.left( _model.indexOf( "'" ) );
		model = _model.trimmed();
		uiMax.model->setText( tr("Drive OK") );
	}

	_model = temp;

	if ( _model.indexOf( "Identification" ) != -1 )
	{
		_model = _model.right( _model.length() - ( _model.indexOf( "Identification : '" ) + 18 ) );
		_model = _model.left( _model.indexOf( "'" ) );
		model = model + " " + _model.trimmed();
		uiMax.model->setText( tr("Drive OK") );
	}
}

// If process has finished
void CDWriter::finishedInfo( int, QProcess::ExitStatus )
{
	setIdle( true );
	delete procModel;
}

void CDWriter::readLine( void )
{
	QString in = process->readAllStandardOutput();
	if ( mode == 1 && in.indexOf( "remaining" ) != -1 )
	{
		setStatus( in );
		return;
	}
	if ( in[0] == 0x0d ) {
		setStatus( in );
	} else {
		append( in );
	}
	
	QString err = process->readAllStandardError();
	if ( mode == 2 && err.indexOf( "Wrote" ) != -1 && err.indexOf( "blocks" ) == -1 )
	{
		setStatus( err );
	} else
		uiMax.errors->append( err ) ;
}

void CDWriter::setStatus( QString str )
{
	int written, towrite, fifoi, bufi;
	QString speed;
	
	if ( mode == 1 ) // using cdrecord
	{
		written = str.mid( 11, 4).toInt();
		towrite = str.mid( 19, 4).toInt();
		fifoi = str.mid( 41, 3 ).toInt();
		bufi = str.mid( 52, 3 ).toInt();
		speed = str.mid( 59, 5);
		uiMin.progressBar->setValue( (written * 100) / towrite );
		uiMax.progressBar->setValue( (written * 100) / towrite );
		uiMax.fifoBar->setValue( fifoi );
		uiMax.bufferBar->setValue( bufi );
		uiMax.currSpeed->setText( "Current Speed: " + speed );
	}
	
	if ( mode == 2 ) // using growisofs
	{
		int indexOfer = str.indexOf( "%" );
		uiMin.progressBar->setValue( str.mid( indexOfer-4, 2 ).toInt() );
		uiMax.progressBar->setValue( str.mid( indexOfer-4, 2 ).toInt() );
		indexOfer = str.indexOf( "@" );
		int indexOfer2 = str.indexOf( ",");
		uiMax.currSpeed->setText( "Current Speed: " + str.mid( indexOfer+1, indexOfer2-indexOfer-1 ) );
		indexOfer = str.indexOf( "RBU" );
		indexOfer2 = str.indexOf( ".", indexOfer );
		uiMax.fifoBar->setValue( str.mid( indexOfer+4, indexOfer2-indexOfer-4 ).toInt() );
	}
		
	if ( mode == 3 ) // using cdrdao
	{
		int indexOfer = str.indexOf( "of" );
		int indexOfer2 = str.indexOf ( "MB" );
		if ( str.mid( indexOfer +3, indexOfer2 - indexOfer - 3 ).toInt() != 0 )
		{
			uiMin.progressBar->setValue( ( str.mid( 6, indexOfer - 6 ).toInt() * 100 ) /
					str.mid( indexOfer + 3, indexOfer2 - indexOfer - 3 ).toInt() );
			uiMax.progressBar->setValue( ( str.mid( 6, indexOfer - 6 ).toInt() * 100 ) /
					str.mid( indexOfer + 3, indexOfer2 - indexOfer - 3 ).toInt() );
		}
		indexOfer = str.indexOf( "Buffers" ) + 8;
		indexOfer2 = str.indexOf( "%" );
		int indexOfer3 = str.lastIndexOf( "%" );
		uiMax.fifoBar->setValue( str.mid( indexOfer, indexOfer2 - indexOfer ).toInt() );
		uiMax.bufferBar->setValue( str.mid( indexOfer2 + 2, indexOfer3 - indexOfer2 - 2).toInt() );
	}
}

void CDWriter::select( void )
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

	if ( *outFilter == cueFilter )
		cuefile = true;
	else
		cuefile = false;

	imagefile( file );
}

void CDWriter::setIdle( bool isIdle )
{
	idle = isIdle;
	uiMax.idleBox->setChecked( idle );

	if ( !isIdle )
	{
		uiMax.eject->setEnabled( false );
		uiMax.create->setEnabled( false );
		uiMax.blank->setEnabled( false );
		uiMax.image->setEnabled( false );
		uiMax.burn->setEnabled( false );
	} else
	{
		uiMax.eject->setEnabled( true );
		uiMax.create->setEnabled( true );
		uiMax.blank->setEnabled( true );
		uiMax.image->setEnabled( true );
		uiMax.burn->setEnabled( true );
	}
}
