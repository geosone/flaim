//------------------------------------------------------------------------------
// Desc:	Contains the methods for the F_FileHdl class on Windows platforms.
//
// Tabs:	3
//
//		Copyright (c) 1999-2006 Novell, Inc. All Rights Reserved.
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
// $Id: fwin.cpp 3115 2006-01-19 13:24:39 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------

#include "flaimsys.h"
#include "ffilehdl.h"

#if defined( FLM_WIN)

FSTATIC RCODE _DeleteFile(
	char *	path);

/***************************************************************************
Desc:	Maps WIN errors to IO errors.
***************************************************************************/
RCODE MapWinErrorToFlaim(
	DWORD		udErrCode,
	RCODE		defaultRc)
{

	// Switch on passed in error code value

	switch( udErrCode)
	{
		case ERROR_NOT_ENOUGH_MEMORY:
		case ERROR_OUTOFMEMORY:
			return( RC_SET( NE_XFLM_MEM));
			
		case ERROR_BAD_NETPATH:
		case ERROR_BAD_PATHNAME:
		case ERROR_DIRECTORY:
		case ERROR_FILE_NOT_FOUND:
		case ERROR_INVALID_DRIVE:
		case ERROR_INVALID_NAME:
		case ERROR_NO_NET_OR_BAD_PATH:
		case ERROR_PATH_NOT_FOUND:
			return( RC_SET( NE_XFLM_IO_PATH_NOT_FOUND));

		case ERROR_ACCESS_DENIED:
		case ERROR_SHARING_VIOLATION:
		case ERROR_FILE_EXISTS:
		case ERROR_ALREADY_EXISTS:
			return( RC_SET( NE_XFLM_IO_ACCESS_DENIED));

		case ERROR_BUFFER_OVERFLOW:
		case ERROR_FILENAME_EXCED_RANGE:
			return( RC_SET( NE_XFLM_IO_PATH_TOO_LONG));

		case ERROR_DISK_FULL:
		case ERROR_HANDLE_DISK_FULL:
			return( RC_SET( NE_XFLM_IO_DISK_FULL));

		case ERROR_CURRENT_DIRECTORY:
		case ERROR_DIR_NOT_EMPTY:
			return( RC_SET( NE_XFLM_IO_DIRECTORY_ERR));

		case ERROR_DIRECT_ACCESS_HANDLE:
		case ERROR_INVALID_HANDLE:
		case ERROR_INVALID_TARGET_HANDLE:
			return( RC_SET( NE_XFLM_IO_BAD_FILE_HANDLE));

		case ERROR_HANDLE_EOF:
			return( RC_SET( NE_XFLM_IO_END_OF_FILE));

		case ERROR_OPEN_FAILED:
			return( RC_SET( NE_XFLM_IO_OPEN_ERR));

		case ERROR_CANNOT_MAKE:
			return( RC_SET( NE_XFLM_IO_PATH_CREATE_FAILURE));

		case ERROR_LOCK_FAILED:
		case ERROR_LOCK_VIOLATION:
			return( RC_SET( NE_XFLM_IO_FILE_LOCK_ERR));

		case ERROR_NEGATIVE_SEEK:
		case ERROR_SEEK:
		case ERROR_SEEK_ON_DEVICE:
			return( RC_SET( NE_XFLM_IO_SEEK_ERR));

		case ERROR_NO_MORE_FILES:
		case ERROR_NO_MORE_SEARCH_HANDLES:
			return( RC_SET( NE_XFLM_IO_NO_MORE_FILES));

		case ERROR_TOO_MANY_OPEN_FILES:
			return( RC_SET( NE_XFLM_IO_TOO_MANY_OPEN_FILES));

		case NO_ERROR:
			return( NE_XFLM_OK);

		case ERROR_DISK_CORRUPT:
		case ERROR_DISK_OPERATION_FAILED:
		case ERROR_FILE_CORRUPT:
		case ERROR_FILE_INVALID:
		case ERROR_NOT_SAME_DEVICE:
		case ERROR_IO_DEVICE:
		default:
			return( RC_SET( defaultRc));

   }
}

/****************************************************************************
Desc:
****************************************************************************/
F_FileHdl::F_FileHdl()
{
	m_pNext = NULL;
	m_pPrev = NULL;
	m_bInList = FALSE;
	m_uiAvailTime = 0;
	m_bFileOpened = FALSE;
	m_bDeleteOnRelease = FALSE;
	m_bOpenedReadOnly = FALSE;
	m_pszFileName = NULL;
	
	m_FileHandle = INVALID_HANDLE_VALUE;
	m_uiBlockSize = 0;
	m_uiBytesPerSector = 0;
	m_ui64NotOnSectorBoundMask = 0;
	m_ui64GetSectorBoundMask = 0;
	m_bDoDirectIO = FALSE;
	m_uiExtendSize = 0;
	m_uiMaxAutoExtendSize = gv_XFlmSysData.uiMaxFileSize;
	m_pucAlignedBuff = NULL;
	m_uiAlignedBuffSize = 0;
	m_ui64CurrentPos = 0;
	m_bCanDoAsync = FALSE;			// Change to TRUE when we want to do async writes.
	m_Overlapped.hEvent = NULL;
}

/****************************************************************************
Desc:
****************************************************************************/
F_FileHdl::~F_FileHdl()
{
	if( m_bFileOpened)
	{
		(void)Close();
	}
	
	if (m_pucAlignedBuff)
	{
		gv_XFlmSysData.pGlobalCacheMgr->decrTotalBytes( m_uiAlignedBuffSize);

		(void)VirtualFree( m_pucAlignedBuff, 0, MEM_RELEASE);
		m_pucAlignedBuff = NULL;
		m_uiAlignedBuffSize = 0;
	}
	
	if (m_Overlapped.hEvent)
	{
		CloseHandle( m_Overlapped.hEvent);
	}
	
	if (m_pszFileName)
	{
		f_free( &m_pszFileName);
	}
}

