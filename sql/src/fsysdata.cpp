//------------------------------------------------------------------------------
// Desc:	This file contains the routines that initialize and shut down FLAIM,
//			as well as routines for configuring FLAIM.
// Tabs:	3
//
// Copyright (c) 1995-2007 Novell, Inc. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; version 2.1
// of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, contact Novell, Inc.
//
// To contact Novell about this file by physical or electronic mail, 
// you may find current contact information at www.novell.com.
//
// $Id$
//------------------------------------------------------------------------------

#define ALLOCATE_SYS_DATA
#define ALLOC_ERROR_TABLES

#include "flaimsys.h"

#define FLM_MIN_FREE_BYTES		(2 * 1024 * 1024)

#ifdef FLM_32BIT
	#if defined( FLM_LINUX)
	
	// With mmap'd memory on Linux, you're effectively limited to about ~2 GB.
	// Userspace only gets ~3GB of usable address space anyway, and then you
	// have all of the thread stacks too, which you can't have 
	// overlapping the heap.
	
		#define FLM_MAX_CACHE_SIZE		(1500 * 1024 * 1024)
	#else
		#define FLM_MAX_CACHE_SIZE		(2000 * 1024 * 1024)
	#endif
#else
	#define FLM_MAX_CACHE_SIZE			(~((FLMUINT)0))
#endif

static FLMATOMIC		gv_flmSysSpinLock = 0;
static FLMBOOL			gv_bToolkitStarted = FALSE;

FSTATIC RCODE flmGetCacheBytes(
	FLMUINT			uiPercent,
	FLMUINT			uiMin,
	FLMUINT			uiMax,
	FLMUINT			uiMinToLeave,
	FLMBOOL			bCalcOnAvailMem,
	FLMUINT			uiBytesCurrentlyInUse,
	FLMUINT *		puiCacheBytes);

FSTATIC RCODE flmVerifyDiskStructOffsets( void);

FSTATIC void flmFreeEvent(
	FEVENT *			pEvent,
	F_MUTEX			hMutex,
	FEVENT **		ppEventListRV);

FSTATIC void flmGetStringParam(
	const char *	pszParamName,
	char **			ppszValue,
	IF_IniFile *	pIniFile);

FSTATIC void flmGetNumParam(
	char **		ppszParam,
	FLMUINT *	puiNum);

FSTATIC void flmGetUintParam(
	const char *	pszParamName,
	FLMUINT			uiDefaultValue,
	FLMUINT *		puiUint,
	IF_IniFile *	pIniFile);

FSTATIC void lockSysData( void);

FSTATIC void unlockSysData( void);
	
void flmGetBoolParam(
	const char *	pszParamName,
	FLMBOOL			uiDefaultValue,
	FLMBOOL *		pbBool,
	IF_IniFile *	pIniFile);

FSTATIC RCODE flmGetIniFileName(
	FLMBYTE *		pszIniFileName,
	FLMUINT			uiBufferSz);

/****************************************************************************
Desc: This routine determines the number of cache bytes to use for caching
		based on a percentage of available physical memory or a percentage
		of physical memory (depending on bCalcOnAvailMem flag).
		uiBytesCurrentlyInUse indicates how many bytes are currently allocated
		by FLAIM - so it can factor that in if the calculation is to be based
		on the available memory.
		Lower limit is 1 megabyte.
****************************************************************************/
FSTATIC RCODE flmGetCacheBytes(
	FLMUINT		uiPercent,
	FLMUINT		uiMin,
	FLMUINT		uiMax,
	FLMUINT		uiMinToLeave,
	FLMBOOL		bCalcOnAvailMem,
	FLMUINT		uiBytesCurrentlyInUse,
	FLMUINT *	puiCacheBytes)
{
	RCODE				rc = NE_SFLM_OK;
	FLMUINT			uiMem = 0;
	FLMUINT64		ui64TotalPhysMem;
	FLMUINT64		ui64AvailPhysMem;
	
	if( RC_BAD( rc = f_getMemoryInfo( &ui64TotalPhysMem, &ui64AvailPhysMem)))
	{
		goto Exit;
	}
	
	if( ui64TotalPhysMem > FLM_MAX_UINT)
	{
		ui64TotalPhysMem = FLM_MAX_UINT;
	}
	
	if( ui64AvailPhysMem > ui64TotalPhysMem)
	{
		ui64AvailPhysMem = ui64TotalPhysMem;
	}
	
	uiMem = (FLMUINT)((bCalcOnAvailMem)
							? (FLMUINT)ui64TotalPhysMem
							: (FLMUINT)ui64AvailPhysMem);
	
	// If we are basing the calculation on available physical memory,
	// take in to account what has already been allocated.

	if (bCalcOnAvailMem)
	{
		if (uiMem > FLM_MAX_UINT - uiBytesCurrentlyInUse)
		{
			uiMem = FLM_MAX_UINT;
		}
		else
		{
			uiMem += uiBytesCurrentlyInUse;
		}
	}

	// If uiMax is zero, use uiMinToLeave to calculate the maximum.

	if (!uiMax)
	{
		if (!uiMinToLeave)
		{
			uiMax = uiMem;
		}
		else if (uiMinToLeave < uiMem)
		{
			uiMax = uiMem - uiMinToLeave;
		}
		else
		{
			uiMax = 0;
		}
	}

	// Calculate memory as a percentage of memory.

	uiMem = (FLMUINT)((uiMem > FLM_MAX_UINT / 100)
							? (FLMUINT)(uiMem / 100) * uiPercent
							: (FLMUINT)(uiMem * uiPercent) / 100);

	// Don't go above the maximum.

	if (uiMem > uiMax)
	{
		uiMem = uiMax;
	}

	// Don't go below the minimum.

	if (uiMem < uiMin)
	{
		uiMem = uiMin;
	}
	
Exit:

	*puiCacheBytes = uiMem;
	return( rc);
}

/***************************************************************************
Desc:	Verify that the distance (in bytes) between pvStart and pvEnd is
		what was specified in uiOffset.
****************************************************************************/
FINLINE void flmVerifyOffset(
	FLMUINT		uiCompilerOffset,
	FLMUINT		uiOffset,
	RCODE *		pRc)
{
	if (RC_OK( *pRc))
	{
		if ( uiCompilerOffset != uiOffset)
		{
			*pRc = RC_SET_AND_ASSERT( NE_SFLM_BAD_PLATFORM_FORMAT);
		}
	}
}

/***************************************************************************
Desc:	Verify the offsets of each member of every on-disk structure.  This
		is a safety check to ensure that things work correctly on every
		platform.
****************************************************************************/
FSTATIC RCODE flmVerifyDiskStructOffsets( void)
{
	RCODE						rc = NE_SFLM_OK;
	FLMUINT					uiSizeOf;

	// Verify the SFLM_DB_HDR offsets.

	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, szSignature[0]),
						  SFLM_DB_HDR_szSignature_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui8IsLittleEndian),
						  SFLM_DB_HDR_ui8IsLittleEndian_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui8DefaultLanguage),
						  SFLM_DB_HDR_ui8DefaultLanguage_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui16BlockSize),
						  SFLM_DB_HDR_ui16BlockSize_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32DbVersion),
						  SFLM_DB_HDR_ui32DbVersion_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui8BlkChkSummingEnabled),
						  SFLM_DB_HDR_ui8BlkChkSummingEnabled_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui8RflKeepFiles),
						  SFLM_DB_HDR_ui8RflKeepFiles_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui8RflAutoTurnOffKeep),
						  SFLM_DB_HDR_ui8RflAutoTurnOffKeep_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui8RflKeepAbortedTrans),
						  SFLM_DB_HDR_ui8RflKeepAbortedTrans_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RflCurrFileNum),
						  SFLM_DB_HDR_ui32RflCurrFileNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui64LastRflCommitID),
						  SFLM_DB_HDR_ui64LastRflCommitID_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RflLastFileNumDeleted),
						  SFLM_DB_HDR_ui32RflLastFileNumDeleted_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RflLastTransOffset),
						  SFLM_DB_HDR_ui32RflLastTransOffset_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RflLastCPFileNum),
						  SFLM_DB_HDR_ui32RflLastCPFileNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RflLastCPOffset),
						  SFLM_DB_HDR_ui32RflLastCPOffset_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui64RflLastCPTransID),
						  SFLM_DB_HDR_ui64RflLastCPTransID_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RflMinFileSize),
						  SFLM_DB_HDR_ui32RflMinFileSize_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RflMaxFileSize),
						  SFLM_DB_HDR_ui32RflMaxFileSize_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui64CurrTransID),
						  SFLM_DB_HDR_ui64CurrTransID_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui64TransCommitCnt),
						  SFLM_DB_HDR_ui64TransCommitCnt_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RblEOF),
						  SFLM_DB_HDR_ui32RblEOF_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32RblFirstCPBlkAddr),
						  SFLM_DB_HDR_ui32RblFirstCPBlkAddr_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32FirstAvailBlkAddr),
						  SFLM_DB_HDR_ui32FirstAvailBlkAddr_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32FirstLFBlkAddr),
						  SFLM_DB_HDR_ui32FirstLFBlkAddr_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32LogicalEOF),
						  SFLM_DB_HDR_ui32LogicalEOF_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32MaxFileSize),
						  SFLM_DB_HDR_ui32MaxFileSize_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui64LastBackupTransID),
						  SFLM_DB_HDR_ui64LastBackupTransID_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32IncBackupSeqNum),
						  SFLM_DB_HDR_ui32IncBackupSeqNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32BlksChangedSinceBackup),
						  SFLM_DB_HDR_ui32BlksChangedSinceBackup_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ucDbSerialNum[0]),
						  SFLM_DB_HDR_ucDbSerialNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ucLastTransRflSerialNum[0]),
						  SFLM_DB_HDR_ucLastTransRflSerialNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ucNextRflSerialNum[0]),
						  SFLM_DB_HDR_ucNextRflSerialNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ucIncBackupSerialNum[0]),
						  SFLM_DB_HDR_ucIncBackupSerialNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32DbKeyLen),
						  SFLM_DB_HDR_ui32DbKeyLen, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ucReserved[0]),
						  SFLM_DB_HDR_ucReserved_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ui32HdrCRC),
						  SFLM_DB_HDR_ui32HdrCRC_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(SFLM_DB_HDR, ucDbKey[0]),
						  SFLM_DB_HDR_ucDbKey, &rc);

	// Have to use a variable for sizeof.  If we don't, compiler barfs
	// because we are comparing two constants.

	uiSizeOf = SFLM_DB_HDR_ucDbKey + SFLM_MAX_ENC_KEY_SIZE;
	if( sizeof( SFLM_DB_HDR) != uiSizeOf)
	{
		rc = RC_SET_AND_ASSERT( NE_SFLM_BAD_PLATFORM_FORMAT);
	}

	// Verify the offsets in the F_BLK_HDR structure.

	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui32BlkAddr),
						  F_BLK_HDR_ui32BlkAddr_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui32PrevBlkInChain),
						  F_BLK_HDR_ui32PrevBlkInChain_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui32NextBlkInChain),
						  F_BLK_HDR_ui32NextBlkInChain_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui32PriorBlkImgAddr),
						  F_BLK_HDR_ui32PriorBlkImgAddr_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui64TransID),
						  F_BLK_HDR_ui64TransID_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui32BlkCRC),
						  F_BLK_HDR_ui32BlkCRC_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui16BlkBytesAvail),
						  F_BLK_HDR_ui16BlkBytesAvail_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui8BlkFlags),
						  F_BLK_HDR_ui8BlkFlags_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.stdBlkHdr.ui8BlkType),
						  F_BLK_HDR_ui8BlkType_OFFSET, &rc);

	// Have to use a variable for sizeof.  If we don't, compiler barfs
	// because we are comparing two constants.

	uiSizeOf = SIZEOF_STD_BLK_HDR;
	if (sizeof( F_BLK_HDR) != uiSizeOf)
	{
		rc = RC_SET_AND_ASSERT( NE_SFLM_BAD_PLATFORM_FORMAT);
	}

	// Verify the offsets in the F_BTREE_BLK_HDR structure

	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.BTreeBlkHdr.stdBlkHdr),
						  F_BTREE_BLK_HDR_stdBlkHdr_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.BTreeBlkHdr.ui16LogicalFile),
						  F_BTREE_BLK_HDR_ui16LogicalFile_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.BTreeBlkHdr.ui16NumKeys),
						  F_BTREE_BLK_HDR_ui16NumKeys_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.BTreeBlkHdr.ui8BlkLevel),
						  F_BTREE_BLK_HDR_ui8BlkLevel_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.BTreeBlkHdr.ui8BTreeFlags),
						  F_BTREE_BLK_HDR_ui8BTreeFlags_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LARGEST_BLK_HDR, all.BTreeBlkHdr.ui16HeapSize),
						  F_BTREE_BLK_HDR_ui16HeapSize_OFFSET, &rc);

	// Have to use a variable for sizeof.  If we don't, compiler barfs
	// because we are comparing two constants.

	uiSizeOf = 40;
	if (sizeof( F_BTREE_BLK_HDR) != uiSizeOf)
	{
		rc = RC_SET_AND_ASSERT( NE_SFLM_BAD_PLATFORM_FORMAT);
	}

	// Have to use a variable for sizeof.  If we don't, compiler barfs
	// because we are comparing two constants.

	uiSizeOf = 40;
	if (sizeof( F_LARGEST_BLK_HDR) != uiSizeOf)
	{
		rc = RC_SET_AND_ASSERT( NE_SFLM_BAD_PLATFORM_FORMAT);
	}

	// Verify the offsets in the F_LF_HDR structure

	flmVerifyOffset( (FLMUINT)f_offsetof(F_LF_HDR, ui64NextRowId),
						  F_LF_HDR_ui64NextRowId_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LF_HDR, ui32LfType),
						  F_LF_HDR_ui32LfType_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LF_HDR, ui32RootBlkAddr),
						  F_LF_HDR_ui32RootBlkAddr_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LF_HDR, ui32LfNum),
						  F_LF_HDR_ui32LfNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LF_HDR, ui32EncDefNum),
						  F_LF_HDR_ui32EncDefNum_OFFSET, &rc);
	flmVerifyOffset( (FLMUINT)f_offsetof(F_LF_HDR, ucZeroes[0]),
						  F_LF_HDR_ucZeroes_OFFSET, &rc);

	// Have to use a variable for sizeof.  If we don't, compiler barfs
	// because we are comparing two constants.

	uiSizeOf = 64;
	if (sizeof( F_LF_HDR) != uiSizeOf)
	{
		rc = RC_SET_AND_ASSERT( NE_SFLM_BAD_PLATFORM_FORMAT);
	}

	return( rc);
}

