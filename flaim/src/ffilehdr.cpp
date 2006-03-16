//-------------------------------------------------------------------------
// Desc:	Database header routines.
// Tabs:	3
//
//		Copyright (c) 1995-2006 Novell, Inc. All Rights Reserved.
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
// $Id: ffilehdr.cpp 12256 2006-01-19 14:37:14 -0700 (Thu, 19 Jan 2006) dsanders $
//-------------------------------------------------------------------------

#include "flaimsys.h"

/********************************************************************
Desc: 	Initializes the file prefix for the .db database file.
*********************************************************************/
void flmSetFilePrefix(
	FLMBYTE *		pPrefix,
	FLMUINT			uiMajorVer,
	FLMUINT			uiMinorVer)
{
	f_memset( pPrefix, 0, 16);
	pPrefix [0] = 0xFF;
	pPrefix [1] = f_toascii('W');
	pPrefix [2] = f_toascii('P');
	pPrefix [3] = f_toascii('C');
		
	UD2FBA( (FLMUINT32)16, &pPrefix [4]);

	// Fill these out with the old NDS values so FUSION code will work.

	pPrefix [8] = 0xF3;		// old wp product type
	pPrefix [9] = 0x01;		// old wp file type
	pPrefix [10] = (FLMBYTE)uiMajorVer;
	pPrefix [11] = (FLMBYTE)uiMinorVer;

	// Bytes 12 and 13 are the encryption key

	pPrefix [12] = pPrefix [13] = 0;

	// Bytes 14 and 15 point are the offset to file specific packets

	pPrefix [14] = pPrefix [15] = 0;
}

/********************************************************************
Desc:	This routine adjusts the block size that is passe in (wBlkSize)
		to the nearest valid block size.
*********************************************************************/
FLMUINT flmAdjustBlkSize(
	FLMUINT	uiBlkSize)
{
	FLMUINT	uiTmpBlkSize;
	
	uiTmpBlkSize = MIN_BLOCK_SIZE;
	while( (uiBlkSize > uiTmpBlkSize) && (uiTmpBlkSize < MAX_BLOCK_SIZE))
	{
		uiTmpBlkSize <<= 1;
	}

	return( uiTmpBlkSize);
}

/***************************************************************************
Desc:	This routine extracts and verifies the information within
		the file header.
*****************************************************************************/
RCODE flmGetFileHdrInfo(
	FLMBYTE *		pPrefixBuf,			/* Buffer containing file prefix
													information. */
	FLMBYTE *		pFileHdrBuf,		/* Buffer containing file header
													information. */
	FILE_HDR_p		pFileHdrRV)			/* Returns file header information. */
{
	RCODE				rc = FERR_OK;
	FLMUINT			uiVersionNum;
	FLMUINT			uiTmpBlkSize;

	/* Get the create options. */

	pFileHdrRV->uiBlockSize = (FLMUINT)FB2UW( &pFileHdrBuf [DB_BLOCK_SIZE]);
	pFileHdrRV->uiAppMajorVer = pPrefixBuf [10];
	pFileHdrRV->uiAppMinorVer = pPrefixBuf [11];
	pFileHdrRV->uiDefaultLanguage = pFileHdrBuf [DB_DEFAULT_LANGUAGE];
	pFileHdrRV->uiVersionNum = uiVersionNum =
								((FLMUINT16)(pFileHdrBuf [FLM_VER_POS] - ASCII_ZERO) * 100 +
							 	 (FLMUINT16)(pFileHdrBuf [FLM_MINOR_VER_POS] - ASCII_ZERO) * 10 +
							 	 (FLMUINT16)(pFileHdrBuf [FLM_SMINOR_VER_POS] - ASCII_ZERO));

	uiTmpBlkSize = pFileHdrRV->uiBlockSize;
	if( !VALID_BLOCK_SIZE( uiTmpBlkSize))
	{
		uiTmpBlkSize = flmAdjustBlkSize( pFileHdrRV->uiBlockSize);
	}

	// Get other log header elements.

	pFileHdrRV->uiFirstLFHBlkAddr =
		(FLMUINT)FB2UD( &pFileHdrBuf [DB_1ST_LFH_ADDR]);

	// See if it is: 1) a WordPerfect file, 2) a FLAIM file,
	// and 3) if the block size is valid.

	if( (pPrefixBuf [1] != f_toascii('W')) ||
		 (pPrefixBuf [2] != f_toascii('P')) ||
		 (pPrefixBuf [3] != f_toascii('C')) || 
		 (!VALID_BLOCK_SIZE( pFileHdrRV->uiBlockSize)))
	{
		rc = RC_SET( FERR_NOT_FLAIM);
		goto Exit;
	}

	if( pFileHdrBuf [FLAIM_NAME_POS     ] != f_toascii( FLAIM_NAME[0]) || 
		 pFileHdrBuf [FLAIM_NAME_POS + 1 ] != f_toascii( FLAIM_NAME[1]) || 
		 pFileHdrBuf [FLAIM_NAME_POS + 2 ] != f_toascii( FLAIM_NAME[2]) || 
		 pFileHdrBuf [FLAIM_NAME_POS + 3 ] != f_toascii( FLAIM_NAME[3]) || 
		 pFileHdrBuf [FLAIM_NAME_POS + 4 ] != f_toascii( FLAIM_NAME[4]))
	{
		rc = RC_SET( FERR_NOT_FLAIM);
		goto Exit;
	}

	pFileHdrRV->uiSigBitsInBlkSize = flmGetSigBits( pFileHdrRV->uiBlockSize);

	// Check the FLAIM version number

	if( RC_BAD( rc = flmCheckVersionNum( uiVersionNum)))
	{
		goto Exit;
	}

	f_memcpy( pFileHdrRV->ucFileHdr, pFileHdrBuf, FLM_FILE_HEADER_SIZE);
	
Exit:

	return( rc);
}

