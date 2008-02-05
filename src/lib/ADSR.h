/*
 * Hydrogen
 * Copyright(c) 2002-2005 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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
 * $Id: ADSR.h,v 1.6 2005/05/01 19:51:23 comix Exp $
 *
 */

#ifndef ADSR_H
#define ADSR_H

#include "Object.h"

/// ADSR envelope
class ADSR
{
	public:
		enum ADSRState {
			ATTACK,
			DECAY,
			SUSTAIN,
			RELEASE,
			IDLE
		};

		float m_fAttack;		///< Attack time (in samples)
		float m_fDecay;			///< Decay time (in samples)
		float m_fSustain;		///< Sustain level
		float m_fRelease;		///< Release time (in samples)

		ADSR(
				float fAttack = 0.0,
				float fDecay = 0.0,
				float fSustain = 1.0,
				float fRelease = 1000
		);
		ADSR( const ADSR& orig );
		float getValue( float fStep );
		float release();

	private:
		ADSRState m_state;
		float m_fTicks;
		float m_fValue;
		float m_fReleaseValue;
};



#endif