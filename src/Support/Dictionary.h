/* ===================================================================== *
 * Dictionary.h (MeV/Support)
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
 * ===========================================================================
 *		This template class implements an association list container,
 *	sometimes known as a map or dictionary. Data of type "valueType"
 *	is stored and retrieved using a key value of "keyType".
 *
 *		This class is implemented using dynamically growing hash table,
 *	so it should be pretty fast. Also, whenever an item is found, it moves
 *	that item to the head of the hash chain so it will be found faster
 *	the next time.
 *
 *		Operation of the class is simple, there are only 5 member
 *	functions:
 *		Value &Add( const Key &key, const Value &initializer );
 *		Value &Add( const Key &key );
 *		bool Clear( const Key &key );
 *		Value *operator[]( const Key &key );
 *		int32 Count( void );
 *
 *	Example use:
 *		CDictionary<char *,int> myDict;		// Declare dictionary object
 *		myDict.Add( "Talin", 12 );			// Add int "12" under key "Talin"
 *		int *result = myDict[ "Talin" ];	// Retrieve data via key.
 *
 *		The keys can be any type, however to get meaningful results,
 *	you will need to supply a comparator and a hash function. There
 *	are already such supplied for character pointers (char *) and
 *	integral types such as int. (Note that for character pointers
 *	the dictionary does not make a copy of the string! If this is
 *	a problem, I suggest using the C string class or something similar.)
 *
 *		To create your own comparator function, simply create
 *	a function called "KeyCompare" which takes two keys and
 *	returns a boolean value which returns true if they are equal.
 *	Here is the version for character pointers which is already
 *	defined.
 *		bool KeyCompare( const char *k1, const char *k2 )
 *		{
 *			return !strcmp( k1, k2 ); }
 *		}
 *
 *	To define your own hash function, simply create a function
 *	called "KeyHash" which takes a key and returns a hash value.
 *	Here is the version for character pointers which is already
 *	defined.
 *		uint32 KeyHash( const char *k )
 *		{
 *			uint32		v = 0;
 *
 *			while (*k) v = v * 13 + *k++;
 *			return v;
 *		}
 *
 *		There is also an iterator template, which iterates through
 *	all the data elements in order by hash code. However, at the time
 *	of this writing, it has not been tested.
 *
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

#ifndef __C_Dictionary_H__
#define __C_Dictionary_H__

// Gnu C Library
#include <malloc.h>
// Kernel Kit
#include <OS.h>

// ---------------------------------------------------------------------------
//	A real class to base the dictionary template class on

struct LinkBase {
	LinkBase	*next;
	uint32		hashVal;
};

class CDictionaryBase {
protected:

	friend class CDictionaryIteratorBase;

	typedef uint32	HVAL;				// Hash key type

	uint32			numEntries,
					hashSize;
	LinkBase		**hashTable;

	bool GrowHash( void );

	void Link( LinkBase *lp );
	void _Link( LinkBase *lp );

	LinkBase **head( HVAL h ) const { return &hashTable[ h % hashSize ]; }

	CDictionaryBase( void )
	{
		numEntries = hashSize = 0;
		hashTable = NULL;
	}
	~CDictionaryBase()
	{
		if (hashTable) free( hashTable );
	}
};

// ---------------------------------------------------------------------------
//	A real class to base the dictionary iterator template class on

class CDictionaryIteratorBase {
protected:
	CDictionaryBase	&b;
	LinkBase			*node;
	uint32			hashVal;

	CDictionaryIteratorBase( CDictionaryBase &db ) : b( db )
	{
		hashVal = 0;
		node = NULL;
	}

	LinkBase *First() { hashVal = 0; node = NULL; return Next(); }
	LinkBase *Next();
};

// ---------------------------------------------------------------------------
//	General template class to calculate the hash values for a given
//	key item

template<class Key>
uint32 KeyHash( const Key &k )
{
	return (uint32)k;
}

template<class Key>
bool KeyCompare( const Key &k1, const Key &k2 )
{
	return k1 == k2;
}

// ---------------------------------------------------------------------------
//	Special template class to calculate hash values for strings.

typedef char * CharPtr;

inline uint32 KeyHash( const CharPtr &k )
{
	char		*p = k;
	uint32		v = 0;

	while (*p) v = v * 13 + *p++;
	return v;
}

inline bool KeyCompare( const CharPtr &k1, const CharPtr &k2 )
{
	return !strcmp( k1, k2 );
}

/** ===========================================================================
		This template class implements an association list container,
	sometimes known as a map or dictionary. Data of type "valueType"
	is stored and retrieved using a key value of "keyType".

		This class is implemented using dynamically growing hash table,
	so it should be pretty fast. Also, whenever an item is found, it moves
	that item to the head of the hash chain so it will be found faster
	the next time.

		Operation of the class is simple, there are only 5 member
	functions:<pre>
		Value &Add( const Key &key, const Value &initializer );
		Value &Add( const Key &key );
		bool Clear( const Key &key );
		Value *operator[]( const Key &key );
		int32 Count( void );</pre>

	Example use:<pre>
		CDictionary&ltchar *,int&gt myDict;	// Declare dictionary object
		myDict.Add( "Talin", 12 );			// Add int "12" under key "Talin"
		int *result = myDict[ "Talin" ];	// Retrieve data via key.</pre>

		The keys can be any type, however to get meaningful results,
	you will need to supply a comparator and a hash function. There
	are already such supplied for character pointers (char *) and
	integral types such as int. (Note that for character pointers
	the dictionary does not make a copy of the string! If this is
	a problem, I suggest using the C string class or something similar.)

		To create your own comparator function, simply create
	a function called "KeyCompare" which takes two keys and
	returns a boolean value which returns true if they are equal.
	Here is the version for character pointers which is already
	defined.<pre>
		bool KeyCompare( const char *k1, const char *k2 )
		{
			return !strcmp( k1, k2 ); }
		}
		</pre>

	To define your own hash function, simply create a function
	called "KeyHash" which takes a key and returns a hash value.
	Here is the version for character pointers which is already
	defined.<pre>
		uint32 KeyHash( const char *k )
		{
			uint32		v = 0;

			while (*k) v = v * 13 + *k++;
			return v;
		}</pre>

		There is also an iterator template, which iterates through
	all the data elements in order by hash code. However, at the time
	of this writing, it has not been tested.
*/

