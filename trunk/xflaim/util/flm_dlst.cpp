//------------------------------------------------------------------------------
//	Desc: Dynamic, interactive list manager
//
// Tabs:	3
//
//		Copyright (c) 2003-2006 Novell, Inc. All Rights Reserved.
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
// $Id: flm_dlst.cpp 3117 2006-01-19 13:34:36 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------

#include "flaimsys.h"
#include "ftx.h"
#include "flm_dlst.h"
#include "sharutil.h"

/****************************************************************************
Name:	N/A
Desc:	Default constructor
*****************************************************************************/
F_DynamicList::F_DynamicList( void)
{
	m_pFirst = NULL;
	m_pLast = NULL;
	m_pCur = NULL;
	m_pListWin = NULL;
	m_uiRow = 0;
	m_uiListRows = 0;
	m_uiListCols = 0;
	m_bChanged = TRUE;
	m_bShowHorizontalSelector = TRUE;
}

/****************************************************************************
Name:	N/A
Desc:	Default destructor
*****************************************************************************/
F_DynamicList::~F_DynamicList( void)
{
	DLIST_NODE *		pTmp = m_pFirst;
	DLIST_NODE *		pTmp2;

	while( pTmp)
	{
		pTmp2 = pTmp;
		pTmp = pTmp->pNext;
		freeNode( pTmp2);
	}

	if( m_pListWin)
	{
		FTXWinFree( &m_pListWin);
	}
}

/****************************************************************************
Name:	setup
Desc:	Allocates the list window and prepares the object for use
*****************************************************************************/
RCODE F_DynamicList::setup( FTX_WINDOW * pInitializedWindow)
{
	RCODE				rc = NE_XFLM_OK;

	flmAssert( pInitializedWindow != NULL);
	m_pListWin = pInitializedWindow;

	/*
	Create the list window
	*/
	FTXWinClear( m_pListWin);

	FTXWinSetCursorType( m_pListWin, WPS_CURSOR_INVISIBLE);
	FTXWinSetScroll( m_pListWin, FALSE);
	FTXWinSetLineWrap( m_pListWin, FALSE);
	FTXWinGetCanvasSize( m_pListWin, &m_uiListCols, &m_uiListRows);

	return( rc);
}

/****************************************************************************
Name:	insert
Desc:
*****************************************************************************/
RCODE F_DynamicList::insert(
	FLMUINT					uiKey,
	F_DLIST_DISP_HOOK 	pDisplayHook,
	void *					pvData,
	FLMUINT					uiDataLen)
{
	RCODE				rc = NE_XFLM_OK;
	DLIST_NODE *	pTmp;
	DLIST_NODE *	pNew = NULL;

	if( getNode( uiKey) != NULL)
	{
		rc = RC_SET( NE_XFLM_EXISTS);
		goto Exit;
	}

	// Allocate the new node

	if( RC_BAD( rc = f_alloc( sizeof( DLIST_NODE), &pNew)))
	{
		goto Exit;
	}

	f_memset( pNew, 0, sizeof( DLIST_NODE));

	// Set the members of the new node

	pNew->uiKey = uiKey;

	if( pDisplayHook)
	{
		pNew->pDispHook = pDisplayHook;
	}
	else
	{
		pNew->pDispHook = dlistDefaultDisplayHook;
	}

	if( uiDataLen)
	{
		if( RC_BAD( rc = f_alloc( uiDataLen, &pNew->pvData)))
		{
			goto Exit;
		}

		f_memcpy( pNew->pvData, pvData, uiDataLen);
		pNew->uiDataLen = uiDataLen;
	}

	// Find the insertion point

	if( !m_pFirst)
	{
		m_pFirst = m_pLast = m_pCur = pNew;
	}
	else
	{
		pTmp = m_pFirst;
		while( pTmp && pTmp->uiKey < uiKey)
		{
			pTmp = pTmp->pNext;
		}

		if( pTmp)
		{
			if( pTmp == m_pFirst)
			{
				pNew->pNext = m_pFirst;
				m_pFirst->pPrev = pNew;
				m_pFirst = pNew;
			}
			else
			{
				pNew->pNext = pTmp;
				pNew->pPrev = pTmp->pPrev;

				if( pTmp->pPrev)
				{
					pTmp->pPrev->pNext = pNew;
				}

				pTmp->pPrev = pNew;
			}
		}
		else
		{
			// Insert at end
			m_pLast->pNext = pNew;
			pNew->pPrev = m_pLast;
			m_pLast = pNew;
		}
	}

	m_bChanged = TRUE;

Exit:

	if( RC_BAD( rc))
	{
		if( pNew)
		{
			freeNode( pNew);
		}
	}

	return( rc);
}