/****************************************************************************
Desc:		Logs the reason for the "must close" flag being set
****************************************************************************/
void F_Database::logMustCloseReason(
	const char *	pszFileName,
	FLMINT			iLineNumber)
{
	char *						pszMsgBuf = NULL;
	IF_LogMessageClient *	pLogMsg = NULL;

	// Log a message indicating why the "must close" flag was set

	if( (pLogMsg = flmBeginLogMessage( SFLM_GENERAL_MESSAGE)) != NULL)
	{
		if( RC_OK( f_alloc( F_PATH_MAX_SIZE + 512, &pszMsgBuf)))
		{
			f_sprintf( (char *)pszMsgBuf,
				"Database (%s) must be closed because of a 0x%04X error, "
				"File=%s, Line=%d.",
				(char *)(m_pszDbPath ? (char *)m_pszDbPath : (char *)""),
				(unsigned)m_rcMustClose,
				pszFileName, (int)iLineNumber);

			pLogMsg->changeColor( FLM_YELLOW, FLM_BLACK);
			pLogMsg->appendString( pszMsgBuf);
		}
		flmEndLogMessage( &pLogMsg);
	}

	if( pszMsgBuf)
	{
		f_free( &pszMsgBuf);
	}
}

/****************************************************************************
Desc: This shuts down the background threads
Note:	This routine assumes that the global mutex is locked.  The mutex will
		be unlocked internally, but will always be locked on exit.
****************************************************************************/
void F_Database::shutdownDatabaseThreads( void)
{
	RCODE					rc = NE_SFLM_OK;
	F_BKGND_IX	*		pBackgroundIx;
	IF_Thread *			pThread;
	FLMUINT				uiThreadId;
	FLMUINT				uiThreadCount;
	FLMBOOL				bMutexLocked = TRUE;

	// Signal all background indexing threads to shutdown that are associated
	// with this F_Database.

	for( ;;)
	{
		uiThreadCount = 0;

		// Shut down all background threads.

		uiThreadId = 0;
		for( ;;)
		{
			if( RC_BAD( rc = gv_SFlmSysData.pThreadMgr->getNextGroupThread(
				&pThread, gv_SFlmSysData.uiIndexingThreadGroup, &uiThreadId)))
			{
				if( rc == NE_SFLM_NOT_FOUND)
				{
					rc = NE_SFLM_OK;
					break;
				}
				else
				{
					RC_UNEXPECTED_ASSERT( rc);
				}
			}
			else
			{
				pBackgroundIx = (F_BKGND_IX *)pThread->getParm1();
				if( pBackgroundIx && pBackgroundIx->pDatabase == this)
				{
					// Set the thread's terminate flag.

					uiThreadCount++;
					pThread->setShutdownFlag();
				}

				pThread->Release();
				pThread = NULL;
			}
		}

		if( !uiThreadCount)
		{
			break;
		}

		// Unlock the global mutex

		f_mutexUnlock( gv_SFlmSysData.hShareMutex);
		bMutexLocked = FALSE;

		// Give the threads a chance to terminate

		f_sleep( 50);

		// Re-lock the mutex and see if any threads are still active

		f_mutexLock( gv_SFlmSysData.hShareMutex);
		bMutexLocked = TRUE;

	}
	
	// Shut down the maintenance thread

	if( m_pMaintThrd)
	{
		flmAssert( bMutexLocked);
		
		m_pMaintThrd->setShutdownFlag();
		f_semSignal( m_hMaintSem);
		
		f_mutexUnlock( gv_SFlmSysData.hShareMutex);
		m_pMaintThrd->stopThread();
		f_mutexLock( gv_SFlmSysData.hShareMutex);
		
		m_pMaintThrd->Release();
		m_pMaintThrd = NULL;
		f_semDestroy( &m_hMaintSem);
	}

	// Re-lock the mutex

	if( !bMutexLocked)
	{
		f_mutexLock( gv_SFlmSysData.hShareMutex);
	}
}

/****************************************************************************
Desc: This routine frees a registered event.
****************************************************************************/
FSTATIC void flmFreeEvent(
	FEVENT *			pEvent,
	F_MUTEX			hMutex,
	FEVENT **		ppEventListRV)
{
	pEvent->pEventClient->Release();
	f_mutexLock( hMutex);
	if (pEvent->pPrev)
	{
		pEvent->pPrev->pNext = pEvent->pNext;
	}
	else
	{
		*ppEventListRV = pEvent->pNext;
	}
	if (pEvent->pNext)
	{
		pEvent->pNext->pPrev = pEvent->pPrev;
	}
	f_mutexUnlock( hMutex);
	f_free( &pEvent);
}

/****************************************************************************
Desc:		This routine links a notify request into a notification list and
			then waits to be notified that the event has occurred.
Notes:	This routine assumes that the shared mutex is locked and that
			it is supposed to unlock it.
****************************************************************************/
RCODE flmWaitNotifyReq(
	F_MUTEX			hMutex,
	F_SEM				hSem,
	FNOTIFY **		ppNotifyListRV,
	void *			pvUserData)
{
	RCODE          rc = NE_SFLM_OK;
	RCODE          TempRc;
	FNOTIFY      	notifyItem;

	notifyItem.hSem = hSem;
	notifyItem.uiThreadId = f_threadId();
	notifyItem.pRc = &rc;
	notifyItem.pvUserData = pvUserData;
	notifyItem.pNext = *ppNotifyListRV;
	*ppNotifyListRV = &notifyItem;

	f_mutexUnlock( hMutex);
	
	if( RC_BAD( TempRc = f_semWait( notifyItem.hSem, F_WAITFOREVER)))
	{
		rc = TempRc;
	}

	f_mutexLock( hMutex);
	return( rc);
}

/****************************************************************************
Desc: This routine links an F_Database structure to its name hash bucket.
		NOTE: This function assumes that the global mutex has been
		locked.
****************************************************************************/
RCODE F_Database::linkToBucket( void)
{
	RCODE				rc = NE_SFLM_OK;
	F_Database *	pTmpDatabase;
	F_BUCKET *		pBucket;
	FLMUINT			uiBucket;

	pBucket = gv_SFlmSysData.pDatabaseHashTbl;
	uiBucket = f_strHashBucket( m_pszDbPath, pBucket, FILE_HASH_ENTRIES);
	pBucket = &pBucket [uiBucket];
	if (pBucket->pFirstInBucket)
	{
		pTmpDatabase = (F_Database *)pBucket->pFirstInBucket;
		pTmpDatabase->m_pPrev = this;
	}

	m_uiBucket = uiBucket;
	m_pPrev = NULL;
	m_pNext = (F_Database *)pBucket->pFirstInBucket;
	pBucket->pFirstInBucket = this;

	return( rc);
}

/****************************************************************************
Desc: This routine checks unused structures to see if any have been unused
		longer than the maximum unused time.  If so, it frees them up.
Note: This routine assumes that the calling routine has locked the global
		mutex prior to calling this routine.  The mutex may be unlocked and
		re-locked by one of the called routines.
****************************************************************************/
void F_DbSystem::checkNotUsedObjects( void)
{
}

/****************************************************************************
Desc: This routine links an FDB structure to an F_Database structure.
		NOTE: This routine assumes that the global mutex has been
		locked.
****************************************************************************/
RCODE F_Db::linkToDatabase(
	F_Database *			pDatabase)
{
	RCODE						rc = NE_SFLM_OK;
	F_SuperFileClient *	pSFileClient = NULL;

	// If the use count on the file used to be zero, unlink it from the
	// unused list.

	flmAssert( !m_pDatabase);
	m_pPrevForDatabase = NULL;
	if ((m_pNextForDatabase = pDatabase->m_pFirstDb) != NULL)
	{
		pDatabase->m_pFirstDb->m_pPrevForDatabase = this;
	}

	pDatabase->m_pFirstDb = this;
	m_pDatabase = pDatabase;

	if (!(m_uiFlags & FDB_INTERNAL_OPEN))
	{
		pDatabase->incrOpenCount();
	}

	// Allocate the super file object

	if( !m_pSFileHdl)
	{
		if( (m_pSFileHdl = f_new F_SuperFileHdl) == NULL)
		{
			rc = RC_SET( NE_SFLM_MEM);
			goto Exit;
		}

		if( (pSFileClient = f_new F_SuperFileClient) == NULL)
		{
			rc = RC_SET( NE_SFLM_MEM);
			goto Exit;
		}

		if( RC_BAD( rc = pSFileClient->setup( 
			pDatabase->m_pszDbPath, pDatabase->m_pszDataDir)))
		{
			goto Exit;
		}

		if( RC_BAD( rc = m_pSFileHdl->setup( pSFileClient, 
			gv_SFlmSysData.pFileHdlCache, gv_SFlmSysData.uiFileOpenFlags,
			gv_SFlmSysData.uiFileCreateFlags)))
		{
			goto Exit;
		}

		if( pDatabase->m_lastCommittedDbHdr.ui32DbVersion)
		{
//			JMC - FIXME: commented out due to missing functionality in flaimtk.h
//			m_pSFileHdl->setBlockSize( pDatabase->m_uiBlockSize);
		}
	}

Exit:

	if( pSFileClient)
	{
		pSFileClient->Release();
	}

	return( rc);
}

/****************************************************************************
Desc: This routine unlinks F_Db object from its F_Database structure.
		NOTE: This routine assumes that the global mutex has been
		locked.
****************************************************************************/
void F_Db::unlinkFromDatabase( void)
{
	if (!m_pDatabase)
	{
		return;
	}

	// Unlink the F_Db from the F_Database.

	if (m_pNextForDatabase)
	{
		m_pNextForDatabase->m_pPrevForDatabase = m_pPrevForDatabase;
	}

	if (m_pPrevForDatabase)
	{
		m_pPrevForDatabase->m_pNextForDatabase = m_pNextForDatabase;
	}
	else
	{
		m_pDatabase->m_pFirstDb = m_pNextForDatabase;
	}
	m_pNextForDatabase = m_pPrevForDatabase = NULL;

	// Decrement use counts in the F_Database, unless this was
	// an internal open.

	if (!(m_uiFlags & FDB_INTERNAL_OPEN))
	{
		m_pDatabase->decrOpenCount();
	}
	m_pDatabase = NULL;
}

/****************************************************************************
Desc: This routine dynamically adjusts the cache limit if that is the mode
		we are in.
****************************************************************************/
RCODE F_GlobalCacheMgr::adjustCache(
	FLMUINT *	puiCurrTime,
	FLMUINT *	puiLastCacheAdjustTime)
{
	RCODE			rc = NE_SFLM_OK;
	FLMUINT		uiCurrTime = *puiCurrTime;
	FLMUINT		uiLastCacheAdjustTime = *puiLastCacheAdjustTime;
	FLMBOOL		bMutexLocked = FALSE;
	
	if (m_bDynamicCacheAdjust &&
		 FLM_ELAPSED_TIME( uiCurrTime, uiLastCacheAdjustTime) >=
			m_uiCacheAdjustInterval)
	{
		FLMUINT	uiCacheBytes;
		
		lockMutex();
		bMutexLocked = TRUE;

		// Make sure the dynamic adjust flag is still set.

		if (m_bDynamicCacheAdjust &&
			 FLM_ELAPSED_TIME( uiCurrTime, uiLastCacheAdjustTime) >=
				m_uiCacheAdjustInterval)
		{
			if( RC_BAD( rc = flmGetCacheBytes( m_uiCacheAdjustPercent,
				m_uiCacheAdjustMin, m_uiCacheAdjustMax,
				m_uiCacheAdjustMinToLeave, TRUE, totalBytes(), &uiCacheBytes)))
			{
				goto Exit;
			}
			
			if (RC_BAD( rc = setCacheLimit( uiCacheBytes, FALSE)))
			{
				unlockMutex();
				goto Exit;
			}
		}
		
		unlockMutex();
		bMutexLocked = FALSE;
		
		*puiCurrTime = *puiLastCacheAdjustTime = FLM_GET_TIMER();
	}
	
Exit:

	if( bMutexLocked)
	{
		unlockMutex();
	}

	return( rc);
}