/********************************************************************
Desc: This routine initializes a FILE_HDR structure from the
		create options that are passed in.  It also initializes the
		file header buffer (pFileHdrBuf) that will be written to disk.
*********************************************************************/
void flmInitFileHdrInfo(
	CREATE_OPTS *	pCreateOpts,
	FILE_HDR_p		pFileHdr,
	FLMBYTE *		pFileHdrBuf
	)
{
	f_memset( pFileHdrBuf, 0, FLM_FILE_HEADER_SIZE);

	// If pCreateOpts is non-NULL, copy it into the file header.

	if (pCreateOpts)
	{
		pFileHdr->uiBlockSize = pCreateOpts->uiBlockSize;
		pFileHdr->uiDefaultLanguage = pCreateOpts->uiDefaultLanguage;
		pFileHdr->uiAppMajorVer = pCreateOpts->uiAppMajorVer;
		pFileHdr->uiAppMinorVer = pCreateOpts->uiAppMinorVer;
	}
	else
	{

		// If pCreateOpts is NULL, initialize some default values.

		pFileHdr->uiBlockSize = DEFAULT_BLKSIZ;
		pFileHdr->uiDefaultLanguage = DEFAULT_LANG;
		pFileHdr->uiAppMajorVer =
		pFileHdr->uiAppMinorVer = 0;
	}

	// Only allow database to be created with current version number

	pFileHdr->uiVersionNum = FLM_CURRENT_VERSION_NUM;
	f_memcpy( &pFileHdrBuf [FLM_VER_POS], (FLMBYTE *)FLM_CURRENT_VER_STR,
					FLM_VER_LEN);

	// Round block size up to nearest legal block size.

	pFileHdr->uiBlockSize =
		flmAdjustBlkSize( pFileHdr->uiBlockSize);

	pFileHdr->uiSigBitsInBlkSize = flmGetSigBits( pFileHdr->uiBlockSize);
	f_memcpy( &pFileHdrBuf [FLAIM_NAME_POS], (FLMBYTE *)FLAIM_NAME,
				 FLAIM_NAME_LEN);

	pFileHdrBuf [DB_DEFAULT_LANGUAGE] =
		(FLMBYTE)pFileHdr->uiDefaultLanguage;
	UW2FBA( (FLMUINT16)pFileHdr->uiBlockSize,
		&pFileHdrBuf [DB_BLOCK_SIZE]);
	pFileHdr->uiFirstLFHBlkAddr = FSBlkAddress(1, 0);
	UD2FBA( pFileHdr->uiFirstLFHBlkAddr, &pFileHdrBuf [DB_1ST_LFH_ADDR]);

	if (pFileHdr->uiVersionNum < FLM_VER_4_3)
	{

		// Things to maintain for backward compatibility - pre 4.3.

		FLMUINT	uiFirstPcodeAddr = pFileHdr->uiFirstLFHBlkAddr +
											 pFileHdr->uiBlockSize;

		UD2FBA( pFileHdr->uiBlockSize, &pFileHdrBuf [DB_INIT_LOG_SEG_ADDR]);
		UD2FBA( DB_LOG_HEADER_START, &pFileHdrBuf [DB_LOG_HEADER_ADDR]);
		UD2FBA( uiFirstPcodeAddr, &pFileHdrBuf [DB_1ST_PCODE_ADDR]);
	}
	
	f_memcpy( pFileHdr->ucFileHdr, pFileHdrBuf, FLM_FILE_HEADER_SIZE);
}

