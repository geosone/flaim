//-------------------------------------------------------------------------
// Desc:	Calculate block checksum.
// Tabs:	3
//
//		Copyright (c) 1991-2001,2003-2006 Novell, Inc. All Rights Reserved.
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
// $Id: flblksum.cpp 12260 2006-01-19 14:40:25 -0700 (Thu, 19 Jan 2006) dsanders $
//-------------------------------------------------------------------------

#include "flaimsys.h"

/********************************************************************
Desc: Compares or sets the checksum value in a block.  
		Operates to compare the block checksum with the actual checksum.
Ret:	if (Compare) returns FERR_BLOCK_CHECKSUM block checksum does
		not agree with checksum header values.  
*********************************************************************/
RCODE BlkCheckSum(
	FLMBYTE *	pucBlkPtr,		// Points to block
	FLMINT		iCompare,		// TRUE compare checksums, FALSE set chksum
										// Use CHECKSUM_CHECK or CHECKSUM_SET
	FLMUINT		uiBlkAddress,	// Expected block address (3.x version)
	FLMUINT		uiBlkSize		// Used to verify that we don't read outside
										// of an allocation.
	)
{
	RCODE				rc = FERR_OK;
#if !((defined( FLM_WIN) && !defined( FLM_64BIT)) || defined( FLM_NLM))
	FLMBYTE			ucTmp;
	FLMBYTE *		pucCur;
	FLMBYTE *		pucEnd;
#endif
	FLMUINT			uiAdds;
	FLMUINT			uiXORs;
	FLMUINT			uiCurrCheckSum = 0;
	FLMUINT			uiNewCheckSum;
	FLMUINT			uiEncryptSize;
	FLMBYTE *		pucSaveBlkPtr = pucBlkPtr;

	// Check the block length against the maximum block size

	uiEncryptSize = (FLMUINT)getEncryptSize( pucBlkPtr);
	if( uiEncryptSize > uiBlkSize || uiEncryptSize < BH_OVHD)
	{
		rc = RC_SET( FERR_BLOCK_CHECKSUM);
		goto Exit;
	}	

	// If we are comparing, but there is no current checksum just return.
	// The next time the checksum is modified, the comparison will be performed. 
	// Version 3.x will store the full block address or if
	// a checksum is used, the lost low byte of block address is checksummed.

	if( iCompare == CHECKSUM_CHECK)
	{
		uiCurrCheckSum = (FLMUINT)(((FLMUINT)pucBlkPtr[ BH_CHECKSUM_HIGH] << 8) + 
											 (FLMUINT)pucBlkPtr[ BH_CHECKSUM_LOW]);
	}

	// We need to checksum the data that is encrypted. 
	// This is done by the getEncryptSize() call.

	// Check all of block, except for embedded checksum bytes.
	// For speed, the initial values of uiAdds and uiXORs effectively ignore/skip
	// the checksum values already embedded in the source: (a - a) == 0 and 
	// (a ^ a) == 0 so the initial values, net of the 2nd operations, equal zero
	// too.  

	uiAdds = 0 - (pucBlkPtr[ BH_CHECKSUM_LOW] + pucBlkPtr[ BH_CHECKSUM_HIGH]);
	uiXORs = pucBlkPtr[ BH_CHECKSUM_LOW] ^ pucBlkPtr[ BH_CHECKSUM_HIGH];

	// The 3.x version checksums the low byte of the address.

	if( uiBlkAddress != BT_END)
	{
		uiAdds += (FLMBYTE)uiBlkAddress;
		uiXORs ^= (FLMBYTE)uiBlkAddress;
	}

#if defined( FLM_NLM) || (defined( FLM_WIN) && !defined( FLM_64BIT))

	FastBlockCheckSum( pucBlkPtr, &uiAdds, &uiXORs,
		(unsigned long)uiEncryptSize);

#else

#ifdef FLM_64BIT
	pucCur = pucBlkPtr;
	pucEnd = pucBlkPtr + (uiEncryptSize & 0xFFFFFFFFFFFFFFF8);
#else
	pucCur = pucBlkPtr;
	pucEnd = pucBlkPtr + (uiEncryptSize & 0xFFFFFFFC);
#endif

	while( pucCur < pucEnd)
	{
		FLMUINT	uiValue = *(FLMUINT *)pucCur;

		uiXORs ^= uiValue;

		uiAdds += (FLMBYTE)uiValue;

		uiValue >>= 8;
		uiAdds += (FLMBYTE)uiValue;

		uiValue >>= 8;
		uiAdds += (FLMBYTE)uiValue;

#ifdef FLM_64BIT
		uiValue >>= 8;
		uiAdds += (FLMBYTE)uiValue;

		uiValue >>= 8;
		uiAdds += (FLMBYTE)uiValue;

		uiValue >>= 8;
		uiAdds += (FLMBYTE)uiValue;

		uiValue >>= 8;
		uiAdds += (FLMBYTE)uiValue;
#endif

		uiAdds += (FLMBYTE)(uiValue >> 8);
		pucCur += sizeof( FLMUINT);
	}

	ucTmp = (FLMBYTE)uiXORs;
	ucTmp ^= (FLMBYTE)(uiXORs >> 8);
	ucTmp ^= (FLMBYTE)(uiXORs >> 16);
	ucTmp ^= (FLMBYTE)(uiXORs >> 24);
#ifdef FLM_64BIT
	ucTmp ^= (FLMBYTE)(uiXORs >> 32);
	ucTmp ^= (FLMBYTE)(uiXORs >> 40);
	ucTmp ^= (FLMBYTE)(uiXORs >> 48);
	ucTmp ^= (FLMBYTE)(uiXORs >> 56);
#endif
	uiXORs = ucTmp;

	pucCur = pucEnd;
	pucEnd = pucBlkPtr + uiEncryptSize;

	while( pucCur < pucEnd)	
	{
		uiAdds += *pucCur;
		uiXORs ^= *pucCur++;
	}
#endif

	uiNewCheckSum = (((uiAdds << 8) + uiXORs) & 0xFFFF);
	
	// Set the checksum
	
	if (iCompare == CHECKSUM_SET)
	{
		pucSaveBlkPtr[ BH_CHECKSUM_HIGH] = (FLMBYTE)(uiNewCheckSum >> 8);
		pucSaveBlkPtr[ BH_CHECKSUM_LOW] = (FLMBYTE)uiNewCheckSum;
		goto Exit;
	}


	// The checksum is different from the stored checksum.
	// For version 3.x database we don't store the low byte of the
	// address.  Thus, it will have to be computed from the checksum.

	if( uiBlkAddress == BT_END)
	{
		FLMBYTE		byXor;
		FLMBYTE		byAdd;
		FLMBYTE		byDelta;
		
		// If there is a one byte value that will satisfy both
		// sides of the checksum, the checksum is OK and that value
		// is the first byte value.
		
		byXor = (FLMBYTE) uiNewCheckSum;
		byAdd = (FLMBYTE) (uiNewCheckSum >> 8);
		byDelta = byXor ^ pucSaveBlkPtr [BH_CHECKSUM_LOW];
		
		// Here is the big check, if byDelta is also what is
		// off with the add portion of the checksum, we have
		// a good value.
		
		if( ((FLMBYTE) (byAdd + byDelta)) == pucSaveBlkPtr[ BH_CHECKSUM_HIGH] )
		{
			// Set the low checksum value with the computed value.
			
			pucSaveBlkPtr[ BH_CHECKSUM_LOW] = byDelta;
			goto Exit;
		}
	}
	else
	{
		// This has the side effect of setting the low block address byte
		// in the block thus getting rid of the low checksum byte.

		// NOTE: We are allowing the case where the calculated checksum is
		// zero and the stored checksum is one because we used to change
		// a calculated zero to a one in old databases and store the one.
		// This is probably a somewhat rare case (1 out of 65536 checksums
		// will be zero), so forgiving it will be OK most of the time.
		// So that those don't cause us to report block checksum errors,
		// we just allow it - checksumming isn't a perfect check anyway.
		// VISIT: We do eventually want to get rid of this forgiving code.
		
		if (uiNewCheckSum == uiCurrCheckSum ||
			 ((!uiNewCheckSum) && (uiCurrCheckSum == 1)))
		{
			pucSaveBlkPtr [BH_CHECKSUM_LOW] = (FLMBYTE) uiBlkAddress;
			goto Exit;
		}
	}
	
	// Otherwise, we have a checksum error.
	
	rc = RC_SET( FERR_BLOCK_CHECKSUM);

Exit:

	return( rc);
}

