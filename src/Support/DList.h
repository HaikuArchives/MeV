/* ===================================================================== *
 * DList.h (MeV/Support)
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
 *  Doubly-linked list classes
 *
 *	DNode is a base class for doubly-linked list nodes.
 *	It is to be used in conjunction with the DList class, which represents
 *	the head of the list.
 *
 *	Each DNode has a pointer to the next and the previous node.
 *
 *	DNodes are "invasive" which means in order to use them,
 *	you have to incorporate them into a structure or class.
 *	"Invasive" lists are faster and require less memory
 *	than "non-invasive" ones which don't require changing
 *	the structures that are placed on a list.
 *
 *	To use a DNode, simply make it the base class of any
 *	structure or class which is to be added to a DList.
 *	
 *	DList is a double-linked list. It can be used by itself or
 *	as a member structure. Using DLists is very fast and efficient.
 *
 *	Note that DLists use the "Amiga-style" linked lists which
 *	have three pointers, a pointer to the first node, a "dummy"
 *	pointer which makes the list header look like both the
 *	first and last node of the list, and a pointer to the
 *	last node in the list. The dummy pointer is a clever trick
 *	which is too complex to explain here.
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

#ifndef __C_DList_H__
#define __C_DList_H__

// Kernel Kit
#include <OS.h>

class DNode
{
	friend class DList;

public:							// Constructor/Destructor

	// Constructor links DNode to itelf to allow removal even when on "no list"
								DNode()
								{
									m_succ = m_pred = this;
								}

public:							// Accessors

	// Return the pointer to the next node in the list, 
	// or NULL if this is the last node.
	DNode *						Next() const
								{
									return m_succ->m_succ ? m_succ : NULL;
								}

	// Return the pointer to the previous node in the list, 
	// or NULL if this is the first node.
	DNode *						Prev() const
								{
									return m_pred->m_pred ? m_pred : NULL;
								}

public:							// Operations

	// Remove this node from the list.
	DNode *						Remove();

	// Insert a new node previous to this one.
	DNode *						InsertBefore(
									DNode *newNode);

	// Insert a new node just after this one.
	DNode *						InsertAfter(
									DNode *newNode);

private:						// Instance Data

	// ptr to next in list
	DNode						*m_succ;
	// ptr to prev in list
	DNode						*m_pred;


};

class DList
{

public:							// Constructor/Destructor

								DList()
								{
									m_head = (DNode *)&m_overlap;
									m_tail = (DNode *)&m_head;
									m_overlap = NULL;
								}

public:							// Accessors

	// Test if list is empty
	bool						Empty() const
								{
									return m_head->m_succ == NULL;
								}
	
	// Count number of nodes in the list
	int32						Count() const;

	// Return pointer to first node in list, or NULL if empty list
	DNode *						First() const
								{
									return m_head->m_succ ? m_head : NULL;
								}

	// Return pointer to last node in list, or NULL if empty list
	DNode *						Last() const
								{
									return m_tail->m_pred ? m_tail : NULL;
								}
	
public:							// Operations

	// Add node to head of list
	void						AddHead(
									DNode *node);

	// Add node to tail of list
	void						AddTail(
									DNode *node);
	
	// Remove the first node of the list and return it, or NULL if empty list
	DNode *						RemHead();

	// Remove the last node of the list and return it, or NULL of empty list
	DNode *						RemTail();

	// Select the Nth node in the list.
	DNode *						Select(
									int32 index) const;

private:						// Instance Data

	DNode						*m_head;

	DNode						*m_overlap;

	DNode						*m_tail;
};

#endif /* __C_DList_H__ */
