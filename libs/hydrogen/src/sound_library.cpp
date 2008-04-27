/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QDir>

#include <hydrogen/SoundLibrary.h>
#include <hydrogen/instrument.h>
#include <hydrogen/sample.h>
#include <hydrogen/LocalFileMng.h>
#include <hydrogen/h2_exception.h>
#include <hydrogen/hydrogen.h>
#include <hydrogen/adsr.h>
#include <hydrogen/Preferences.h>

#include <cstdlib>

#include <zlib.h>	// used for drumkit install
#include <libtar.h>	// used for drumkit install
#include <fcntl.h>
#include <errno.h>

namespace H2Core
{

SoundLibrary::SoundLibrary()
		: Object( "SoundLibrary" )
{
}



SoundLibrary::~SoundLibrary()
{
}



// ::::::::::



Drumkit::Drumkit()
		: Object( "Drumkit" )
		, m_pInstrumentList( NULL )
{
}



Drumkit::~Drumkit()
{
	delete m_pInstrumentList;
}




Drumkit* Drumkit::load( const QString& sFilename )
{
	LocalFileMng mng;
	return mng.loadDrumkit( sFilename );
}



std::vector<QString> Drumkit::getUserDrumkitList()
{
	LocalFileMng mng;
	return mng.getUserDrumkitList();
}



std::vector<QString> Drumkit::getSystemDrumkitList()
{
	LocalFileMng mng;
	return mng.getSystemDrumkitList();
}



void Drumkit::dump()
{
	INFOLOG( "Drumkit dump" );
	INFOLOG( "\t|- Name = " + m_sName );
	INFOLOG( "\t|- Author = " + m_sAuthor );
	INFOLOG( "\t|- Info = " + m_sInfo );

	INFOLOG( "\t|- Instrument list" );
	for ( unsigned nInstrument = 0; nInstrument < m_pInstrumentList->get_size(); ++nInstrument ) {
		Instrument *pInstr = m_pInstrumentList->get( nInstrument );
		INFOLOG( "\t\t|- (" + to_string( nInstrument ) + " of " + to_string( m_pInstrumentList->get_size() ) + ") Name = " + pInstr->get_name() );
		for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
			InstrumentLayer *pLayer = pInstr->get_layer( nLayer );
			if ( pLayer ) {
				Sample *pSample = pLayer->get_sample();
				if ( pSample ) {
					INFOLOG( "\t\t   |- " + pSample->get_filename() );
				} else {
					INFOLOG( "\t\t   |- NULL sample" );
				}
			} else {
				INFOLOG( "\t\t   |- NULL Layer" );
			}

		}
//		cout << "\t\t" << i << " - " << instr->getSample()->getFilename() << endl;
	}
}



void Drumkit::install( const QString& filename )
{
	_INFOLOG( "[Drumkit::install] drumkit = " + filename );
	QString dataDir = Preferences::getInstance()->getDataDirectory();

	// GUNZIP !!!
	QString gunzippedName = filename.left( filename.indexOf( "." ) );
	gunzippedName += ".tar";
	FILE *pGunzippedFile = fopen( gunzippedName.toAscii(), "wb" );
	gzFile gzipFile = gzopen( filename.toAscii(), "rb" );
	uchar buf[4096];
	while ( gzread( gzipFile, buf, 4096 ) > 0 ) {
		fwrite( buf, sizeof( uchar ), 4096, pGunzippedFile );
	}
	gzclose( gzipFile );
	fclose( pGunzippedFile );


	// UNTAR !!!
	TAR *tarFile;

	char tarfilename[1024];
	strcpy( tarfilename, gunzippedName.toAscii() );

	if ( tar_open( &tarFile, tarfilename, NULL, O_RDONLY, 0, TAR_VERBOSE | TAR_GNU ) == -1 ) {
		_ERRORLOG( QString( "[Drumkit::install] tar_open(): %1" ).arg( strerror( errno ) ) );
	}

	char destDir[1024];
	strcpy( destDir, dataDir.toAscii() );
	if ( tar_extract_all( tarFile, destDir ) != 0 ) {
		_ERRORLOG( QString( "[Drumkit::install] tar_extract_all(): %1" ).arg( strerror( errno ) ) );
	}

	if ( tar_close( tarFile ) != 0 ) {
		_ERRORLOG( QString( "[Drumkit::install] tar_close(): %1" ).arg( strerror( errno ) ) );
	}
}