/***************************************************************************
Desc:	Open or create a file.
***************************************************************************/
RCODE F_FileHdl::OpenOrCreate(
	const char *	pszFileName,
   FLMUINT			uiAccess,
	FLMBOOL			bCreateFlag)
{
	char			szSaveFileName[ F_PATH_MAX_SIZE];
	RCODE			rc = NE_XFLM_OK;
	DWORD			udAccessMode = 0;
	DWORD			udShareMode = 0;
	DWORD			udCreateMode = 0;
	DWORD			udAttrFlags = 0;
	DWORD			udErrCode;

	m_bDoDirectIO = (uiAccess & XFLM_IO_DIRECT) ? TRUE : FALSE;

	/* Save the file name in case we have to create the directory. */

	if ((bCreateFlag) && (uiAccess & XFLM_IO_CREATE_DIR))
	{
		f_strcpy( szSaveFileName, pszFileName);
	}

	// If doing direct IO, need to get the sector size.

	if (m_bDoDirectIO)
	{
		if (!m_uiBlockSize)
		{
			m_bDoDirectIO = FALSE;
		}
		else
		{
			if (RC_BAD( rc = gv_pFileSystem->GetSectorSize(
				pszFileName, &m_uiBytesPerSector)))
			{
				goto Exit;
			}
			
			m_ui64NotOnSectorBoundMask = m_uiBytesPerSector - 1;
			m_ui64GetSectorBoundMask = ~m_ui64NotOnSectorBoundMask;

			// Can't do direct IO if the block size isn't a multiple of
			// the sector size.

			if (m_uiBlockSize < m_uiBytesPerSector ||
				 m_uiBlockSize % m_uiBytesPerSector != 0)
			{
				m_bDoDirectIO = FALSE;
			}
		}
	}

	// Only enable asynchronous writes if direct I/O is enabled.

	if (m_bDoDirectIO)
	{
		m_bCanDoAsync = gv_XFlmSysData.bOkToDoAsyncWrites;
	}

	// Set up the file characteristics requested by caller.

   if (uiAccess & XFLM_IO_SH_DENYRW)
   {
      udShareMode = 0;
      uiAccess &= ~XFLM_IO_SH_DENYRW;
   }
   else if (uiAccess & XFLM_IO_SH_DENYWR)
   {
      udShareMode = FILE_SHARE_READ;
      uiAccess &= ~XFLM_IO_SH_DENYWR;
   }
	else if (uiAccess & XFLM_IO_SH_DENYNONE)
   {
      udShareMode = (FILE_SHARE_READ | FILE_SHARE_WRITE);
      uiAccess &= ~XFLM_IO_SH_DENYNONE;
   }
	else
	{
      udShareMode = (FILE_SHARE_READ | FILE_SHARE_WRITE);
	}

	// Begin setting the CreateFile flags and fields

   udAttrFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS;
	if (m_bDoDirectIO)
	{
		udAttrFlags |= FILE_FLAG_NO_BUFFERING;
	}
	
	if (m_bCanDoAsync)
	{
		udAttrFlags |= FILE_FLAG_OVERLAPPED;
	}

   if (bCreateFlag)
   {
   	if (uiAccess & XFLM_IO_EXCL)
		{
	  		udCreateMode = CREATE_NEW;
		}
		else
		{
		   udCreateMode = CREATE_ALWAYS;
		}
   }
	else
   {
		udCreateMode = OPEN_EXISTING;
   }

   udAccessMode = GENERIC_READ | GENERIC_WRITE;

   if( (!bCreateFlag) && (uiAccess & XFLM_IO_RDONLY))
	{
      udAccessMode = GENERIC_READ;
	}

Retry_Create:

	// Try to create or open the file

	if( (m_FileHandle = CreateFile( (LPCTSTR)pszFileName, udAccessMode,
					udShareMode, NULL, udCreateMode,
					udAttrFlags, NULL)) == INVALID_HANDLE_VALUE)
	{
		udErrCode = GetLastError();
		if ((udErrCode == ERROR_PATH_NOT_FOUND) && (uiAccess & XFLM_IO_CREATE_DIR))
		{
			char		szTemp[ F_PATH_MAX_SIZE];
			char		szDirPath[ F_PATH_MAX_SIZE];

			uiAccess &= ~XFLM_IO_CREATE_DIR;

			// Remove the file name for which we are creating the directory.

			if( RC_OK( gv_pFileSystem->pathReduce( szSaveFileName, szDirPath, szTemp)))
			{
				if( RC_OK( rc = gv_pFileSystem->CreateDir( szDirPath)))
				{
					goto Retry_Create;
				}
				else
				{
					goto Exit;
				}
			}
		}
		
		rc = MapWinErrorToFlaim( udErrCode,
						(RCODE)(bCreateFlag
								  ? (RCODE)(m_bDoDirectIO
												? (RCODE)NE_XFLM_DIRECT_CREATING_FILE
												: (RCODE)NE_XFLM_CREATING_FILE)
								  : (RCODE)(m_bDoDirectIO
												? (RCODE)NE_XFLM_DIRECT_OPENING_FILE
												: (RCODE)NE_XFLM_OPENING_FILE)));
		goto Exit;
	}
	
Exit:

	if( RC_BAD( rc))
	{
		m_FileHandle = INVALID_HANDLE_VALUE;
	}
	
   return( rc);
}

