//-------------------------------------------------------------------------
// Desc:	Routines to handle BCD numbers.
// Tabs:	3
//
//		Copyright (c) 1999-2001,2003-2006 Novell, Inc. All Rights Reserved.
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
// $Id: fbcd.cpp 12334 2006-01-23 12:45:35 -0700 (Mon, 23 Jan 2006) dsanders $
//-------------------------------------------------------------------------

#include "flaimsys.h"

// Static Data

FLMBYTE ucMaxBcdINT32[ ]
		= {0x21, 0x47, 0x48, 0x36, 0x47};

FLMBYTE ucMinBcdINT32[ ]
		= {0xB2, 0x14, 0x74, 0x83, 0x64, 0x8F};

FLMBYTE ucMaxBcdUINT32[ ]
		= {0x42, 0x94, 0x96, 0x72, 0x95};


/****************************************************************************
Desc:		Given an unsigned number create the matching FLAIM-specific BCD
			number.
Note:		If terminating byte is half-full, low-nibble value is
			undefined.  Example: -125 creates B1-25-FX
Method:	Using a MOD algorithm, stack BCD values -- popping to
			destination reverses the order for correct final sequence
****************************************************************************/
FLMEXP RCODE FLMAPI FlmUINT2Storage(
	FLMUINT		uiNum,
	FLMUINT *	puiBufLength,	// [IN] size of pBuf, must be atleast F_MAX_NUM_BUF
										// [OUT] actual amount of pBuf used.
	FLMBYTE *	pBuf)
{
	FLMBYTE		ucNibStk[ F_MAX_NUM_BUF + 1];	/* spare byte for odd BCD counts */
	FLMBYTE *	pucNibStk;

	flmAssert( *puiBufLength >= F_MAX_NUM_BUF);

	/* push spare (undefined) nibble for possible half-used terminating byte */

	pucNibStk = &ucNibStk[ 1];

	/* push terminator nibble -- popped last */

	*pucNibStk++ = 0x0F;

	/* push digits */
	/* do 32 bit division until we get down to 16 bits */

	while( uiNum >= 10)
	{
		*pucNibStk++ = (FLMBYTE)(uiNum % 10);	/* push BCD nibbles in reverse order */
		uiNum /= 10;
	}
	*pucNibStk++ = (FLMBYTE)uiNum;				/* push last nibble of number */

	/* count: nibbleCount/2 & truncate */

	*puiBufLength =  ((pucNibStk - ucNibStk) >> 1);		

	/* Pop stack and pack nibbles into byte stream a pair at a time */

	do
	{
		*pBuf++ = (FLMBYTE)((pucNibStk[ -1] << 4) | pucNibStk[ -2]);
	}
	while( (pucNibStk -= 2) > &ucNibStk[ 1]);	/* spare stack byte stops seg wrap */

	return( FERR_OK);
}

/****************************************************************************
Desc: 	Given an signed number create the matching FLAIM-specific BCD
			number.
Note:		If terminating byte is half-full, low-nibble value is
			undefined.  Example: -125 creates B1-25-FX
Method:	Using a MOD algorithm, stack BCD values -- popping to
			destination reverses the order for correct final sequence
WARNING:	-2,147,483,648 may yield different results on different platforms
****************************************************************************/
FLMEXP RCODE FLMAPI FlmINT2Storage(
	FLMINT		iNum,
	FLMUINT *	puiBufLength,	// [IN] size of pBuf, must be atleast F_MAX_NUM_BUF
										// [OUT] actual amount of pBuf used.
	FLMBYTE *	pBuf)
{
	FLMUINT		uiNum;
	FLMBYTE		ucNibStk[ F_MAX_NUM_BUF + 1];	/* spare byte for odd BCD counts */
	FLMBYTE *	pucNibStk;
	FLMINT		iNegFlag;

	flmAssert( *puiBufLength >= F_MAX_NUM_BUF);

	/* push spare (undefined) nibble for possible half-used terminating byte */

	pucNibStk = &ucNibStk[ 1];

	/* push terminator nibble -- popped last */

	*pucNibStk++ = 0x0F;

	/* separate sign from magnituted; (FLMUINT)un = +/- n & flag */

	uiNum = ((iNegFlag = iNum < 0) != 0)
		?	-iNum
		:	iNum;

	/* push digits */
	/* do 32 bit division until we get down to 16 bits */

	while( uiNum >= 10)
	{
		*pucNibStk++ = (FLMBYTE)(uiNum % 10);	/* push BCD nibbles in reverse order */
		uiNum /= 10;
	}
	*pucNibStk++ = (FLMBYTE)uiNum;				/* push last nibble of number */

	if( iNegFlag)
		*pucNibStk++ = 0x0B;							/* push sign nibble last */

	/* Determine number of bytes required for BCD number */
	/* count: nibbleCount/2 & truncate */

	*puiBufLength = ((pucNibStk - ucNibStk) >> 1); 	

	/* Pop stack and pack nibbles into byte stream a pair at a time */

	do
	{
		*pBuf++ = (FLMBYTE)((pucNibStk[ -1] << 4) | pucNibStk[ -2]);
	}
	while( (pucNibStk -= 2) > &ucNibStk[ 1]);	/* spare stack byte stops seg wrap */

	return( FERR_OK);
}

