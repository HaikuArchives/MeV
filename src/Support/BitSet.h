/* ===================================================================== *
 * BitSet.h (MeV/User Interface)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Netscape Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/NPL/
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
 *  A template class representing an array of bits
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */


	//*	A class to manage a fixed-size bit array
template<int N> class BitSet {

	enum {
		size = (N + 31) / 32
	};

	long			bits[ size ];

public:
		//*	Clear all bits
	void Clear() { memset( bits, 0, sizeof bits ); }

		//*	Default constructor, sets all bits to zero
	BitSet() { Clear(); }

		//*	Copy constructor
	BitSet( const BitSet &a ) { memcpy( bits, a.bits, sizeof bits ); }
	
		//*	Set a bit
	void Set( int bit )
		{	bits[ bit >> 5 ] |= (1 << ((~bit) & 0x1f)); }

		//*	Clear a bit
	void Reset( int bit )
		{	bits[ bit >> 5 ] &= ~(1 << ((~bit) & 0x1f)); }
		
		//*	Set a bit to a value
	void Set( int bit, int val )
		{	if (val) Set( bit ); else Reset( bit ); }

		//*	Return the state of the Nth bit
	int Test( int bit ) const
		{	return bits[ bit >> 5 ] & (1 << ((~bit) & 0x1f));	}
	
		//*	Return the state of the Nth bit
	int operator[]( int bit ) const
		{	return bits[ bit >> 5 ] & (1 << ((~bit) & 0x1f));	}

		//*	The '&amp;' operator
	BitSet operator&( const BitSet &a )
	{
		BitSet	result;
	
		for (int i = 0; i < size; i++)
		{	
			result.bits[ i ] = a.bits[ i ] & bits[ i ];
		}
		return result;
	}

		//*	The '|' operator
	BitSet operator|( const BitSet &a )
	{
		BitSet	result;
	
		for (int i = 0; i < size; i++)
		{	
			result.bits[ i ] = a.bits[ i ] | bits[ i ];
		}
		return result;
	}

		//*	The '^' operator
	BitSet operator^( const BitSet &a )
	{
		BitSet	result;
	
		for (int i = 0; i < size; i++)
		{	
			result.bits[ i ] = a.bits[ i ] ^ bits[ i ];
		}
		return result;
	}

		//*	The '~' operator
	BitSet operator~( void )
	{
		BitSet	result;
	
		for (int i = 0; i < size; i++)
		{	
			result.bits[ i ] = ~bits[ i ];
		}
		return result;
	}

		//*	The '==' operator
	int operator==( const BitSet &a )
	{
		return memcmp( a.bits, bits, sizeof bits ) ? FALSE : TRUE;
	}

		//*	The '!=' operator
	int operator!=( const BitSet &a )
	{
		return memcmp( a.bits, bits, sizeof bits ) ? TRUE : FALSE;
	}

		//*	The '&amp;=' operator
	void operator&= (const BitSet &a)
	{
		for (int i = 0; i < size; i++) bits[ i ] &= a.bits[ i ];
	}

		//*	The '|=' operator
	void operator|= (const BitSet &a)
	{
		for (int i = 0; i < size; i++) bits[ i ] |= a.bits[ i ];
	}

		//*	The '^=' operator
	void operator^= (const BitSet &a)
	{
		for (int i = 0; i < size; i++) bits[ i ] ^= a.bits[ i ];
	}
};