/****************************************************************************
Name:	update
Desc:
*****************************************************************************/
RCODE F_DynamicList::update(
	FLMUINT					uiKey,
	F_DLIST_DISP_HOOK 	pDisplayHook,
	void *					pvData,
	FLMUINT					uiDataLen)
{
	DLIST_NODE *	pTmp;
	RCODE				rc = NE_XFLM_OK;

	if( (pTmp = getNode( uiKey)) == NULL)
	{
		rc = insert( uiKey, pDisplayHook, pvData, uiDataLen);
		goto Exit;
	}

	if( !pTmp->pvData || pTmp->uiDataLen != uiDataLen)
	{
		if( pTmp->pvData)
		{
			f_free( &pTmp->pvData);
			pTmp->uiDataLen = 0;
		}

		if( uiDataLen)
		{
			if( RC_BAD( rc = f_alloc( uiDataLen, &pTmp->pvData)))
			{
				goto Exit;
			}
		}
	}

	if( uiDataLen)
	{
		f_memcpy( pTmp->pvData, pvData, uiDataLen);
		pTmp->uiDataLen = uiDataLen;
	}

	m_bChanged = TRUE;

Exit:

	return( rc);
}

/****************************************************************************
Name:	remove
Desc:
*****************************************************************************/
RCODE F_DynamicList::remove(
	FLMUINT	uiKey)
{
	DLIST_NODE *	pTmp;
	RCODE				rc = NE_XFLM_OK;

	if( (pTmp = getNode( uiKey)) == NULL)
	{
		rc = RC_SET( NE_XFLM_NOT_FOUND);
		goto Exit;
	}

	if( pTmp->pPrev)
	{
		pTmp->pPrev->pNext = pTmp->pNext;
	}

	if( pTmp->pNext)
	{
		pTmp->pNext->pPrev = pTmp->pPrev;
	}

	if( m_pCur == pTmp)
	{
		if( pTmp->pNext)
		{
			m_pCur = pTmp->pNext;
		}
		else
		{
			m_pCur = pTmp->pPrev;
		}
	}

	if( m_pFirst == pTmp)
	{
		m_pFirst = pTmp->pNext;
	}

	if( m_pLast == pTmp)
	{
		m_pLast = pTmp->pPrev;
		if( m_uiRow)
		{
			m_uiRow--;
		}
	}

	freeNode( pTmp);
	m_bChanged = TRUE;

Exit:

	return( rc);
}

/****************************************************************************
Name:	refresh
Desc:
*****************************************************************************/
void F_DynamicList::refresh( void)
{
	DLIST_NODE *	pTmp;
	FLMUINT			uiLoop;
	FTX_SCREEN *	pScreen = NULL;

	if( !m_bChanged)
	{
		return;
	}

	if ( FTXWinGetScreen( m_pListWin, &pScreen) != FTXRC_SUCCESS)
	{
		flmAssert( 0);
		goto Exit;
	}

	FTXSetRefreshState( pScreen->pFtxInfo, TRUE);
	pTmp = m_pCur;
	uiLoop = m_uiRow;
	while( pTmp && uiLoop > 0)
	{
		pTmp = pTmp->pPrev;
		uiLoop--;
	}

	if( !pTmp)
	{
		pTmp = m_pFirst;
	}

	uiLoop = 0;
	while( pTmp && uiLoop < m_uiListRows)
	{
		pTmp->pDispHook( m_pListWin,
			(FLMBOOL)(pTmp == m_pCur ? TRUE : FALSE),
			uiLoop, pTmp->uiKey, pTmp->pvData, pTmp->uiDataLen, this);
		pTmp = pTmp->pNext;
		uiLoop++;
		f_yieldCPU();
	}

	if( uiLoop < m_uiListRows)
	{
		FTXWinClearXY( m_pListWin, 0, uiLoop);
	}
	FTXSetRefreshState( pScreen->pFtxInfo, FALSE);
	m_bChanged = FALSE;
Exit:
	;
}