/****************************************************************************
Desc:	Create a file
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Create(
	const char *	pszFileName,
	FLMUINT			uiIoFlags )
{
	RCODE				rc = NE_XFLM_OK;

	flmAssert( m_bFileOpened == FALSE);

	if( m_bDeleteOnRelease)
	{
		// This file handle had better not been used for another file
		// before.  Otherwise, we will get a memory leak.

		flmAssert( m_pszFileName == NULL);

		if( RC_BAD( rc = f_alloc( F_PATH_MAX_SIZE, &m_pszFileName)))
		{
			goto Exit;
		}

		f_strcpy( m_pszFileName, pszFileName);
	}

	if( RC_BAD( rc = OpenOrCreate( pszFileName, uiIoFlags, TRUE)))
	{
		goto Exit;
	}
	
	m_bFileOpened = TRUE;
	m_ui64CurrentPos = 0;
	m_bOpenedExclusive = (uiIoFlags & XFLM_IO_SH_DENYRW) ? TRUE : FALSE;

Exit:

	if( RC_BAD( rc) && m_bDeleteOnRelease && m_pszFileName)
	{
		f_free( &m_pszFileName);
	}

	return rc;
}

/****************************************************************************
Desc:	Create a unique file name in the specified directory
****************************************************************************/
RCODE XFLMAPI F_FileHdl::CreateUnique(
	const char *	pszDirName,
	const char *	pszFileExtension,
	FLMUINT			uiIoFlags)
{
	RCODE			rc = NE_XFLM_OK;
	char *		pszTmp;
	FLMBOOL		bModext = TRUE;
	FLMUINT		uiBaseTime = 0;
	FLMBYTE		ucHighByte = 0;
	char			szFileName[ F_FILENAME_SIZE];
	char			szDirPath[ F_PATH_MAX_SIZE];
	char			szTmpPath[ F_PATH_MAX_SIZE];
	FLMUINT		uiCount;

	szFileName[0] = '\0';
	szTmpPath[0] = '\0';
	flmAssert( m_bFileOpened == FALSE);

	if( m_bDeleteOnRelease)
	{

		// This file handle had better not been used for another file
		// before.  Otherwise, we will get a memory leak.

		flmAssert( m_pszFileName == NULL);

	}
	f_strcpy( szDirPath, pszDirName);

	/*
   Search backwards replacing trailing spaces with NULLs.
	*/

	pszTmp = (char *) szDirPath;
	pszTmp += (f_strlen( pszTmp) - 1);
	while( pszTmp >= (char *) szDirPath && (*pszTmp == 0x20))
	{
		*pszTmp = 0;
		pszTmp--;
	}

	/* Append a backslash if one isn't already there. */

	if (pszTmp >= (char *) szDirPath && *pszTmp != '\\')
	{
		pszTmp++;
		*pszTmp++ = '\\';
	}
	else
	{
		pszTmp++;
	}
	*pszTmp = 0;

	if ((pszFileExtension) && (f_strlen( pszFileExtension) >= 3))
	{
		bModext = FALSE;
	}

	uiCount = 0;
	do
	{
		gv_pFileSystem->pathCreateUniqueName( &uiBaseTime,  szFileName, pszFileExtension,
										&ucHighByte, bModext);

		//need to strcpy to the buffer b/c it is uninitialized
		f_strcpy( szTmpPath, szDirPath);
		gv_pFileSystem->pathAppend( szTmpPath, szFileName);
		if( m_pszFileName)
		{
			f_free( &m_pszFileName);
		}

		rc = Create( szTmpPath, uiIoFlags | XFLM_IO_EXCL);
		if (rc == NE_XFLM_IO_DISK_FULL)
		{
			(void)_DeleteFile( szTmpPath);
			goto Exit;
		}
		if ((rc == NE_XFLM_IO_PATH_NOT_FOUND) || (rc == NE_XFLM_IO_INVALID_PASSWORD))
		{
			goto Exit;
		}
	} while ((rc != NE_XFLM_OK) && (uiCount++ < 10));

   /* Check if the path was created. */

   if ((uiCount >= 10) && (rc != NE_XFLM_OK))
   {
		rc = RC_SET( NE_XFLM_IO_PATH_CREATE_FAILURE);
		goto Exit;
   }
	m_bFileOpened = TRUE;
	m_bOpenedExclusive = (uiIoFlags & XFLM_IO_SH_DENYRW) ? TRUE : FALSE;

	// Created file name needs to be returned.
	f_strcpy( pszDirName, szTmpPath);

Exit:

	if( RC_BAD( rc) && m_pszFileName)
	{
		f_free( &m_pszFileName);
	}
	
	return( rc);
}

/****************************************************************************
Desc:	Open a file
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Open(
	const char *	pszFileName,
	FLMUINT			uiIoFlags)
{
	RCODE				rc = NE_XFLM_OK;

	flmAssert( m_bFileOpened == FALSE);

	if( m_bDeleteOnRelease)
	{

		// This file handle had better not been used for another file
		// before.  Otherwise, we will get a memory leak.

		flmAssert( m_pszFileName == NULL);

		if( RC_BAD( rc = f_alloc( F_PATH_MAX_SIZE, &m_pszFileName)))
		{
			goto Exit;
		}

		f_strcpy( m_pszFileName, pszFileName);
	}

	// Loop on error open conditions.

	for(;;)
	{
		if( RC_OK( rc = OpenOrCreate( pszFileName, uiIoFlags, FALSE)))
		{
			break;
		}

		if( rc != NE_XFLM_IO_TOO_MANY_OPEN_FILES )
		{
			goto Exit;
		}

		// If for some reason we cannot open the file, then
		// try to close some other file handle in the list.

		gv_XFlmSysData.pFileHdlMgr->releaseOneAvail( FALSE);
	}

	m_bFileOpened = TRUE;
	m_ui64CurrentPos = 0;
	m_bOpenedReadOnly = (uiIoFlags & XFLM_IO_RDONLY) ? TRUE : FALSE;
	m_bOpenedExclusive = (uiIoFlags & XFLM_IO_SH_DENYRW) ? TRUE : FALSE;

Exit:

	if( RC_BAD( rc) && m_bDeleteOnRelease && m_pszFileName)
	{
		f_free( &m_pszFileName);
	}

	return rc;
}

/****************************************************************************
Desc:	Close a file
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Close( void)
{
	FLMBOOL	bDeleteAllowed = TRUE;
	RCODE		rc = NE_XFLM_OK;

	if( !m_bFileOpened)
	{
		goto Exit;
	}

	if (!CloseHandle( m_FileHandle))
	{
		rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_CLOSING_FILE);
		goto Exit;
	}

	m_FileHandle = INVALID_HANDLE_VALUE;
	m_bFileOpened = m_bOpenedReadOnly = m_bOpenedExclusive = FALSE;

	if (m_bDeleteOnRelease )
	{
		flmAssert( NULL != m_pszFileName );

		if( bDeleteAllowed)
		{
			(void)_DeleteFile( m_pszFileName);
		}
		m_bDeleteOnRelease = FALSE;
		f_free( &m_pszFileName);
	}

Exit:

	return( rc);
}

/****************************************************************************
Desc:	Flush IO to disk
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Flush( void)
{
	RCODE		rc = NE_XFLM_OK;

	if( !m_bDoDirectIO)
	{
		if( !FlushFileBuffers( m_FileHandle))
  		{
			rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_FLUSHING_FILE);
		}
	}
	return( rc);
}

/****************************************************************************
Desc:	Allocate an aligned buffer.
****************************************************************************/
RCODE F_FileHdl::AllocAlignBuffer( void)
{
	RCODE	rc = NE_XFLM_OK;

	// Allocate at least 64K - this will handle most read and write
	// operations and will also be a multiple of the sector size most of
	// the time.  The calculation below rounds it up to the next sector
	// boundary if it is not already on one.

	m_uiAlignedBuffSize = RoundToNextSector( 64 * 1024);
	if ((m_pucAlignedBuff = (FLMBYTE *)VirtualAlloc( NULL,
								(DWORD)m_uiAlignedBuffSize,
								MEM_COMMIT, PAGE_READWRITE)) == NULL)
	{
		rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_MEM);
		goto Exit;
	}
	
	gv_XFlmSysData.pGlobalCacheMgr->incrTotalBytes( m_uiAlignedBuffSize);