/****************************************************************************
Desc: 	Returns a signed value from a BCD value.
			The data may be a number type, or context type. 
****************************************************************************/
FLMEXP RCODE FLMAPI FlmStorage2INT(
	FLMUINT				uiValueType,
	FLMUINT				uiValueLength,
	const FLMBYTE *	pucValue,		// Internal Storage Format
	FLMINT *				piNum)			// [OUT] return value.
{
	RCODE			rc = FERR_OK;
	BCD_TYPE		bcd;

	if( RC_OK(rc = flmBcd2Num( uiValueType, uiValueLength, pucValue, &bcd)))
	{
		if( bcd.bNegFlag)
		{
			*piNum = -((FLMINT)bcd.uiNum);
			return(
				(bcd.uiNibCnt < 11) ||
				(bcd.uiNibCnt == 11 && 
					(!bcd.pucPtr || (f_memcmp( bcd.pucPtr, ucMinBcdINT32, 6) <= 0)))
				?	FERR_OK
				:	RC_SET( FERR_CONV_NUM_UNDERFLOW)
			);
		}
		else
		{
			*piNum = (FLMINT)bcd.uiNum;
			return(
				(bcd.uiNibCnt < 10) ||
				(bcd.uiNibCnt == 10 && 
					(!bcd.pucPtr || (f_memcmp( bcd.pucPtr, ucMaxBcdINT32, 5) <= 0)))
				?	FERR_OK
				:	RC_SET( FERR_CONV_NUM_OVERFLOW)
			);
		}
	}
	return( rc);
}

/****************************************************************************
Desc: 	Returns a unsigned value from a BCD value.
			The data may be a number type, or context type. 
****************************************************************************/
FLMEXP RCODE FLMAPI FlmStorage2UINT(
	FLMUINT				uiValueType,
	FLMUINT				uiValueLength,
	const FLMBYTE *	pucValue,				// Internal Storage Format
	FLMUINT *			puiNum)			// [OUT] return value.
{
	RCODE			rc = FERR_OK;
	BCD_TYPE		bcd;

	if( RC_OK(rc = flmBcd2Num( uiValueType, uiValueLength, pucValue, &bcd)))
	{
		*puiNum = bcd.uiNum;
		
		if( bcd.bNegFlag)
		{
			rc = RC_SET( FERR_CONV_NUM_UNDERFLOW);
		}
		else if( bcd.uiNibCnt < 10)
		{
			rc = FERR_OK;
		}
		else if( bcd.uiNibCnt == 10) 
		{
			rc = (!bcd.pucPtr || (f_memcmp( bcd.pucPtr, ucMaxBcdUINT32, 5) <= 0))
					?	FERR_OK
					:	RC_SET( FERR_CONV_NUM_OVERFLOW);
		}
		else
		{
			rc = RC_SET( FERR_CONV_NUM_OVERFLOW);
		}
	}
	return( rc);
}