/****************************************************************************
Name:	cursorUp
Desc:
*****************************************************************************/
void F_DynamicList::cursorUp( void)
{
	if( m_pCur && m_pCur->pPrev)
	{
		m_pCur = m_pCur->pPrev;
		if( m_uiRow > 0)
		{
			m_uiRow--;
		}
		m_bChanged = TRUE;
	}
}

/****************************************************************************
Name:	cursorDown
Desc:
*****************************************************************************/
void F_DynamicList::cursorDown( void)
{
	if( m_pCur && m_pCur->pNext)
	{
		m_pCur = m_pCur->pNext;
		if( m_uiRow < (m_uiListRows - 1))
		{
			m_uiRow++;
		}
		m_bChanged = TRUE;
	}
}

/****************************************************************************
Name:	pageUp
Desc:
*****************************************************************************/
void F_DynamicList::pageUp( void)
{
	FLMUINT			uiLoop = 0;
	DLIST_NODE *	pTmp;

	while( m_pCur && uiLoop < m_uiListRows)
	{
		m_pCur = m_pCur->pPrev;
		uiLoop++;
	}

	if( m_pCur == NULL)
	{
		m_pCur = m_pFirst;
		m_uiRow = 0;
	}
	else
	{
		pTmp = m_pCur->pPrev;
		uiLoop = 0;
		while( pTmp && uiLoop < m_uiRow)
		{
			pTmp = pTmp->pPrev;
			uiLoop++;
		}

		if( uiLoop < m_uiRow)
		{
			m_uiRow = uiLoop;
		}
	}

	m_bChanged = TRUE;
}

/****************************************************************************
Name:	pageDown
Desc:
*****************************************************************************/
void F_DynamicList::pageDown( void)
{
	DLIST_NODE *	pOldCur = m_pCur;
	FLMUINT			uiLoop;

	if( m_pCur == m_pLast)
	{
		return;
	}

	uiLoop = 0;
	while( m_pCur && uiLoop < m_uiListRows)
	{
		m_pCur = m_pCur->pNext;
		uiLoop++;
	}

	if( m_pCur == NULL)
	{
		m_pCur = m_pLast;
		while( m_pCur != pOldCur && m_uiRow < (m_uiListRows - 1))
		{
			pOldCur = pOldCur->pNext;
			m_uiRow++;
		}
	}

	m_bChanged = TRUE;
}

/****************************************************************************
Name:	home
Desc:
*****************************************************************************/
void F_DynamicList::home( void)
{
	m_pCur = m_pFirst;
	m_uiRow = 0;
	m_bChanged = TRUE;
}

/****************************************************************************
Name:	end
Desc:
*****************************************************************************/
void F_DynamicList::end( void)
{
	DLIST_NODE *		pTmp;

	pTmp = m_pCur = m_pLast;
	m_uiRow = 0;
	while( pTmp && m_uiRow < m_uiListRows)
	{
		pTmp = pTmp->pPrev;
		m_uiRow++;
	}

	if( m_uiRow)
	{
		m_uiRow--;
	}

	m_bChanged = TRUE;
}

/****************************************************************************
Name:	defaultKeyAction
Desc:
*****************************************************************************/
void F_DynamicList::defaultKeyAction(
	FLMUINT uiKey)
{
	switch( uiKey)
	{
		case WPK_UP:
			cursorUp();
			break;
		case WPK_DOWN:
			cursorDown();
			break;
		case WPK_PGUP:
			pageUp();
			break;
		case WPK_PGDN:
			pageDown();
			break;
		case WPK_HOME:
			home();
			break;
		case WPK_END:
			end();
			break;
		case 'd':
		case 'D':
		{
			RCODE rc = NE_XFLM_OK;
			rc = this->dumpToFile();
			break;
		}
	}
	refresh();
}

/****************************************************************************
Name:	getNode
Desc:
*****************************************************************************/
DLIST_NODE * F_DynamicList::getNode(
	FLMUINT		uiKey)
{
	DLIST_NODE *		pTmp = m_pFirst;

	while( pTmp)
	{
		if( pTmp->uiKey == uiKey)
		{
			break;
		}
		pTmp = pTmp->pNext;
	}

	return( pTmp);
}

/****************************************************************************
Name:	freeNode
Desc:
*****************************************************************************/
void F_DynamicList::freeNode(
	DLIST_NODE *		pNode)
{
	if( pNode->pvData)
	{
		f_free( &pNode->pvData);
		f_free( &pNode);
	}
}

