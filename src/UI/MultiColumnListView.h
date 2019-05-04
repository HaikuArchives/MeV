/* ===================================================================== *
 * MultiColumnListView.h (MeV/User Interface)
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
 * History:
 *	1997		Talin
 *	Original implementation
 *	04/08/2000	cell
 *	General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_MultiColumnListView_H__
#define __C_MultiColumnListView_H__

#include <ListView.h>
#include <ListItem.h>

/**
 *		Defines a column in a multi-column list.
 *		@author	Talin, Christoper Lenz.
 */

class CMultiColumnListView;
class CMultiColumnListItem;

class CColumnField {
	friend class CMultiColumnListView;
	friend class CMultiColumnListItem;
	friend class CChildListView;

protected:
	CMultiColumnListView	&list;
	int16			minWidth,
					elasticity,
					actualWidth,
					fieldIndex;
	bool				useEllipses,
					useHighlight,
					selectable,
					draggable;
	const char			*title;
	alignment		align;

	int32 Justify( int32 totalWidth, int32 contentWidth );
	void DrawString( BView *drawView, BRect bounds, const char *str );

protected:

		/**	Function to draw one cell. */
	virtual void Draw(	CMultiColumnListItem	*item,
						BView				*drawview,
						BRect 				bounds,
						bool					complete = false ) = 0;

public:

		/**	Constructor */
	CColumnField(	CMultiColumnListView	&inList,
					int32				inMinWidth,
					int32				inElasticity,
					const char				*inTitle );

		/**	Change the justification of this columns contents. */
	void SetAlignment( alignment inAlign ) { align = inAlign; }

		/**	Set whether this column is draggable, i.e. don't return
			the column click until we know a drag has occured.
		*/
	void SetDraggable( bool in ) { draggable = in; }

	int32 Width() { return actualWidth; }
};

/** ---------------------------------------------------------------------------
	A column in a multi-column list that displays numeric data.
*/

class CNumericColumnField : public CColumnField {
	void Draw(	CMultiColumnListItem	*item,
				BView				*drawview,
				BRect 				bounds,
				bool					complete = false );
public:
		/**	Constructor */
	CNumericColumnField(	CMultiColumnListView	&inList,
						int32				inBaseWidth,
						int32				inElasticity,
						const char				*inTitle )
		: CColumnField( inList, inBaseWidth, inElasticity, inTitle ) {}
};

/** ---------------------------------------------------------------------------
	A column in a multi-column list that displays string data.
*/

class CStringColumnField : public CColumnField {
	void Draw(	CMultiColumnListItem	*item,
				BView				*drawview,
				BRect 				bounds,
				bool					complete = false );

public:
		/**	Constructor */
	CStringColumnField(	CMultiColumnListView	&inList,
						int32				inBaseWidth,
						int32				inElasticity,
						const char				*inTitle )
		: CColumnField( inList, inBaseWidth, inElasticity, inTitle ) {}
};

/** ---------------------------------------------------------------------------
	A column in a multi-column list that displays boolean data.
*/

class CCheckmarkColumnField : public CColumnField {

	void Draw(	CMultiColumnListItem	*item,
				BView				*drawview,
				BRect 				bounds,
				bool					complete = false );

public:
		/**	Constructor */
	CCheckmarkColumnField(	CMultiColumnListView	&inList,
							int32				inBaseWidth,
							int32				inElasticity,
							const char				*inTitle )
		: CColumnField( inList, inBaseWidth, inElasticity, inTitle )
	{
		selectable = false;
	}
};

/** ---------------------------------------------------------------------------
	A column in a multi-column list that displays RGB data.
*/

class CColorSwatchColumnField : public CColumnField {
	void Draw(	CMultiColumnListItem	*item,
				BView				*drawview,
				BRect 				bounds,
				bool					complete = false );

public:
		/**	Constructor */
	CColorSwatchColumnField(	CMultiColumnListView	&inList,
							int32				inBaseWidth,
							int32				inElasticity,
							const char				*inTitle )
		: CColumnField( inList, inBaseWidth, inElasticity, inTitle ) {}
};

/** ---------------------------------------------------------------------------
	An item in a multi-column list.
*/

class CMultiColumnListItem : public BListItem {
protected:
	void					*rowData;

	void DrawItem(	BView *owner,
					BRect bounds,
					bool complete = false);
public:
		/**	Constructor */
	CMultiColumnListItem( void *inData ) : BListItem()
	{
		rowData = inData;
	}

	CMultiColumnListItem( void *inData, int32 inLevel, bool inExpanded = false )
		: BListItem( inLevel, inExpanded )
	{
		rowData = inData;
	}

		/**	Get the integer value of the Nth field. */
	virtual int32 GetFieldIntData( int32 inIndex ) { return 0; }

		/**	Get the string value of the Nth field. */
	virtual const char *GetFieldStringData( int32 inIndex ) { return NULL; }

		/**	Get the binary pointer value of the Nth field. */
	virtual void *GetFieldData( int32 inIndex ) { return NULL; }

		/**	Get pointer to row data. */
	void *RowData() { return rowData; }
};

/** ---------------------------------------------------------------------------
	A list view with multiple columns.
*/

class CMultiColumnListView : public BView {
	friend class CMultiColumnListItem;
	friend class CColumnField;
	friend class CChildListView;

	BList				columns;
	BListView			*listView;
	BScrollView			*scrollView;
	BMessage				*columnMsg;
	int32				labelHeight,
						labelBaseline;
	font_height			fh;
	int32				clickColumn;
	int32				clickRow;
	bool					stayFocused;