Exit:

	return( rc);
}

/****************************************************************************
Desc:	Position and do a single read operation.
****************************************************************************/
RCODE F_FileHdl::DoOneRead(
	FLMUINT64		ui64ReadOffset,
	FLMUINT			uiBytesToRead,
	void *			pvReadBuffer,
	FLMUINT *		puiBytesRead)
{
	RCODE				rc = NE_XFLM_OK;
	OVERLAPPED *	pOverlapped;
	LARGE_INTEGER	liTmp;

	// Position the file to the specified offset.

	if (!m_bCanDoAsync)
	{
		liTmp.QuadPart = ui64ReadOffset;
		if( !SetFilePointerEx( m_FileHandle, liTmp, NULL, FILE_BEGIN))
		{
			rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_POSITIONING_IN_FILE);
			goto Exit;
		}

		pOverlapped = NULL;
	}
	else
	{
		if (!m_Overlapped.hEvent)
		{
			if ((m_Overlapped.hEvent = CreateEvent( NULL, TRUE,
													FALSE, NULL)) == NULL)
			{
				rc = MapWinErrorToFlaim( GetLastError(),
								NE_XFLM_SETTING_UP_FOR_READ);
				goto Exit;
			}
		}
		
		pOverlapped = &m_Overlapped;
		pOverlapped->Offset = (DWORD)(ui64ReadOffset & 0xFFFFFFFF);
		pOverlapped->OffsetHigh = (DWORD)(ui64ReadOffset >> 32);
		
		if( !ResetEvent( pOverlapped->hEvent))
		{
			rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_SETTING_UP_FOR_READ);
			goto Exit;
		}
	}
	
	// Do the read

	if( !ReadFile( m_FileHandle, pvReadBuffer, uiBytesToRead, 
		puiBytesRead, pOverlapped))
	{
		DWORD		udErr = GetLastError();
		
		if( udErr == ERROR_IO_PENDING && m_bCanDoAsync)
		{
			if( !GetOverlappedResult( m_FileHandle, 
				pOverlapped, puiBytesRead, TRUE))
			{
				rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_READING_FILE);
				goto Exit;
			}
		}
		else
		{
			rc = MapWinErrorToFlaim( udErr, NE_XFLM_READING_FILE);
			goto Exit;
		}
	}
	
Exit:

	return( rc);
}

/****************************************************************************
Desc:	Read from a file - reads using aligned buffers and offsets - only
		sector boundaries
****************************************************************************/
RCODE F_FileHdl::DirectRead(
	FLMUINT64		ui64ReadOffset,
	FLMUINT			uiBytesToRead,
   void *			pvBuffer,
	FLMBOOL			bBuffHasFullSectors,
   FLMUINT *		puiBytesReadRV)
{
	RCODE				rc = NE_XFLM_OK;
	FLMUINT			uiBytesRead;
	FLMBYTE *		pucReadBuffer;
	FLMBYTE *		pucDestBuffer;
	FLMUINT			uiMaxBytesToRead;
	FLMBOOL			bHitEOF;

	flmAssert( m_bFileOpened);

	if( puiBytesReadRV)
	{
		*puiBytesReadRV = 0;
	}

	if( ui64ReadOffset == XFLM_IO_CURRENT_POS)
	{
		ui64ReadOffset = m_ui64CurrentPos;
	}

	// This loop does multiple reads (if necessary) to get all of the
	// data.  It uses aligned buffers and reads at sector offsets.

	pucDestBuffer = (FLMBYTE *)pvBuffer;
	for (;;)
	{

		// See if we are using an aligned buffer.  If not, allocate
		// one (if not already allocated), and use it.

		if ((ui64ReadOffset & m_ui64NotOnSectorBoundMask) ||
			 (((FLMUINT64)pucDestBuffer) & m_ui64NotOnSectorBoundMask) ||
			 (((FLMUINT64)uiBytesToRead & m_ui64NotOnSectorBoundMask) &&
			  (!bBuffHasFullSectors)))
		{
			if (!m_pucAlignedBuff)
			{
				if (RC_BAD( rc = AllocAlignBuffer()))
				{
					goto Exit;
				}
			}
			pucReadBuffer = m_pucAlignedBuff;

			// Must read enough bytes to cover all of the sectors that
			// contain the data we are trying to read.  The value of
			// (ui64ReadOffset & m_ui64NotOnSectorBoundMask) will give us the
			// number of additional bytes that are in the sector prior to
			// the read offset.  We then round that up to the next sector
			// to get the total number of bytes we are going to read.

			uiMaxBytesToRead = RoundToNextSector( uiBytesToRead +
									  (ui64ReadOffset & m_ui64NotOnSectorBoundMask));

			// Can't read more than the aligned buffer will hold.

			if (uiMaxBytesToRead > m_uiAlignedBuffSize)
			{
				uiMaxBytesToRead = m_uiAlignedBuffSize;
			}
		}
		else
		{
			uiMaxBytesToRead = RoundToNextSector( uiBytesToRead);
			flmAssert( uiMaxBytesToRead >= uiBytesToRead);
			pucReadBuffer = pucDestBuffer;
		}

		bHitEOF = FALSE;
		if (RC_BAD( rc = DoOneRead( TruncateToPrevSector( ui64ReadOffset),
			uiMaxBytesToRead, pucReadBuffer, &uiBytesRead)))
		{
			goto Exit;
		}
		
		if( uiBytesRead < uiMaxBytesToRead)
		{
			bHitEOF = TRUE;
		}

		// If the offset we want to read from is not on a sector
		// boundary, increment the read buffer pointer to the
		// offset where the data we need starts and decrement the
		// bytes read by the difference between the start of the
		// sector and the actual read offset.

		if (ui64ReadOffset & m_ui64NotOnSectorBoundMask)
		{
			pucReadBuffer += (ui64ReadOffset & m_ui64NotOnSectorBoundMask);
			flmAssert( uiBytesRead >= m_uiBytesPerSector);
			uiBytesRead -= (ui64ReadOffset & m_ui64NotOnSectorBoundMask);
		}

		// If bytes read is more than we actually need, truncate it back
		// so that we only copy what we actually need.

		if( uiBytesRead > uiBytesToRead)
		{
			uiBytesRead = uiBytesToRead;
		}
		
		uiBytesToRead -= uiBytesRead;
		
		if( puiBytesReadRV)
		{
			(*puiBytesReadRV) += uiBytesRead;
		}
		
		m_ui64CurrentPos = ui64ReadOffset + uiBytesRead;

		// If using a different buffer for reading, copy the
		// data read into the destination buffer.

		if (pucDestBuffer != pucReadBuffer)
		{
			f_memcpy( pucDestBuffer, pucReadBuffer, uiBytesRead);
		}
		
		if (!uiBytesToRead)
		{
			break;
		}

		// Still more to read - did we hit EOF above?

		if (bHitEOF)
		{
			rc = RC_SET( NE_XFLM_IO_END_OF_FILE);
			break;
		}
		
		pucDestBuffer += uiBytesRead;
		ui64ReadOffset += uiBytesRead;
	}

Exit:
	return( rc );
}