template<class _Key, class _Value> class CDictionary : public CDictionaryBase {
private:

		//	Embedded class to define an association between key and map
	class Link : public LinkBase {
	public:
		_Key			k;
		_Value		v;

		Link( const _Key &ik, const _Value &iv ) : k(ik), v(iv) {;}
		Link( const _Key &ik ) : k(ik) {;}
	};
	typedef Link	*LinkPtr;

		//	Upcast routine to get head of link chain.
	LinkPtr *head( HVAL h ) const
		{ return (LinkPtr *)CDictionaryBase::head( h ); }

public:
	~CDictionary()
	{
			//	Delete all entries from the dictionary
		for (uint32 i = 0; i < hashSize; i++)
		{
			LinkPtr	*hp = head( i ),
					lp;
			
			while ((lp = *hp) != NULL)
			{
				*hp = (LinkPtr)lp->next;
				//Hash<Key>::free( lp->k );
				delete lp;
				numEntries--;
			}
		}
	}

		/**	Add an entry to the dictionary and initialize it to a given value
			@param	inKey	The key of the new item
			@param	inVal	The value to be stored under this key
			@return	A reference to the new value item.
		*/
	_Value &Add( const _Key &inKey, const _Value &inVal )
	{
		LinkPtr		lp = new Link( inKey, inVal );

		lp->hashVal = KeyHash( inKey );
		CDictionaryBase::Link( lp );
		numEntries++;
		return lp->v;
	}

		/**	This function adds a "blank" value (i.e. initialized with
			the default constructor).
			@param	inKey	The key of the new item
			@return	A reference to the new value item.
		*/
	_Value &Add( const _Key &inKey )
	{
		LinkPtr		lp = new Link( inKey );

		lp->hashVal = KeyHash( inKey );
		CDictionaryBase::Link( lp );
		numEntries++;
		return lp->v;
	}

		/**	This function removes a key from the dictionary */
	bool Clear( const _Key &inKey )
	{
		HVAL		h = KeyHash( inKey );
		if (hashTable == NULL) return false;
		LinkPtr		*hp = head( h ),
					*lp;

		for (lp = hp; *lp; lp = (LinkPtr *)&(*lp)->next )
		{
			if (	(*lp)->hashVal == h
				&&	KeyCompare( inKey, (*lp)->k ))
			{
				LinkPtr	l = *lp;

				*lp = (LinkPtr)l->next;
				//Hash<Key>::free( l->k );
				delete l;
				numEntries--;
				return TRUE;
			}
		}
		return FALSE;
	}

		/**	This function removes all keys from the dictionary */
	void MakeEmpty()
	{
		if (hashTable == NULL) return;
		
		for (uint32 i = 0; i < hashSize; i++)
		{
			LinkPtr	p;
			
			while (hashTable[ i ])
			{
				p = (LinkPtr)hashTable[ i ];
				hashTable[ i ] = p->next;
				delete p;
			}
		}
		delete hashTable;
		hashTable = NULL;
		hashSize = 0;
	}
	
		/**	Retrieve an element from a dictionary given a key value.

			@return	Returns a pointer to the value item, or NULL if
					the indicated key was not found.
		*/
	_Value *operator[]( const _Key &k ) const
	{
		HVAL		h = KeyHash( k );
		if (hashTable == NULL) return NULL;
		LinkPtr		*hp = (LinkPtr *)head( h ),
					*lp;

		for (lp = hp; *lp; lp = (LinkPtr *)&(*lp)->next )
		{
			if (	(*lp)->hashVal == h
				&&	KeyCompare( k, (*lp)->k ))
			{
				LinkPtr	l = *lp;

				*lp = (LinkPtr)l->next;
				l->next = *hp;
				*hp = l;
				return &l->v;
			}
		}
		return NULL;
	}

		/** Returns the number of items stored in the dictionary */
	int32 Count() { return numEntries; }

		/** Iterator class for dictionary. */
	class Iterator : private CDictionaryIteratorBase {
	public:
			/** Constructor */
		Iterator( CDictionaryBase &db ) : CDictionaryIteratorBase( db ) {;}
		
			/** Returns a pointer to the current Key. */
		const _Key *Key() { return node ? &((LinkPtr)node)->k : NULL; }

			/** Returns a pointer to the current value */
		_Value *Value() { return node ? &((LinkPtr)node)->v : NULL; }
		
			/** Returns the first key. */
		_Key *First()
		{
			if (CDictionaryIteratorBase::First()) return &((LinkPtr)node)->k;
			return NULL;
		}

			/** Returns the next key in the iteration. */
		_Key *Next()
		{
			if (CDictionaryIteratorBase::Next()) return &((LinkPtr)node)->k;
			return NULL;
		}
	};
};

#endif /* __C_Dictionary_H__ */