/****************************************************************************
Desc: This routine functions as a thread.  It monitors open files and
		frees up files which have been closed longer than the maximum
		close time.
****************************************************************************/
RCODE SQFAPI F_DbSystem::monitorThrd(
	IF_Thread *		pThread)
{
	FLMUINT		uiLastUnusedCleanupTime = 0;
	FLMUINT		uiCurrTime;
	FLMUINT		uiLastCacheAdjustTime = 0;

	for (;;)
	{
		// See if we should shut down

		if( pThread->getShutdownFlag())
		{
			break;
		}

		uiCurrTime = FLM_GET_TIMER();

		// Check the not used stuff and lock timeouts.

		if ( FLM_ELAPSED_TIME( uiCurrTime, uiLastUnusedCleanupTime) >=
					gv_SFlmSysData.pGlobalCacheMgr->m_uiUnusedCleanupInterval)
		{
			// See if any unused structures have bee unused longer than the
			// maximum unused time.  Free them if they have.
			// May unlock and re-lock the global mutex.

			f_mutexLock( gv_SFlmSysData.hShareMutex);
			F_DbSystem::checkNotUsedObjects();
			f_mutexUnlock( gv_SFlmSysData.hShareMutex);

			// Reset the timer

			uiCurrTime = uiLastUnusedCleanupTime = FLM_GET_TIMER();
		}

		// Check the adjusting cache limit

		(void)gv_SFlmSysData.pGlobalCacheMgr->adjustCache( &uiCurrTime,
								&uiLastCacheAdjustTime);

		f_sleep( 1000);
	}

	return( NE_SFLM_OK);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE F_DbSystem::cacheCleanupThrd(
	IF_Thread *		pThread)
{
	FLMUINT		uiCurrTime;
	FLMUINT		uiLastDefragTime = 0;
	FLMUINT		uiLastCleanupTime = 0;
	FLMUINT		uiDefragInterval;
	FLMUINT		uiCleanupInterval = gv_SFlmSysData.pGlobalCacheMgr->m_uiCacheCleanupInterval;
	FLMBOOL		bDoRowCacheFirst = TRUE;
	
	uiDefragInterval = FLM_SECS_TO_TIMER_UNITS( 120);
	
	for (;;)
	{
		if( pThread->getShutdownFlag())
		{
			break;
		}
		
		uiCurrTime = FLM_GET_TIMER();
		
		// Alternate between reducing row cache and block cache first.
		
		if (gv_SFlmSysData.pGlobalCacheMgr->cacheOverLimit() ||
			FLM_ELAPSED_TIME( uiCurrTime, uiLastCleanupTime) >= uiCleanupInterval)
		{
			if (bDoRowCacheFirst)
			{
				f_mutexLock( gv_SFlmSysData.hRowCacheMutex);
				gv_SFlmSysData.pRowCacheMgr->reduceCache();
				f_mutexUnlock( gv_SFlmSysData.hRowCacheMutex);
				
				f_mutexLock( gv_SFlmSysData.hBlockCacheMutex);
				(void)gv_SFlmSysData.pBlockCacheMgr->reduceCache( NULL);
				f_mutexUnlock( gv_SFlmSysData.hBlockCacheMutex);
				
				bDoRowCacheFirst = FALSE;
			}
			else
			{
				f_mutexLock( gv_SFlmSysData.hBlockCacheMutex);
				(void)gv_SFlmSysData.pBlockCacheMgr->reduceCache( NULL);
				f_mutexUnlock( gv_SFlmSysData.hBlockCacheMutex);

				f_mutexLock( gv_SFlmSysData.hRowCacheMutex);
				gv_SFlmSysData.pRowCacheMgr->reduceCache();
				f_mutexUnlock( gv_SFlmSysData.hRowCacheMutex);
				
				bDoRowCacheFirst = TRUE;
			}

			uiLastCleanupTime = FLM_GET_TIMER();
		}
		
		if( FLM_ELAPSED_TIME( uiCurrTime, uiLastDefragTime) >= uiDefragInterval)
		{
			gv_SFlmSysData.pBlockCacheMgr->defragmentMemory();
			gv_SFlmSysData.pRowCacheMgr->defragmentMemory();
			uiLastDefragTime = FLM_GET_TIMER();
		}

		f_sleep( 500);
	}

	return( NE_SFLM_OK);
}

/****************************************************************************
Desc: This routine does an event callback.  Note that the mutex is
		locked during the callback.
****************************************************************************/
void flmDoEventCallback(
	eEventCategory	eCategory,
	eEventType			eEvent,
	F_Db *				pDb,
	FLMUINT				uiThreadId,
	FLMUINT64			ui64TransID,
	FLMUINT				uiIndexOrTable,
	FLMUINT64			ui64RowId,
	RCODE					rc)
{
	FEVENT *	pEvent;

	f_mutexLock( gv_SFlmSysData.EventHdrs [eCategory].hMutex);
	pEvent = gv_SFlmSysData.EventHdrs [eCategory].pEventCBList;
	while (pEvent)
	{
		pEvent->pEventClient->catchEvent( eEvent, pDb, uiThreadId, ui64TransID,
										uiIndexOrTable, ui64RowId, rc);
		pEvent = pEvent->pNext;
	}
	f_mutexUnlock( gv_SFlmSysData.EventHdrs [eCategory].hMutex);
}

/****************************************************************************
Desc:		This routine sets the "must close" flags on the
			F_Database and its FDBs
****************************************************************************/
void F_Database::setMustCloseFlags(
	RCODE				rcMustClose,
	FLMBOOL			bMutexLocked)
{
	F_Db *				pTmpDb;

	if( !bMutexLocked)
	{
		f_mutexLock( gv_SFlmSysData.hShareMutex);
	}

	if( !m_bMustClose)
	{
		m_bMustClose = TRUE;
		m_rcMustClose = rcMustClose;
		pTmpDb = m_pFirstDb;
		while( pTmpDb)
		{
			pTmpDb->m_bMustClose = TRUE;
			pTmpDb = pTmpDb->m_pNextForDatabase;
		}

		// Log a message indicating why the "must close" flag has been
		// set.  Calling checkState with the bMustClose flag
		// already set to TRUE will cause a message to be logged.

		(void)checkState( __FILE__, __LINE__);
	}

	if( !bMutexLocked)
	{
		f_mutexUnlock( gv_SFlmSysData.hShareMutex);
	}
}

/***************************************************************************
Desc:	Lock the system data structure for access - called only by startup
		and shutdown.  NOTE: On platforms that do not support atomic exchange
		this is less than perfect - won't handle tight race conditions.
***************************************************************************/
void lockSysData( void)
{
	while( f_atomicExchange( &gv_flmSysSpinLock, 1) == 1)
	{
		f_sleep( 10);
	}
}

/***************************************************************************
Desc:	Unlock the system data structure for access - called only by startup
		and shutdown.
***************************************************************************/
void unlockSysData( void)
{
	(void)f_atomicExchange( &gv_flmSysSpinLock, 0);
}

/****************************************************************************
Desc :	Constructor for global cache manager.
****************************************************************************/
F_GlobalCacheMgr::F_GlobalCacheMgr()
{
	m_pSlabManager = NULL;
	m_bCachePreallocated = FALSE;
	
	m_bDynamicCacheAdjust = f_canGetMemoryInfo();
	m_uiCacheAdjustPercent = SFLM_DEFAULT_CACHE_ADJUST_PERCENT;
	m_uiCacheAdjustMin = SFLM_DEFAULT_CACHE_ADJUST_MIN;
	m_uiCacheAdjustMax = SFLM_DEFAULT_CACHE_ADJUST_MAX;
	m_uiCacheAdjustMinToLeave = SFLM_DEFAULT_CACHE_ADJUST_MIN_TO_LEAVE;
	m_uiCacheAdjustInterval = FLM_SECS_TO_TIMER_UNITS( 
										SFLM_DEFAULT_CACHE_ADJUST_INTERVAL);
	flmGetCacheBytes( m_uiCacheAdjustPercent, m_uiCacheAdjustMin,
		m_uiCacheAdjustMax, m_uiCacheAdjustMinToLeave, TRUE, 0, &m_uiMaxBytes);
	m_uiCacheCleanupInterval = FLM_SECS_TO_TIMER_UNITS( 
											SFLM_DEFAULT_CACHE_CLEANUP_INTERVAL);
	m_uiUnusedCleanupInterval = FLM_SECS_TO_TIMER_UNITS( 
											SFLM_DEFAULT_UNUSED_CLEANUP_INTERVAL);
	m_hMutex = F_MUTEX_NULL;
	m_uiMaxSlabs = 0;
}
	
/****************************************************************************
Desc:
****************************************************************************/
F_GlobalCacheMgr::~F_GlobalCacheMgr()
{
	if (m_pSlabManager)
	{
		m_pSlabManager->Release();
	}

	if (m_hMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &m_hMutex);
	}
}
	
/****************************************************************************
Desc:
****************************************************************************/
RCODE F_GlobalCacheMgr::setup( void)
{
	RCODE			rc = NE_SFLM_OK;
	FLMBYTE		szTmpBuffer[ 64];
	
	flmAssert( !m_pSlabManager);
	
	if (RC_BAD( rc = f_mutexCreate( &m_hMutex)))
	{
		goto Exit;
	}
	
	if( RC_BAD( rc = FlmAllocSlabManager( &m_pSlabManager)))
	{
		goto Exit;
	}
	
	f_getenv( "SFLM_PREALLOC_CACHE_SIZE", szTmpBuffer, sizeof( szTmpBuffer));
	
	if( RC_BAD( rc = m_pSlabManager->setup( f_atoi( (char *)szTmpBuffer)))) 
	{
		m_pSlabManager->Release();
		m_pSlabManager = NULL;
		goto Exit;
	}
	
	// Need to set max slabs here because we didn't know the slab size
	// in the constructor.
	
	m_uiMaxSlabs = m_uiMaxBytes / m_pSlabManager->getSlabSize();

Exit:

	return( rc);
}