/****************************************************************************
Desc:	Read from a file
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Read(
	FLMUINT64	ui64ReadOffset,
	FLMUINT		uiBytesToRead,
   void *		pvBuffer,
   FLMUINT *	puiBytesReadRV)
{
	RCODE			rc = NE_XFLM_OK;
	FLMUINT		uiBytesRead;

	// Do the direct IO call if enabled.

	if (m_bDoDirectIO)
	{
		rc = DirectRead( ui64ReadOffset, uiBytesToRead,
									pvBuffer, FALSE, puiBytesReadRV);
		goto Exit;
	}

	// If not doing direct IO, a single read call will do.

	flmAssert( m_bFileOpened);
	if( puiBytesReadRV)
	{
		*puiBytesReadRV = 0;
	}

	if( ui64ReadOffset == XFLM_IO_CURRENT_POS)
	{
		ui64ReadOffset = m_ui64CurrentPos;
	}

	if( RC_BAD( rc = DoOneRead( ui64ReadOffset, uiBytesToRead,
		pvBuffer, &uiBytesRead)))
	{
		goto Exit;
	}

	if( puiBytesReadRV)
	{
		*puiBytesReadRV = uiBytesRead;
	}

	m_ui64CurrentPos = ui64ReadOffset + uiBytesRead;

	if (uiBytesRead < uiBytesToRead)
	{
		rc = RC_SET( NE_XFLM_IO_END_OF_FILE);
		goto Exit;
	}

Exit:
	return( rc );
}

/****************************************************************************
Desc:	Sets current position of file.
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Seek(
	FLMUINT64			ui64Offset,
	FLMINT				iWhence,
	FLMUINT64 *			pui64NewOffset)
{
	RCODE	rc = NE_XFLM_OK;

	switch (iWhence)
	{
		case XFLM_IO_SEEK_CUR:
			m_ui64CurrentPos += ui64Offset;
			break;
		case XFLM_IO_SEEK_SET:
			m_ui64CurrentPos = ui64Offset;
			break;
		case XFLM_IO_SEEK_END:
			if( RC_BAD( rc = Size( &m_ui64CurrentPos )))
			{
				goto Exit;
			}
			break;
		default:
			rc = RC_SET_AND_ASSERT( NE_XFLM_NOT_IMPLEMENTED);
			goto Exit;
	}
	
	if( pui64NewOffset)
	{
		*pui64NewOffset = m_ui64CurrentPos;
	}
	
Exit:

	return( rc);
}

/****************************************************************************
Desc:	Return the size of the file
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Size(
	FLMUINT64 *		pui64Size)
{
	RCODE					rc = NE_XFLM_OK;
	LARGE_INTEGER		liTmp;
	
	if( !GetFileSizeEx( m_FileHandle, &liTmp))
	{
		rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_GETTING_FILE_SIZE);
		goto Exit;
	}
	
	*pui64Size = liTmp.QuadPart;

Exit:

	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Tell(
	FLMUINT64 *		pui64Offset)
{
	*pui64Offset = m_ui64CurrentPos;
	return( NE_XFLM_OK);
}

/****************************************************************************
Desc:		Truncate the file to the indicated size
WARNING: Direct IO methods are calling this method.  Make sure that all changes
			to this method work in direct IO mode.
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Truncate(
	FLMUINT64		ui64Size)
{
	RCODE					rc = NE_XFLM_OK;
	LARGE_INTEGER		liTmp;

	flmAssert( m_bFileOpened);

	// Position the file to the nearest sector below the read offset.
	
	liTmp.QuadPart = ui64Size;
	if( !SetFilePointerEx( m_FileHandle, liTmp, NULL, FILE_BEGIN))
	{
		rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_POSITIONING_IN_FILE);
		goto Exit;
	}
		
   // Set the new file size.

	if( !SetEndOfFile( m_FileHandle))
	{
		rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_TRUNCATING_FILE);
		goto Exit;
	}
	
Exit:

	return( rc);
}

/****************************************************************************
Desc:		Handles when a file is extended in direct IO mode.  May extend the
			file some more.  Will always call FlushFileBuffers to ensure that
			the new file size gets written out.
****************************************************************************/
RCODE F_FileHdl::extendFile(
	FLMUINT64		ui64EndOfLastWrite,	// Must be on a sector boundary
	FLMUINT			uiMaxBytesToExtend,
	FLMBOOL			bFlush)
{
	RCODE				rc = NE_XFLM_OK;
	OVERLAPPED *	pOverlapped;
	FLMUINT			uiTotalBytesToExtend;
	FLMUINT			uiBytesToWrite;
	FLMUINT			uiBytesWritten;
	LARGE_INTEGER	liTmp;

	if ((uiTotalBytesToExtend = uiMaxBytesToExtend) != 0)
	{
		if (ui64EndOfLastWrite > m_uiMaxAutoExtendSize)
		{
			uiTotalBytesToExtend = 0;
		}
		else
		{
			// Don't extend beyond maximum file size.

			if (m_uiMaxAutoExtendSize - ui64EndOfLastWrite < uiTotalBytesToExtend)
			{
				uiTotalBytesToExtend = m_uiMaxAutoExtendSize - ui64EndOfLastWrite;
			}

			// If the extend size is not on a sector boundary, round it down.

			uiTotalBytesToExtend = TruncateToPrevSector( uiTotalBytesToExtend);
		}
	}

	if (uiTotalBytesToExtend)
	{
		// Allocate an aligned buffer if we haven't already.

		if (!m_pucAlignedBuff)
		{
			if (RC_BAD( rc = AllocAlignBuffer()))
			{
				goto Exit;
			}
		}
		
		f_memset( m_pucAlignedBuff, 0, m_uiAlignedBuffSize);
	}

	// Extend the file until we run out of bytes to write.

	while (uiTotalBytesToExtend)
	{
		if ((uiBytesToWrite = m_uiAlignedBuffSize) > uiTotalBytesToExtend)
		{
			uiBytesToWrite = uiTotalBytesToExtend;
		}
		
		if (!m_bCanDoAsync)
		{
			liTmp.QuadPart = ui64EndOfLastWrite;
			if( !SetFilePointerEx( m_FileHandle, liTmp, NULL, FILE_BEGIN))
			{
				rc = MapWinErrorToFlaim( GetLastError(), 
							NE_XFLM_POSITIONING_IN_FILE);
				goto Exit;
			}
		
			pOverlapped = NULL;
		}
		else
		{
			pOverlapped = &m_Overlapped;
			if (!pOverlapped->hEvent)
			{
				if ((pOverlapped->hEvent = CreateEvent( NULL, TRUE,
														FALSE, NULL)) == NULL)
				{
					rc = MapWinErrorToFlaim( GetLastError(),
								NE_XFLM_SETTING_UP_FOR_WRITE);
					goto Exit;
				}
			}
			
			pOverlapped->Offset = (DWORD)(ui64EndOfLastWrite & 0xFFFFFFFF);
			pOverlapped->OffsetHigh = (DWORD)(ui64EndOfLastWrite >> 32);
			
			if (!ResetEvent( pOverlapped->hEvent))
			{
				rc = MapWinErrorToFlaim( GetLastError(),
							NE_XFLM_SETTING_UP_FOR_WRITE);
				goto Exit;
			}
		}
		
		// Do the write

		if( !WriteFile( m_FileHandle, m_pucAlignedBuff,
			uiBytesToWrite, &uiBytesWritten, pOverlapped))
		{
			rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_WRITING_FILE);
			
			// Don't care if it is a disk full error, because
			// extending the file is optional work.

			if( rc == NE_XFLM_IO_DISK_FULL)
			{
				rc = NE_XFLM_OK;
				break;
			}
			
			goto Exit;
		}

		// NO more room on disk, but that's OK because we were only
		// extending the file beyond where it needed to be.  If that
		// fails, we will just flush what we have done so far.

		if( uiBytesWritten < uiBytesToWrite)
		{
			break;
		}
		
		uiTotalBytesToExtend -= uiBytesToWrite;
		ui64EndOfLastWrite += uiBytesToWrite;
	}

	// Flush the file buffers to ensure that the file size gets written
	// out.

	if( bFlush)
	{
		if( !FlushFileBuffers( m_FileHandle))
  		{
			rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_FLUSHING_FILE);
			goto Exit;
		}
	}

