//-------------------------------------------------------------------------
// Desc:	Copy database.
// Tabs:	3
//
//		Copyright (c) 1998-2001,2003-2006 Novell, Inc. All Rights Reserved.
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
// $Id: fdbcopy.cpp 12329 2006-01-20 17:49:30 -0700 (Fri, 20 Jan 2006) ahodgkinson $
//-------------------------------------------------------------------------

#include "flaimsys.h"

typedef struct Copied_Name
{
	char				szPath[ F_PATH_MAX_SIZE];
	Copied_Name *	pNext;
} COPIED_NAME;

FSTATIC RCODE flmCopyDb(
	FLMUINT				uiDbVersion,
	const char *		pszSrcDbName,
	const char *		pszSrcDataDir,
	const char *		pszSrcRflDir,
	const char *		pszDestDbName,
	const char *		pszDestDataDir,
	const char *		pszDestRflDir,
	STATUS_HOOK			fnStatusCallback,
	void *				UserData);

FSTATIC RCODE flmCopyFile(
	F_FileSystemImp *	pFileSystem,
	FLMUINT				uiStartOffset,
	FLMUINT				uiEndOffset,
	DB_COPY_INFO *		pDbCopyInfo,
	COPIED_NAME **		ppCopiedListRV,
	FLMBYTE *			pucInMemLogHdr,
	FLMBOOL				bOkToTruncate,
	STATUS_HOOK			fnStatusCallback,
	void *				UserData);

/*******************************************************************************
Desc:	Copies a database, including roll-forward log files.
*******************************************************************************/
FLMEXP RCODE FLMAPI FlmDbCopy(
	const char *		pszSrcDbName,
	const char *		pszSrcDataDir,
	const char *		pszSrcRflDir,
	const char *		pszDestDbName,
	const char *		pszDestDataDir,
	const char *		pszDestRflDir,
	STATUS_HOOK			fnStatusCallback,
	void *				UserData)
{
	RCODE				rc = FERR_OK;
	FLMBYTE *		pucLastCommittedLogHdr;
	HFDB				hDb = HFDB_NULL;
	FDB *				pDb;
	FLMBOOL			bDbLocked = FALSE;
	FLMUINT			uiDbVersion;

	// Make sure the destination database is closed

	if (RC_BAD( rc = FlmConfig( FLM_CLOSE_FILE,
		(void *)pszDestDbName, (void *)pszDestDataDir)))
	{
		goto Exit;
	}

	// Open the database so we can force a checkpoint.

	if (RC_BAD( rc = FlmDbOpen( pszSrcDbName, pszSrcDataDir, pszSrcRflDir,
								0, NULL, &hDb)))
	{
		goto Exit;
	}
	pDb = (FDB *)hDb;

	// Need to lock the database, because we want to do a checkpoint
	// and then the copy immediately after without letting other
	// threads have the opportunity to get in and update the
	// database.

	if (RC_BAD( rc = FlmDbLock( hDb, FLM_LOCK_EXCLUSIVE, 0, FLM_NO_TIMEOUT)))
	{
		goto Exit;
	}
	bDbLocked = TRUE;

	// Force a checkpoint

	if (RC_BAD( rc = FlmDbCheckpoint( hDb, FLM_NO_TIMEOUT)))
	{
		goto Exit;
	}

	pucLastCommittedLogHdr = &pDb->pFile->ucLastCommittedLogHdr[ 0];

	// Get the low and high RFL log file numbers from the log
	// header.

	uiDbVersion = pDb->pFile->FileHdr.uiVersionNum;

	// Once we get this far, we have exclusive access to the database
	// and we have forced a checkpoint.  The database's contents are
	// guaranteed to be on disk at this point, and they will not
	// change.

	rc = flmCopyDb( uiDbVersion, pszSrcDbName, pszSrcDataDir, pszSrcRflDir,
						pszDestDbName, pszDestDataDir, pszDestRflDir,
						fnStatusCallback, UserData);

Exit:

	// Unlock and close the database

	if (bDbLocked)
	{
		FlmDbUnlock( hDb);
	}

	if (hDb != HFDB_NULL)
	{
		(void)FlmDbClose( &hDb);
		(void)FlmConfig( FLM_CLOSE_FILE, (void *)pszSrcDbName,
								(void *)pszSrcDataDir);
	}
	return( rc);
}