void Drumkit::save( const QString& sName, const QString& sAuthor, const QString& sInfo )
{
	_INFOLOG( "Saving drumkit" );

	Drumkit *pDrumkitInfo = new Drumkit();
	pDrumkitInfo->setName( sName );
	pDrumkitInfo->setAuthor( sAuthor );
	pDrumkitInfo->setInfo( sInfo );

	Song *pSong = Hydrogen::get_instance()->getSong();
	InstrumentList *pSongInstrList = pSong->get_instrument_list();
	InstrumentList *pInstrumentList = new InstrumentList();

	for ( uint nInstrument = 0; nInstrument < pSongInstrList->get_size(); nInstrument++ ) {
		Instrument *pOldInstr = pSongInstrList->get( nInstrument );
		Instrument *pNewInstr = new Instrument( pOldInstr->get_id(), pOldInstr->get_name(), new ADSR( *( pOldInstr->get_adsr() ) ) );
		pNewInstr->set_volume( pOldInstr->get_volume() );
		pNewInstr->set_pan_l( pOldInstr->get_pan_l() );
		pNewInstr->set_pan_r( pOldInstr->get_pan_r() );
		pNewInstr->set_muted( pOldInstr->is_muted() );
		pNewInstr->set_random_pitch_factor( pOldInstr->get_random_pitch_factor() );
		pNewInstr->set_mute_group( pOldInstr->get_mute_group() );

		pNewInstr->set_filter_active( pOldInstr->is_filter_active() );
		pNewInstr->set_filter_cutoff( pOldInstr->get_filter_cutoff() );
		pNewInstr->set_filter_resonance( pOldInstr->get_filter_resonance() );


		QString sInstrDrumkit = pOldInstr->get_drumkit_name();

		for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
			InstrumentLayer *pOldLayer = pOldInstr->get_layer( nLayer );
			if ( pOldLayer ) {
				Sample *pSample = pOldLayer->get_sample();

				Sample *pNewSample = new Sample( 0, pSample->get_filename() );	// is not a real sample, it contains only the filename information
				InstrumentLayer *pLayer = new InstrumentLayer( pNewSample );
				pLayer->set_gain( pOldLayer->get_gain() );
				pLayer->set_pitch( pOldLayer->get_pitch() );
				pLayer->set_start_velocity( pOldLayer->get_start_velocity() );
				pLayer->set_end_velocity( pOldLayer->get_end_velocity() );

				pNewInstr->set_layer( pLayer, nLayer );
			} else {
				pNewInstr->set_layer( NULL, nLayer );
			}
		}
		pInstrumentList->add ( pNewInstr );
	}

	pDrumkitInfo->setInstrumentList( pInstrumentList );

	LocalFileMng fileMng;
	int err = fileMng.saveDrumkit( pDrumkitInfo );
	if ( err != 0 ) {
		_ERRORLOG( "Error saving the drumkit" );
		throw H2Exception( "Error saving the drumkit" );
	}

	// delete the drumkit info
	delete pDrumkitInfo;
	pDrumkitInfo = NULL;

}




void Drumkit::removeDrumkit( const QString& sDrumkitName )
{
	_INFOLOG( "Removing drumkit: " + sDrumkitName );

	QString dataDir = Preferences::getInstance()->getDataDirectory();
	dataDir += sDrumkitName;
	QString cmd = QString( "rm -rf \"" ) + dataDir + "\"";
	_INFOLOG( cmd );
	if ( system( cmd.toAscii() ) != 0 ) {
		_ERRORLOG( "Error executing '" + cmd + "'" );
		throw H2Exception( QString( "Error executing '%1'" ).arg( cmd ) );
	}
}

};