/****************************************************************************
Desc: This routine sets the limits for record cache and block cache - dividing
		the total cache between the two caches.  It uses the same ratio
		currently in force.
****************************************************************************/
RCODE F_GlobalCacheMgr::setCacheLimit(
	FLMUINT		uiNewTotalCacheSize,
	FLMBOOL		bPreallocateCache)
{
	RCODE			rc = NE_FLM_OK;
	FLMUINT		uiOldCacheSize = m_uiMaxBytes;
	
	if( uiNewTotalCacheSize > FLM_MAX_CACHE_SIZE)
	{
		uiNewTotalCacheSize = FLM_MAX_CACHE_SIZE;
	}
	
	if( bPreallocateCache)
	{
		if( m_bDynamicCacheAdjust)
		{
			// Can't pre-allocate and dynamically adjust.

			bPreallocateCache = FALSE;
		}
		else
		{
			if( RC_BAD( rc = m_pSlabManager->resize( 
				uiNewTotalCacheSize, TRUE, &uiNewTotalCacheSize)))
			{
				bPreallocateCache = FALSE;
			}
		}
	}
	
	m_uiMaxBytes = uiNewTotalCacheSize;
	m_uiMaxSlabs = m_uiMaxBytes / m_pSlabManager->getSlabSize();
	m_bCachePreallocated = bPreallocateCache;
	
	if( uiNewTotalCacheSize < uiOldCacheSize)
	{
		f_mutexLock( gv_SFlmSysData.hRowCacheMutex);
		gv_SFlmSysData.pRowCacheMgr->reduceCache();
		f_mutexUnlock( gv_SFlmSysData.hRowCacheMutex);
		
		f_mutexLock( gv_SFlmSysData.hBlockCacheMutex);
		gv_SFlmSysData.pBlockCacheMgr->reduceCache( NULL);
		f_mutexUnlock( gv_SFlmSysData.hBlockCacheMutex);
	}
	
	if( !bPreallocateCache)
	{
		m_pSlabManager->resize( uiNewTotalCacheSize, FALSE);
	}
	
	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE F_GlobalCacheMgr::clearCache(
	F_Db *		pDb)
{
	RCODE			rc = NE_SFLM_OK;
	FLMUINT		uiSavedMaxBytes;
	FLMUINT		uiSavedMaxSlabs;

	lockMutex();

	uiSavedMaxBytes = m_uiMaxBytes;
	uiSavedMaxSlabs = m_uiMaxSlabs;
	
	m_uiMaxBytes = 0;
	m_uiMaxSlabs = 0;

	f_mutexLock( gv_SFlmSysData.hRowCacheMutex);
	gv_SFlmSysData.pRowCacheMgr->reduceCache();
	f_mutexUnlock( gv_SFlmSysData.hRowCacheMutex);
	
	f_mutexLock( gv_SFlmSysData.hBlockCacheMutex);
	rc = gv_SFlmSysData.pBlockCacheMgr->reduceCache( (F_Db *)pDb);
	f_mutexUnlock( gv_SFlmSysData.hBlockCacheMutex);

	if( RC_BAD( rc))
	{
		goto Exit;
	}

Exit:

	m_uiMaxBytes = uiSavedMaxBytes;
	m_uiMaxSlabs = uiSavedMaxSlabs;
	unlockMutex();
	return( rc);
}

/****************************************************************************
Desc : Startup the database engine.
Notes: This routine may be called multiple times.  However, if that is done
		 exit() should be called for each time this is called successfully.
		 This routine does not handle race conditions on platforms that do
		 not support atomic increment.
****************************************************************************/
RCODE F_DbSystem::init( void)
{
	RCODE				rc = NE_SFLM_OK;
	FLMINT			iEventCategory;
#ifdef FLM_USE_NICI
	int				iHandle;
#endif

	lockSysData();

	// See if FLAIM has already been started.  If so,
	// we are done.

	if (++m_uiFlmSysStartupCount > 1)
	{
		goto Exit;
	}
	
	if( RC_BAD( rc = ftkStartup()))
	{
		goto Exit;
	}
	gv_bToolkitStarted = TRUE;

	// The memset needs to be first.

	f_memset( &gv_SFlmSysData, 0, sizeof( FLMSYSDATA));
	gv_SFlmSysData.uiMaxFileSize = f_getMaxFileSize();

	// Get the thread manager

	if( RC_BAD( rc = FlmGetThreadMgr( &gv_SFlmSysData.pThreadMgr)))
	{
		goto Exit;
	}

	// Get the file system manager

	if( RC_BAD( rc = FlmGetFileSystem( &gv_SFlmSysData.pFileSystem)))
	{
		goto Exit;
	}
	
	// Set up a file handle cache
	
	if( RC_BAD( rc = gv_SFlmSysData.pFileSystem->allocFileHandleCache( 32, 120, 
		&gv_SFlmSysData.pFileHdlCache)))
	{
		goto Exit;
	}
	
	gv_SFlmSysData.uiIndexingThreadGroup = 
		gv_SFlmSysData.pThreadMgr->allocGroupId();
		
	gv_SFlmSysData.uiCheckpointThreadGroup = 
		gv_SFlmSysData.pThreadMgr->allocGroupId(); 
	
	// Sanity check -- make sure we are using the correct
	// byte-swap macros for this platform

	flmAssert( FB2UD( (FLMBYTE *)"\x0A\x0B\x0C\x0D") == 0x0D0C0B0A);
	flmAssert( FB2UW( (FLMBYTE *)"\x0A\x0B") == 0x0B0A);

	gv_SFlmSysData.hRowCacheMutex = F_MUTEX_NULL;
	gv_SFlmSysData.hBlockCacheMutex = F_MUTEX_NULL;
	gv_SFlmSysData.hShareMutex = F_MUTEX_NULL;
	gv_SFlmSysData.hStatsMutex = F_MUTEX_NULL;
	gv_SFlmSysData.hLoggerMutex = F_MUTEX_NULL;
	gv_SFlmSysData.hIniMutex = F_MUTEX_NULL;
	
	// Initialize the event categories to have no mutex.

	for (iEventCategory = 0;
		  iEventCategory < SFLM_MAX_EVENT_CATEGORIES;
		  iEventCategory++)
	{
		gv_SFlmSysData.EventHdrs [iEventCategory].hMutex = F_MUTEX_NULL;
	}

	initFastBlockCheckSum();

	if (RC_BAD( rc = flmVerifyDiskStructOffsets()))
	{
		goto Exit;
	}

	gv_SFlmSysData.uiFileOpenFlags = 
		FLM_IO_RDWR | FLM_IO_SH_DENYNONE | FLM_IO_DIRECT;

	gv_SFlmSysData.uiFileCreateFlags = 
		gv_SFlmSysData.uiFileOpenFlags | FLM_IO_EXCL | FLM_IO_CREATE_DIR;

	// Initialize all of the fields

	gv_SFlmSysData.uiMaxUnusedTime = FLM_SECS_TO_TIMER_UNITS( 
			SFLM_DEFAULT_MAX_UNUSED_TIME);
	gv_SFlmSysData.uiMaxCPInterval = FLM_SECS_TO_TIMER_UNITS( 
			SFLM_DEFAULT_MAX_CP_INTERVAL);
		
	gv_SFlmSysData.uiRehashAfterFailureBackoffTime = 
			FLM_SECS_TO_TIMER_UNITS( SFLM_DEFAULT_REHASH_BACKOFF_INTERVAL);
		
	if ((gv_SFlmSysData.pGlobalCacheMgr = f_new F_GlobalCacheMgr) == NULL)
	{
		rc = RC_SET( NE_SFLM_MEM);
		goto Exit;
	}

	if( RC_BAD( rc = gv_SFlmSysData.pGlobalCacheMgr->setup()))
	{
		goto Exit;
	}

	// Create the mutexes for controlling access to cache and global structures.

	if (RC_BAD( rc = f_mutexCreate( &gv_SFlmSysData.hRowCacheMutex)))
	{
		goto Exit;
	}
	if (RC_BAD( rc = f_mutexCreate( &gv_SFlmSysData.hBlockCacheMutex)))
	{
		goto Exit;
	}
	if (RC_BAD( rc = f_mutexCreate( &gv_SFlmSysData.hShareMutex)))
	{
		goto Exit;
	}

	if ((gv_SFlmSysData.pBlockCacheMgr = f_new F_BlockCacheMgr) == NULL)
	{
		rc = RC_SET( NE_SFLM_MEM);
		goto Exit;
	}
	if (RC_BAD( rc = gv_SFlmSysData.pBlockCacheMgr->initCache()))
	{
		goto Exit;
	}
	if ((gv_SFlmSysData.pRowCacheMgr = f_new F_RowCacheMgr) == NULL)
	{
		rc = RC_SET( NE_SFLM_MEM);
		goto Exit;
	}
	if (RC_BAD( rc = gv_SFlmSysData.pRowCacheMgr->initCache()))
	{
		goto Exit;
	}

	if (RC_BAD( rc = f_mutexCreate( &gv_SFlmSysData.hQueryMutex)))
	{
		goto Exit;
	}

	if (RC_BAD( rc = f_mutexCreate( &gv_SFlmSysData.hIniMutex)))
	{
		goto Exit;
	}
	
	// Initialize a statistics structure.

	if (RC_BAD( rc = f_mutexCreate( &gv_SFlmSysData.hStatsMutex)))
	{
		goto Exit;
	}
	
	f_memset( &gv_SFlmSysData.Stats, 0, sizeof( SFLM_STATS));
	gv_SFlmSysData.bStatsInitialized = TRUE;
	
	// Initialize the logging mutex
	
	if( RC_BAD( rc = f_mutexCreate( &gv_SFlmSysData.hLoggerMutex)))
	{
		goto Exit;
	}

	// Allocate memory for the file name hash table.

	if (RC_BAD(rc = f_allocHashTable( FILE_HASH_ENTRIES,
								&gv_SFlmSysData.pDatabaseHashTbl)))
	{
		goto Exit;
	}
	
#ifdef FLM_DBG_LOG
	flmDbgLogInit();
#endif

	// Set up mutexes for the event table.

	for (iEventCategory = 0;
		  iEventCategory < SFLM_MAX_EVENT_CATEGORIES;
		  iEventCategory++)
	{
		if (RC_BAD( rc = f_mutexCreate(
								&gv_SFlmSysData.EventHdrs [iEventCategory].hMutex)))
		{
			goto Exit;
		}
	}

	// Start the monitor thread

	if (RC_BAD( rc = gv_SFlmSysData.pThreadMgr->createThread( 
		&gv_SFlmSysData.pMonitorThrd,
		F_DbSystem::monitorThrd, "DB Monitor")))
	{
		goto Exit;
	}

	// Start the cache cleanup thread

	if (RC_BAD( rc = gv_SFlmSysData.pThreadMgr->createThread( 
		&gv_SFlmSysData.pCacheCleanupThrd,
		F_DbSystem::cacheCleanupThrd, "Cache Cleanup Thread")))
	{
		goto Exit;
	}
	
	if ((gv_SFlmSysData.pBtPool = f_new F_BtPool()) == NULL)
	{
		rc = RC_SET( NE_SFLM_MEM);
		goto Exit;
	}

	if (RC_BAD( rc = gv_SFlmSysData.pBtPool->btpInit()))
	{
		goto Exit;
	}

	if ((gv_SFlmSysData.pBTreeIStreamPool = f_new F_BTreeIStreamPool) == NULL)
	{
		rc = RC_SET( NE_SFLM_MEM);
		goto Exit;
	}
	
	if (RC_BAD( rc = gv_SFlmSysData.pBTreeIStreamPool->setup()))
	{
		goto Exit;
	}
	
#ifdef FLM_USE_NICI
	iHandle  = f_getpid();

	// Initialize NICI

	if( RC_BAD( rc = CCS_Init( &iHandle)))
	{
		rc = RC_SET( NE_SFLM_NICI_INIT_FAILED);
		goto Exit;
	}
#endif

	// Initialize the XFlaim cache settings from the .ini file (if present)

	readIniFile();

Exit:

	// If not successful, free up any resources that were allocated.

	if (RC_BAD( rc))
	{
		cleanup();
	}

	unlockSysData();
	return( rc);
}

/****************************************************************************
Desc:		Shuts the database engine down.
Notes:	This routine allows itself to be called multiple times, even before
			init() is called or if  init() fails.  May not handle race
			conditions very well on platforms that do not support atomic
			exchange.
****************************************************************************/
void F_DbSystem::exit( void)
{
	lockSysData();
	cleanup();
	unlockSysData();
}

/************************************************************************
Desc : Cleans up - assumes that the spin lock has already been
		 obtained.  This allows it to be called directly from
		 init() on error conditions.
************************************************************************/
void F_DbSystem::cleanup( void)
{
	FLMUINT		uiCnt;
	FLMINT		iEventCategory;

	// NOTE: We are checking and decrementing a global variable here.
	// However, on platforms that properly support atomic exchange,
	// we are OK, because the caller has obtained a spin lock before
	// calling this routine, so we are guaranteed to be the only thread
	// executing this code at this point.  On platforms that don't
	// support atomic exchange, our spin lock will be less reliable for
	// really tight race conditions.  But in reality, nobody should be
	// calling FlmStartup and FlmShutdown in race conditions like that
	// anyway.  We are only doing the spin lock stuff to try and be
	// nice about it if they are.

	// This check allows FlmShutdown to be called before calling
	// FlmStartup, or even if FlmStartup fails.

	if (!m_uiFlmSysStartupCount)
	{
		return;
	}

	// If we decrement the count and it doesn't go to zero, we are not
	// ready to do cleanup yet.

	if (--m_uiFlmSysStartupCount > 0)
	{
		return;
	}

	// Free any queries that have been saved in the query list.

	if (gv_SFlmSysData.hQueryMutex != F_MUTEX_NULL)
	{

		// Setting uiMaxQueries to zero will cause flmFreeSavedQueries
		// to free the entire list.  Also, embedded queries will not be
		// added back into the list when uiMaxQueries is zero.

		gv_SFlmSysData.uiMaxQueries = 0;
		flmFreeSavedQueries( FALSE);
	}

	// Shut down the monitor thread, if there is one.
	
	if( gv_SFlmSysData.pMonitorThrd)
	{
		gv_SFlmSysData.pMonitorThrd->stopThread();
		gv_SFlmSysData.pMonitorThrd->Release();
		gv_SFlmSysData.pMonitorThrd = NULL;
	}
	
	// Shut down the cache reduce thread
	
	if( gv_SFlmSysData.pCacheCleanupThrd)
	{
		gv_SFlmSysData.pCacheCleanupThrd->stopThread();
		gv_SFlmSysData.pCacheCleanupThrd->Release();
		gv_SFlmSysData.pCacheCleanupThrd = NULL;
	}

	// Release the file handle cache
	
	if( gv_SFlmSysData.pFileHdlCache)
	{
		gv_SFlmSysData.pFileHdlCache->Release();
		gv_SFlmSysData.pFileHdlCache = NULL;
	}
	
	// Release the thread manager

	if( gv_SFlmSysData.pThreadMgr)
	{
		gv_SFlmSysData.pThreadMgr->Release();
		gv_SFlmSysData.pThreadMgr = NULL;
	}

	// Free all of the files and associated structures

	if (gv_SFlmSysData.pDatabaseHashTbl)
	{
		F_BUCKET *   pDatabaseHashTbl;

		// F_Database destructor expects the global mutex to be locked
		// IMPORTANT NOTE: pDatabaseHashTbl is ALWAYS allocated
		// AFTER the mutex is allocated, so we are guaranteed
		// to have a mutex if pDatabaseHashTbl is non-NULL.

		f_mutexLock( gv_SFlmSysData.hShareMutex);
		for (uiCnt = 0, pDatabaseHashTbl = gv_SFlmSysData.pDatabaseHashTbl;
			  uiCnt < FILE_HASH_ENTRIES;
			  uiCnt++, pDatabaseHashTbl++)
		{
			F_Database *	pDatabase = (F_Database *)pDatabaseHashTbl->pFirstInBucket;
			F_Database *	pTmpDatabase;

			while (pDatabase)
			{
				pTmpDatabase = pDatabase;
				pDatabase = pDatabase->m_pNext;
				pTmpDatabase->freeDatabase();
			}
			pDatabaseHashTbl->pFirstInBucket = NULL;
		}

		// Unlock the global mutex
		f_mutexUnlock( gv_SFlmSysData.hShareMutex);

		// Free the hash table
		f_free( &gv_SFlmSysData.pDatabaseHashTbl);
	}

	// Free the statistics.

	if (gv_SFlmSysData.bStatsInitialized)
	{
		f_mutexLock( gv_SFlmSysData.hStatsMutex);
		flmStatFree( &gv_SFlmSysData.Stats);
		f_mutexUnlock( gv_SFlmSysData.hStatsMutex);
		gv_SFlmSysData.bStatsInitialized = FALSE;
	}
	
	if (gv_SFlmSysData.hStatsMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_SFlmSysData.hStatsMutex);
	}

	// Make sure the purge list is empty

	if( gv_SFlmSysData.pRowCacheMgr->m_pPurgeList)
	{
		f_mutexLock( gv_SFlmSysData.hRowCacheMutex);
		gv_SFlmSysData.pRowCacheMgr->cleanupPurgedCache();
		f_mutexUnlock( gv_SFlmSysData.hRowCacheMutex);
		flmAssert( !gv_SFlmSysData.pRowCacheMgr->m_pPurgeList);
	}

	// Free the row cache manager

	if( gv_SFlmSysData.pRowCacheMgr)
	{
		gv_SFlmSysData.pRowCacheMgr->Release();
		gv_SFlmSysData.pRowCacheMgr = NULL;
	}
	
	// Free the block cache manager

	if( gv_SFlmSysData.pBlockCacheMgr)
	{
		gv_SFlmSysData.pBlockCacheMgr->Release();
		gv_SFlmSysData.pBlockCacheMgr = NULL;
	}

	// Free up callbacks that have been registered for events.

	for (iEventCategory = 0;
		  iEventCategory < SFLM_MAX_EVENT_CATEGORIES;
		  iEventCategory++)
	{
		if (gv_SFlmSysData.EventHdrs [iEventCategory].hMutex != F_MUTEX_NULL)
		{
			while (gv_SFlmSysData.EventHdrs [iEventCategory].pEventCBList)
			{
				flmFreeEvent(
					gv_SFlmSysData.EventHdrs [iEventCategory].pEventCBList,
					gv_SFlmSysData.EventHdrs [iEventCategory].hMutex,
					&gv_SFlmSysData.EventHdrs [iEventCategory].pEventCBList);
			}
			f_mutexDestroy( &gv_SFlmSysData.EventHdrs [iEventCategory].hMutex);
		}
	}

	// Release the file system object

	if (gv_SFlmSysData.pFileSystem)
	{
		gv_SFlmSysData.pFileSystem->Release();
		gv_SFlmSysData.pFileSystem = NULL;
	}

#ifdef FLM_DBG_LOG
	flmDbgLogExit();
#endif

	// Release the logger
	
	if( gv_SFlmSysData.pLogger)
	{
		flmAssert( !gv_SFlmSysData.uiPendingLogMessages);
		gv_SFlmSysData.pLogger->Release();
		gv_SFlmSysData.pLogger = NULL;
	}

	if( gv_SFlmSysData.hLoggerMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_SFlmSysData.hLoggerMutex);
	}
	
	// Free the btree pool

	if( gv_SFlmSysData.pBtPool)
	{
		gv_SFlmSysData.pBtPool->Release();
		gv_SFlmSysData.pBtPool = NULL;
	}

	// Free the b-tree i-stream pool

	if( gv_SFlmSysData.pBTreeIStreamPool)
	{
		gv_SFlmSysData.pBTreeIStreamPool->Release();
		gv_SFlmSysData.pBTreeIStreamPool = NULL;
	}

	// Release the global cache manager.  NOTE: This must happen
	// only AFTER releasing the row cache manager and block cache
	// manager.

	if( gv_SFlmSysData.pGlobalCacheMgr)
	{
		gv_SFlmSysData.pGlobalCacheMgr->Release();
		gv_SFlmSysData.pGlobalCacheMgr = NULL;
	}