	void Draw( BRect inUpdateRect );
	void AttachedToWindow()
	{
		BView::AttachedToWindow();
		listView->SetViewColor( B_TRANSPARENT_32_BIT );
		if (Parent()) SetViewColor( Parent()->ViewColor() );
		Layout();
	}
	void FrameResized(float w,float h)
	{
		BView::FrameResized( w, h );
		Layout();
	}
	void Layout();

public:
		/**	Constructor */
	CMultiColumnListView(
		BRect			frame,
		const char		*name,
		list_view_type	type = B_SINGLE_SELECTION_LIST,
		uint32			resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32			flags = B_WILL_DRAW | B_FRAME_EVENTS );

		/**	Destructor -- deletes all column information. */
	~CMultiColumnListView();

		/**	Returns which column corresponds to this x-coordinate. */
	int32 PickColumn( int32 inX );

		/**	returns true if this drag message is acceptable. Override
			in subclass. All this does is to tell the list to hide the
			dragged item and provide an insertion cursor. */
	virtual bool IsDragAcceptable( const BMessage *inMsg ) { return false; }

		/** Handle an item dropped on the list. */
	virtual void OnDrop( BMessage *inMsg, int32 inIndex ) {}

		/**	StayFocused -- TRUE if item should retain focus after operations.
			Defaults to true.
		*/
	void StayFocused( bool in ) { stayFocused = in; }

	void Invalidate()
	{
		BView::Invalidate();
		listView->Invalidate();
	}

#if 0
virtual	void			MouseDown(BPoint where);
virtual	void			KeyDown(const char *bytes, int32 numBytes);
#endif

	void	 MakeFocus( bool state = TRUE ) { listView->MakeFocus( state ); }

#if 0
virtual	void			FrameResized(float newWidth, float newHeight);
#endif

	virtual bool	AddItem( CMultiColumnListItem *item )
		{ return listView->AddItem( item ); }
	virtual bool AddItem( CMultiColumnListItem *item, int32 atIndex)
		{ return listView->AddItem( item, atIndex ); }
	virtual bool	AddList( BList *newItems )
		{ return listView->AddList( newItems ); }
	virtual bool AddList( BList *newItems, int32 atIndex )
		{ return listView->AddList( newItems, atIndex ); }
	virtual bool RemoveItem( CMultiColumnListItem *item )
		{ return listView->RemoveItem( item ); }
	virtual BListItem *RemoveItem(int32 index)
		{ return listView->RemoveItem( index ); }
	virtual bool RemoveItems(int32 index, int32 count)
		{ return listView->RemoveItems( index, count ); }

	virtual void SetSelectionMessage( BMessage *message )
		{ listView->SetSelectionMessage( message ); }
	virtual void SetInvocationMessage( BMessage *message )
		{ listView->SetInvocationMessage( message ); }
	virtual void SetColumnClickMessage( BMessage *message )
	{
		delete columnMsg;
		columnMsg = message;
	}

	BMessage *SelectionMessage() const { return listView->SelectionMessage(); }
	uint32 SelectionCommand() const { return listView->SelectionCommand(); }
	BMessage *InvocationMessage() const { return listView->InvocationMessage(); }
	uint32 InvocationCommand() const { return listView->InvocationCommand(); }

	virtual void SetListType( list_view_type type )
		{ listView->SetListType( type ); }
	list_view_type ListType() const { return listView->ListType(); }

	BListItem *ItemAt(int32 index) const { return listView->ItemAt(index); }
	int32 IndexOf(BPoint point) const { return listView->IndexOf(point); }
	int32 IndexOf(BListItem *item) const { return listView->IndexOf(item); }
	BListItem *FirstItem() const { return listView->FirstItem(); }
	BListItem *LastItem() const { return listView->LastItem(); }
	bool	 HasItem(BListItem *item) const { return listView->HasItem(item); }
	int32 CountItems() const { return listView->CountItems(); }
	virtual	void MakeEmpty() { listView->MakeEmpty(); }
	bool IsEmpty() const { return listView->IsEmpty(); }
	void DoForEach(bool (*func)(BListItem *))
		{ listView->DoForEach( func ); }
	void DoForEach(bool (*func)(BListItem *, void *), void *d)
		{ listView->DoForEach( func,d ); }
	const BListItem **Items() const { return listView->Items(); }
	void InvalidateItem(int32 index) { listView->InvalidateItem(index); }
	void ScrollToSelection() { listView->ScrollToSelection(); }

	BRect ItemFrame(int32 index)
	{
		BRect	r( listView->ItemFrame( index ) );
		r.OffsetBy( 0.0, labelHeight + 2 );
		return r;
	}

	void Select(int32 index, bool extend = FALSE)
		{ listView->Select( index, extend ); }
	void Select(int32 from, int32 to, bool extend = FALSE)
		{ listView->Select( from, to, extend ); }
	bool IsItemSelected(int32 index) const
		{ return listView->IsItemSelected(index); }
	int32 CurrentSelection(int32 index = 0) const
		{ return listView->CurrentSelection(index); }
// virtual status_t Invoke(BMessage *msg = NULL);

	void DeselectAll()
		{ listView->DeselectAll(); }
	void DeselectExcept(int32 except_from, int32 except_to)
		{ listView->DeselectExcept(except_from, except_to); }
	void Deselect(int32 index)
		{ listView->Deselect(index); }

// virtual void SelectionChanged() { listView->DeselectAll(); }

	void SortItems(int (*cmp)(const void *, const void *))
		{ listView->SortItems( cmp ); }
};

inline void DeleteListItems( CMultiColumnListView *inView )
{
	BListItem		*listItem;

	while ((listItem = inView->RemoveItem( (long)0 ))) delete listItem;
}

#endif /* __C_MultiColumnListView_H__ */
