/*
 * Hydrogen
 * Copyright(c) 2002-2004 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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
 * $Id: SMFEvent.cpp,v 1.4 2005/05/01 19:51:41 comix Exp $
 *
 */

#include "SMFEvent.h"

void SMFBuffer::writeByte( short int nByte )
{
//	infoLog( "[writeByte] " + toString( nByte ) );
	m_buffer.push_back( nByte );
}



void SMFBuffer::writeWord( int nVal )
{
//	infoLog( "writeWord" );
	writeByte( nVal >> 8 );
	writeByte( nVal );
}



void SMFBuffer::writeDWord( long nVal )
{
	writeByte( nVal >> 24 );
	writeByte( nVal >> 16 );
	writeByte( nVal >> 8 );
	writeByte( nVal );
}



void SMFBuffer::writeString( std::string sMsg )
{
//	infoLog( "writeString" );
	writeVarLen( sMsg.length() );

	for (unsigned i = 0; i < sMsg.length(); i++) {
		writeByte( sMsg.c_str()[ i ] );
	}
}



void SMFBuffer::writeVarLen( long value )
{
//	infoLog( "[writeVarLen]" );
	long buffer;
	buffer = value & 0x7f;
	while ( ( value >>= 7 ) > 0 ) {
		infoLog( "." );
		buffer <<= 8;
		buffer |= 0x80;
		buffer += ( value & 0x7f );
	}

	while ( true ) {
//		putc( buffer, outfile );
		writeByte( (char)buffer );
		if ( buffer & 0x80 ) {
			buffer >>= 8;
		}
		else {
			break;
		}
	}
}


// ::::::::::::::::::

SMFTrackNameMetaEvent::SMFTrackNameMetaEvent( std::string sTrackName, unsigned nTicks )
 : SMFEvent( "SMFTrackNameMetaEvent", nTicks )
 , m_sTrackName( sTrackName )
{
	// it's always at the start of the song
	m_nDeltaTime = 0;
}


std::vector<char> SMFTrackNameMetaEvent::getBuffer()
{
	SMFBuffer buf;
	buf.writeVarLen( m_nDeltaTime );
	buf.writeByte( 0xFF );
	buf.writeByte( TRACK_NAME );
	buf.writeString( m_sTrackName );

	return buf.getBuffer();
}


// :::::::::::::


SMFEvent::SMFEvent( std::string sEventName, unsigned nTicks )
 : Object( sEventName )
 , m_nTicks( nTicks )
 , m_nDeltaTime( -1 )
{
	//infoLog( "INIT" );
}



SMFEvent::~SMFEvent()
{
	//infoLog( "DESTROY" );
}


// ::::::::::::::


SMFNoteOnEvent::SMFNoteOnEvent( unsigned nTicks, int nChannel, int nPitch, int nVelocity )
 : SMFEvent( "SMFNoteOnEvent", nTicks )
 , m_nChannel( nChannel )
 , m_nPitch( nPitch )
 , m_nVelocity( nVelocity )
{
	if ( nChannel >= 16 ) {
		errorLog( "[SMFNoteOnEvent] nChannel >= 16! nChannel=" + toString(nChannel) );
	}
}



std::vector<char> SMFNoteOnEvent::getBuffer()
{
	SMFBuffer buf;
	buf.writeVarLen( m_nDeltaTime );
	buf.writeByte( NOTE_ON + m_nChannel );
	buf.writeByte( m_nPitch );
	buf.writeByte( m_nVelocity );

	return buf.getBuffer();
}


// :::::::::::



SMFNoteOffEvent::SMFNoteOffEvent( unsigned nTicks, int nChannel, int nPitch, int nVelocity )
 : SMFEvent( "SMFNoteOffEvent", nTicks )
 , m_nChannel( nChannel )
 , m_nPitch( nPitch )
 , m_nVelocity( nVelocity )
{
	if ( nChannel >= 16 ) {
		errorLog( "[SMFNoteOffEvent] nChannel >= 16! nChannel=" + toString(nChannel) );
	}
}



std::vector<char> SMFNoteOffEvent::getBuffer()
{
	SMFBuffer buf;
	buf.writeVarLen( m_nDeltaTime );
	buf.writeByte( NOTE_OFF + m_nChannel );
	buf.writeByte( m_nPitch );
	buf.writeByte( m_nVelocity );

	return buf.getBuffer();
}