#ifdef FLM_USE_NICI
	CCS_Shutdown();
#endif

	// Free the mutexes last of all.

	if (gv_SFlmSysData.hQueryMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_SFlmSysData.hQueryMutex);
	}

	if (gv_SFlmSysData.hRowCacheMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_SFlmSysData.hRowCacheMutex);
	}
	if (gv_SFlmSysData.hBlockCacheMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_SFlmSysData.hBlockCacheMutex);
	}
	if (gv_SFlmSysData.hShareMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_SFlmSysData.hShareMutex);
	}

	if (gv_SFlmSysData.hIniMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_SFlmSysData.hIniMutex);
	}
	
	if( gv_bToolkitStarted)
	{
		ftkShutdown();
		gv_bToolkitStarted = FALSE;
	}
}

/****************************************************************************
Desc:	Configures how memory will be dynamically regulated.
****************************************************************************/
RCODE F_GlobalCacheMgr::setDynamicMemoryLimit(
	FLMUINT		uiCacheAdjustPercent,
	FLMUINT		uiCacheAdjustMin,
	FLMUINT		uiCacheAdjustMax,
	FLMUINT		uiCacheAdjustMinToLeave)
{
	RCODE			rc = NE_SFLM_OK;
	FLMUINT		uiCacheBytes;
	FLMBOOL		bMutexLocked = FALSE;
	
	if( !f_canGetMemoryInfo())
	{
		rc = RC_SET( NE_SFLM_NOT_IMPLEMENTED);
		goto Exit;
	}

	lockMutex();
	bMutexLocked = TRUE;
	
	m_bDynamicCacheAdjust = TRUE;
	flmAssert( uiCacheAdjustPercent > 0 && uiCacheAdjustPercent <= 100);
	m_uiCacheAdjustPercent = uiCacheAdjustPercent;
	m_uiCacheAdjustMin = uiCacheAdjustMin;
	m_uiCacheAdjustMax = uiCacheAdjustMax;
	m_uiCacheAdjustMinToLeave = uiCacheAdjustMinToLeave;
	
	if( RC_BAD( rc = flmGetCacheBytes( m_uiCacheAdjustPercent,
							m_uiCacheAdjustMin, m_uiCacheAdjustMax,
							m_uiCacheAdjustMinToLeave, TRUE, totalBytes(), 
							&uiCacheBytes)))
	{
		goto Exit;
	}

	if( RC_BAD( rc = setCacheLimit( uiCacheBytes, FALSE)))
	{
		goto Exit;
	}
	
Exit:
	
	if( bMutexLocked)
	{
		unlockMutex();
	}
	
	return( rc);
}

/****************************************************************************
Desc:		Sets a hard memory limit for cache.
****************************************************************************/
RCODE F_GlobalCacheMgr::setHardMemoryLimit(
	FLMUINT		uiPercent,
	FLMBOOL		bPercentOfAvail,
	FLMUINT		uiMin,
	FLMUINT		uiMax,
	FLMUINT		uiMinToLeave,
	FLMBOOL		bPreallocate)
{
	RCODE			rc = NE_SFLM_OK;
	FLMBOOL		bMutexLocked = FALSE;

	lockMutex();
	bMutexLocked = TRUE;
	
	m_bDynamicCacheAdjust = FALSE;
	if (uiPercent)
	{
		FLMUINT	uiCacheBytes;

		if( RC_BAD( rc = flmGetCacheBytes( uiPercent, uiMin,
			uiMax, uiMinToLeave, bPercentOfAvail, 
			totalBytes(), &uiCacheBytes)))
		{
			goto Exit;
		}
		
		if( RC_BAD( rc = setCacheLimit( uiCacheBytes, bPreallocate)))
		{
			goto Exit;
		}
	}
	else
	{
		if( RC_BAD( rc = setCacheLimit( uiMax, bPreallocate)))
		{
			goto Exit;
		}
	}
	
Exit:

	if( bMutexLocked)
	{
		unlockMutex();
	}
	
	return( rc);
}

/****************************************************************************
Desc:		Returns information about memory usage
****************************************************************************/
void F_GlobalCacheMgr::getCacheInfo(
	SFLM_CACHE_INFO *			pCacheInfo)
{
	f_memset( pCacheInfo, 0, sizeof( SFLM_CACHE_INFO));
	
	lockMutex();
	pCacheInfo->uiMaxBytes = m_uiMaxBytes;
	pCacheInfo->uiTotalBytesAllocated = totalBytes();
	pCacheInfo->bDynamicCacheAdjust = m_bDynamicCacheAdjust;
	pCacheInfo->uiCacheAdjustPercent = m_uiCacheAdjustPercent;
	pCacheInfo->uiCacheAdjustMin = m_uiCacheAdjustMin;
	pCacheInfo->uiCacheAdjustMax = m_uiCacheAdjustMax;
	pCacheInfo->uiCacheAdjustMinToLeave = m_uiCacheAdjustMinToLeave;
	pCacheInfo->bPreallocatedCache = m_bCachePreallocated;
	unlockMutex();

	// Return block cache information.

	f_mutexLock( gv_SFlmSysData.hBlockCacheMutex);
	f_memcpy( &pCacheInfo->BlockCache, &gv_SFlmSysData.pBlockCacheMgr->m_Usage,
					sizeof( SFLM_CACHE_USAGE));

	pCacheInfo->uiFreeBytes = gv_SFlmSysData.pBlockCacheMgr->m_uiFreeBytes;
	pCacheInfo->uiFreeCount = gv_SFlmSysData.pBlockCacheMgr->m_uiFreeCount;
	pCacheInfo->uiReplaceableCount = gv_SFlmSysData.pBlockCacheMgr->m_uiReplaceableCount;
	pCacheInfo->uiReplaceableBytes = gv_SFlmSysData.pBlockCacheMgr->m_uiReplaceableBytes;

	f_mutexUnlock( gv_SFlmSysData.hBlockCacheMutex);
		
	// Return row cache information.

	f_mutexLock( gv_SFlmSysData.hRowCacheMutex);
	f_memcpy( &pCacheInfo->RowCache, &gv_SFlmSysData.pRowCacheMgr->m_Usage,
					sizeof( SFLM_CACHE_USAGE));
	f_mutexUnlock( gv_SFlmSysData.hRowCacheMutex);
	
	// Gather per-database statistics

	f_mutexLock( gv_SFlmSysData.hShareMutex);
	if( gv_SFlmSysData.pDatabaseHashTbl)
	{
		FLMUINT			uiLoop;
		F_Database *	pDatabase;

		for( uiLoop = 0; uiLoop < FILE_HASH_ENTRIES; uiLoop++)
		{
			if( (pDatabase = (F_Database *)gv_SFlmSysData.pDatabaseHashTbl[
				uiLoop].pFirstInBucket) != NULL)
			{
				while( pDatabase)
				{
					if( pDatabase->m_uiDirtyCacheCount)
					{
						pCacheInfo->uiDirtyBytes +=
							pDatabase->m_uiDirtyCacheCount * pDatabase->m_uiBlockSize;
						pCacheInfo->uiDirtyCount += pDatabase->m_uiDirtyCacheCount;
					}

					if( pDatabase->m_uiNewCount)
					{
						pCacheInfo->uiNewBytes +=
							pDatabase->m_uiNewCount * pDatabase->m_uiBlockSize;
						pCacheInfo->uiNewCount += pDatabase->m_uiNewCount;
					}

					if( pDatabase->m_uiLogCacheCount)
					{
						pCacheInfo->uiLogBytes +=
							pDatabase->m_uiLogCacheCount * pDatabase->m_uiBlockSize;
						pCacheInfo->uiLogCount += pDatabase->m_uiLogCacheCount;
					}

					pDatabase = pDatabase->m_pNext;
				}
			}
		}
	}

	f_mutexUnlock( gv_SFlmSysData.hShareMutex);
}

/****************************************************************************
Desc:		Close all files in the file handle manager that have not been
			used for the specified number of seconds.
****************************************************************************/
RCODE F_DbSystem::closeUnusedFiles(
	FLMUINT		uiSeconds)
{
	RCODE			rc = NE_SFLM_OK;
	FLMUINT		uiValue;
	FLMUINT		uiCurrTime;
	FLMUINT		uiSave;

	// Convert seconds to timer units

	uiValue = FLM_SECS_TO_TIMER_UNITS( uiSeconds);

	// Free any other unused structures that have not been used for the
	// specified amount of time.

	uiCurrTime = (FLMUINT)FLM_GET_TIMER();
	f_mutexLock( gv_SFlmSysData.hShareMutex);

	// Temporarily set the maximum unused seconds in the FLMSYSDATA structure
	// to the value that was passed in to Value1.  Restore it after
	// calling checkNotUsedObject.

	uiSave = gv_SFlmSysData.uiMaxUnusedTime;
	gv_SFlmSysData.uiMaxUnusedTime = uiValue;

	// May unlock and re-lock the global mutex.

	checkNotUsedObjects();
	gv_SFlmSysData.uiMaxUnusedTime = uiSave;
	f_mutexUnlock( gv_SFlmSysData.hShareMutex);

	return( rc);
}

/****************************************************************************
Desc:		Enable/disable cache debugging mode
****************************************************************************/
void F_DbSystem::enableCacheDebug(
	FLMBOOL		bDebug)
{
#ifdef FLM_DEBUG
	gv_SFlmSysData.pBlockCacheMgr->m_bDebug = bDebug;
	gv_SFlmSysData.pRowCacheMgr->m_bDebug = bDebug;
#else
	F_UNREFERENCED_PARM( bDebug);
#endif
}

/****************************************************************************
Desc:		Returns cache debugging mode
****************************************************************************/
FLMBOOL F_DbSystem::cacheDebugEnabled( void)
{
#ifdef FLM_DEBUG
		return( gv_SFlmSysData.pBlockCacheMgr->m_bDebug ||
				  gv_SFlmSysData.pRowCacheMgr->m_bDebug);
#else
		return( FALSE);
#endif
}

/****************************************************************************
Desc:		Start gathering statistics.
****************************************************************************/
void F_DbSystem::startStats( void)
{
	f_mutexLock( gv_SFlmSysData.hStatsMutex);
	flmStatStart( &gv_SFlmSysData.Stats);
	f_mutexUnlock( gv_SFlmSysData.hStatsMutex);

	// Start query statistics, if they have not
	// already been started.

	f_mutexLock( gv_SFlmSysData.hQueryMutex);
	if (!gv_SFlmSysData.uiMaxQueries)
	{
		gv_SFlmSysData.uiMaxQueries = 20;
		gv_SFlmSysData.bNeedToUnsetMaxQueries = TRUE;
	}
	f_mutexUnlock( gv_SFlmSysData.hQueryMutex);
}

/****************************************************************************
Desc:		Stop gathering statistics.
****************************************************************************/
void F_DbSystem::stopStats( void)
{
	f_mutexLock( gv_SFlmSysData.hStatsMutex);
	flmStatStop( &gv_SFlmSysData.Stats);
	f_mutexUnlock( gv_SFlmSysData.hStatsMutex);

	// Stop query statistics, if they were
	// started by a call to FLM_START_STATS.

	f_mutexLock( gv_SFlmSysData.hQueryMutex);
	if (gv_SFlmSysData.bNeedToUnsetMaxQueries)
	{
		gv_SFlmSysData.uiMaxQueries = 0;
		flmFreeSavedQueries( TRUE);
		// NOTE: flmFreeSavedQueries unlocks the mutex.
	}
	else
	{
		f_mutexUnlock( gv_SFlmSysData.hQueryMutex);
	}
}