/****************************************************************************
Name:	dlistDefaultDisplayHook
Desc:
*****************************************************************************/
RCODE dlistDefaultDisplayHook(
	FTX_WINDOW *		pWin,
	FLMBOOL				bSelected,
	FLMUINT				uiRow,
	FLMUINT				uiKey,
	void *				pvData,
	FLMUINT				uiDataLen,
	F_DynamicList*		pDynamicList)
{
	FLMUINT	uiBack = WPS_CYAN;
	FLMUINT	uiFore = WPS_WHITE;

	F_UNREFERENCED_PARM( uiKey);
	F_UNREFERENCED_PARM( uiDataLen);

	FTXWinSetCursorPos( pWin, 0, uiRow);
	FTXWinClearToEOL( pWin);
	FTXWinPrintf( pWin, "%s",
		(FLMBYTE *)
		(pvData ?
			pvData :
			//the following cast is required by gcc 2.96
			(FLMBYTE*)"Unknown"));
	if( bSelected && pDynamicList->getShowHorizontalSelector())
	{
		FTXWinPaintRow( pWin, &uiBack, &uiFore, uiRow);
	}
	return( NE_XFLM_OK);
}

RCODE F_DynamicList::dumpToFile()
{
	DLIST_NODE *	pTmp;
	FLMUINT			uiLoop;
	IF_FileHdl *	pFileHdl = NULL;
	RCODE				rc = NE_XFLM_OK;
	F_FileSystem	FileSystem;
#define DLST_RESP_SIZE 256
	char				szResponse[ DLST_RESP_SIZE];
	FLMUINT			uiTermChar;
	FTX_SCREEN *	pScreen;

	f_strcpy( szResponse, DLIST_DUMPFILE_PATH);

	FTXWinGetScreen( m_pListWin, &pScreen);
	FTXGetInput(
		pScreen,
 		"enter filename to dump to",
		szResponse,
		DLST_RESP_SIZE-1,
		&uiTermChar);

	if ( uiTermChar != WPK_ENTER)
	{
		goto Exit;
	}

	if (RC_BAD( rc = FileSystem.Exists( szResponse)))
	{
		//create file if it doesn't already exist
		if ( rc == NE_XFLM_IO_PATH_NOT_FOUND)
		{
			rc = FileSystem.Create( szResponse, XFLM_IO_RDWR, &pFileHdl);
		}
		else
		{
			goto Exit_local;
		}
	}
	else
	{
		rc = FileSystem.Open( szResponse, XFLM_IO_RDWR, &pFileHdl);
	}

	TEST_RC_LOCAL( rc);

	{
		FLMUINT64	ui64FileSize = 0;
		FLMUINT		uiBytesWritten = 0;

		//figure out size of file currently, so you can append to it
		
		pFileHdl->Size( &ui64FileSize);
		pTmp = m_pFirst;

		uiLoop = 0;
		while( pTmp)
		{
			FLMBYTE * pszNextLine = (FLMBYTE*)(pTmp->pvData);

			TEST_RC_LOCAL( rc = pFileHdl->Write(
				ui64FileSize,					//offset to current file size
				f_strlen( pszNextLine),
				pszNextLine,
				&uiBytesWritten));

			ui64FileSize += uiBytesWritten;

			TEST_RC_LOCAL( rc = pFileHdl->Write(
				ui64FileSize,					//add in newline
				1,
				(FLMBYTE*)"\n",
				&uiBytesWritten));

			ui64FileSize += uiBytesWritten;
			pTmp = pTmp->pNext;
		}

		(void)pFileHdl->Close();

	}

Exit_local:
	{//give success/fail message

		char				szMessage[ 256];
		FLMUINT			uiChar;

		FTXWinGetScreen( m_pListWin, &pScreen);
		if ( RC_OK( rc))
		{
			f_sprintf( szMessage,
				"contents of focused list appended to %s", DLIST_DUMPFILE_PATH);
		}
		else
		{
			f_sprintf( szMessage, "error rc=%u dumping to file %s",
				(unsigned)rc, DLIST_DUMPFILE_PATH);
		}
		FTXDisplayMessage( pScreen, WPS_RED, WPS_WHITE, szMessage,
			"press ESC or ENTER to close dialog", &uiChar);
	}


Exit:
	if (pFileHdl)
	{
		pFileHdl->Release();
		pFileHdl = NULL;
	}
	return rc;
}