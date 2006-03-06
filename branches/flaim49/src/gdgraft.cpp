//-------------------------------------------------------------------------
// Desc:	Graft a node into a GEDCOM tree.
// Tabs:	3
//
//		Copyright (c) 1990-1993,1995-2000,2003,2005-2006 Novell, Inc. All Rights Reserved.
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
// $Id: gdgraft.cpp 12307 2006-01-19 15:06:34 -0700 (Thu, 19 Jan 2006) dsanders $
//-------------------------------------------------------------------------

#include "flaimsys.h"

/****************************************************************************
Desc:	Links a node or sub-tree as a child of an existing node.
****************************************************************************/
NODE * GedChildGraft(
	NODE * 	parent,
	NODE * 	child,
	FLMINT	nth)
{
	NODE *	lastChildNode;							/* Used when child is sub-tree */

	if( parent && child)
	{
		FLMINT level = GedNodeLevel( parent) + 1;

		if( GedChild( parent))							/* Parent already has child(ren) */
		{
			GedSibGraft(
				GedChild( parent),							/* graft as sib of 1st child instead */
				child,
				(FLMINT)(nth == GED_FIRST							/* subtacting 1 could wrap to GED_LAST */
				?	GED_FIRST											/* don't wrap */
				:	nth - 1)												/* graft now relative to 1st child */
			);
		}
		else																/* Parent has no children */
		{
			for(															/* find end of child's sub-tree */
				lastChildNode = child
			;	lastChildNode->next
			;	GedNodeLevelAdd( lastChildNode, level),	/* adjust level of sub-tree nodes */
				lastChildNode = lastChildNode->next
			);
			child->prior = parent;
			GedNodeLevelAdd( lastChildNode, level);		/* adjust last node's level */
			lastChildNode->next = parent->next;
			if( parent->next)
				parent->next->prior = lastChildNode;
			parent->next = child;
		}
	}
	return( parent);
}
/****************************************************************************
Desc:		Links a node or sub-tree as a sibling of an existing node.
Notes:	GED_LAST and GED_FIRST are #define's of +/- 32k.  These are
			practical assumptions.  Their actual value is not tested -- the 
			loop continues for that count or until no further siblings are
			found.  That is to say, if there are too many siblings in the 
			list, a true LAST or FIRST is not reached.  Since all "nth"
			parameters are WORD typed, other routines would have the same
			problem.
****************************************************************************/
NODE * GedSibGraft(
	NODE *	self,
	NODE *	sib,
	FLMINT	nth)
{
	NODE *	lastSibNode;
	NODE *	returnNode;
	FLMINT	deltaLevel;
	FLMUINT	level;
	FLMUINT	linkAt = TRUE;

	if( ! sib)
		return( self);
	if( ! self)
		return( sib);

	for(																	/* find end of sib's sub-tree */
		level = GedNodeLevel( self),
		deltaLevel = (FLMINT)(level - GedNodeLevel( sib)),
															/* Compute delta to modify sib tree*/
		lastSibNode = sib
	;	lastSibNode->next
	;	GedNodeLevelAdd( lastSibNode, deltaLevel),		/* adjust level of sub-tree nodes */
		lastSibNode = lastSibNode->next
	);

	GedNodeLevelAdd( lastSibNode, deltaLevel);		/* adjust last node's level */
	if (nth != GED_LAST )									/* Adjust nth to be zero based */
		nth++;															/* in order to work with algorithm */

	if( nth <= 0)													/* going up sibling list */
	{
		returnNode = sib;
		while( nth)													/* stop when nth is -1 */
		{
			if( self->prior)
			{
				self = self->prior;
				if( GedNodeLevel( self) > level)				/* nephew here */
					continue;											/* skip sub-tree */
				else if( GedNodeLevel( self) == level)	/* true sibling here */
				{
					nth++;												/* count up to zero (nth < 0) */
					continue;
				}
																				/* else no prior siblings; at parent */
				self = self->next;							/* point back to first child */
			}
			break;														/* graft as first sibling in list */
		}
	}
	else																	/* going down sibling list */
	{
		returnNode = self;
		while( nth)													/* stop after nth node */
		{
			if( self->next)
			{
				self = self->next;
				if( GedNodeLevel( self) > level)
					continue;											/* skip sub-tree */
				else if( GedNodeLevel( self) == level)
				{
					nth--;												/* count down to zero (nth > 0) */
					continue;
				}
																				/* else no following siblings */
				self = self->prior;							/* point back to prior node */
			}
			linkAt = FALSE;										/* Link AFTER the self node */
			break;														/* graft as last sibling in list */
		}
	}
	if( linkAt)
	{
		/* Link the sib tree AT the current self location - link before self */
		sib->prior = self->prior;
		lastSibNode->next = self;
		if( self->prior)
			self->prior->next = sib;
		self->prior = lastSibNode;
	}
	else		/* link AFTER */
	{
		/* Link the sib tree AFTER the current self location */
		sib->prior = self;
		lastSibNode->next = self->next;
		if( self->next)
			self->next->prior = lastSibNode;
		self->next = sib;
	}
	return( returnNode);
}