/****************************************************************************
Desc:		Reset statistics.
****************************************************************************/
void F_DbSystem::resetStats( void)
{
	FLMUINT		uiSaveMax;

	// Reset the block cache statistics.

	f_mutexLock( gv_SFlmSysData.hBlockCacheMutex);
	gv_SFlmSysData.pBlockCacheMgr->m_uiIoWaits = 0;
	gv_SFlmSysData.pBlockCacheMgr->m_Usage.uiCacheHits = 0;
	gv_SFlmSysData.pBlockCacheMgr->m_Usage.uiCacheHitLooks = 0;
	gv_SFlmSysData.pBlockCacheMgr->m_Usage.uiCacheFaults = 0;
	gv_SFlmSysData.pBlockCacheMgr->m_Usage.uiCacheFaultLooks = 0;
	f_mutexUnlock( gv_SFlmSysData.hBlockCacheMutex);

	// Reset the row cache statistics.

	f_mutexLock( gv_SFlmSysData.hRowCacheMutex);
	gv_SFlmSysData.pRowCacheMgr->m_uiIoWaits = 0;
	gv_SFlmSysData.pRowCacheMgr->m_Usage.uiCacheHits = 0;
	gv_SFlmSysData.pRowCacheMgr->m_Usage.uiCacheHitLooks = 0;
	gv_SFlmSysData.pRowCacheMgr->m_Usage.uiCacheFaults = 0;
	gv_SFlmSysData.pRowCacheMgr->m_Usage.uiCacheFaultLooks = 0;
	f_mutexUnlock( gv_SFlmSysData.hRowCacheMutex);

	f_mutexLock( gv_SFlmSysData.hStatsMutex);
	flmStatReset( &gv_SFlmSysData.Stats, TRUE);
	f_mutexUnlock( gv_SFlmSysData.hStatsMutex);

	f_mutexLock( gv_SFlmSysData.hQueryMutex);
	uiSaveMax = gv_SFlmSysData.uiMaxQueries;
	gv_SFlmSysData.uiMaxQueries = 0;
	flmFreeSavedQueries( TRUE);
	// NOTE: flmFreeSavedQueries unlocks the mutex.

	// Restore the old maximum

	if(  uiSaveMax)
	{
		// flmFreeSavedQueries unlocks the mutex, so we
		// must relock it to restore the old maximum.

		f_mutexLock( gv_SFlmSysData.hQueryMutex);
		gv_SFlmSysData.uiMaxQueries = uiSaveMax;
		f_mutexUnlock( gv_SFlmSysData.hQueryMutex);
	}
}

/****************************************************************************
Desc:		Returns statistics that have been collected for a share.
Notes:	The statistics returned will be the statistics for ALL databases
****************************************************************************/
RCODE F_DbSystem::getStats(
	SFLM_STATS *				pFlmStats)
{
	RCODE			rc = NE_SFLM_OK;

	// Get the statistics

	f_mutexLock( gv_SFlmSysData.hStatsMutex);
	rc = flmStatCopy( pFlmStats, &gv_SFlmSysData.Stats);
	f_mutexUnlock( gv_SFlmSysData.hStatsMutex);
	if (RC_BAD( rc))
	{
		goto Exit;
	}

Exit:

	return( rc);
}

/****************************************************************************
Desc:		Frees memory allocated to a FLM_STATS structure
****************************************************************************/
void F_DbSystem::freeStats(
	SFLM_STATS *			pFlmStats)
{
	flmStatFree( pFlmStats);
}

/****************************************************************************
Desc:		Sets the path for all temporary files that come into use within a
			FLAIM share structure.  The share mutex should be locked when
			settting when called from FlmConfig().
****************************************************************************/
RCODE F_DbSystem::setTempDir(
	const char *		pszPath)
{
	RCODE		rc = NE_SFLM_OK;

	f_mutexLock( gv_SFlmSysData.hShareMutex);

	// First, test the path

	if( RC_BAD( rc = gv_SFlmSysData.pFileSystem->doesFileExist( pszPath)))
	{
		goto Exit;
	}

	f_strcpy( gv_SFlmSysData.szTempDir, pszPath);
	gv_SFlmSysData.bTempDirSet = TRUE;

Exit:

	f_mutexUnlock( gv_SFlmSysData.hShareMutex);
	return( rc);
}

/****************************************************************************
Desc:		Get the temporary directory.
****************************************************************************/
RCODE F_DbSystem::getTempDir(
	char *		pszPath)
{
	RCODE		rc = NE_SFLM_OK;

	f_mutexLock( gv_SFlmSysData.hShareMutex);

	if( !gv_SFlmSysData.bTempDirSet )
	{
		*pszPath = 0;
		rc = RC_SET( NE_FLM_IO_PATH_NOT_FOUND);
		goto Exit;
	}
	else
	{
		f_strcpy( pszPath, gv_SFlmSysData.szTempDir);
	}

Exit:

	f_mutexUnlock( gv_SFlmSysData.hShareMutex);
	return( rc);
}

/****************************************************************************
Desc:		Sets the maximum seconds between checkpoints
****************************************************************************/
void F_DbSystem::setCheckpointInterval(
	FLMUINT						uiSeconds)
{
	gv_SFlmSysData.uiMaxCPInterval = FLM_SECS_TO_TIMER_UNITS( uiSeconds);
}

/****************************************************************************
Desc:		Gets the maximum seconds between checkpoints
****************************************************************************/
FLMUINT F_DbSystem::getCheckpointInterval( void)
{
	return( FLM_TIMER_UNITS_TO_SECS( gv_SFlmSysData.uiMaxCPInterval));
}

/****************************************************************************
Desc:		Sets the interval for dynamically adjusting the cache limit.
****************************************************************************/
void F_DbSystem::setCacheAdjustInterval(
	FLMUINT						uiSeconds)
{
	gv_SFlmSysData.pGlobalCacheMgr->m_uiCacheAdjustInterval = 
		FLM_SECS_TO_TIMER_UNITS( uiSeconds);
}

/****************************************************************************
Desc:		Sets the interval for dynamically adjusting the cache limit.
****************************************************************************/
FLMUINT F_DbSystem::getCacheAdjustInterval( void)
{
	return( FLM_TIMER_UNITS_TO_SECS(
		gv_SFlmSysData.pGlobalCacheMgr->m_uiCacheAdjustInterval));
}

/****************************************************************************
Desc:		Sets the interval for dynamically cleaning out old
			cache blocks and records
****************************************************************************/
void F_DbSystem::setCacheCleanupInterval(
	FLMUINT						uiSeconds)
{
	gv_SFlmSysData.pGlobalCacheMgr->m_uiCacheCleanupInterval = 
		FLM_SECS_TO_TIMER_UNITS( uiSeconds);
}

/****************************************************************************
Desc:		Gets the interval for dynamically cleaning out old
			cache blocks and records
****************************************************************************/
FLMUINT F_DbSystem::getCacheCleanupInterval( void)
{
	return( FLM_TIMER_UNITS_TO_SECS(
		gv_SFlmSysData.pGlobalCacheMgr->m_uiCacheCleanupInterval));
}

/****************************************************************************
Desc:		Set interval for cleaning up unused structures
****************************************************************************/
void F_DbSystem::setUnusedCleanupInterval(
	FLMUINT						uiSeconds)
{
	gv_SFlmSysData.pGlobalCacheMgr->m_uiUnusedCleanupInterval = 
		FLM_SECS_TO_TIMER_UNITS( uiSeconds);
}

/****************************************************************************
Desc:		Gets the interval for cleaning up unused structures
****************************************************************************/
FLMUINT F_DbSystem::getUnusedCleanupInterval( void)
{
	return( FLM_TIMER_UNITS_TO_SECS(
		gv_SFlmSysData.pGlobalCacheMgr->m_uiUnusedCleanupInterval));
}

/****************************************************************************
Desc:		Set the maximum time for an item to be unused before it is
			cleaned up
****************************************************************************/
void F_DbSystem::setMaxUnusedTime(
	FLMUINT						uiSeconds)
{
	gv_SFlmSysData.uiMaxUnusedTime = FLM_SECS_TO_TIMER_UNITS( uiSeconds);
}

/****************************************************************************
Desc:		Gets the maximum time for an item to be unused
****************************************************************************/
FLMUINT F_DbSystem::getMaxUnusedTime( void)
{
	return( FLM_TIMER_UNITS_TO_SECS( gv_SFlmSysData.uiMaxUnusedTime));
}

/****************************************************************************
Desc:		Sets the logging object to be used for internal status messages
****************************************************************************/
void F_DbSystem::setLogger(
	IF_LoggerClient *		pLogger)
{
	IF_LoggerClient *		pOldLogger = NULL;
	
	f_mutexLock( gv_SFlmSysData.hLoggerMutex);
	
	for( ;;)	
	{
		// While waiting for the pending message count to go to zero, other
		// threads may have come in to set different logger objects.  To handle
		// this case, we need to check gv_SFlmSysData.pLogger and release it
		// if non-NULL.
	
		if( gv_SFlmSysData.pLogger)
		{
			if( pOldLogger)
			{
				pOldLogger->Release();
			}
			
			pOldLogger = gv_SFlmSysData.pLogger;
			gv_SFlmSysData.pLogger = NULL;
		}
		
		if( !gv_SFlmSysData.uiPendingLogMessages)
		{
			break;
		}
		
		f_mutexUnlock( gv_SFlmSysData.hLoggerMutex);
		f_sleep( 100);
		f_mutexLock( gv_SFlmSysData.hLoggerMutex);
	}
	
	if( pOldLogger)
	{
		pOldLogger->Release();
	}
	
	if( (gv_SFlmSysData.pLogger = pLogger) != NULL)
	{
		gv_SFlmSysData.pLogger->AddRef();
	}
	
	f_mutexUnlock( gv_SFlmSysData.hLoggerMutex);
}

/****************************************************************************
Desc:		Enables or disables the use of ESM (if available)
****************************************************************************/
void F_DbSystem::enableExtendedServerMemory(
	FLMBOOL						bEnable)
{
	gv_SFlmSysData.bOkToUseESM = bEnable;
}

/****************************************************************************
Desc:		Returns the state of ESM
****************************************************************************/
FLMBOOL F_DbSystem::extendedServerMemoryEnabled( void)
{
	return( gv_SFlmSysData.bOkToUseESM);
}

/****************************************************************************
Desc:		Deactivates open database handles, forcing the database to be
			(eventually) closed
Notes:	Passing NULL for the path values will cause all active database
			handles to be deactivated
****************************************************************************/
void F_DbSystem::deactivateOpenDb(
	const char *	pszDbFileName,
	const char *	pszDataDir)
{
	F_Database *	pTmpDatabase;

	f_mutexLock( gv_SFlmSysData.hShareMutex);
	if( pszDbFileName)
	{
		// Look up the file using findDatabase to see if we have the
		// file open.  May unlock and re-lock the global mutex.

		if( RC_OK( findDatabase( pszDbFileName,
			pszDataDir, &pTmpDatabase)) && pTmpDatabase)
		{
			pTmpDatabase->setMustCloseFlags( NE_SFLM_OK, TRUE);
		}
	}
	else
	{
		if( gv_SFlmSysData.pDatabaseHashTbl)
		{
			FLMUINT		uiLoop;

			for( uiLoop = 0; uiLoop < FILE_HASH_ENTRIES; uiLoop++)
			{
				pTmpDatabase =
					(F_Database *)gv_SFlmSysData.pDatabaseHashTbl[ uiLoop].pFirstInBucket;

				while( pTmpDatabase)
				{
					pTmpDatabase->setMustCloseFlags( NE_SFLM_OK, TRUE);
					pTmpDatabase = pTmpDatabase->m_pNext;
				}
			}
		}
	}

	f_mutexUnlock( gv_SFlmSysData.hShareMutex);
}

/****************************************************************************
Desc:		Sets the maximum number of queries to save
****************************************************************************/
void F_DbSystem::setQuerySaveMax(
	FLMUINT						uiMaxToSave)
{
	f_mutexLock( gv_SFlmSysData.hQueryMutex);
	gv_SFlmSysData.uiMaxQueries = uiMaxToSave;
	gv_SFlmSysData.bNeedToUnsetMaxQueries = FALSE;
	flmFreeSavedQueries( TRUE);
}

/****************************************************************************
Desc:		Gets the maximum number of queries to save
****************************************************************************/
FLMUINT F_DbSystem::getQuerySaveMax( void)
{
	return( gv_SFlmSysData.uiMaxQueries);
}

/****************************************************************************
Desc:		Sets the maximum amount of dirty cache allowed
****************************************************************************/
void F_DbSystem::setDirtyCacheLimits(
	FLMUINT	uiMaxDirty,
	FLMUINT	uiLowDirty)
{
	f_mutexLock( gv_SFlmSysData.hBlockCacheMutex);
	if( !uiMaxDirty)
	{
		gv_SFlmSysData.pBlockCacheMgr->m_bAutoCalcMaxDirty = TRUE;
		gv_SFlmSysData.pBlockCacheMgr->m_uiMaxDirtyCache = 0;
		gv_SFlmSysData.pBlockCacheMgr->m_uiLowDirtyCache = 0;
	}
	else
	{
		gv_SFlmSysData.pBlockCacheMgr->m_bAutoCalcMaxDirty = FALSE;
		gv_SFlmSysData.pBlockCacheMgr->m_uiMaxDirtyCache = uiMaxDirty;

		// Low threshhold must be no higher than maximum!

		if ((gv_SFlmSysData.pBlockCacheMgr->m_uiLowDirtyCache = uiLowDirty) >
			gv_SFlmSysData.pBlockCacheMgr->m_uiMaxDirtyCache)
		{
			gv_SFlmSysData.pBlockCacheMgr->m_uiLowDirtyCache =
				gv_SFlmSysData.pBlockCacheMgr->m_uiMaxDirtyCache;
		}
	}
	f_mutexUnlock( gv_SFlmSysData.hBlockCacheMutex);
}

/****************************************************************************
Desc:		Gets the maximum amount of dirty cache allowed
****************************************************************************/
void F_DbSystem::getDirtyCacheLimits(
	FLMUINT *	puiMaxDirty,
	FLMUINT *	puiLowDirty)
{
	f_mutexLock( gv_SFlmSysData.hBlockCacheMutex);
	if( puiMaxDirty)
	{
		*puiMaxDirty = gv_SFlmSysData.pBlockCacheMgr->m_uiMaxDirtyCache;
	}

	if( puiLowDirty)
	{
		*puiLowDirty = gv_SFlmSysData.pBlockCacheMgr->m_uiLowDirtyCache;
	}

	f_mutexUnlock( gv_SFlmSysData.hBlockCacheMutex);
}

