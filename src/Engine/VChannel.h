/* ===================================================================== *
 * VChannel.h (MeV/Engine)
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
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_VChannelEntry_H__
#define __C_VChannelEntry_H__

#include "BitSet.h"

// Interface Kit
#include <InterfaceDefs.h>

const int			Max_VChannels = 64;			// 64 virtual channels

/* ============================================================================ *
                                VChannelEntry

	VChannels, or virtual channels, allow routing and remapping of MIDI data
	upon playback. There are up to 128 channels supported.
 * ============================================================================ */

class VChannelEntry {
public:

	enum vChannelFlags {
		transposable	= (1<<0),				// channel is transposable
		mute			= (1<<1),				// channel is muted
		muteFromSolo	= (1<<2),				// channel muted because of solo
		solo			= (1<<3)				// channel is solo'd
	};

	// REM: Replace with string class
//	char				name[ 24 ];				// channel name

	uint8				port,					// midi port number
						channel,				// real midi channel
						flags,					// various flags
						velocityContour,		// which envelope to use
						VUMeter,				// Vu-like meter bar height
						pad;					// not used

	int8				transpose,				// transposition for channel
						initialTranspose;		// initial transposition value

	rgb_color			fillColor,				// fill color
						highlightColor;			// hightlight color
};

typedef VChannelEntry   VChannelTable[ Max_VChannels ];

/* ============================================================================ *
   VBitTable -- array of bits, one for each possible virtual channel.
 * ============================================================================ */

typedef BitSet<Max_VChannels> VBitTable;

/* ============================================================================ *
                               VelocityContour

	Up to 8 "Velocity Contours" are supported. Each contour is an envelope which
	can scale or compress the velocity information being processed through
	a virtual channel. (I could have decided to have a seperate velocity
	contour for each virtual channel, however I realized that in most situations,
	you would want to control the velocyt scaling of many virtual channels in
	parallel.
 * ============================================================================ */

//#define maxVContours	8

#endif /* __C_VChannelEntry_H__ */