/* ===================================================================== *
 * StripFrameView.h (MeV/UI)
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
 *  Frame which contains scrolling strips
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	10/08/2000	cell
 *		Merged functionality of the CTrackEditFrame subclass into this
 *		class. Added serialization caps.
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_StripFrameView_H__
#define __C_StripFrameView_H__

#include "Scroller.h"
#include "TimeUnits.h"

// Support Kit
#include <List.h>
#include <String.h>

class CIFFReader;
class CIFFWriter;
class CStripSplitter;
class CStripView;
class CTrack;

class CStripFrameView
	:	public CScrollerTarget
{

public:							// Constants

	static const int32			FILE_CHUNK_ID;

public:							// Constructor/Destructor

								CStripFrameView(
									BRect frame,
									char *name,
									CTrack *track,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP);

								~CStripFrameView();

public:							// Accessors

	/** Get the clock type being viewed. */
	TClockType					ClockType() const
								{ return m_clockType; }
	void						SetClockType(
									TClockType type);

	BPoint						FrameSize() const
								{ return BPoint(Frame().Width() - 14.0 - 20.0,
												Frame().Height()); }

	CScrollerTarget *			Ruler() const
								{ return m_ruler; }
	void						SetRuler(
									CScrollerTarget *ruler)
								{ m_ruler = ruler; }

	CTrack *					Track() const
								{ return m_track; }

	int32						ZoomValue() const
								{ return m_horizontalZoom; }

	float						MinimumHeight() const;

public:							// Operations

	void						AddType(
									BString name,
									BBitmap *icon = NULL);
	int32						CountTypes() const
								{ return m_types.CountItems(); }
	status_t					GetIconForType(
									int32 index,
									BBitmap **outIcon) const;
	BString						TypeAt(
									int32 index) const;

	bool						AddStrip(
									CStripView *view,
									float proportion = 0.0,
									int32 index = -1,
									bool fixedSize = false);									
	int32						CountStrips() const
								{ return m_strips.CountItems(); }
	int32						IndexOf(
									CStripView *view) const;
	void						PackStrips();
	bool						RemoveStrip(
									CStripView *view);
	CStripView *				StripAt(
									int32 index) const;
	void						SwapStrips(
									CStripView *strip1,
									CStripView *strip2);

	/**	Convert time interval into x-coordinate. */
	float						TimeToViewCoords(
									long time,
									TClockType clockType) const;

	/**	Convert pixel x-coordinate into time interval. */
	long						ViewCoordsToTime(
									float x,
									TClockType clockType) const;
	
	/** Adjust the scroll range of this frame to match track. */
	void						RecalcScrollRange();
	
	void						ZoomBy(
									int32 diff);

public:							// Serialization

	virtual void				ExportSettings(
									BMessage *settings) const;

	virtual void				ImportSettings(
									const BMessage *settings);

	static void					ReadState(
									CIFFReader &reader,
									BMessage *windowSettings);

	static void					WriteState(
									CIFFWriter &writer,
									const BMessage *windowSettings);

public:							// CScrollerTarget Implementation

	virtual void				AttachedToWindow();

	virtual void				FrameResized(
									float width,
									float height);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				SetScrollValue(
									float position,
									orientation posture);

protected:						// Internal Operations

	void						ArrangeViews();

	void						UpdateProportions();

	void						UpdateSplitters();

protected:						// Instance Data

	CTrack *					m_track;

	// contains strip_info objects
	BList						m_strips;

	// a list of available strips
	BList						m_types;

	// Optional horizontal ruler frame
	CScrollerTarget *			m_ruler;

	TClockType					m_clockType;

	float						m_pixelsPerTimeUnit;

	int32						m_horizontalZoom;

private:						// Internal Types

	struct strip_type
	{
		BString				name;
		BBitmap *			icon;
	};

	struct strip_info
	{
		CStripView *		strip;
		BView *				container;
		CStripSplitter *	splitter;		// splitter above the strip
		float				vertical_offset;
		float				height;
		float				proportion;		// Ideal proportion
		bool				fixed_size;		// true if size of item is fixed
	};
};

#endif /* __C_StripFrameView_H__ */