/****************************************************************************
Desc:		Returns information about threads owned by the database engine
****************************************************************************/
RCODE F_DbSystem::getThreadInfo(
	IF_ThreadInfo **	ppThreadInfo)
{
	return( FlmGetThreadInfo( ppThreadInfo));
}

/****************************************************************************
Desc:
****************************************************************************/
void F_DbSystem::getFileSystem(
	IF_FileSystem **		ppFileSystem)
{
	FlmGetFileSystem( ppFileSystem);
}

/****************************************************************************
Desc:		Registers for an event
****************************************************************************/
RCODE F_DbSystem::registerForEvent(
	eEventCategory			eCategory,
	IF_EventClient *		pEventClient)
{
	RCODE				rc = NE_SFLM_OK;
	FEVENT *			pEvent;

	// Make sure it is a legal event category to register for.

	if (eCategory >= SFLM_MAX_EVENT_CATEGORIES)
	{
		rc = RC_SET_AND_ASSERT( NE_SFLM_NOT_IMPLEMENTED);
		goto Exit;
	}

	// Allocate an event structure

	if (RC_BAD( rc = f_calloc(
		(FLMUINT)(sizeof( FEVENT)), &pEvent)))
	{
		goto Exit;
	}

	// Initialize the structure members and linkt to the
	// list of events off of the event category.

	pEvent->pEventClient = pEventClient;
	pEvent->pEventClient->AddRef();
	// pEvent->pPrev = NULL;		// done by f_calloc above.

	// Mutex should be locked to link into list.

	f_mutexLock( gv_SFlmSysData.EventHdrs [eCategory].hMutex);
	if ((pEvent->pNext =
			gv_SFlmSysData.EventHdrs [eCategory].pEventCBList) != NULL)
	{
		pEvent->pNext->pPrev = pEvent;
	}
	gv_SFlmSysData.EventHdrs [eCategory].pEventCBList = pEvent;
	f_mutexUnlock( gv_SFlmSysData.EventHdrs [eCategory].hMutex);

Exit:

	return( rc);
}

/****************************************************************************
Desc:		De-registers for an event
****************************************************************************/
void F_DbSystem::deregisterForEvent(
	eEventCategory		eCategory,
	IF_EventClient *		pEventClient)
{
	FEVENT *	pEvent = gv_SFlmSysData.EventHdrs [eCategory].pEventCBList;

	// Find the event and remove from the list.  If it is not
	// there, don't worry about it.

	while (pEvent)
	{
		if (pEventClient == pEvent->pEventClient)
		{
			flmFreeEvent( pEvent,
				gv_SFlmSysData.EventHdrs[ eCategory].hMutex,
				&gv_SFlmSysData.EventHdrs[ eCategory].pEventCBList);
			break;
		}
		pEvent = pEvent->pNext;
	}
}

/****************************************************************************
Desc:		Returns TRUE if the specified error indicates that the database
			is corrupt
****************************************************************************/
RCODE F_DbSystem::getNextMetaphone(
	IF_IStream *	pIStream,
	FLMUINT *		puiMetaphone,
	FLMUINT *		puiAltMetaphone)
{
	return( f_getNextMetaphone( 
		pIStream, puiMetaphone, puiAltMetaphone));
}

/****************************************************************************
Desc:		Returns TRUE if the specified error indicates that the database
			is corrupt
****************************************************************************/
FLMBOOL F_DbSystem::errorIsFileCorrupt(
	RCODE						rc)
{
	FLMBOOL		bIsCorrupt = FALSE;

	switch( rc)
	{
		case NE_SFLM_BTREE_ERROR :
		case NE_SFLM_DATA_ERROR :
		case NE_SFLM_NOT_FLAIM :
		case NE_SFLM_BLOCK_CRC :
		case NE_SFLM_HDR_CRC :
		case NE_SFLM_INCOMPLETE_LOG :
			bIsCorrupt = TRUE;
			break;
		default :
			break;
	}

	return( bIsCorrupt);
}

/****************************************************************************
Desc:		Increment the database system use count
****************************************************************************/
FLMINT F_DbSystem::AddRef(void)
{
	FLMINT		iRefCnt = f_atomicInc( &m_refCnt);

	// Note: We don't bother with a call to LockModule() here because
	// it's done in the constructor.  In fact, it's unlikely that
	// this AddRef() will ever be called, because the constructor
	// already sets the ref count to 1 and it's unlikely that there
	// will be to references to the same DbSystem object...

	return( iRefCnt);
}

/****************************************************************************
Desc:		Decrement the database system use count
****************************************************************************/
FLMINT F_DbSystem::Release(void)
{
	FLMINT	iRefCnt = f_atomicDec( &m_refCnt);

	if (iRefCnt == 0)
	{
		delete this;	
	}

	return( iRefCnt);
}

/****************************************************************************
Desc:	Check for the existence of FLAIM performance tuning file. If
		found this routine will parse the contents and set the global 
		performance tuning variables.

		Looks for the following strings within the .ini file
			cache=<cacheBytes>				# Set a hard memory limit
			cache=<cache options>			# Set a hard memory limit or dynamically
													# adjusting limit.
			<cache options>					# Multiple options may be specified, in
													# any order, separated by commas.  All
													# are optional.
				%:<percentage>					# percentage of available or physical memory.
				AVAIL or TOTAL					# percentage is for available physical
													# memory or total physical memory.  NOTE:
													# these are ignored for a dynamic limit.
				MIN:<bytes>						# minimum number of bytes
				MAX:<bytes>						# maximum number of bytes
				LEAVE:<bytes>					# minimum nmber of bytes to leave
				DYN or HARD						# dynamic or hard limit
				PRE								# preallocate cache - valid only with HARD setting.
				
		Examples:
	
			cache=DYN,%:75,MIN:16000000	# Dynamic limit of 75% available memory,
													# minimum of 16 megabytes.
			cache=HARD,%:75,MIN:16000000	# Hard limit of 75% total physical memory,
													# minimum of 16 megabytes.
			cache=HARD,%:75,MIN:16000000,PRE
													# Hard limit of 75% total physical memory,
													# minimum of 16 megabytes, preallocated.
			cache=8000000						# Old style - hard limit of 8 megabytes.

			cacheadjustinterval=<seconds>
			cachecleanupinterval=<seconds>
****************************************************************************/

#define SFLM_INI_FILE_NAME "_sflm.ini"

RCODE F_DbSystem::readIniFile()
{
	RCODE				rc = NE_SFLM_OK;
	FLMBOOL			bMutexLocked = FALSE;
	IF_IniFile *	pIniFile = NULL;
	char				szIniFileName [F_PATH_MAX_SIZE];
	FLMUINT			uiParamValue;
	FLMUINT			uiMaxDirtyCache;
	FLMUINT			uiLowDirtyCache;

	// Initialize the ini file object...
	
	if( RC_BAD( rc = FlmAllocIniFile( &pIniFile)))
	{
		goto Exit;
	}

	// Lock the ini file mutex

	f_mutexLock( gv_SFlmSysData.hIniMutex);
	bMutexLocked = TRUE;

	if (RC_BAD( rc = flmGetIniFileName( (FLMBYTE *)szIniFileName,
													F_PATH_MAX_SIZE)))
	{
		goto Exit;
	}

	if (RC_BAD( rc = pIniFile->read( szIniFileName)))
	{
		goto Exit;
	}

	// Set FLAIM parameters from the buffer read from the .ini file.

	setCacheParams( pIniFile);
	
	flmGetUintParam( SFLM_INI_CACHE_ADJUST_INTERVAL, 
							SFLM_DEFAULT_CACHE_CLEANUP_INTERVAL,
							&uiParamValue, pIniFile);
	setCacheAdjustInterval( uiParamValue);
	
	flmGetUintParam( SFLM_INI_CACHE_CLEANUP_INTERVAL, 
							SFLM_DEFAULT_CACHE_CLEANUP_INTERVAL,
							&uiParamValue, pIniFile);
	setCacheCleanupInterval( uiParamValue);

	flmGetUintParam( SFLM_INI_MAX_DIRTY_CACHE, 0, &uiMaxDirtyCache, pIniFile);

	flmGetUintParam( SFLM_INI_LOW_DIRTY_CACHE, 0, &uiLowDirtyCache, pIniFile);

	// Use FLAIM's default behavior if uiMaxDirtyCache is zero.  FLAIM's
	// default behavior is to use a maximum value for dirty cache.

	if (uiMaxDirtyCache)
	{
		setDirtyCacheLimits( uiMaxDirtyCache, uiLowDirtyCache);
	}

Exit:

	if (bMutexLocked)
	{
		f_mutexUnlock( gv_SFlmSysData.hIniMutex);
	}

	pIniFile->Release();
	return( rc);
}

/****************************************************************************
Desc: 	Given a tag and a value this function will open/create the 
			ini file and replace or insert the tag with it's value.
			NOTE: This function expects gv_SFlmSysData.hIniMutex to already be
			locked!
****************************************************************************/
RCODE F_DbSystem::updateIniFile(
	const char *	pszParamName,
	const char *	pszValue)
{
	RCODE				rc = NE_SFLM_OK;
	IF_IniFile *	pIniFile = NULL;
	char				szIniFileName [F_PATH_MAX_SIZE];
	
	if( RC_BAD( rc = FlmAllocIniFile( &pIniFile)))
	{
		goto Exit;
	}

	if (RC_BAD( rc = flmGetIniFileName( (FLMBYTE *)szIniFileName,
													F_PATH_MAX_SIZE)))
	{
		goto Exit;
	}

	if (RC_BAD( rc = pIniFile->read( szIniFileName)))
	{
		goto Exit;
	}

	if (RC_BAD( rc = pIniFile->setParam( pszParamName, pszValue)))
	{
		goto Exit;
	}

	if (RC_BAD( rc = pIniFile->write()))
	{
		goto Exit;
	}

Exit:

	pIniFile->Release();
	return( rc);
}

/****************************************************************************
Desc:	Sets the parameters for controlling cache.
****************************************************************************/
RCODE F_DbSystem::setCacheParams(
	IF_IniFile *	pIniFile)
{
	RCODE				rc = NE_SFLM_OK;
	char *			pszCacheParam = NULL;
	char				cChar;
	char *			pszSave;
	char *			pszValueName;
	FLMBOOL			bDynamicCacheAdjust;
	FLMBOOL			bCalcLimitOnAvail = FALSE;
	FLMUINT			uiCachePercent;
	FLMUINT			uiCacheMin;
	FLMUINT			uiCacheMax;
	FLMUINT			uiCacheMinToLeave;
	FLMBOOL			bPreallocated = FALSE;

	// Set up appropriate defaults.

	bDynamicCacheAdjust = getDynamicCacheSupported();

	if( bDynamicCacheAdjust)
	{
		uiCachePercent = SFLM_DEFAULT_CACHE_ADJUST_PERCENT;
		bCalcLimitOnAvail = TRUE;
		uiCacheMin = SFLM_DEFAULT_CACHE_ADJUST_MIN;
		uiCacheMax = SFLM_DEFAULT_CACHE_ADJUST_MAX;
		uiCacheMinToLeave = 0;
	}
	else
	{
		uiCachePercent = 0;
		uiCacheMin =
		uiCacheMax = SFLM_DEFAULT_CACHE_ADJUST_MIN;
		uiCacheMinToLeave = 0;
	}

	// Extract values from the cache parameter
	flmGetStringParam( SFLM_INI_CACHE, &pszCacheParam, pIniFile);
	
	if ((pszCacheParam != NULL) && (pszCacheParam[0] != '\0'))
	{
		// Parse until we hit white space.

		while (*pszCacheParam &&
				 *pszCacheParam != ' ' &&
				 *pszCacheParam != '\n' &&
				 *pszCacheParam != '\r' &&
				 *pszCacheParam != '\t')
		{

			// Get the value name.

			pszValueName = pszCacheParam;
			while (*pszCacheParam &&
					 *pszCacheParam != ' ' &&
					 *pszCacheParam != '\n' &&
					 *pszCacheParam != '\r' &&
					 *pszCacheParam != '\t' &&
					 *pszCacheParam != ':' &&
					 *pszCacheParam != ',' &&
					 *pszCacheParam != ';')
			{
				pszCacheParam++;
			}
			pszSave = pszCacheParam;
			cChar = *pszSave;
			*pszSave = 0;

			// New options only supported on platforms that have ways
			// of getting physical memory size and available physical
			// memory size.

			if ((f_stricmp( pszValueName, "MAX") == 0) ||
				 (f_stricmp( pszValueName, "MAXIMUM") == 0))
			{
				if (cChar == ':')
				{
					pszCacheParam++;
					flmGetNumParam( &pszCacheParam,
						&uiCacheMax);
				}
			}
			else if ((f_stricmp( pszValueName, "MIN") == 0) ||
						(f_stricmp( pszValueName, "MINIMUM") == 0))
			{
				if (cChar == ':')
				{
					pszCacheParam++;
					flmGetNumParam( &pszCacheParam,
						&uiCacheMin);
				}
			}
			else if ((f_stricmp( pszValueName, "%") == 0) ||
						(f_stricmp( pszValueName, "PERCENT") == 0))
			{
				if (cChar == ':')
				{
					pszCacheParam++;
					flmGetNumParam( &pszCacheParam,
						&uiCachePercent);
				}
			}
			else if ((f_stricmp( pszValueName, "DYNAMIC") == 0) ||
						(f_stricmp( pszValueName, "DYN") == 0))
			{
				if (cChar == ':' || cChar == ',' || cChar == ';')
				{
					pszCacheParam++;
				}
				bDynamicCacheAdjust = TRUE;
			}
			else if (f_stricmp( pszValueName, "AVAIL") == 0)
			{
				if (cChar == ':' || cChar == ',' || cChar == ';')
				{
					pszCacheParam++;
				}
				bCalcLimitOnAvail = TRUE;
			}
			else if (f_stricmp( pszValueName, "TOTAL") == 0)
			{
				if (cChar == ':' || cChar == ',' || cChar == ';')
				{
					pszCacheParam++;
				}
				bCalcLimitOnAvail = FALSE;
			}
			else if (f_stricmp( pszValueName, "LEAVE") == 0)
			{
				if (cChar == ':')
				{
					pszCacheParam++;
					flmGetNumParam( &pszCacheParam,
										 &uiCacheMinToLeave);
				}
			}
			else if (f_stricmp( pszValueName, "HARD") == 0)
			{
				if (cChar == ':' || cChar == ',' || cChar == ';')
				{
					pszCacheParam++;
				}
				bDynamicCacheAdjust = FALSE;
			}
			else if (f_stricmp( pszValueName, "PRE") == 0)
			{
				if (cChar == ':' || cChar == ',' || cChar == ';')
				{
					pszCacheParam++;
				}
				bPreallocated = TRUE;
			}
			else
			{
				*pszSave = cChar;

				// This will handle the old way where they just put in a
				// number for the byte maximum.

				bDynamicCacheAdjust = FALSE;
				uiCachePercent = 0;
				uiCacheMax = f_atol( pszValueName);

				// Don't really have to set these, but do it just to
				// be clean.

				uiCacheMin = 0;
				uiCacheMinToLeave = 0;
				bCalcLimitOnAvail = FALSE;
				break;
			}
			*pszSave = cChar;
		}
	}

	// Set up the stuff for controlling cache - hard limit or dynamically
	// adjusting limit.

	if( bDynamicCacheAdjust)
	{
		if (RC_BAD( rc = setDynamicMemoryLimit(
								uiCachePercent,
								uiCacheMin, uiCacheMax,
								uiCacheMinToLeave)))
		{
			goto Exit;
		}
	}
	else
	{
		if( RC_BAD( rc = setHardMemoryLimit( uiCachePercent,
									bCalcLimitOnAvail, uiCacheMin,
									uiCacheMax, uiCacheMinToLeave,
									bPreallocated)))
		{
			goto Exit;
		}
	}

Exit:

	return rc;
}