Exit:

	return( rc);
}

/****************************************************************************
Desc:		Write to a file using direct IO
****************************************************************************/
RCODE F_FileHdl::DirectWrite(
	FLMUINT64			ui64WriteOffset,
	FLMUINT				uiBytesToWrite,
   const void *		pvBuffer,
	F_IOBuffer *		pBufferObj,
	FLMBOOL				bBuffHasFullSectors,
	FLMBOOL				bZeroFill,
   FLMUINT *			puiBytesWrittenRV)
{
	RCODE				rc = NE_XFLM_OK;
	FLMUINT			uiBytesRead;
	FLMUINT			uiBytesWritten;
	FLMBYTE *		pucWriteBuffer;
	FLMBYTE *		pucSrcBuffer;
	FLMUINT			uiMaxBytesToWrite;
	FLMUINT64		ui64CurrFileSize;
	FLMUINT			uiBytesBeingOutput;
	OVERLAPPED *	pOverlapped;
	DWORD				udErr;
	FLMBOOL			bExtendFile = FALSE;
	FLMBOOL			bDoAsync = (pBufferObj != NULL)
										? TRUE
										: FALSE;
	FLMBOOL			bDidAsync = FALSE;
	FLMUINT64		ui64LastWriteOffset;
	FLMUINT			uiLastWriteSize;
	LARGE_INTEGER	liTmp;

	flmAssert( m_bFileOpened);

#ifdef FLM_DEBUG
	if (bDoAsync)
	{
		flmAssert( m_bCanDoAsync);
	}
#endif

	if( puiBytesWrittenRV)
	{
		*puiBytesWrittenRV = 0;
	}

	if( ui64WriteOffset == XFLM_IO_CURRENT_POS)
	{
		ui64WriteOffset = m_ui64CurrentPos;
	}

	// Determine if the write will extend the file beyond its
	// current size.  If so, we will need to call FlushFileBuffers

	if( !GetFileSizeEx( m_FileHandle, &liTmp))
	{
		rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_GETTING_FILE_SIZE);
		goto Exit;
	}
	
	ui64CurrFileSize = liTmp.QuadPart;
	
	if( ui64WriteOffset + uiBytesToWrite > ui64CurrFileSize &&
		 m_uiExtendSize != (FLMUINT)(~0))
	{
		bExtendFile = TRUE;

		if( ui64WriteOffset > ui64CurrFileSize)
		{

			// Fill in the empty space.

			if (RC_BAD( rc = extendFile( ui64CurrFileSize,
				RoundToNextSector( ui64WriteOffset - ui64CurrFileSize), FALSE)))
			{
				goto Exit;
			}
		}

		// Can't do asynchronous if we are going to extend the file.

		bDoAsync = FALSE;
	}

	// This loop is for direct IO - must make sure we use
	// aligned buffers.

	pucSrcBuffer = (FLMBYTE *)pvBuffer;
	for (;;)
	{

		// See if we are using an aligned buffer.  If not, allocate
		// one (if not already allocated), and use it.

		if ((ui64WriteOffset & m_ui64NotOnSectorBoundMask) ||
			 (((FLMUINT)pucSrcBuffer) & m_ui64NotOnSectorBoundMask) ||
			 ((uiBytesToWrite & m_ui64NotOnSectorBoundMask) &&
			  (!bBuffHasFullSectors)))
		{

			// Cannot be using a temporary write buffer if we are doing
			// asynchronous writes!

			flmAssert( !bDoAsync || !m_bCanDoAsync);
			if (!m_pucAlignedBuff)
			{
				if (RC_BAD( rc = AllocAlignBuffer()))
				{
					goto Exit;
				}
			}
			pucWriteBuffer = m_pucAlignedBuff;

			// Must write enough bytes to cover all of the sectors that
			// contain the data we are trying to write out.  The value of
			// (ui64WriteOffset & m_ui64NotOnSectorBoundMask) will give us the
			// number of additional bytes that are in the sector prior to
			// the read offset.  We then round to the next sector to get the
			// total number of bytes we are going to write.

			uiMaxBytesToWrite = RoundToNextSector( uiBytesToWrite +
									  (ui64WriteOffset & m_ui64NotOnSectorBoundMask));

			// Can't write more than the aligned buffer will hold.

			if (uiMaxBytesToWrite > m_uiAlignedBuffSize)
			{
				uiMaxBytesToWrite = m_uiAlignedBuffSize;
				uiBytesBeingOutput = uiMaxBytesToWrite -
										(ui64WriteOffset & m_ui64NotOnSectorBoundMask);
			}
			else
			{
				uiBytesBeingOutput = uiBytesToWrite;
			}

			// If the write offset is not on a sector boundary, we must
			// read at least the first sector into the buffer.

			if (ui64WriteOffset & m_ui64NotOnSectorBoundMask)
			{

				// Read the first sector that is to be written out.
				// Read one sector's worth of data - so that we will
				// preserve what is already in the sector before
				// writing it back out again.

				if (RC_BAD( rc = DoOneRead( TruncateToPrevSector( ui64WriteOffset),
					m_uiBytesPerSector, pucWriteBuffer, &uiBytesRead)))
				{
					goto Exit;
				}
			}

			// If we are writing more than one sector, and the last sector's
			// worth of data we are writing out is only a partial sector,
			// we must read in this sector as well.

			if ((uiMaxBytesToWrite > m_uiBytesPerSector) &&
				 (uiMaxBytesToWrite > uiBytesToWrite) &&
				 (!bBuffHasFullSectors))
			{

				// Read the last sector that is to be written out.
				// Read one sector's worth of data - so that we will
				// preserve what is already in the sector before
				// writing it back out again.

				if (RC_BAD( rc = DoOneRead(
					(TruncateToPrevSector( ui64WriteOffset)) +
						(uiMaxBytesToWrite - m_uiBytesPerSector),
					m_uiBytesPerSector,
					(&pucWriteBuffer [uiMaxBytesToWrite - m_uiBytesPerSector]),
					&uiBytesRead)))
				{
					if (rc == NE_XFLM_IO_END_OF_FILE)
					{
						rc = NE_XFLM_OK;
						f_memset( &pucWriteBuffer [uiMaxBytesToWrite - m_uiBytesPerSector],
										0, m_uiBytesPerSector);
					}
					else
					{
						goto Exit;
					}
				}
			}

			// Finally, copy the data from the source buffer into the
			// write buffer.

			f_memcpy( &pucWriteBuffer[ ui64WriteOffset & m_ui64NotOnSectorBoundMask],
								pucSrcBuffer, uiBytesBeingOutput);
		}
		else
		{
			uiMaxBytesToWrite = RoundToNextSector( uiBytesToWrite);
			uiBytesBeingOutput = uiBytesToWrite;
			pucWriteBuffer = pucSrcBuffer;
			if( bZeroFill && uiMaxBytesToWrite > uiBytesToWrite)
			{
				f_memset( &pucWriteBuffer [uiBytesToWrite], 0,
							uiMaxBytesToWrite - uiBytesToWrite);
			}
		}

		// Position the file to the nearest sector below the write offset.

		ui64LastWriteOffset = TruncateToPrevSector( ui64WriteOffset);
		if (!m_bCanDoAsync)
		{
			liTmp.QuadPart = ui64LastWriteOffset;
			if( !SetFilePointerEx( m_FileHandle, liTmp, NULL, FILE_BEGIN))
			{
				rc = MapWinErrorToFlaim( GetLastError(),
							NE_XFLM_POSITIONING_IN_FILE);
				goto Exit;
			}
			
			pOverlapped = NULL;
		}
		else
		{
			if (!pBufferObj)
			{
				pOverlapped = &m_Overlapped;
			}
			else
			{
				pOverlapped = pBufferObj->getOverlapped();
				pBufferObj->setFileHandle( m_FileHandle);
			}

			if (!pOverlapped->hEvent)
			{
				if ((pOverlapped->hEvent = CreateEvent( NULL, TRUE,
														FALSE, NULL)) == NULL)
				{
					rc = MapWinErrorToFlaim( GetLastError(),
								NE_XFLM_SETTING_UP_FOR_WRITE);
					goto Exit;
				}
			}
			
			pOverlapped->Offset = (DWORD)(ui64LastWriteOffset & 0xFFFFFFFF);
			pOverlapped->OffsetHigh = (DWORD)(ui64LastWriteOffset >> 32);
			
			if (!ResetEvent( pOverlapped->hEvent))
			{
				rc = MapWinErrorToFlaim( GetLastError(),
								NE_XFLM_SETTING_UP_FOR_WRITE);
				goto Exit;
			}
		}

		// Do the write

		uiLastWriteSize = uiMaxBytesToWrite;
		if( !WriteFile( m_FileHandle, (LPVOID)pucWriteBuffer,
							(DWORD)uiMaxBytesToWrite, &uiBytesWritten,
						  pOverlapped))
		{
			udErr = GetLastError();
			if (udErr == ERROR_IO_PENDING && m_bCanDoAsync)
			{

				// If an async structure was passed in, we better have
				// been able to finish in a single write operation.
				// Otherwise, we are in deep trouble because we are not
				// set up to do multiple async write operations within
				// a single call.

				if( bDoAsync)
				{
					pBufferObj->makePending();
					bDidAsync = TRUE;
					break;
				}

				if (!GetOverlappedResult( m_FileHandle, pOverlapped,
							&uiBytesWritten, TRUE))
				{
					rc = MapWinErrorToFlaim( GetLastError(),
								NE_XFLM_WRITING_FILE);
					goto Exit;
				}
			}
			else
			{
				rc = MapWinErrorToFlaim( udErr, NE_XFLM_WRITING_FILE);
				goto Exit;
			}
		}

		if (uiBytesWritten < uiMaxBytesToWrite)
		{
			rc = RC_SET( NE_XFLM_IO_DISK_FULL);
			goto Exit;
		}

		uiBytesToWrite -= uiBytesBeingOutput;
		
		if( puiBytesWrittenRV)
		{
			(*puiBytesWrittenRV) += uiBytesBeingOutput;
		}
		
		m_ui64CurrentPos = ui64WriteOffset + uiBytesBeingOutput;
		
		if (!uiBytesToWrite)
		{
			break;
		}

		pucSrcBuffer += uiBytesBeingOutput;
		ui64WriteOffset += uiBytesBeingOutput;
	}

	// See if we extended the file.  If so, we may want to extend it some
	// more and then also do a flush.

	if (bExtendFile)
	{
		// NOTE: ui64LastWriteOffset + uiLastWrite is guaranteed to be
		// on a sector boundary.

		if (RC_BAD( rc = extendFile(
								ui64LastWriteOffset + uiLastWriteSize,
								m_uiExtendSize, TRUE)))
		{
			goto Exit;
		}
	}

