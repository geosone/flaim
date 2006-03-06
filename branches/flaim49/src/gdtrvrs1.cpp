//-------------------------------------------------------------------------
// Desc:	GEDCOM tree traversal routines.
// Tabs:	3
//
//		Copyright (c) 1990-1993,1996-2000,2003,2005-2006 Novell, Inc. All Rights Reserved.
//
//		This program is free software; you can redistribute it and/or
//		modify it under the terms of version 2 of the GNU General Public
//		License as published by the Free Software Foundation.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//		GNU General Public License for more details.
//
//		You should have received a copy of the GNU General Public License
//		along with this program; if not, contact Novell, Inc.
//
//		To contact Novell about this file by physical or electronic mail,
//		you may find current contact information at www.novell.com
//
// $Id: gdtrvrs1.cpp 12308 2006-01-19 15:08:11 -0700 (Thu, 19 Jan 2006) dsanders $
//-------------------------------------------------------------------------

#include "flaimsys.h"

/****************************************************************************
Desc:		Returns a pointer to the sibling node that is after the input node.
			NULL is returned if there is not a next sibling.
****************************************************************************/
NODE * GedSibNext(
	NODE *		node)
{
	FLMUINT	lev;

	if( node)
	{
		lev = GedNodeLevel( node);
		while(															/*skip children*/
			((node = node->next) != NULL) &&
			(GedNodeLevel( node) > lev)
		);
	}

	return(
		(node && (GedNodeLevel( node) == lev))
		?	node
		:	NULL);
}

/****************************************************************************
Desc:		Returns a pointer to the parent of the input node.
			NULL is returned if there is not a parent.
****************************************************************************/
NODE * GedParent(
		NODE *		node)
{
	if( node)
	{
		FLMUINT	lev = GedNodeLevel( node);
		while(															/*skip nephews & siblings*/
			((node = node->prior) != NULL) &&
			(GedNodeLevel( node) >= lev)
		);
	}
	return( node);
}

/****************************************************************************
Desc:		Returns a pointer to the child of the input node.
			NULL is returned if there is not a child.
****************************************************************************/
NODE * GedChild(
	NODE *		node)
{
	return(
		node &&
		node->next &&
		(GedNodeLevel( node->next) > GedNodeLevel( node))
		?	node->next
		:	NULL
	);
}

/****************************************************************************
Desc:		Returns a pointer to the previous sibling of the input node.
****************************************************************************/
NODE * GedSibPrev(
	NODE *		node)
{
	FLMUINT	lev;

	if( node)
	{
		lev = GedNodeLevel( node);
		while(										/* skip nephews */
			((node = node->prior) != NULL) &&
			(GedNodeLevel( node) > lev)
		);
	}
	return(
		(node && (GedNodeLevel( node) == lev))
		?	node
		:	NULL
	);
}



