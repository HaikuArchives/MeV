/* ===================================================================== *
 * DList.cpp (MeV/Support)
 * ===================================================================== */

#include "DList.h"

// ---------------------------------------------------------------------------
// DList Accessors

int32
DList::Count() const
{
	DNode *node;
	int32 count;

	for (node = m_head, count = -1; node; node = node->m_succ)
	{
		count++;
	}

	return count;
}
	
// ---------------------------------------------------------------------------
// DList Operations

void
DList::AddHead(
	DNode *node)
{
	node->m_succ = m_head;
	node->m_pred = (DNode *)&m_head;
	m_head->m_pred = node;
	m_head = node;
}

void
DList::AddTail(
	DNode *node)
{
	node->m_pred = m_tail;
	node->m_succ = (DNode *)&m_overlap;
	m_tail->m_succ = node;
	m_tail = node;
}

DNode *
DList::RemHead()
{
	DNode *node = m_head;

	if (node->m_succ)
	{
		m_head = node->m_succ;
		m_head->m_pred = (DNode *)&m_head;

		//	Nodes can be unlinked indefinately without crashing.
		node->m_succ = node->m_pred = node;
		return node;
	}
	return NULL;
}

DNode *
DList::RemTail()
{
	DNode *node = m_tail;

	if (node->m_pred)
	{
		m_tail = node->m_pred;
		m_tail->m_succ = (DNode *)&m_overlap;

		//	Nodes can be unlinked indefinately without crashing.
		node->m_succ = node->m_pred = node;
		return node;
	}
	return NULL;
}

DNode *
DList::Select(
	int32 index) const
{
	if (index < 0)
	{
		return NULL;
	}

	DNode *node;

	for (node = m_head; node->m_succ; node = node->m_succ)
	{
		if (index <= 0)
		{
			return NULL;
		}
		index--;
	}

	return node;
}

// ---------------------------------------------------------------------------
// DNode Operations

DNode *
DNode::Remove()
{
	m_succ->m_pred = m_pred;
	m_pred->m_succ = m_succ;

	//	Nodes can be unlinked indefinately without crashing.
	m_succ = m_pred = this;

	return this;
}

DNode *
DNode::InsertBefore(
	DNode *newNode)
{
	newNode->m_succ = this;
	newNode->m_pred = m_pred;

	m_pred->m_succ = newNode;
	m_pred = newNode;

	return newNode;
}

DNode *
DNode::InsertAfter(
	DNode *newNode)
{
	newNode->m_succ = m_succ;
	newNode->m_pred = this;

	m_succ->m_pred = newNode;
	m_succ = newNode;

	return newNode;
}