Exit:

	if( !bDidAsync && pBufferObj)
	{
		pBufferObj->notifyComplete( rc);
	}

	return( rc );
}

/****************************************************************************
Desc:	Write to a file
****************************************************************************/
RCODE XFLMAPI F_FileHdl::Write(
	FLMUINT64		ui64WriteOffset,
	FLMUINT			uiBytesToWrite,
	const void *	pvBuffer,
	FLMUINT *		puiBytesWrittenRV)
{
	RCODE				rc = NE_XFLM_OK;
	FLMUINT			uiBytesWritten;
	OVERLAPPED *	pOverlapped;
	DWORD				udErr;
	LARGE_INTEGER	liTmp;

	if (m_bDoDirectIO)
	{
		rc = DirectWrite( ui64WriteOffset, uiBytesToWrite, pvBuffer,
								NULL, FALSE, FALSE, puiBytesWrittenRV);
		goto Exit;
	}

	// If not doing direct IO, a single write call will do.

	flmAssert( m_bFileOpened);

	if( puiBytesWrittenRV)
	{
		*puiBytesWrittenRV = 0;
	}

	if( ui64WriteOffset == XFLM_IO_CURRENT_POS)
	{
		ui64WriteOffset = m_ui64CurrentPos;
	}

	// Position the file.

	if (!m_bCanDoAsync)
	{
		liTmp.QuadPart = ui64WriteOffset;
		if( !SetFilePointerEx( m_FileHandle, liTmp, NULL, FILE_BEGIN))
		{
			rc = MapWinErrorToFlaim( GetLastError(),
						NE_XFLM_POSITIONING_IN_FILE);
			goto Exit;
		}
		
		pOverlapped = NULL;
	}
	else
	{
		if (!m_Overlapped.hEvent)
		{
			if ((m_Overlapped.hEvent = CreateEvent( NULL, TRUE,
													FALSE, NULL)) == NULL)
			{
				rc = MapWinErrorToFlaim( GetLastError(),
							NE_XFLM_SETTING_UP_FOR_WRITE);
				goto Exit;
			}
		}
		
		pOverlapped = &m_Overlapped;
		pOverlapped->Offset = (DWORD)(ui64WriteOffset & 0xFFFFFFFF);
		pOverlapped->OffsetHigh = (DWORD)(ui64WriteOffset >> 32);
		
		if( !ResetEvent( pOverlapped->hEvent))
		{
			rc = MapWinErrorToFlaim( GetLastError(),
						NE_XFLM_SETTING_UP_FOR_WRITE);
			goto Exit;
		}
	}

	if (!WriteFile( m_FileHandle, (LPVOID)pvBuffer,
						(DWORD)uiBytesToWrite, &uiBytesWritten,
					   pOverlapped))
	{
		udErr = GetLastError();
		if (udErr == ERROR_IO_PENDING && m_bCanDoAsync)
		{
			if (!GetOverlappedResult( m_FileHandle, pOverlapped,
						&uiBytesWritten, TRUE))
			{
				rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_WRITING_FILE);
				goto Exit;
			}
		}
		else
		{
			rc = MapWinErrorToFlaim( udErr, NE_XFLM_WRITING_FILE);
			goto Exit;
		}
	}

	if( puiBytesWrittenRV)
	{
		*puiBytesWrittenRV = uiBytesWritten;
	}

	m_ui64CurrentPos = ui64WriteOffset + uiBytesWritten;
	
	if (uiBytesWritten < uiBytesToWrite)
	{
		rc = RC_SET( NE_XFLM_IO_DISK_FULL);
		goto Exit;
	}

Exit:
	return( rc );
}

/****************************************************************************
Desc:	Deletes a file
****************************************************************************/
FSTATIC RCODE _DeleteFile(
	char *	pszPath)
{
	RCODE			rc = NE_XFLM_OK;

   if( DeleteFile( (LPTSTR)pszPath) == FALSE)
	{
		rc = MapWinErrorToFlaim( GetLastError(), NE_XFLM_IO_DELETING_FILE);
	}

	return rc;
}

#endif // #if defined( FLM_WIN)

#if defined( FLM_WATCOM_NLM)
	int gv_winDummy(void)
	{
		return( 0);
	}
#endif