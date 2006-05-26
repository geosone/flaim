//------------------------------------------------------------------------------
// Desc:	Insert and delete keys in an index B-Tree.
//
// Tabs:	3
//
//		Copyright (c) 1991-2006 Novell, Inc. All Rights Reserved.
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
// $Id: fsrefupd.cpp 3114 2006-01-19 13:22:45 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------

#include "flaimsys.h"

/***************************************************************************
Desc:	Update (add or delete) a single reference
*****************************************************************************/
RCODE F_Db::refUpdate(
	F_INDEX *		pIndex,
	KREF_ENTRY *	pKrefEntry,
	FLMBOOL			bNormalUpdate)
{
	RCODE				rc = NE_SFLM_OK;
	F_Btree *		pbtree = NULL;
	IXKeyCompare	compareObject;

	// Get a btree

	if (RC_BAD( rc = gv_SFlmSysData.pBtPool->btpReserveBtree( &pbtree)))
	{
		goto Exit;
	}

	flmAssert( pIndex->lfInfo.uiRootBlk);

	compareObject.setIxInfo( this, pIndex);
	if (bNormalUpdate && pKrefEntry->bDelete)
	{
		compareObject.setOldRow( pKrefEntry->pRow);
	}
	if( RC_BAD( rc = pbtree->btOpen( this, &pIndex->lfInfo,
		(pIndex->uiFlags & IXD_ABS_POS) ? TRUE : FALSE,
		(pIndex->pDataIcds) ? TRUE : FALSE, &compareObject)))
	{
		goto Exit;
	}

	if (!pKrefEntry->bDelete)
	{
		pbtree->btResetBtree();
		if( RC_BAD( rc = pbtree->btInsertEntry(
							(FLMBYTE *)&pKrefEntry [1], pKrefEntry->ui16KeyLen,
							pKrefEntry->uiDataLen
							? ((FLMBYTE *)(&pKrefEntry [1])) + 1 + pKrefEntry->ui16KeyLen
							: NULL,
							pKrefEntry->uiDataLen, TRUE, TRUE)))
		{
			goto Exit;
		}
	}
	else
	{
		pbtree->btResetBtree();
		if (RC_BAD( rc = pbtree->btRemoveEntry(
							(FLMBYTE *)&pKrefEntry [1], pKrefEntry->ui16KeyLen)))
		{
			if (rc == NE_SFLM_NOT_FOUND)
			{
				// Already been deleted, ignore the error condition and go on.

				RC_UNEXPECTED_ASSERT( rc);
				rc = NE_SFLM_OK;
			}

			goto Exit;
		}
	}

Exit:

	if (pbtree)
	{
		gv_SFlmSysData.pBtPool->btpReturnBtree( &pbtree);
	}

	return( rc);
}