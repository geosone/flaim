//------------------------------------------------------------------------------
// Desc:	B-Tree pool class file
//
// Tabs:	3
//
//		Copyright (c) 2002-2006 Novell, Inc. All Rights Reserved.
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
// $Id: f_btpool.cpp 3111 2006-01-19 13:10:50 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------

#include "flaimsys.h"

/****************************************************************************
Desc:	Method for initializing the Btree Pool.
****************************************************************************/
RCODE F_BtPool::btpInit( void)
{
	RCODE				rc = NE_SFLM_OK;

	flmAssert( !m_bInitialized);

	// Create a mutex to control access to the pool.
	if (RC_BAD( rc = f_mutexCreate( &m_hMutex)))
	{
		goto Exit;
	}

	m_bInitialized = TRUE;

Exit:

	return rc;
}

/****************************************************************************
Desc:	Method to reserve a btree object from the pool.
****************************************************************************/
RCODE F_BtPool::btpReserveBtree(
	F_Btree **		ppBtree)
{
	RCODE				rc = NE_SFLM_OK;

	flmAssert( m_bInitialized);

	// Lock the mutex first!
	f_mutexLock( m_hMutex);

	if (m_pBtreeList)
	{
		*ppBtree = m_pBtreeList;
		m_pBtreeList = m_pBtreeList->m_pNext;
		(*ppBtree)->m_pNext = NULL;
	}
	else
	{
		if (( *ppBtree = f_new F_Btree()) == NULL)
		{
			rc = RC_SET( NE_SFLM_MEM);
			goto Exit;
		}
	}

Exit:

	f_mutexUnlock( m_hMutex);

	return rc;
}

/****************************************************************************
Desc:	Method to return a Btree to the pool.
****************************************************************************/
void F_BtPool::btpReturnBtree(
	F_Btree **		ppBtree)
{
	flmAssert( m_bInitialized);

	// Close the Btree
	
	(*ppBtree)->btClose();

	// Lock the mutex first!
	
	f_mutexLock( m_hMutex);

	(*ppBtree)->m_pNext = m_pBtreeList;
	m_pBtreeList = *ppBtree;
	*ppBtree = NULL;

	f_mutexUnlock( m_hMutex);
}