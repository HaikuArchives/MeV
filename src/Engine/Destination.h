/* ===================================================================== *
 * Destination.h (MeV/Engine)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Defines virtual channel table
 * ---------------------------------------------------------------------
 * History:
 *	1997		Joe Pearce
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  07/28/2000  dwalton
 *      Changed from struct to class.
 *		Added producer pointer.
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_Destination_H__
#define __C_Destination_H__

#include "BitSet.h"
#include <MidiProducer.h>
//Support Kit.
#include <String.h>
// Interface Kit
#include <InterfaceDefs.h>

const int			Max_Destinations = 64;			

/* ============================================================================ *
                                Destination

	Destinations, allow routing and remapping of MIDI data
	upon playback. There are up to 128 channels supported.
 * ============================================================================ */

class Destination {
public:

	enum destinationFlags {
		transposable	= (1<<0),				// channel is transposable
		mute			= (1<<1),				// channel is muted
		muteFromSolo	= (1<<2),				// channel muted because of solo
		solo			= (1<<3),				// channel is solo'd
		disabled		= (1<<4),				// channel disabled because
												// of vanished midi port.
		deleted			= (1<<5)				// only one channel should be in this catagory.
	};

	// REM: Replace with string class
	//char				name[ 24 ];				// channel name
	
	//uint8				port;					// midi port number
	uint8				channel,				// real midi channel
						flags,					// various flags
						velocityContour,		// which envelope to use
						VUMeter,				// Vu-like meter bar height
						pad;					// not used
						
						//defined;				// pad replaced with defined,
												// so the manager knows what is
												// defined.
	
	BString 			name;					// in the future we may not need this.
	BMidiLocalProducer *m_producer;	
	BString				producer_name;			//for reconnections.				
	int8				transpose,				// transposition for channel
						initialTranspose;		// initial transposition value

	rgb_color			fillColor,				// fill color
						highlightColor;			// hightlight color
};


/* ============================================================================ *
   VBitTable -- array of bits, one for each possible virtual channel.
 * ============================================================================ */

typedef BitSet<Max_Destinations> VBitTable;

/* ============================================================================ *
                               VelocityContour

	Up to 8 "Velocity Contours" are supported. Each contour is an envelope which
	can scale or compress the velocity information being processed through
	a virtual channel. (I could have decided to have a seperate velocity
	contour for each virtual channel, however I realized that in most situations,
	you would want to control the velocyt scaling of many Destinations in
	parallel.
 * ============================================================================ */

//#define maxVContours	8

#endif /* __C_Destination_H__ */