/* ===================================================================== *
 * DestinationMonitorView.h (MeV/Midi)
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
 *		Dan Walton (dwalton)
 *
 * ---------------------------------------------------------------------
 * History:
 *	12/25/2000	cell
 *		Original implementation
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_DestinationMonitorView_H__
#define __C_DestinationMonitorView_H__

#include "ConsoleView.h"

class BMessageRunner;

namespace Midi
{

class CMidiDestination;
class CNoteMonitorView;
class CPitchBendMonitorView;
class CProgramChangeMonitorView;
class CControlChangeMonitorView;

class CDestinationMonitorView
	:	public CConsoleView
{

public:							// Constants

	enum messages
	{
								TICK = 'dmvA',

								/** Note On event
								 *	int8	"mev:note"		note number
								 *	int8	"mev:velocity"	attack velocity
								 */
								NOTE_ON,

								/** Note Off event
								 *	int8	"mev:note"		note number
								 *	int8	"mev:velocity"	release velocity
								 */
								NOTE_OFF,

								/** Program Change event
								 *	int16	"mev:bank"		program bank
								 *	int8	"mev:program"	program number
								 */
								PROGRAM_CHANGE,

								/** MIDI Pitch Bend event
								 *	int16	"mev:pitch"		pitch bend value
								 */
								PITCH_BEND,

								/** Control Change event
								 *	int8	"mev:control"	control number
								 *	int8	"mev:value"		control value (single-byte)
								 *	int16	"mev:value"		control value (two-byte)
								 */
								CONTROL_CHANGE
	};

public:							// Constructor/Destructor

								CDestinationMonitorView(
									BRect frame,
									CMidiDestination *destination);

	virtual						~CDestinationMonitorView();

public:							// Accessors

	CMidiDestination *			Destination() const
								{ return m_destination; }

public:							// CConsoleView Implementation

	virtual void				AttachedToWindow();

	virtual void				DetachedFromWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				Expanded(
									bool expanded);

	virtual void				GetPreferredSize(
									float *width,
									float *height);

	virtual void				MessageReceived(
									BMessage *message);

	virtual bool				SubjectReleased(
									CObservable *subject);

	virtual void				SubjectUpdated(
									BMessage *message);

private:						// Instance Data

	CMidiDestination *			m_destination;

	BMessageRunner *			m_messageRunner;

	CNoteMonitorView *			m_noteMeter;
								
	CProgramChangeMonitorView *	m_programMeter;

	CPitchBendMonitorView *		m_pitchBendMeter;

	CControlChangeMonitorView *	m_controllerMeter;
};

};

#endif /* __C_DestinationMonitorView_H__ */