/****************************************************************************
Desc:	Copy a database's files, including roll-forward log files.
*****************************************************************************/
FSTATIC RCODE flmCopyDb(
	FLMUINT				uiDbVersion,
	const char *		pszSrcDbName,
	const char *		pszSrcDataDir,
	const char *		pszSrcRflDir,
	const char *		pszDestDbName,
	const char *		pszDestDataDir,
	const char *		pszDestRflDir,
	STATUS_HOOK			fnStatusCallback,
	void *				UserData)
{
	RCODE						rc = FERR_OK;
	DB_COPY_INFO			DbCopyInfo;
	F_SuperFileHdl			SrcSFileHdl;
	F_SuperFileHdl			DestSFileHdl;
	FLMUINT					uiFileNumber;
	FLMUINT					uiHighFileNumber;
	FLMUINT					uiHighLogFileNumber;
	FLMUINT					uiFileSize;
	FFILE *					pFile;
	FLMBOOL					bMutexLocked = FALSE;
	F_FileHdlImp *			pLockFileHdl = NULL;
	F_FileHdlImp *			pTmpFileHdl = NULL;
	F_DirHdl *				pDirHdl = NULL;
	FLMBOOL					bFileLocked = FALSE;
	FLMBOOL					bWriteLocked = FALSE;
	ServerLockObject *	pWriteLockObj = NULL;
	ServerLockObject *	pFileLockObj = NULL;
	COPIED_NAME *			pCopiedList = NULL;
	FLMBOOL					bUsedFFile = FALSE;
	FLMBYTE *				pucInMemLogHdr = NULL;
	FLOCK_INFO				LockInfo;
	char *					pszActualSrcRflPath = NULL;
	char *					pszSrcPrefix = NULL;
	char *					pszActualDestRflPath = NULL;
	char *					pszDestPrefix = NULL;
	FLMBOOL					bCreatedDestRflDir = FALSE;

	f_memset( &DbCopyInfo, 0, sizeof( DbCopyInfo));

	// Should not do anything if the source and destination names
	// are the same.

	if (f_stricmp( pszSrcDbName, pszDestDbName) == 0)
	{
		goto Exit;
	}

	// Allocate memory for paths we don't want to push onto the stack.

	if (RC_BAD( rc = f_calloc( 
		(F_PATH_MAX_SIZE + F_FILENAME_SIZE) * 2, &pszActualSrcRflPath)))
	{
		goto Exit;
	}

	pszSrcPrefix = &pszActualSrcRflPath[ F_PATH_MAX_SIZE];
	pszActualDestRflPath = &pszSrcPrefix[ F_FILENAME_SIZE];
	pszDestPrefix = &pszActualDestRflPath[ F_PATH_MAX_SIZE];

	// Set up the super file object for the source database.
	// Must at least open the control file.

	if (RC_BAD( rc = SrcSFileHdl.Setup( NULL, pszSrcDbName, pszSrcDataDir)))
	{
		goto Exit;
	}

	SrcSFileHdl.SetDbVersion( uiDbVersion);

	// Lock the destination database, if not already locked.
	// This is so we can overwrite it without necessarily
	// deleting it.  May unlock and re-lock the global mutex.

	f_mutexLock( gv_FlmSysData.hShareMutex);
	bMutexLocked = TRUE;

	if (RC_BAD( rc = flmFindFile( pszDestDbName, pszDestDataDir, &pFile)))
	{
		goto Exit;
	}

	// If we didn't find an FFILE structure, get an
	// exclusive lock on the file.

	if (!pFile)
	{
		f_mutexUnlock( gv_FlmSysData.hShareMutex);
		bMutexLocked = FALSE;

		// Attempt to get an exclusive lock on the file.

		if (RC_BAD( rc = flmCreateLckFile( pszDestDbName, &pLockFileHdl)))
		{
			goto Exit;
		}
	}
	else
	{
		// The call to flmVerifyFileUse will wait if the file is in
		// the process of being opened by another thread.

		if (RC_BAD( rc = flmVerifyFileUse( gv_FlmSysData.hShareMutex, &pFile)))
		{
			goto Exit;
		}

		// Increment the use count on the FFILE so it will not
		// disappear while we are copying the file.

		if (++pFile->uiUseCount == 1)
		{
			flmUnlinkFileFromNUList( pFile);
		}
		bUsedFFile = TRUE;

		f_mutexUnlock( gv_FlmSysData.hShareMutex);
		bMutexLocked = FALSE;
		pucInMemLogHdr = &pFile->ucLastCommittedLogHdr [0];

		// Lock the destination file object and transaction
		// object, if not already locked.

		pFile->pFileLockObj->GetLockInfo( (FLMINT)0, &LockInfo);
		if ((LockInfo.eCurrLockType != FLM_LOCK_EXCLUSIVE) ||
			 (LockInfo.uiThreadId != f_threadId()))
		{
			pFileLockObj = pFile->pFileLockObj;
			pFileLockObj->AddRef();
			if (RC_BAD( rc = pFileLockObj->Lock( TRUE, NULL, FALSE, TRUE, 15, 0)))
			{
				goto Exit;
			}
			bFileLocked = TRUE;
		}

		// Lock the write object, if not already locked

		pFile->pWriteLockObj->GetLockInfo( (FLMINT)0, &LockInfo);
		if ((LockInfo.eCurrLockType != FLM_LOCK_EXCLUSIVE) ||
			 (LockInfo.uiThreadId != (FLMUINT)f_threadId()))
		{
			pWriteLockObj = pFile->pWriteLockObj;
			pWriteLockObj->AddRef();

			// Only contention here is with the checkpoint thread - wait
			// forever until the checkpoint thread gives it up.

			if (RC_BAD( rc = dbWriteLock( pFile)))
			{
				goto Exit;
			}
			bWriteLocked = TRUE;
		}
	}

	// Set up the super file object for the destination database.

	if (RC_BAD( rc = DestSFileHdl.Setup( NULL, pszDestDbName,
											pszDestDataDir)))
	{
		goto Exit;
	}

	DestSFileHdl.SetDbVersion( uiDbVersion);

	// Setup the ECache manager

	if( pFile && pFile->pECacheMgr)
	{
		DestSFileHdl.setECacheMgr( pFile->pECacheMgr);
	}

	// See how many files we have and calculate the total size.

	uiHighFileNumber = 0;
	for (;;)
	{
		if ((RC_BAD( rc = SrcSFileHdl.GetFileSize( 
			uiHighFileNumber, &uiFileSize))) || !uiFileSize )
		{
			if (rc == FERR_IO_PATH_NOT_FOUND ||
				 rc == FERR_IO_INVALID_PATH ||
				 !uiFileSize)
			{
				// If the control file doesn't exist, we will return
				// path not found.

				if (!uiHighFileNumber)
				{
					goto Exit;
				}
				uiHighFileNumber--;
				rc = FERR_OK;
				break;
			}
			goto Exit;
		}

		DbCopyInfo.ui64BytesToCopy += (FLMUINT64)uiFileSize;
		if (uiHighFileNumber == MAX_DATA_BLOCK_FILE_NUMBER( uiDbVersion))
		{
			break;
		}
		uiHighFileNumber++;
	}

	// See how many rollback log files we have, and calculate
	// their total size.

	uiHighLogFileNumber = FIRST_LOG_BLOCK_FILE_NUMBER( uiDbVersion);
	for (;;)
	{
		if ((RC_BAD( rc = SrcSFileHdl.GetFileSize( 
			uiHighLogFileNumber, &uiFileSize))) || !uiFileSize)
		{
			if (rc == FERR_IO_PATH_NOT_FOUND ||
				 rc == FERR_IO_INVALID_PATH ||
				 !uiFileSize)
			{
				if (uiHighLogFileNumber ==
							FIRST_LOG_BLOCK_FILE_NUMBER( uiDbVersion))
				{
					uiHighLogFileNumber = 0;
				}
				else
				{
					uiHighLogFileNumber--;
				}
				rc = FERR_OK;
				break;
			}
			goto Exit;
		}

		DbCopyInfo.ui64BytesToCopy += (FLMUINT64)uiFileSize;
		if (uiHighLogFileNumber == MAX_LOG_BLOCK_FILE_NUMBER( uiDbVersion))
		{
			break;
		}
		uiHighLogFileNumber++;
	}

	// Get the sizes of the roll-forward log files

	if( uiDbVersion < FLM_VER_4_3)
	{
		// For pre-4.3 versions, only need to copy one RFL file.

		if (RC_BAD( rc = rflGetFileName( uiDbVersion,
			pszSrcDbName, pszSrcRflDir, 1, pszActualSrcRflPath)))
		{
			goto Exit;
		}

		if (RC_BAD( rc = gv_FlmSysData.pFileSystem->Open( 
							pszActualSrcRflPath,
							F_IO_RDWR | F_IO_SH_DENYNONE | F_IO_DIRECT,
							(F_FileHdl **)&pTmpFileHdl)))
		{
			if (rc == FERR_IO_PATH_NOT_FOUND ||
				 rc == FERR_IO_INVALID_PATH)
			{
				rc = FERR_OK;
			}
			else
			{
				goto Exit;
			}
		}
		else
		{
			if (RC_BAD( rc = pTmpFileHdl->Size( &uiFileSize)))
			{
				goto Exit;
			}

			DbCopyInfo.ui64BytesToCopy += (FLMUINT64)uiFileSize;
			pTmpFileHdl->Close();
			pTmpFileHdl->Release();
			pTmpFileHdl = NULL;
		}
	}
	else
	{
		if( RC_BAD( rc = rflGetDirAndPrefix( uiDbVersion, pszSrcDbName, 
			pszSrcRflDir, pszActualSrcRflPath, pszSrcPrefix)))
		{
			goto Exit;
		}

		if (RC_BAD( rc = gv_FlmSysData.pFileSystem->OpenDir(
									pszActualSrcRflPath, (char *)"*", &pDirHdl)))
		{
			goto Exit;
		}

		for (;;)
		{
			if( RC_BAD( rc = pDirHdl->Next()))
			{
				if (rc == FERR_IO_NO_MORE_FILES)
				{
					rc = FERR_OK;
					break;
				}
				else
				{
					goto Exit;
				}
			}

			// If the current file is an RFL file, increment ui64BytesToCopy

			if( rflGetFileNum( uiDbVersion, pszSrcPrefix,
				pDirHdl->CurrentItemName(), &uiFileNumber))
			{
				DbCopyInfo.ui64BytesToCopy += (FLMUINT64)pDirHdl->CurrentItemSize();
			}
		}

		pDirHdl->Release();
		pDirHdl = NULL;
	}

	// Close all file handles in the source and destination

	SrcSFileHdl.ReleaseFiles( TRUE);
	DestSFileHdl.ReleaseFiles( TRUE);

	// Copy the database files.

	for (uiFileNumber = 0; uiFileNumber <= uiHighFileNumber; uiFileNumber++)
	{

		// Get the source file path and destination file path.

		if( RC_BAD( rc = SrcSFileHdl.GetFilePath( 
			uiFileNumber, DbCopyInfo.szSrcFileName)))
		{
			goto Exit;
		}
		if( RC_BAD( rc = DestSFileHdl.GetFilePath( 
			uiFileNumber, DbCopyInfo.szDestFileName)))
		{
			goto Exit;
		}

		// For file #0, don't copy first 2K - it will be set up to show
		// maintenance in progress.  Then the first 2K will be copied
		// later.

		if (!uiFileNumber)
		{
			DbCopyInfo.bNewSrcFile = TRUE;
			if (RC_BAD( rc = flmCopyFile( gv_FlmSysData.pFileSystem,
										2048, 0xFFFFFFFF,
										&DbCopyInfo, &pCopiedList, pucInMemLogHdr, TRUE,
										fnStatusCallback, UserData)))
			{
				goto Exit;
			}

		}
		else
		{
			DbCopyInfo.bNewSrcFile = TRUE;
			if (RC_BAD( rc = flmCopyFile( gv_FlmSysData.pFileSystem,
										0, 0xFFFFFFFF,
										&DbCopyInfo, &pCopiedList, NULL, TRUE,
										fnStatusCallback, UserData)))
			{
				goto Exit;
			}
		}
	}

	// Copy the additional rollback log files, if any.

	for (uiFileNumber = FIRST_LOG_BLOCK_FILE_NUMBER( uiDbVersion);
		  uiFileNumber <= uiHighLogFileNumber; uiFileNumber++)
	{

		// Get the source file path and destination file path.

		if (RC_BAD( rc = SrcSFileHdl.GetFilePath( uiFileNumber,
									DbCopyInfo.szSrcFileName)))
		{
			goto Exit;
		}

		if (RC_BAD( rc = DestSFileHdl.GetFilePath( uiFileNumber,	
			DbCopyInfo.szDestFileName)))
		{
			goto Exit;
		}

		DbCopyInfo.bNewSrcFile = TRUE;
		if (RC_BAD( rc = flmCopyFile( gv_FlmSysData.pFileSystem,
									0, 0xFFFFFFFF,
									&DbCopyInfo, &pCopiedList, NULL, TRUE,
									fnStatusCallback, UserData)))
		{
			goto Exit;
		}
	}

	// Copy the RFL files

	if( uiDbVersion < FLM_VER_4_3)
	{
		// Get the source file path and the destination file path.

		if (RC_BAD( rc = rflGetFileName( uiDbVersion, pszSrcDbName,
										pszSrcRflDir, 1,
										DbCopyInfo.szSrcFileName)))
		{
			goto Exit;
		}

		if (RC_BAD( rc = rflGetFileName( uiDbVersion, pszDestDbName, 
									pszDestRflDir, 1,
									DbCopyInfo.szDestFileName)))
		{
			goto Exit;
		}

		DbCopyInfo.bNewSrcFile = TRUE;
		if (RC_BAD( rc = flmCopyFile( gv_FlmSysData.pFileSystem,
									0, 0xFFFFFFFF,
									&DbCopyInfo, &pCopiedList, NULL, TRUE,
									fnStatusCallback, UserData)))
		{
			goto Exit;
		}
	}
	else
	{
		// Create the destination RFL directory, if needed.  The purpose of this
		// code is two-fold: 1) We want to keep track of the fact that we tried
		// to create the destination RFL directory so we can try to remove it
		// if the copy fails; 2) If the destination RFL directory path specifies
		// a directory with existing files, we want to remove them.

		if( RC_BAD( rc = rflGetDirAndPrefix( uiDbVersion, pszDestDbName, 
			pszDestRflDir, pszActualDestRflPath, pszDestPrefix)))
		{
			goto Exit;
		}

		if( RC_OK( gv_FlmSysData.pFileSystem->Exists( pszActualDestRflPath)))
		{
			if( gv_FlmSysData.pFileSystem->IsDir( pszActualDestRflPath))
			{
				// Remove the existing directory and all files, etc.

				(void)gv_FlmSysData.pFileSystem->RemoveDir( 
					pszActualDestRflPath, TRUE);
			}
			else
			{
				(void)gv_FlmSysData.pFileSystem->Delete( pszActualDestRflPath);
			}
		}

		// Try to create the destination RFL directory.  This might fail if
		// another process was accessing the directory for some reason 
		// (i.e., from a command prompt), when we tried to remove it above.
		// We really don't care if the call to CreateDir is sucessful, because
		// when we try to create the destination files (below), the FLAIM file
		// file system code will try to create any necessary directories.

		(void)gv_FlmSysData.pFileSystem->CreateDir( pszActualDestRflPath);
		bCreatedDestRflDir = TRUE;

		// Copy the RFL files.  NOTE:  We need to copy all of the RFL files
		// in the source RFL directory so that they will be available
		// when performing a database restore operation.

		if (RC_BAD( rc = gv_FlmSysData.pFileSystem->OpenDir(
									pszActualSrcRflPath, (char *)"*", &pDirHdl)))
		{
			goto Exit;
		}

		for (;;)
		{
			if( RC_BAD( rc = pDirHdl->Next()))
			{
				if (rc == FERR_IO_NO_MORE_FILES)
				{
					rc = FERR_OK;
					break;
				}
				else
				{
					goto Exit;
				}
			}

			// If the current file is an RFL file, copy it to the destination

			if( rflGetFileNum( uiDbVersion, pszSrcPrefix,
				pDirHdl->CurrentItemName(), &uiFileNumber))
			{
				// Get the source file path and the destination file path.

				if (RC_BAD( rc = rflGetFileName( uiDbVersion, pszSrcDbName,
												pszSrcRflDir, uiFileNumber,
												DbCopyInfo.szSrcFileName)))
				{
					goto Exit;
				}

				if (RC_BAD( rc = rflGetFileName( uiDbVersion, pszDestDbName, 
											pszDestRflDir, uiFileNumber,
											DbCopyInfo.szDestFileName)))
				{
					goto Exit;
				}

				DbCopyInfo.bNewSrcFile = TRUE;
				if (RC_BAD( rc = flmCopyFile( gv_FlmSysData.pFileSystem,
											0, 0xFFFFFFFF,
											&DbCopyInfo, &pCopiedList, NULL, TRUE,
											fnStatusCallback, UserData)))
				{
					goto Exit;
				}
			}
		}

		pDirHdl->Release();
		pDirHdl = NULL;
	}

	// Do one final copy on the control file to copy just the first 2K

	if (RC_BAD( rc = SrcSFileHdl.GetFilePath( 0, DbCopyInfo.szSrcFileName)))
	{
		goto Exit;
	}

	if (RC_BAD( rc = DestSFileHdl.GetFilePath( 0, DbCopyInfo.szDestFileName)))
	{
		goto Exit;
	}

	DbCopyInfo.bNewSrcFile = FALSE;
	if (RC_BAD( rc = flmCopyFile( gv_FlmSysData.pFileSystem,
								0, 2048,
								&DbCopyInfo, NULL, pucInMemLogHdr, FALSE,
								fnStatusCallback, UserData)))
	{
		goto Exit;
	}

Exit:

	if (bUsedFFile)
	{
		if (!bMutexLocked)
		{
			f_mutexLock( gv_FlmSysData.hShareMutex);
			bMutexLocked = TRUE;
		}

		if (!(--pFile->uiUseCount))
		{
			flmLinkFileToNUList( pFile);
		}
	}

	if (bMutexLocked)
	{
		f_mutexUnlock( gv_FlmSysData.hShareMutex);
		bMutexLocked = FALSE;
	}

	/* Unlock the file, if it is locked. */

	if (bWriteLocked)
	{
		dbWriteUnlock( pFile);
		bWriteLocked = FALSE;
	}

	if (bFileLocked)
	{
		RCODE	rc3;

		if (RC_BAD( rc3 = pFileLockObj->Unlock( TRUE, NULL)))
		{
			if (RC_OK( rc))
				rc = rc3;
		}
		bFileLocked = FALSE;
	}

	if (pWriteLockObj)
	{
		pWriteLockObj->Release();
		pWriteLockObj = NULL;
	}

	if (pFileLockObj)
	{
		pFileLockObj->Release();
		pFileLockObj = NULL;
	}

	if (pLockFileHdl)
	{
		(void)pLockFileHdl->Close();
		pLockFileHdl->Release();
		pLockFileHdl = NULL;
	}

	if( pTmpFileHdl)
	{
		pTmpFileHdl->Release();
	}

	if( pDirHdl)
	{
		pDirHdl->Release();
	}

	// Free all the names of files that were copied.
	// If the copy didn't finish, try to delete any files
	// that were copied.

	while (pCopiedList)
	{
		COPIED_NAME *	pNext = pCopiedList->pNext;

		// If the overall copy failed, delete the copied file.

		if (RC_BAD( rc))
		{
			(void)gv_FlmSysData.pFileSystem->Delete( pCopiedList->szPath);
		}

		f_free( &pCopiedList);
		pCopiedList = pNext;
	}

	if( RC_BAD( rc) && bCreatedDestRflDir)
	{
		(void)gv_FlmSysData.pFileSystem->RemoveDir( pszActualDestRflPath);
	}

	if( pszActualSrcRflPath)
	{
		f_free( &pszActualSrcRflPath);
	}

	return( rc);
}