/********************************************************************
Desc: ?
*********************************************************************/
FLMUINT lgHdrCheckSum(
	FLMBYTE *	pucLogHdr,
	FLMBOOL		bCompare
	)
{
	FLMUINT	uiCnt;
	FLMUINT	uiTempSum;
	FLMUINT	uiCurrCheckSum;
	FLMUINT	uiTempSum2;
	FLMUINT	uiBytesToChecksum;

	uiBytesToChecksum = (FB2UW( &pucLogHdr [LOG_FLAIM_VERSION]) < FLM_VER_4_3)
								? LOG_HEADER_SIZE_VER40
								: LOG_HEADER_SIZE;

/*
If we are comparing, but there is no current checksum, return
zero to indicate success.  The next time the checksum is
modified, the comparison will be performed.
*/

/*
Unconverted databases may have a 0xFFFF or a zero in the checksum
If 0xFFFF, change to a zero so we only have to deal with one value.
*/

	if( (uiCurrCheckSum = (FLMUINT)FB2UW( &pucLogHdr[ LOG_HDR_CHECKSUM])) == 0xFFFF)
		uiCurrCheckSum = 0;

	if( (bCompare) && (!uiCurrCheckSum))
		return( 0);

	for(
		/*
		Check all of log header except for the bytes which contain the
		checksum.

		For speed, uiTempSum is initialized to effectively ignore or skip
		the checksum embedded in the source:  (a - a) == 0 so we store a negative
		that the later addition clears out.  Also, the loop counter, i,
		is 1 larger than the number of FLMUINT16's so that we can
		pre-decrement by "for(;--i != 0;)" -- basically "loop-non-zero".
		*/

		uiTempSum = 0 - (FLMUINT)FB2UW( &pucLogHdr[ LOG_HDR_CHECKSUM]),
		uiCnt = 1 + uiBytesToChecksum / sizeof( FLMUINT16)
	;	--uiCnt != 0; )
	{
		uiTempSum += (FLMUINT)FB2UW( pucLogHdr);
		pucLogHdr += sizeof( FLMUINT16);
	}

/* Don't want a zero or 0xFFFF checksum - change to 1 */

	if( (0 == (uiTempSum2 = (uiTempSum & 0xFFFF))) || (uiTempSum2 == 0xFFFF))
		uiTempSum2 = 1;

	return( (FLMUINT)(((bCompare) && (uiTempSum2 == uiCurrCheckSum))
							? (FLMUINT)0
							: uiTempSum2) );
}
