/* ===================================================================== *
 * Dictionary.cpp (MeV/Support)
 * ===================================================================== */

#include "Dictionary.h"

bool CDictionaryBase::GrowHash( void )
{
	LinkBase		**newTable;
	uint32			newSize;

		//	An interesting factoid is that 2*(4^n)-1 is prime for small
		//	values of n. (at least, for any value of N I can calculate!)

		//	Calculate a prime number larger than numEntries / 2
	for (newSize = 2; newSize < 2048; newSize <<= 2)
	{
		if (newSize > numEntries/2) break;
	}
	newSize--;

	if (newSize == hashSize) return TRUE;

		//	Allocate the new table, and zero the memory
	if ((newTable = (LinkBase**)calloc( newSize, sizeof (LinkBase*) )) == NULL)
		return FALSE;

		//	Now, we've got to re-index the keys
	if (hashTable && hashSize > 0)
	{
		for (int i = 0; i < (int)hashSize; i++)
		{
			LinkBase	*l, *next;

			for (l = hashTable[ i ]; l; l = next )
			{
				LinkBase	**h = &newTable[ l->hashVal % newSize ];

				next = l->next;
				l->next = *h; *h = l;
			}
		}

			//	And free the old table
		free( hashTable );
	}

	hashSize = newSize;
	hashTable = newTable;
	return TRUE;
}

void CDictionaryBase::_Link( LinkBase *l )
{
	LinkBase	**h = &hashTable[ l->hashVal % hashSize ];

	l->next = *h; *h = l;
}

void CDictionaryBase::Link( LinkBase *l )
{
	if (	hashTable == NULL
		||	numEntries > hashSize * 4
		||	numEntries < hashSize / 2)
	{
			//	It doesn't matter if it failed, we can deal with it.
		GrowHash();

			//	If there's NO hashtable, we're in trouble!!!
		if (hashTable == NULL)
		{
//			throw ???
		}
	}

	_Link( l );
}

LinkBase *CDictionaryIteratorBase::Next( void )
{
	if (node) node = node->next;
	while (node == NULL)
	{
		if (hashVal >= b.hashSize || b.hashTable == NULL) return NULL;
		node = b.hashTable[ hashVal++ ];
	}

	return node;
}

// END - Dictionary.cpp