/****************************************************************************
Desc:	Copy a file that is one of the files of the database.
*****************************************************************************/
FSTATIC RCODE flmCopyFile(
	F_FileSystemImp *		pFileSystem,
	FLMUINT					uiStartOffset,
	FLMUINT					uiEndOffset,
	DB_COPY_INFO *			pDbCopyInfo,
	COPIED_NAME **			ppCopiedListRV,
	FLMBYTE *				pucInMemLogHdr,
	FLMBOOL					bOkToTruncate,
	STATUS_HOOK				fnStatusCallback,
	void *					UserData)
{
	RCODE				rc = FERR_OK;
	FLMBYTE *		pucBuffer = NULL;
	F_FileHdlImp *	pSrcFileHdl = NULL;
	F_FileHdlImp *	pDestFileHdl = NULL;
	FLMUINT			uiBufferSize = 32768;
	FLMUINT			uiBytesToRead;
	FLMUINT			uiBytesRead;
	FLMUINT			uiBytesWritten;
	FLMUINT			uiOffset;
	FLMBYTE			ucLogHdr [LOG_HEADER_SIZE];
	FLMUINT			uiNewChecksum;
	FLMBOOL			bCreatedDestFile = FALSE;

	// Open the source file.

	if( RC_BAD( rc = gv_FlmSysData.pFileSystem->Open( pDbCopyInfo->szSrcFileName,
		F_IO_RDWR | F_IO_SH_DENYNONE | F_IO_DIRECT, 
		(F_FileHdl **)&pSrcFileHdl)))
	{
		goto Exit;
	}

	// First attempt to open the destination file.  If it does
	// not exist, attempt to create it.

	if (RC_BAD( rc = gv_FlmSysData.pFileSystem->Open( pDbCopyInfo->szDestFileName,
							F_IO_RDWR | F_IO_SH_DENYNONE | F_IO_DIRECT, 
							(F_FileHdl **)&pDestFileHdl)))
	{
		if (rc != FERR_IO_PATH_NOT_FOUND &&
			 rc != FERR_IO_INVALID_PATH)
		{
			goto Exit;
		}

		if( RC_BAD( rc = gv_FlmSysData.pFileSystem->Create( pDbCopyInfo->szDestFileName,
								F_IO_RDWR | F_IO_EXCL | F_IO_SH_DENYNONE |
								F_IO_CREATE_DIR | F_IO_DIRECT, 
								(F_FileHdl **)&pDestFileHdl)))
		{
			goto Exit;
		}
		bCreatedDestFile = TRUE;
	}
								
	// Allocate a buffer for reading and writing.

	if (RC_BAD( rc = f_alloc( uiBufferSize, &pucBuffer)))
	{
		goto Exit;
	}

	// If uiStartOffset is 2048, it is the special case of
	// the control file, and we are not copying the first 2K.
	// However, we need to set up the first 2K so that if
	// someone reads the first 2K, it will return them a
	// maintenance in progress error.

	if (uiStartOffset == 2048)
	{
		// Read the first 2K of the source file.

		if (RC_BAD( rc = pSrcFileHdl->SectorRead( 0L, 2048,
									pucBuffer, &uiBytesRead)))
		{
			if (rc == FERR_IO_END_OF_FILE)
			{
				rc = FERR_OK;
			}
			else
			{
				goto Exit;
			}
		}

		// Zero out whatever part of the 2K we didn't get on the read.

		if (uiBytesRead < 2048)
		{
			f_memset( &pucBuffer [uiBytesRead], 0, (int)(2048 - uiBytesRead));
		}

		// Attempt to read the log header from the destination file.
		// It is OK if we can't read it, because if we created the
		// destination file, these bytes may not be present.

		if ((bCreatedDestFile) ||
			 (RC_BAD( pDestFileHdl->Read( 16L, LOG_HEADER_SIZE,
											ucLogHdr, &uiBytesRead))))
		{
			f_memset( ucLogHdr, 0, sizeof(ucLogHdr));
		}

		/*
		Set the transaction ID to zero.  MUST ALSO SET THE TRANS ACTIVE FLAG
		TO FALSE - OTHERWISE READERS WILL ATTEMPT TO DECREMENT THE
		TRANSACTION ID AND WILL END UP WITH 0xFFFFFFFF - very bad!
		We must use zero, because it is the only transaction ID that will not
		appear on ANY block.
		*/

		UD2FBA( 0L, &ucLogHdr [LOG_CURR_TRANS_ID]);

		/*
		Recalculate the log header checksum so that readers will not get a
		checksum error.
		*/

		uiNewChecksum = lgHdrCheckSum( ucLogHdr, FALSE);
		UW2FBA( (FLMUINT16)uiNewChecksum, &ucLogHdr [LOG_HDR_CHECKSUM]);
		f_memcpy( &pucBuffer [16], ucLogHdr, LOG_HEADER_SIZE);

		// Write this "special" first 2K into the destination file.
		// The real first 2K from the source file will be copied in
		// at a later time.

		if (RC_BAD( rc = pDestFileHdl->Write( 0L, 2048, 
			pucBuffer, &uiBytesWritten)))
		{
			goto Exit;
		}

		// Save the log header to the in-memory version of the log
		// header as well - if pucInMemLogHdr is NULL, it is pointing
		// to the pFile->ucLastCommittedLogHdr buffer.

		if (pucInMemLogHdr)
		{
			f_memcpy( pucInMemLogHdr, ucLogHdr, LOG_HEADER_SIZE);
		}
	}

	// Read from source file until we hit EOF in the file or
	// we hit the end offset.

	uiOffset = uiStartOffset;
	for (;;)
	{
		uiBytesToRead = (FLMUINT)((uiEndOffset - uiOffset >=
											uiBufferSize)
										 ? uiBufferSize
										 : (FLMUINT)(uiEndOffset - uiOffset));

		// Read data from source file.

		if (RC_BAD( rc = pSrcFileHdl->SectorRead( uiOffset, uiBytesToRead,
									pucBuffer, &uiBytesRead)))
		{
			if (rc == FERR_IO_END_OF_FILE)
			{
				rc = FERR_OK;
				if (!uiBytesRead)
				{
					break;
				}
			}
			else
			{
				goto Exit;
			}
		}

		// Write data to destination file.

		if (RC_BAD( rc = pDestFileHdl->Write( uiOffset,
									uiBytesRead, pucBuffer, &uiBytesWritten)))
		{
			goto Exit;
		}

		// See if we wrote out the buffer that has the log header
		// If so, we need to copy it back to the in-memory log
		// header.

		if ((pucInMemLogHdr) &&
			 (uiOffset <= 16) &&
			 (uiOffset + uiBytesWritten >= 16 + LOG_HEADER_SIZE))
		{
			f_memcpy( pucInMemLogHdr, &pucBuffer [16 - uiOffset],
								LOG_HEADER_SIZE);
		}

		uiOffset += uiBytesWritten;

		// Do callback to report progress.

		if (fnStatusCallback)
		{
			pDbCopyInfo->ui64BytesCopied += (FLMUINT64)uiBytesWritten;
			if (RC_BAD( rc = (*fnStatusCallback)( FLM_DB_COPY_STATUS,
										(void *)pDbCopyInfo,
										(void *)0, UserData)))
			{
				goto Exit;
			}
			pDbCopyInfo->bNewSrcFile = FALSE;
		}

		// Quit once we reach the end offset or we read fewer bytes
		// than we asked for.

		if (uiOffset >= uiEndOffset || uiBytesRead < uiBytesToRead)
			break;
	}

	// If we overwrote the destination file, as opposed to creating
	// it, truncate it in case it was larger than the number of
	// bytes we actually copied.

	if (!bCreatedDestFile && bOkToTruncate)
	{
		if (RC_BAD( rc = pDestFileHdl->Truncate( uiOffset)))
		{
			goto Exit;
		}
	}

	// If the copy succeeded, add the destination name to a list
	// of destination files.  This is done so we can clean up
	// copied files if we fail somewhere in the overall database
	// copy.

	if (ppCopiedListRV)
	{
		COPIED_NAME *	pCopyName;

		if( RC_BAD( rc = f_alloc( 
			(FLMUINT)sizeof( COPIED_NAME), &pCopyName)))
		{
			goto Exit;
		}
		f_strcpy( pCopyName->szPath, pDbCopyInfo->szDestFileName);
		pCopyName->pNext = *ppCopiedListRV;
		*ppCopiedListRV = pCopyName;
	}

Exit:

	if (pucBuffer)
	{
		f_free( &pucBuffer);
	}

	if (pSrcFileHdl)
	{
		pSrcFileHdl->Release();
	}

	if (pDestFileHdl)
	{
		pDestFileHdl->Flush();
		pDestFileHdl->Release();
	}

	// Attempt to delete the destination file if
	// we didn't successfully copy it.

	if (RC_BAD( rc))
	{
		(void)pFileSystem->Delete( pDbCopyInfo->szDestFileName);
	}

	return( rc);
}