/****************************************************************************
Desc: 	Returns a unsigned value from a BCD value.
			The data may be a number type, or context type. 
****************************************************************************/
FLMEXP RCODE FLMAPI FlmStorage2UINT32(
	FLMUINT				uiValueType,
	FLMUINT				uiValueLength,
	const FLMBYTE *	pucValue,		// Internal Storage Format
	FLMUINT32 *			pui32Num)		// [OUT] return value.
{
	RCODE			rc = FERR_OK;
	BCD_TYPE		bcd;

	if( RC_OK(rc = flmBcd2Num( uiValueType, uiValueLength, pucValue, &bcd)))
	{
		*pui32Num = (FLMUINT32)bcd.uiNum;
		
		if( bcd.bNegFlag)
		{
			rc = RC_SET( FERR_CONV_NUM_UNDERFLOW);
		}
		else if( bcd.uiNibCnt < 10)
		{
			rc = FERR_OK;
		}
		else if( bcd.uiNibCnt == 10) 
		{
			rc = (!bcd.pucPtr || (f_memcmp( bcd.pucPtr, ucMaxBcdUINT32, 5) <= 0))
					?	FERR_OK
					:	RC_SET( FERR_CONV_NUM_OVERFLOW);
		}
		else
		{
			rc = RC_SET( FERR_CONV_NUM_OVERFLOW);
		}
	}
	return( rc);
}


/****************************************************************************
Desc: 	Converts FT_NUMBER and FT_CONTEXT storage buffers to a number
****************************************************************************/
RCODE flmBcd2Num(
	FLMUINT				uiValueType,
	FLMUINT				uiValueLength,
	const FLMBYTE *	pucValue,				// Internal Storage Format
	BCD_TYPE  *			bcd)
{
	if( pucValue == NULL)								
		return( RC_SET( FERR_CONV_NULL_SRC));

	switch( uiValueType)
	{
		case FLM_NUMBER_TYPE :
		{
			FLMUINT 		uiTotalNum = 0;
			FLMUINT		uiByte;
			FLMUINT		uiNibCnt;

			bcd->pucPtr = pucValue;

			/* Get each nibble and use to create the number */

			for( bcd->bNegFlag = (FLMBOOL)(uiNibCnt = ((*pucValue & 0xF0) == 0xB0) ? 1 : 0);
				uiNibCnt <= FLM_MAX_NIB_CNT;
				uiNibCnt++ )
			{

				uiByte = (uiNibCnt & 0x01)		/* See if on ODD/low nibble */
						? (FLMUINT)(0x0F & *pucValue++)	/* Get low nibble, increment pucPtr */
						: (FLMUINT)(*pucValue >> 4);	  	/* Get high nibble */

				if( uiByte == 0x0F)			/* See if at end of BCD number */
					break;						/* break if nibble == 0x0F */
													/* don't count in uiNibCnt */

				/* multiply by 10 and add n */
				/* NOTE: 10y = 8y + 2y = (y << 3) + (y << 1) */
				/* faster than using the long multiply (10 * y) */

				uiTotalNum = (uiTotalNum << 3) + (uiTotalNum << 1) + uiByte;
			}

			bcd->uiNibCnt = uiNibCnt;
			bcd->uiNum = uiTotalNum;
			break;
		}

		case FLM_TEXT_TYPE : 
		{
			FLMUINT		uiNumber = 0;

			/* If it is a TEXT Value, convert to a numeric value */
			/* WARNING: The text is not null terminated. */
			while( uiValueLength--)
			{
				if( *pucValue < ASCII_ZERO || *pucValue > ASCII_NINE)
					break;
				uiNumber = (uiNumber * 10) + (*pucValue - ASCII_ZERO);
				pucValue++;
			}
			bcd->uiNum = uiNumber;
			bcd->uiNibCnt = 0;
			bcd->bNegFlag = FALSE;
			break;
		}

		case FLM_CONTEXT_TYPE :
		{
			if( uiValueLength == sizeof( FLMUINT32))
			{
				bcd->uiNum = (FLMUINT)( FB2UD( pucValue));

				bcd->bNegFlag = 0;
				/* Now set the uiNibCnt, the uiNibCnt will not be totally accurate,
					but it's close enough to get the value out... */
				if( bcd->uiNum < FLM_MAX_UINT8)
					bcd->uiNibCnt = 3;
				else if( bcd->uiNum < FLM_MAX_UINT16)
					bcd->uiNibCnt = 5;
				else
					bcd->uiNibCnt = 9;
			}
			break;
		}

		default :
		{
			flmAssert( 0);
			return( RC_SET( FERR_CONV_ILLEGAL));
		}
	}

	return( FERR_OK);
}