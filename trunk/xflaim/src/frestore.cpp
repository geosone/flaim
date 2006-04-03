//------------------------------------------------------------------------------
// Desc:	Methods used during restore
//
// Tabs:	3
//
//		Copyright (c) 2001-2006 Novell, Inc. All Rights Reserved.
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
// $Id: frestore.cpp 3114 2006-01-19 13:22:45 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------

#include "flaimsys.h"

/****************************************************************************
Desc: Constructor
****************************************************************************/
F_FSRestore::F_FSRestore()
{
	m_pFileHdl = NULL;
	m_pFileHdl64 = NULL;
	m_ui64Offset = 0;
	m_bSetupCalled = FALSE;
	m_szDbPath[ 0] = 0;
	m_uiDbVersion = 0;
	m_szBackupSetPath[ 0] = 0;
	m_szRflDir[ 0] = 0;
	m_bOpen = FALSE;
}

/****************************************************************************
Desc: Destructor
****************************************************************************/
F_FSRestore::~F_FSRestore()
{
	if( m_bOpen)
	{
		(void)close();
	}
}


/****************************************************************************
Desc:
****************************************************************************/
RCODE F_FSRestore::setup(
	const char *		pszDbPath,
	const char *		pszBackupSetPath,
	const char *		pszRflDir)
{
	flmAssert( !m_bSetupCalled);
	flmAssert( pszDbPath);
	flmAssert( pszBackupSetPath);

	f_strcpy( m_szDbPath, pszDbPath);
	f_strcpy( m_szBackupSetPath, pszBackupSetPath);

	if( pszRflDir)
	{
		f_strcpy( m_szRflDir, pszRflDir);
	}


	m_bSetupCalled = TRUE;
	return( NE_XFLM_OK);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE F_FSRestore::openBackupSet( void)
{
	RCODE			rc = NE_XFLM_OK;

	flmAssert( m_bSetupCalled);
	flmAssert( !m_pFileHdl64);

	if( (m_pFileHdl64 = f_new F_64BitFileHandle) == NULL)
	{
		rc = RC_SET( NE_XFLM_MEM);
		goto Exit;
	}

	if( RC_BAD( rc = m_pFileHdl64->Open( m_szBackupSetPath)))
	{
		m_pFileHdl64->Release();
		m_pFileHdl64 = NULL;
		goto Exit;
	}

	m_ui64Offset = 0;
	m_bOpen = TRUE;

Exit:

	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE F_FSRestore::openRflFile(
	FLMUINT			uiFileNum)
{
	RCODE				rc = NE_XFLM_OK;
	char				szRflPath[ F_PATH_MAX_SIZE];
	char				szBaseName[ F_FILENAME_SIZE];
	FLMUINT			uiBaseNameSize;
	XFLM_DB_HDR			dbHdr;
	IF_FileHdl *	pFileHdl = NULL;

	flmAssert( m_bSetupCalled);
	flmAssert( uiFileNum);
	flmAssert( !m_pFileHdl);

	// Read the database header to determine the version number

	if( !m_uiDbVersion)
	{

		if( RC_BAD( rc = gv_pFileSystem->Open( m_szDbPath,
			XFLM_IO_RDWR | XFLM_IO_SH_DENYNONE | XFLM_IO_DIRECT, &pFileHdl)))
		{
			goto Exit;
		}

		if( RC_BAD( rc = flmReadAndVerifyHdrInfo( NULL, pFileHdl, &dbHdr)))
		{
			goto Exit;
		}

		pFileHdl->Close();
		pFileHdl->Release();
		pFileHdl = NULL;

		m_uiDbVersion = (FLMUINT)dbHdr.ui32DbVersion;
	}

	// Generate the log file name.

	if( RC_BAD( rc = rflGetDirAndPrefix( m_szDbPath, m_szRflDir, szRflPath)))
	{
		goto Exit;
	}

	uiBaseNameSize = sizeof( szBaseName);
	rflGetBaseFileName( uiFileNum, szBaseName, &uiBaseNameSize, NULL);
	gv_pFileSystem->pathAppend( szRflPath, szBaseName);

	// Open the file.

	if( RC_BAD( rc = gv_pFileSystem->OpenBlockFile( szRflPath,
		XFLM_IO_RDWR | XFLM_IO_SH_DENYNONE | XFLM_IO_DIRECT, 512, &m_pFileHdl)))
	{
		goto Exit;
	}

	m_bOpen = TRUE;
	m_ui64Offset = 0;

Exit:

	if( pFileHdl)
	{
		pFileHdl->Release();
	}

	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE F_FSRestore::openIncFile(
	FLMUINT			uiFileNum)
{
	char			szIncPath[ F_PATH_MAX_SIZE];
	char			szIncFile[ F_FILENAME_SIZE];
	RCODE			rc = NE_XFLM_OK;

	flmAssert( m_bSetupCalled);
	flmAssert( !m_pFileHdl64);

	/*
	Since this is a non-interactive restore, we will "guess"
	that incremental backups are located in the same parent
	directory as the main backup set.  We will further assume
	that the incremental backup sets have been named XXXXXXXX.INC,
	where X is a hex digit.
	*/

	if( RC_BAD( rc = gv_pFileSystem->pathReduce( m_szBackupSetPath,
		szIncPath, NULL)))
	{
		goto Exit;
	}

	f_sprintf( szIncFile, "%08X.INC", (unsigned)uiFileNum);
	gv_pFileSystem->pathAppend( szIncPath, szIncFile);

	if( (m_pFileHdl64 = f_new F_64BitFileHandle) == NULL)
	{
		rc = RC_SET( NE_XFLM_MEM);
		goto Exit;
	}

	if( RC_BAD( rc = m_pFileHdl64->Open( szIncPath)))
	{
		m_pFileHdl64->Release();
		m_pFileHdl64 = NULL;
		goto Exit;
	}

	m_ui64Offset = 0;
	m_bOpen = TRUE;

Exit:

	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE F_FSRestore::read(
	FLMUINT			uiLength,
	void *			pvBuffer,
	FLMUINT *		puiBytesRead)
{
	FLMUINT		uiBytesRead = 0;
	RCODE			rc = NE_XFLM_OK;

	flmAssert( m_bSetupCalled);
	flmAssert( m_pFileHdl || m_pFileHdl64);

	if( m_pFileHdl64)
	{
		if( RC_BAD( rc = m_pFileHdl64->Read( m_ui64Offset,
			uiLength, pvBuffer, &uiBytesRead)))
		{
			goto Exit;
		}
	}
	else
	{
		if( RC_BAD( rc = m_pFileHdl->Read( (FLMUINT)m_ui64Offset,
			uiLength, pvBuffer, &uiBytesRead)))
		{
			goto Exit;
		}
	}

Exit:

	m_ui64Offset += uiBytesRead;

	if( puiBytesRead)
	{
		*puiBytesRead = uiBytesRead;
	}

	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE F_FSRestore::close( void)
{
	flmAssert( m_bSetupCalled);

	if( m_pFileHdl64)
	{
		m_pFileHdl64->Release();
		m_pFileHdl64 = NULL;
	}

	if( m_pFileHdl)
	{
		m_pFileHdl->Release();
		m_pFileHdl = NULL;
	}

	m_bOpen = FALSE;
	m_ui64Offset = 0;

	return( NE_XFLM_OK);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE F_FSRestore::abortFile( void)
{
	return( close());
}