/***************************************************************************
Desc:	This routine reads and verifies the information contained in the
		file header and log header of a FLAIM database.  This routine
		is called by both FlmDbOpen and flmGetHdrInfo.
*****************************************************************************/
RCODE flmReadAndVerifyHdrInfo(
	DB_STATS *		pDbStats,
	F_FileHdl *		pFileHdl,
	FLMBYTE *		pReadBuf,
	FILE_HDR_p		pFileHdrRV,
	LOG_HDR_p		pLogHdrRV,
	FLMBYTE *		pLogHdr)
{
	RCODE				rc = FERR_OK;
	RCODE				rc0;
	RCODE				rc1;
	FLMBYTE *		pBuf;
	FLMBYTE *		pucLogHdr;
	FLMUINT			uiBytesRead;
	FLMUINT			uiVersionNum;

	// Read the fixed information area

	f_memset( pReadBuf, 0, 2048);

	rc0 = pFileHdl->Read( 1L, 2047, &pReadBuf [1], &uiBytesRead);
	
	// Increment bytes read - to account for byte zero, which
	// was not really read in.

	uiBytesRead++;
	pBuf = pReadBuf;
	*pBuf = 0xFF;

	// Before doing any checking, get whatever we can from the
	// first 2048 bytes.  For the flmGetHdrInfo routine, we want
	// to get whatever we can from the headers, even if it is
	// invalid.

	rc1 = flmGetFileHdrInfo( pBuf, &pBuf[ FLAIM_HEADER_START], pFileHdrRV);

	// Get the log header information

	pucLogHdr = &pBuf[ DB_LOG_HEADER_START];

	if( pLogHdr)
	{
		f_memcpy( pLogHdr, pucLogHdr, LOG_HEADER_SIZE);
	}
	
	flmGetLogHdrInfo( pucLogHdr, pLogHdrRV);

	// Take the version from the log header if non-zero.
	// Storing the version in the log header is new to 40 code base.

	uiVersionNum = FB2UW( &pucLogHdr[ LOG_FLAIM_VERSION]);
	if( uiVersionNum)
	{
		pFileHdrRV->uiVersionNum = uiVersionNum;
	}

	// If there is not enough data to satisfy the read, this
	// is probably not a FLAIM file.

	if( RC_BAD( rc0))
	{
		if( rc0 != FERR_IO_END_OF_FILE)
		{
			if( pDbStats)
			{
				pDbStats->uiReadErrors++;
			}

			rc = rc0;
			goto Exit;
		}

		if( uiBytesRead < 2048)
		{
			rc = RC_SET( FERR_NOT_FLAIM);
			goto Exit;
		}
	}

	// See if we got any other errors where we might want to retry
	// the read.

	if( RC_BAD( rc1))
	{
		rc = rc1;
		goto Exit;
	}

	// Verify the checksums in the log header

	if( lgHdrCheckSum( pucLogHdr, TRUE) != 0)
	{
		rc = RC_SET( FERR_BLOCK_CHECKSUM);
		goto Exit;
	}

Exit:

	return( rc);
}

/***************************************************************************
Desc:	Write the version number to disk and flush the write to disk.
*****************************************************************************/
RCODE flmWriteVersionNum(
	F_SuperFileHdl_p		pSFileHdl,
	FLMUINT					uiVersionNum)
{
	RCODE		rc = FERR_OK;
	FLMUINT	uiWriteBytes;
	FLMBYTE	szVersionStr[ 8];

	if( RC_BAD( rc = flmCheckVersionNum( uiVersionNum)))
	{
		flmAssert( 0);
		goto Exit;
	}

	szVersionStr[ 0] = (FLMBYTE)(uiVersionNum / 100) + '0';
	szVersionStr[ 1] = '.';
	szVersionStr[ 2] = (FLMBYTE)((uiVersionNum % 100) / 10) + '0';
	szVersionStr[ 3] = (FLMBYTE)(uiVersionNum % 10) + '0';
	szVersionStr[ 4] = 0;

	if (RC_OK( rc = pSFileHdl->WriteHeader(
					FLAIM_HEADER_START + FLM_VER_POS, FLM_VER_LEN,
					szVersionStr, &uiWriteBytes)))
	{
		if (RC_BAD( rc = pSFileHdl->Flush()))
		{
			goto Exit;
		}
	}

Exit:

	return( rc);
}