/****************************************************************************
Desc:	Gets a string parameter from the .ini file.
****************************************************************************/
FSTATIC void flmGetStringParam(
	const char *		pszParamName,
	char **				ppszValue,
	IF_IniFile *		pIniFile)
{
	flmAssert( pIniFile);
	(void)pIniFile->getParam( pszParamName, ppszValue);
}

/****************************************************************************
Desc: Gets one of the number parameters for setting cache stuff.
****************************************************************************/
FSTATIC void flmGetNumParam(
	char **		ppszParam,
	FLMUINT *	puiNum)
{
	char *		pszTmp = *ppszParam;
	FLMUINT		uiNum = 0;

	while ((*pszTmp >= '0') && (*pszTmp <= '9'))
	{
		uiNum *= 10;
		uiNum += (FLMUINT)(*pszTmp - '0');
		pszTmp++;
	}

	// Run past any other junk up to allowed terminators.

	while (*pszTmp &&
			 *pszTmp != ' ' &&
			 *pszTmp != '\n' &&
			 *pszTmp != '\r' &&
			 *pszTmp != '\t' &&
			 *pszTmp != ':' &&
			 *pszTmp != ',' &&
			 *pszTmp != ';')
	{
		pszTmp++;
	}

	// Skip past trailing colon, comma, or semicolon.

	if (*pszTmp == ':' || *pszTmp == ',' || *pszTmp == ';')
	{
		pszTmp++;
	}

	*puiNum = uiNum;
	*ppszParam = pszTmp;
}

/****************************************************************************
Desc:	Gets a FLMUINT parameter from the .ini file.
****************************************************************************/
FSTATIC void flmGetUintParam(
	const char *	pszParamName,
	FLMUINT			uiDefaultValue,
	FLMUINT *		puiUint,
	IF_IniFile *	pIniFile)
{
	flmAssert( pIniFile);
	if (!pIniFile->getParam( pszParamName, puiUint))
	{
		*puiUint = uiDefaultValue;
	}
}

/****************************************************************************
Desc:	Gets a FLMBOOL parameter from the .ini file.
****************************************************************************/
void flmGetBoolParam(
	const char *	pszParamName,
	FLMBOOL			bDefaultValue,
	FLMBOOL *		pbBool,
	IF_IniFile *	pIniFile)
{
	flmAssert( pIniFile);
	if (!pIniFile->getParam( pszParamName, pbBool))
	{
		*pbBool = bDefaultValue;
	}
}

/****************************************************************************
Desc:
****************************************************************************/
FSTATIC RCODE flmGetIniFileName(
	FLMBYTE *		pszIniFileName,
	FLMUINT			uiBufferSz)
{
	RCODE				rc = NE_SFLM_OK;
	FLMUINT			uiFileNameLen = 0;
	IF_DirHdl *		pDirHdl = NULL;

	if (!uiBufferSz)
	{
		rc = RC_SET_AND_ASSERT( NE_SFLM_FAILURE);
		goto Exit;
	}

	// See if we have an environment variable to tell us wher the ini file should be.

	f_getenv( "SFLM_INI_PATH",
				 pszIniFileName,
				 uiBufferSz,
				 &uiFileNameLen);

	if( !uiFileNameLen)
	{
		// Perhaps we can find a data directory.  If there is one, we will
		// look in there.
		
		if( RC_OK( rc = gv_SFlmSysData.pFileSystem->openDir( 
			(char *)".", (char *)"data", &pDirHdl)))
		{
			if (RC_OK( rc = pDirHdl->next()))
			{
				if (pDirHdl->currentItemIsDir())
				{
					// Directory exists.  We will look there.
					f_strcpy( (char *)pszIniFileName, "data");
				}
				else
				{
					goto NoDataDir;
				}
			}
			else
			{
				rc = NE_SFLM_OK;
				goto NoDataDir;
			}
		}
		else
		{
NoDataDir:
			// Data directory does not exist.  We will look in the
			// current (default) dir.

			f_strcpy( (char *)pszIniFileName, ".");
		}
	}

	gv_SFlmSysData.pFileSystem->pathAppend( 
		(char *)pszIniFileName, SFLM_INI_FILE_NAME);

Exit:

	if (pDirHdl)
	{
		pDirHdl->Release();
	}
	
	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
F_SuperFileClient::F_SuperFileClient()
{
	m_pszCFileName = NULL;
	m_pszDataFileBaseName = NULL;
	m_uiExtOffset = 0;
	m_uiDataExtOffset = 0;
}
	
/****************************************************************************
Desc:
****************************************************************************/
F_SuperFileClient::~F_SuperFileClient()
{
	if( m_pszCFileName)
	{
		f_free( &m_pszCFileName);
	}
}
	
/****************************************************************************
Desc:
****************************************************************************/
RCODE F_SuperFileClient::setup(
	const char *	pszCFileName,
	const char *	pszDataDir)
{
	RCODE				rc = NE_SFLM_OK;
	FLMUINT			uiNameLen;
	FLMUINT			uiDataNameLen;
	char				szDir[ F_PATH_MAX_SIZE];
	char				szBaseName[ F_FILENAME_SIZE];

	if( !pszCFileName && *pszCFileName == 0)
	{
		rc = RC_SET( NE_FLM_IO_INVALID_FILENAME);
		goto Exit;
	}

	uiNameLen = f_strlen( pszCFileName);
	if (pszDataDir && *pszDataDir)
	{
		if (RC_BAD( rc = gv_SFlmSysData.pFileSystem->pathReduce( 
			pszCFileName, szDir, szBaseName)))
		{
			goto Exit;
		}
		f_strcpy( szDir, pszDataDir);
		if (RC_BAD( rc = gv_SFlmSysData.pFileSystem->pathAppend( 
			szDir, szBaseName)))
		{
			goto Exit;
		}
		uiDataNameLen = f_strlen( szDir);

		if (RC_BAD( rc = f_alloc( (uiNameLen + 1) + (uiDataNameLen + 1),
									&m_pszCFileName)))
		{
			goto Exit;
		}

		f_memcpy( m_pszCFileName, pszCFileName, uiNameLen + 1);
		m_pszDataFileBaseName = m_pszCFileName + uiNameLen + 1;
		flmGetDbBasePath( m_pszDataFileBaseName, szDir, &m_uiDataExtOffset);
		m_uiExtOffset = uiNameLen - (uiDataNameLen - m_uiDataExtOffset);
	}
	else
	{
		if (RC_BAD( rc = f_alloc( (uiNameLen + 1) * 2, &m_pszCFileName)))
		{
			goto Exit;
		}

		f_memcpy( m_pszCFileName, pszCFileName, uiNameLen + 1);
		m_pszDataFileBaseName = m_pszCFileName + uiNameLen + 1;
		flmGetDbBasePath( m_pszDataFileBaseName, 
			m_pszCFileName, &m_uiDataExtOffset);
		m_uiExtOffset = m_uiDataExtOffset;
	}

Exit:

	return( rc);
}
	
/****************************************************************************
Desc:
****************************************************************************/
FLMUINT SQFAPI F_SuperFileClient::getFileNumber(
	FLMUINT					uiBlockAddr)
{
	return( FSGetFileNumber( uiBlockAddr));
}
		
/****************************************************************************
Desc:
****************************************************************************/
FLMUINT SQFAPI F_SuperFileClient::getFileOffset(
	FLMUINT					uiBlockAddr)
{
	return( FSGetFileOffset( uiBlockAddr));
}

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT SQFAPI F_SuperFileClient::getBlockAddress(
	FLMUINT					uiFileNumber,
	FLMUINT					uiFileOffset)
{
	// JMC - FIXME: Don't know if this is the correct implementation for this
   return( FSBlkAddress( uiFileNumber, uiFileOffset));
}

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT64 SQFAPI F_SuperFileClient::getMaxFileSize( void)
{
//	JMC - FIXME: commented out due to missing functionality in flaimtk.h
//	if( m_uiDbVersion >= FLM_FILE_FORMAT_VER_4_3)
//	{
//		return( gv_FlmSysData.uiMaxFileSize);
//	}
	
#define MAX_FILE_SIZE_VER40			((FLMUINT)0x7FF00000)
	return( MAX_FILE_SIZE_VER40);
}
		
/****************************************************************************
Desc:
****************************************************************************/
RCODE SQFAPI F_SuperFileClient::getFilePath(
	FLMUINT			uiFileNumber,
	char *			pszPath)
{
	RCODE				rc = NE_SFLM_OK;
	FLMUINT			uiExtOffset;

	if( !uiFileNumber)
	{
		f_strcpy( pszPath, m_pszCFileName);
		goto Exit;
	}

	if( uiFileNumber <= MAX_DATA_BLOCK_FILE_NUMBER)
	{
		f_memcpy( pszPath, m_pszDataFileBaseName, m_uiDataExtOffset);
		uiExtOffset = m_uiDataExtOffset;
	}
	else
	{
		f_memcpy( pszPath, m_pszCFileName, m_uiExtOffset);
		uiExtOffset = m_uiExtOffset;
	}

	// Modify the file's extension.

	bldSuperFileExtension( uiFileNumber, &pszPath[ uiExtOffset]);

Exit:

	return( rc);
}

/****************************************************************************
Desc:		Generates a file name given a super file number.
			Adds ".xx" to pFileExtension.  Use lower case characters.
Notes:	This is a base 24 alphanumeric value where
			{ a, b, c, d, e, f, i, l, o, r, u, v } values are removed.
****************************************************************************/
void F_SuperFileClient::bldSuperFileExtension(
	FLMUINT		uiFileNum,
	char *		pszFileExtension)
{
	char	ucLetter;

	if (uiFileNum <= MAX_DATA_BLOCK_FILE_NUMBER - 1536)
	{
		// No additional letter - File numbers 1 to 511
		// This is just like pre-4.3 numbering.
		ucLetter = 0;
	}
	else if (uiFileNum <= MAX_DATA_BLOCK_FILE_NUMBER - 1024)
	{
		// File numbers 512 to 1023
		ucLetter = 'r';
	}
	else if (uiFileNum <= MAX_DATA_BLOCK_FILE_NUMBER - 512)
	{
		// File numbers 1024 to 1535
		ucLetter = 's';
	}
	else if (uiFileNum <= MAX_DATA_BLOCK_FILE_NUMBER)
	{
		// File numbers 1536 to 2047
		ucLetter = 't';
	}
	else if (uiFileNum <= MAX_LOG_BLOCK_FILE_NUMBER - 1536)
	{
		// File numbers 2048 to 2559
		ucLetter = 'v';
	}
	else if (uiFileNum <= MAX_LOG_BLOCK_FILE_NUMBER - 1024)
	{
		// File numbers 2560 to 3071
		ucLetter = 'w';
	}
	else if (uiFileNum <= MAX_LOG_BLOCK_FILE_NUMBER - 512)
	{
		// File numbers 3072 to 3583
		ucLetter = 'x';
	}
	else
	{
		flmAssert( uiFileNum <= MAX_LOG_BLOCK_FILE_NUMBER);

		// File numbers 3584 to 4095
		ucLetter = 'z';
	}

	*pszFileExtension++ = '.';
	*pszFileExtension++ = (char)(f_getBase24DigitChar( (FLMBYTE)((uiFileNum & 511) / 24)));
	*pszFileExtension++ = (char)(f_getBase24DigitChar( (FLMBYTE)((uiFileNum & 511) % 24)));
	*pszFileExtension++ = ucLetter;
	*pszFileExtension   = 0;
}

