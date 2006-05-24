//------------------------------------------------------------------------------
// Desc:	This file contains misc toolkit functions
//
// Tabs:	3
//
//		Copyright (c) 2000-2006 Novell, Inc. All Rights Reserved.
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
// $Id: ftkmisc.cpp 3115 2006-01-19 13:24:39 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------

#include "ftksys.h"

static FLMUINT						gv_uiStartupCount = 0;
static FLMUINT						gv_uiRandomGenInitCount = 0;
static F_MUTEX						gv_hRandomGenMutex = F_MUTEX_NULL;
static IF_RandomGenerator *	gv_pRandomGenerator = NULL;
static IF_ThreadMgr *			gv_pThreadMgr = NULL;
static IF_FileSystem *			gv_pFileSystem = NULL;
static FLMUINT						gv_uiMaxFileSize = FLM_MAXIMUM_FILE_SIZE;
static F_XML *						gv_pXml = NULL;

FSTATIC RCODE f_initRandomGenerator( void);

FSTATIC void f_freeRandomGenerator( void);

#ifdef FLM_AIX
	#ifndef nsleep
		extern "C"
		{
			extern int nsleep( struct timestruc_t *, struct timestruc_t *);
		}
	#endif
#endif

/****************************************************************************
Desc:
****************************************************************************/
static FLMBYTE gv_ucSENLengthArray[] =
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0   - 15
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 16  - 31
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 32  - 47
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 48  - 63
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64  - 79
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 80  - 95
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96  - 111
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 112 - 127
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 128 - 143
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 144 - 159
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 160 - 175
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 176 - 191
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 192 - 207
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 208 - 223
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,		// 224 - 239
	5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 8, 9		// 240 - 255
};

/****************************************************************************
Desc:
****************************************************************************/
static FLMBYTE ucSENPrefixArray[] =
{
	0,
	0,
	0x80,
	0xC0,
	0xE0,
	0xF0,
	0xF8,
	0xFC,
	0xFE,
	0xFF
};

/****************************************************************************
Desc:
****************************************************************************/
RCODE FLMAPI ftkStartup( void)
{
	RCODE		rc = NE_FLM_OK;
	
	if( ++gv_uiStartupCount > 1)
	{
		goto Exit;
	}
	
	f_memoryInit();


	f_assert( sizeof( f_va_list) == sizeof( va_list));
	
	if( RC_BAD( rc = f_initCharMappingTables()))
	{
		goto Exit;
	}
	
	if( RC_BAD( rc = f_verifyDiskStructOffsets()))
	{
		goto Exit;
	}
	
	if( RC_BAD( rc = f_allocFileSystem( &gv_pFileSystem)))
	{
		goto Exit;
	}
	
	if( RC_BAD( rc = f_allocThreadMgr( &gv_pThreadMgr)))
	{
		goto Exit;
	}
	
	if( RC_BAD( rc = f_initRandomGenerator()))
	{
		goto Exit;
	}
	
	if( RC_BAD( rc = f_initCRCTable()))
	{
		goto Exit;
	}
	
	f_initFastCheckSum();
	
	if( (gv_pXml = f_new F_XML) == NULL)
	{
		rc = RC_SET( NE_FLM_MEM);
		goto Exit;
	}
		
	if( RC_BAD( rc = gv_pXml->setup()))
	{
		goto Exit;
	}

#ifdef FLM_DEBUG
	if( RC_BAD( rc = f_verifyMetaphoneRoutines()))
	{
		goto Exit;
	}
#endif
	
#if defined( FLM_LINUX)
	f_setupLinuxKernelVersion();
	gv_uiMaxFileSize = f_getLinuxMaxFileSize();
#elif defined( FLM_AIX)

	// Call setrlimit to increase the max allowed file size.
	// We don't have a good way to deal with any errors returned by
	// setrlimit(), so we just hope that there aren't any ...
	
	struct rlimit rlim;
	
	rlim.rlim_cur = RLIM_INFINITY;
	rlim.rlim_max = RLIM_INFINITY;
	
	setrlimit( RLIMIT_FSIZE, &rlim);
#endif
	

Exit:

	if( RC_BAD( rc))
	{
		ftkShutdown();
	}
	
	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
void FLMAPI ftkShutdown( void)
{
	if( !gv_uiStartupCount || --gv_uiStartupCount > 0)
	{
		return;
	}
	
	if( gv_pThreadMgr)
	{
		gv_pThreadMgr->Release();
		gv_pThreadMgr = NULL;
	}
	
	if( gv_pFileSystem)
	{
		gv_pFileSystem->Release();
		gv_pFileSystem = NULL;
	}
	
	f_freeCRCTable();
	
	if( gv_pXml)
	{
		gv_pXml->Release();
	}
	
	f_freeRandomGenerator();
	f_freeCharMappingTables();
	f_memoryCleanup();
}

/****************************************************************************
Desc: This routine causes the calling process to delay the given number
		of milliseconds.  Due to the nature of the call, the actual sleep
		time is almost guaranteed to be different from requested sleep time.
****************************************************************************/
#ifdef FLM_UNIX
void FLMAPI f_sleep(
	FLMUINT		uiMilliseconds)
{
#ifdef FLM_AIX
	struct timestruc_t timeout;
	struct timestruc_t remain;
#else
	struct timespec timeout;
#endif

	timeout.tv_sec = (uiMilliseconds / 1000);
	timeout.tv_nsec = (uiMilliseconds % 1000) * 1000000;

#ifdef FLM_AIX
	nsleep(&timeout, &remain);
#else
	nanosleep(&timeout, 0);
#endif
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#ifdef FLM_WIN
void FLMAPI f_sleep(
	FLMUINT		uiMilliseconds)
{
	Sleep( (DWORD)uiMilliseconds);
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#ifdef FLM_LIBC_NLM
void FLMAPI f_sleep( 
	FLMUINT		uiMilliseconds)
{
	if( !uiMilliseconds )
	{
		pthread_yield();
	}
	else
	{
		delay( uiMilliseconds);
	}
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#ifdef FLM_RING_ZERO_NLM
void FLMAPI f_sleep( 
	FLMUINT	uiMilliseconds)
{
	if( !uiMilliseconds)
	{
		kYieldThread();
	}
	else
	{
		kDelayThread( uiMilliseconds);
	}
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
FSTATIC RCODE f_initRandomGenerator( void)
{
	FLMUINT					uiTime;
	RCODE						rc = NE_FLM_OK;

	if (++gv_uiRandomGenInitCount > 1)
	{
		goto Exit;
	}

	if( RC_BAD( rc = f_mutexCreate( &gv_hRandomGenMutex)))
	{
		goto Exit;
	}

	f_timeGetSeconds( &uiTime );

	if( RC_BAD( rc = FlmAllocRandomGenerator( &gv_pRandomGenerator)))
	{
		goto Exit;
	}

	gv_pRandomGenerator->setSeed( (FLMUINT32)(uiTime ^ (FLMUINT)f_getpid()));

Exit:

	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
FSTATIC void f_freeRandomGenerator( void)
{
	if( (--gv_uiRandomGenInitCount) > 0)
	{
		return;
	}
	
	if( gv_pRandomGenerator)
	{
		gv_pRandomGenerator->Release();
		gv_pRandomGenerator = NULL;
	}

	if( gv_hRandomGenMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_hRandomGenMutex);
	}
}

/****************************************************************************
Desc:		This routine will use the operating system calls to generate a
			"globally unique" identifier.  Typically, this is based on the
			MAC address of an ethernet card installed in the machine.  If the
			machine does not have an ethernet card, or if the OS does not
			support generating GUIDs, this routine will generate a pseudo-GUID
			using a random number generator.  A serial number is 16-bytes.
****************************************************************************/
RCODE FLMAPI f_createSerialNumber(
	FLMBYTE *		pszSerialNum)
{
	RCODE						rc = NE_FLM_OK;

#if defined( FLM_WIN)

	UUID			uuidVal;
	RPC_STATUS	err = UuidCreate( &uuidVal);

	if (err == RPC_S_OK || err == RPC_S_UUID_LOCAL_ONLY)
	{
		UD2FBA( (FLMUINT32)uuidVal.Data1, &pszSerialNum[ 0]);
		UW2FBA( (FLMUINT16)uuidVal.Data2, &pszSerialNum[ 4]);
		UW2FBA( (FLMUINT16)uuidVal.Data3, &pszSerialNum[ 6]);
		f_memcpy( &pszSerialNum[ 8], (FLMBYTE *)uuidVal.Data4, 8);
		goto Exit;
	}

#elif defined( FLM_UNIX) || defined( FLM_NLM)

	// Generate a pseudo GUID value

	UD2FBA( f_getRandomUINT32(), &pszSerialNum[ 0]);
	UD2FBA( f_getRandomUINT32(), &pszSerialNum[ 4]);
	UD2FBA( f_getRandomUINT32(), &pszSerialNum[ 8]);
	UD2FBA( f_getRandomUINT32(), &pszSerialNum[ 12]);

#endif

#if defined( FLM_WIN)
Exit:
#endif

	return( rc);
}

/****************************************************************************
Desc: 	
****************************************************************************/
void FLMAPI f_getenv(
	const char *	pszKey,
	FLMBYTE *		pszBuffer,
	FLMUINT			uiBufferSize,
	FLMUINT *		puiValueLen)
{
	FLMUINT			uiValueLen = 0;

	if( !uiBufferSize)
	{
		goto Exit;
	}
	
	pszBuffer[ 0] = 0;
	
#if defined( FLM_WIN) || defined( FLM_UNIX)
	char *	pszValue;
	
   if( (pszValue = getenv( pszKey)) != NULL &&
		 (uiValueLen = f_strlen( pszValue)) < uiBufferSize)
	{
		f_strcpy( (char *)pszBuffer, pszValue);
	}
#else
	F_UNREFERENCED_PARM( pszKey);
#endif

Exit:

	if( puiValueLen)
	{
		*puiValueLen = uiValueLen;
	}

	return;
}

/***************************************************************************
Desc:		Sort an array of items
****************************************************************************/
void FLMAPI f_qsort(
	void *					pvBuffer,
	FLMUINT					uiLowerBounds,
	FLMUINT					uiUpperBounds,
	F_SORT_COMPARE_FUNC	fnCompare,
	F_SORT_SWAP_FUNC		fnSwap)
{
	FLMUINT			uiLBPos;
	FLMUINT			uiUBPos;
	FLMUINT			uiMIDPos;
	FLMUINT			uiCurrentPos;
	FLMUINT			uiLeftItems;
	FLMUINT			uiRightItems;
	FLMINT			iCompare;

Iterate_Larger_Half:

	uiUBPos = uiUpperBounds;
	uiLBPos = uiLowerBounds;
	uiMIDPos = (uiUpperBounds + uiLowerBounds + 1) / 2;
	uiCurrentPos = uiMIDPos;

	for (;;)
	{
		while (uiLBPos == uiMIDPos ||
					((iCompare = 
						fnCompare( pvBuffer, uiLBPos, uiCurrentPos)) < 0))
		{
			if( uiLBPos >= uiUpperBounds)
			{
				break;
			}
			uiLBPos++;
		}

		while( uiUBPos == uiMIDPos ||
					(((iCompare = 
						fnCompare( pvBuffer, uiCurrentPos, uiUBPos)) < 0)))
		{
			if (!uiUBPos)
			{
				break;
			}
			uiUBPos--;
		}
		
		if( uiLBPos < uiUBPos)
		{
			// Exchange [uiLBPos] with [uiUBPos].

			fnSwap( pvBuffer, uiLBPos, uiUBPos);
			uiLBPos++;
			uiUBPos--;
		}
		else
		{
			break;
		}
	}

	// Check for swap( LB, MID ) - cases 3 and 4

	if( uiLBPos < uiMIDPos )
	{

		// Exchange [uiLBPos] with [uiMIDPos]

		fnSwap( pvBuffer, uiMIDPos, uiLBPos);
		uiMIDPos = uiLBPos;
	}
	else if( uiMIDPos < uiUBPos )
	{
		// Exchange [uUBPos] with [uiMIDPos]

		fnSwap( pvBuffer, uiMIDPos, uiUBPos);
		uiMIDPos = uiUBPos;
	}

	// Check the left piece.

	uiLeftItems = (uiLowerBounds + 1 < uiMIDPos)
							? uiMIDPos - uiLowerBounds
							: 0;

	uiRightItems = (uiMIDPos + 1 < uiUpperBounds)
							? uiUpperBounds - uiMIDPos
							: 0;

	if( uiLeftItems < uiRightItems)
	{
		// Recurse on the LEFT side and goto the top on the RIGHT side.

		if( uiLeftItems)
		{
			f_qsort( pvBuffer, uiLowerBounds, uiMIDPos - 1, fnCompare, fnSwap);
		}

		uiLowerBounds = uiMIDPos + 1;
		goto Iterate_Larger_Half;
	}
	else if( uiLeftItems)
	{
		// Recurse on the RIGHT side and goto the top for the LEFT side.

		if (uiRightItems )
		{
			f_qsort( pvBuffer, uiMIDPos + 1, uiUpperBounds, fnCompare, fnSwap);
		}

		uiUpperBounds = uiMIDPos - 1;
		goto Iterate_Larger_Half;
	}
}

/***************************************************************************
Desc:
****************************************************************************/
FLMINT FLMAPI f_qsortUINTCompare(
	void *		pvBuffer,
	FLMUINT		uiPos1,
	FLMUINT		uiPos2)
{
	FLMUINT		uiLeft = *(((FLMUINT *)pvBuffer) + uiPos1);
	FLMUINT		uiRight = *(((FLMUINT *)pvBuffer) + uiPos2);

	if( uiLeft < uiRight)
	{
		return( -1);
	}
	else if( uiLeft > uiRight)
	{
		return( 1);
	}

	return( 0);
}

/***************************************************************************
Desc:
****************************************************************************/
void FLMAPI f_qsortUINTSwap(
	void *		pvBuffer,
	FLMUINT		uiPos1,
	FLMUINT		uiPos2)
{
	FLMUINT *	puiArray = (FLMUINT *)pvBuffer;
	FLMUINT		uiTmp = puiArray[ uiPos1];

	puiArray[ uiPos1] = puiArray[ uiPos2];
	puiArray[ uiPos2] = uiTmp;
}

/****************************************************************************
Desc:
****************************************************************************/
void * FLMAPI f_memcpy(
	void *			pvDest,
	const void *	pvSrc,
	FLMSIZET			iSize)
{
	if( iSize == 1)
	{
		*((FLMBYTE *)pvDest) = *((FLMBYTE *)pvSrc);
		return( pvDest);
	}
	
#ifdef FLM_RING_ZERO_NLM
		CMoveFast( pvSrc, pvDest, iSize);
		return( pvDest);
#else
		return( memcpy( pvDest, pvSrc, iSize));
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
void * FLMAPI f_memmove(
	void *			pvDest,
	const void *	pvSrc,
	FLMSIZET			uiLength)
{
#ifndef FLM_RING_ZERO_NLM

	return( memmove( pvDest, pvSrc, uiLength));
	
#else

	#define CMOVB_THRESHOLD		16
	char			*s = (char *)pvSrc;
	char			*d = (char *)pvDest;
	unsigned		uDiff;

	if( (char *)(s + uiLength) < d || (char *)(d + uiLength) < s)
	{
		// The source and destination do not overlap.

		CMoveFast( (void *)s, d, (LONG)uiLength);
	}
	else if( s < d)
	{
		// Source preceeds the destination, with overlap.

		uDiff = (unsigned)(d - s);
		d += uiLength;
		s += uiLength;
		if( uDiff >= CMOVB_THRESHOLD)
		{
			for( ;;)
			{
				if( uiLength < uDiff)
				{
					break;
				}

				// Copy the tail

				s -= uDiff;
				d -= uDiff;
				uiLength -= uDiff;
				CMoveFast( (void *)s, d, (LONG)uDiff);
			}
		}

		// Copy remaining bytes.

		while( uiLength--)
		{
			*--d = *--s;
		}
	}
	else if( s > d)
	{
		// Source follows the destination, with overlap.

		uDiff = (unsigned)(s - d);
		if( uDiff >= CMOVB_THRESHOLD)
		{
			for( ;;)
			{
				if( uiLength < uDiff)
				{
					break;
				}

				// Copy the head

				CMoveFast( (void *)s, d, (LONG)uDiff);
				uiLength -= uDiff;
				d += uDiff;
				s += uDiff;
			}
		}

		// Copy the remaining bytes

		while( uiLength--)
		{
			*d++ = *s++;
		}
	}

	// Else, the regions overlap completely (s == d).  Do nothing.

	return( pvDest);

#endif
}

/****************************************************************************
Desc:
****************************************************************************/
void * FLMAPI f_memset(
	void *				pvMem,
	unsigned char		ucByte,
	FLMSIZET				uiLength)
{
#ifndef FLM_RING_ZERO_NLM
	return( memset( pvMem, ucByte, uiLength));
#else
	char *			cp = (char *)pvMem;
	unsigned			dwordLength;
	unsigned long	dwordVal;

	dwordVal = ((unsigned long)ucByte << 24) |
		((unsigned long)ucByte << 16) |
		((unsigned long)ucByte << 8) |
		(unsigned long)ucByte;

	while( uiLength && ((long)cp & 3L))
	{
		*cp++ = (char)ucByte;
		uiLength--;
	}

	dwordLength = uiLength >> 2;
	if(  dwordLength != 0)
	{
		CSetD( dwordVal, (void *)cp, dwordLength);
		cp += (dwordLength << 2);
		uiLength -= (dwordLength << 2);
	}

	while( uiLength)
	{
		*cp++ = (char)ucByte;
		uiLength--;
	}

	return( pvMem);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
FLMINT FLMAPI f_memcmp(
	const void *		pvMem1,
	const void *		pvMem2,
	FLMSIZET				uiLength)
{
#ifndef FLM_NLM
	return( memcmp( pvMem1, pvMem2, uiLength));
#else
	unsigned char *	s1;
	unsigned char *	s2;

	for (s1 = (unsigned char *)pvMem1, s2 = (unsigned char *)pvMem2; 
		uiLength > 0; uiLength--, s1++, s2++)
	{
		if (*s1 == *s2)
		{
			continue;
		}
		else if( *s1 > *s2)
		{
			return( 1);
		}
		else
		{
			return( -1);
		}
	}

	return( 0);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
char * FLMAPI f_strcpy(
	char *			pszDest,
	const char *	pszSrc)
{
#ifndef FLM_NLM
	return( strcpy( pszDest, pszSrc));
#else
	while ((*pszDest++ = *pszSrc++) != 0);
	return( pszDest);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
char * FLMAPI f_strncpy(
	char *			pszDest,
	const char *	pszSrc,
	FLMSIZET			uiLength)
{
#ifndef FLM_NLM
	return( strncpy( pszDest, pszSrc, uiLength));
#else
	while( uiLength)
	{
		*pszDest++ = *pszSrc;
		if( *pszSrc)
		{
			pszSrc++;
		}
		
		uiLength--;
	}

	*pszDest = 0;
	return( pszDest);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT FLMAPI f_strlen(
	const char *	pszStr)
{
#ifndef FLM_NLM
	return( strlen( pszStr));
#else
	const char *	pszStart = pszStr;

	while( *pszStr)
	{
		pszStr++;
	}

	return( pszStr - pszStart);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
FLMINT FLMAPI f_strcmp(
	const char *		pszStr1,
	const char *		pszStr2)
{
#ifndef FLM_NLM
	return( strcmp( pszStr1, pszStr2));
#else
	while( *pszStr1 == *pszStr2 && *pszStr1)
	{
		pszStr1++;
		pszStr2++;
	}
	
	return( (FLMINT)(*pszStr1 - *pszStr2));
#endif
}
	
/****************************************************************************
Desc:
****************************************************************************/
FLMINT FLMAPI f_stricmp(
	const char *		pszStr1,
	const char *		pszStr2)
{
#ifdef FLM_WIN
	return( _stricmp( pszStr1, pszStr2));
#else 
	while( f_toupper( *pszStr1) == f_toupper( *pszStr2) && *pszStr1)
	{
		pszStr1++;
		pszStr2++;
	}
	return( (FLMINT)( f_toupper( *pszStr1) - f_toupper( *pszStr2)));
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
FLMINT FLMAPI f_strncmp(
	const char *		pszStr1,
	const char *		pszStr2,
	FLMSIZET				uiLength)
{
#ifndef FLM_NLM
	return( strncmp( pszStr1, pszStr2, uiLength));
#else
	while( *pszStr1 == *pszStr2 && *pszStr1 && uiLength)
	{
		pszStr1++;
		pszStr2++;
		uiLength--;
	}

	if( uiLength)
	{
		return( (*pszStr1 - *pszStr2));
	}

	return( 0);
#endif
}
	
/****************************************************************************
Desc:
****************************************************************************/
FLMINT FLMAPI f_strnicmp(
	const char *		pszStr1,
	const char *		pszStr2,
	FLMSIZET				uiLength)
{
#ifdef FLM_WIN
	return( _strnicmp( pszStr1, pszStr2, uiLength));
#else
	FLMINT				iLen = (FLMINT)uiLength;

	if( !pszStr1 || !pszStr2)
	{
		return( (pszStr1 == pszStr2) 
						? 0 
						: (pszStr1 ? 1 : -1));
	}

	while( iLen-- && *pszStr1 && *pszStr2 && 
		(f_toupper( *pszStr1) == f_toupper( *pszStr2)))
	{
		pszStr1++;
		pszStr2++;
	}

	return(	(iLen == -1)
					?	0
					:	(f_toupper( *pszStr1) - f_toupper( *pszStr2)));

#endif
}

/****************************************************************************
Desc:
****************************************************************************/
char * FLMAPI f_strcat(
	char *				pszDest,
	const char *		pszSrc)
{
#ifndef FLM_NLM
	return( strcat( pszDest, pszSrc));
#else
	const char *	p = pszSrc;
	char * 			q = pszDest;
	
	while (*q++);
	q--;
	while( (*q++ = *p++) != 0);
	
	return( pszDest);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
char * FLMAPI f_strncat(
	char *				pszDest,
	const char *		pszSrc,
	FLMSIZET				uiLength)
{
#ifndef FLM_NLM
	return( strncat( pszDest, pszSrc, uiLength));
#else
	const char *		p = pszSrc;
	char *				q = pszDest;
	
	while (*q++);
	
	q--;
	uiLength++;
	
	while( --uiLength)
	{
		if( (*q++ = *p++) == 0)
		{
			q--;
			break;
		}
	}
	
	*q = 0;
	return( pszDest);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
char * FLMAPI f_strchr(
	const char *		pszStr,
	unsigned char		ucByte)
{
#ifndef FLM_NLM
	return( (char *)strchr( pszStr, ucByte));
#else
	if( !pszStr)
	{
		return( NULL);
	}

	while (*pszStr && *pszStr != ucByte)
	{
		pszStr++;
	}

	return( (char *)((*pszStr == ucByte) 
								? pszStr
								: NULL));
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
char * FLMAPI f_strrchr(
	const char *		pszStr,
	unsigned char		ucByte)
{
#ifndef FLM_NLM
	return( (char *)strrchr( pszStr, ucByte));
#else
	const char * pszLast = NULL;

	if( !pszStr)
	{
		return( NULL);
	}

	while (*pszStr)
	{
		if( *pszStr == ucByte)
		{
			pszLast = pszStr;
		}
		
		pszStr++;
	}
	
	if( ucByte == '\0')
	{
		pszLast = pszStr;
	}

	return( (char *)pszLast);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
char * FLMAPI f_strstr(
	const char *		pszStr1,
	const char *		pszStr2)
{
#ifndef FLM_NLM
	return( (char *)strstr( pszStr1, pszStr2));
#else
	FLMUINT 			i;
	FLMUINT			j;
	FLMUINT			k;

	if ( !pszStr1 || !pszStr2)
	{
		return( NULL);
	}

	for( i = 0; pszStr1[i] != '\0'; i++)
	{
		for( j=i, k=0; pszStr2[k] != '\0' &&
			pszStr1[j] == pszStr2[k]; j++, k++)
		{
			;
		}

		if ( k > 0 && pszStr2[k] == '\0')
		{
			return( (char *)&pszStr1[i]);
		}
	}

	return( NULL);
#endif
}

/****************************************************************************
Desc:
****************************************************************************/
char * FLMAPI f_strupr(
	char *				pszStr)
{
#ifdef FLM_WIN
	return( _strupr( pszStr));
#else
	while( *pszStr)
	{
		*pszStr = f_toupper( *pszStr);
		pszStr++;
	}

	return( pszStr);
#endif
}

/**********************************************************************
Desc:
**********************************************************************/
FLMINT32 FLMAPI f_atomicInc(
	FLMATOMIC *			piTarget)
{
	#if defined( FLM_LIBC_NLM)
	{
		return( (FLMINT32)atomic_retadd( (unsigned long *)piTarget, 1));
	}
	#elif defined( FLM_RING_ZERO_NLM)
	{
		return( nlm_AtomicIncrement( (volatile LONG *)piTarget)); 
	}
	#elif defined( FLM_WIN)
	{
		return( (FLMINT32)InterlockedIncrement( (volatile LONG *)piTarget));
	}
	#elif defined( FLM_AIX)
	{
		return( (FLMINT32)aix_atomic_add( piTarget, 1));
	}
	#elif defined( FLM_OSX)
	{
		return( (FLMINT32)OSAtomicIncrement32( (int32_t *)piTarget));
	}
	#elif defined( FLM_SOLARIS) && defined( FLM_SPARC) && !defined( FLM_GNUC)
	{
		return( sparc_atomic_add_32( piTarget, 1));
	}
	#elif (defined( __i386__) || defined( __x86_64__)) && defined( FLM_GNUC)
	{
		FLMINT32 			i32Tmp;
		
		__asm__ __volatile__ (
						"lock;"
						"xaddl %0, %1"
							: "=r" (i32Tmp), "=m" (*piTarget)
							: "0" (1), "m" (*piTarget));
	
		return( i32Tmp + 1);
	}
	#elif defined( FLM_UNIX)
		return( posix_atomic_add_32( piTarget, 1));
	#else
		#error Atomic operations are not supported
	#endif
}

/**********************************************************************
Desc:
**********************************************************************/
FLMINT32 FLMAPI f_atomicDec(
	FLMATOMIC *			piTarget)
{
	#if defined( FLM_LIBC_NLM)
	{
		return( (FLMINT32)atomic_retadd( (unsigned long *)piTarget, -1));
	}
	#elif defined( FLM_RING_ZERO_NLM)
	{
		return( nlm_AtomicDecrement( (volatile LONG *)piTarget)); 
	}
	#elif defined( FLM_WIN)
	{
		return( (FLMINT32)InterlockedDecrement( (volatile LONG *)piTarget));
	}
	#elif defined( FLM_AIX)
	{
		return( (FLMINT32)aix_atomic_add( piTarget, -1));
	}
	#elif defined( FLM_OSX)
	{
		return( (FLMINT32)OSAtomicDecrement32( (int32_t *)piTarget));
	}
	#elif defined( FLM_SOLARIS) && defined( FLM_SPARC) && !defined( FLM_GNUC)
	{
		return( sparc_atomic_add_32( piTarget, -1));
	}
	#elif (defined( __i386__) || defined( __x86_64__)) && defined( FLM_GNUC)
	{
		FLMINT32				i32Tmp;
		
		__asm__ __volatile__ (
						"lock;" 
						"xaddl %0, %1"
							: "=r" (i32Tmp), "=m" (*piTarget)
							: "0" (-1), "m" (*piTarget));
	
		return( i32Tmp - 1);
	}
	#elif defined( FLM_UNIX)
		return( posix_atomic_add_32( piTarget, -1));
	#else
		#error Atomic operations are not supported
	#endif
}

/**********************************************************************
Desc:
**********************************************************************/
FLMINT32 FLMAPI f_atomicExchange(
	FLMATOMIC *			piTarget,
	FLMINT32				i32NewVal)
{
	#if defined( FLM_NLM)
	{
		return( (FLMINT32)atomic_xchg( (unsigned long *)piTarget, i32NewVal));
	}
	#elif defined( FLM_WIN)
	{
		return( (FLMINT32)InterlockedExchange( (volatile LONG *)piTarget,
			i32NewVal));
	}
	#elif defined( FLM_AIX)
	{
		int		iOldVal;
		
		for( ;;)
		{ 
			iOldVal = (int)*piTarget;
			
			if( compare_and_swap( (int *)piTarget, &iOldVal, i32NewVal))
			{
				break;
			}
		}
		
		return( (FLMINT32)iOldVal);
	}
	#elif defined( FLM_OSX)
	{
		int32_t		iOldVal;

		for( ;;)
		{
			iOldVal = (int32_t)*piTarget;

			if( OSAtomicCompareAndSwap32( iOldVal, i32NewVal, 
					(int32_t *)piTarget))
			{
				break;
			}
		}
		
		return( (FLMINT32)iOldVal);
	}
	#elif defined( FLM_SOLARIS) && defined( FLM_SPARC) && !defined( FLM_GNUC)
	{
		return( sparc_atomic_xchg_32( piTarget, i32NewVal));
	}
	#elif (defined( __i386__) || defined( __x86_64__)) && defined( FLM_GNUC)
	{
		FLMINT32 			i32OldVal;
		
		__asm__ __volatile__ (
						"1:	lock;"
						"		cmpxchgl %2, %0;"
						"		jne 1b"
							: "=m" (*piTarget), "=a" (i32OldVal)
							: "r" (i32NewVal), "m" (*piTarget), "a" (*piTarget));
	
		return( i32OldVal);
	}
	#elif defined( FLM_UNIX)
		return( posix_atomic_xchg_32( piTarget, i32NewVal));
	#else
		#error Atomic operations are not supported
	#endif
}

/**********************************************************************
Desc:
**********************************************************************/
FLMINT FLMAPI F_Object::getRefCount( void)
{
	return( m_refCnt);
}

/**********************************************************************
Desc:
**********************************************************************/
FLMINT FLMAPI F_Object::AddRef( void)
{
	return( ++m_refCnt);
}

/**********************************************************************
Desc:
**********************************************************************/
FLMINT FLMAPI F_Object::Release( void)
{
	FLMINT		iRefCnt = --m_refCnt;

	if( !iRefCnt)
	{
		delete this;
	}

	return( iRefCnt);
}

/**********************************************************************
Desc:
**********************************************************************/
IF_FileSystem * FLMAPI f_getFileSysPtr( void)
{
	return( gv_pFileSystem);
}

/**********************************************************************
Desc:
**********************************************************************/
IF_ThreadMgr * f_getThreadMgrPtr( void)
{
	return( gv_pThreadMgr);
}

/**********************************************************************
Desc:
**********************************************************************/
FLMUINT FLMAPI f_getMaxFileSize( void)
{
	return( gv_uiMaxFileSize);
}

/*****************************************************************************
Desc:
******************************************************************************/
RCODE FLMAPI f_readSEN(
	IF_IStream *	pIStream,
	FLMUINT *		puiValue,
	FLMUINT *		puiLength)
{
	RCODE				rc;
	FLMUINT64		ui64Tmp;

	if( RC_BAD( rc = f_readSEN64( pIStream, &ui64Tmp, puiLength)))
	{
		goto Exit;
	}

	if( ui64Tmp > ~((FLMUINT)0))
	{
		rc = RC_SET_AND_ASSERT( NE_FLM_CONV_DEST_OVERFLOW);
		goto Exit;
	}

	if( puiValue)
	{
		*puiValue = (FLMUINT)ui64Tmp;
	}

Exit:

	return( rc);
}

/*****************************************************************************
Desc:
******************************************************************************/
RCODE FLMAPI f_readSEN64(
	IF_IStream *		pIStream,
	FLMUINT64 *			pui64Value,
	FLMUINT *			puiLength)
{
	RCODE					rc = NE_FLM_OK;
	FLMUINT				uiLen;
	FLMUINT				uiSENLength;
	FLMBYTE				ucBuffer[ 16];
	const FLMBYTE *	pucBuffer;

	uiLen = 1;
	if( RC_BAD( rc = pIStream->read( 
		(char *)&ucBuffer[ 0], uiLen, &uiLen)))
	{
		goto Exit;
	}

	uiSENLength = 	gv_ucSENLengthArray[ ucBuffer[ 0]];
	uiLen = uiSENLength - 1;

	if( puiLength)
	{
		*puiLength = uiSENLength;
	}

	if( pui64Value)
	{
		pucBuffer = &ucBuffer[ 1];
	}
	else
	{
		pucBuffer = NULL;
	}

	if( uiLen)
	{
		if( RC_BAD( rc = pIStream->read( 
			(char *)pucBuffer, uiLen, &uiLen)))
		{
			goto Exit;
		}
	}

	if( pui64Value)
	{
		pucBuffer = &ucBuffer[ 0];
		if( RC_BAD( rc = f_decodeSEN64( &pucBuffer,
			&ucBuffer[ sizeof( ucBuffer)], pui64Value)))
		{
			goto Exit;
		}
	}

Exit:

	return( rc);
}

/*****************************************************************************
Desc:
******************************************************************************/
FLMUINT FLMAPI f_getSENLength(
	FLMBYTE 					ucByte)
{
	return( gv_ucSENLengthArray[ ucByte]);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE FLMAPI f_decodeSEN64(
	const FLMBYTE **		ppucBuffer,
	const FLMBYTE *		pucEnd,
	FLMUINT64 *				pui64Value)
{
	RCODE						rc = NE_FLM_OK;
	FLMUINT					uiSENLength;
	const FLMBYTE *		pucBuffer = *ppucBuffer;

	uiSENLength = gv_ucSENLengthArray[ *pucBuffer];
	if( pucBuffer + uiSENLength > pucEnd)
	{
		if (pui64Value)
		{
			*pui64Value = 0;
		}
		rc = RC_SET( NE_FLM_BAD_SEN);
		goto Exit;
	}

	if (pui64Value)
	{
		switch( uiSENLength)
		{
			case 1:
				*pui64Value = *pucBuffer;
				break;
	
			case 2:
				*pui64Value = (((FLMUINT64)(*pucBuffer & 0x3F)) << 8) + pucBuffer[ 1];
				break;
	
			case 3:
				*pui64Value = (((FLMUINT64)(*pucBuffer & 0x1F)) << 16) +
					(((FLMUINT64)pucBuffer[ 1]) << 8) + pucBuffer[ 2];
				break;
	
			case 4:
				*pui64Value = (((FLMUINT64)(*pucBuffer & 0x0F)) << 24) +
					(((FLMUINT64)pucBuffer[ 1]) << 16) +
					(((FLMUINT64)pucBuffer[ 2]) << 8) + pucBuffer[ 3];
				break;
	
			case 5:
				*pui64Value = (((FLMUINT64)(*pucBuffer & 0x07)) << 32) +
					(((FLMUINT64)pucBuffer[ 1]) << 24) +
					(((FLMUINT64)pucBuffer[ 2]) << 16) +
					(((FLMUINT64)pucBuffer[ 3]) << 8) + pucBuffer[ 4];
				break;
	
			case 6:
				*pui64Value = (((FLMUINT64)(*pucBuffer & 0x03)) << 40) +
					(((FLMUINT64)pucBuffer[ 1]) << 32) +
					(((FLMUINT64)pucBuffer[ 2]) << 24) +
					(((FLMUINT64)pucBuffer[ 3]) << 16) +
					(((FLMUINT64)pucBuffer[ 4]) << 8) + pucBuffer[ 5];
				break;
	
			case 7:
				*pui64Value = (((FLMUINT64)(*pucBuffer & 0x01)) << 48) +
					(((FLMUINT64)pucBuffer[ 1]) << 40) +
					(((FLMUINT64)pucBuffer[ 2]) << 32) +
					(((FLMUINT64)pucBuffer[ 3]) << 24) +
					(((FLMUINT64)pucBuffer[ 4]) << 16) +
					(((FLMUINT64)pucBuffer[ 5]) << 8) + pucBuffer[ 6];
				break;
	
			case 8:
				*pui64Value = (((FLMUINT64)pucBuffer[ 1]) << 48) +
					(((FLMUINT64)pucBuffer[ 2]) << 40) +
					(((FLMUINT64)pucBuffer[ 3]) << 32) +
					(((FLMUINT64)pucBuffer[ 4]) << 24) +
					(((FLMUINT64)pucBuffer[ 5]) << 16) +
					(((FLMUINT64)pucBuffer[ 6]) << 8) + pucBuffer[ 7];
				break;
	
			case 9:
				*pui64Value = (((FLMUINT64)pucBuffer[ 1]) << 56) +
					(((FLMUINT64)pucBuffer[ 2]) << 48) +
					(((FLMUINT64)pucBuffer[ 3]) << 40) +
					(((FLMUINT64)pucBuffer[ 4]) << 32) +
					(((FLMUINT64)pucBuffer[ 5]) << 24) +
					(((FLMUINT64)pucBuffer[ 6]) << 16) +
					(((FLMUINT64)pucBuffer[ 7]) << 8) + pucBuffer[ 8];
				break;
	
			default:
				*pui64Value = 0;
				flmAssert( 0);
				break;
		}
	}

Exit:

	*ppucBuffer = pucBuffer + uiSENLength;

	return( rc);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE FLMAPI f_decodeSEN(
	const FLMBYTE **		ppucBuffer,
	const FLMBYTE *		pucEnd,
	FLMUINT *				puiValue)
{
	RCODE				rc = NE_FLM_OK;
	FLMUINT64		ui64Value;
	
	if( RC_BAD( rc = f_decodeSEN64( ppucBuffer, pucEnd, &ui64Value)))
	{
		return( rc);
	}
	
	if( ui64Value > FLM_MAX_UINT)
	{
		return( RC_SET_AND_ASSERT( NE_FLM_CONV_NUM_OVERFLOW));
	}
	
	if( puiValue)
	{
		*puiValue = (FLMUINT)ui64Value;
	}

	return( rc);
}
	
/****************************************************************************
Desc:
****************************************************************************/
FINLINE FLMBYTE f_shiftRightRetByte(
	FLMUINT64	ui64Num,
	FLMBYTE		ucBits)
{
	return( ucBits < 64 ? (FLMBYTE)(ui64Num >> ucBits) : 0);
}

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT FLMAPI f_getSENByteCount(
	FLMUINT64	ui64Num)
{
	FLMUINT		uiCount = 0;

	if( ui64Num < 0x80)
	{
		return( 1);
	}

	while( ui64Num)
	{
		uiCount++;
		ui64Num >>= 7;
	}

	// If the high bit is set, the counter will be incremented 1 beyond
	// the actual number of bytes need to represent the SEN.  We will need
	// to re-visit this if we ever go beyond 64-bits.

	return( uiCount < FLM_MAX_SEN_LEN ? uiCount : FLM_MAX_SEN_LEN);
}

/****************************************************************************
Desc:		Encodes a number as a SEN
****************************************************************************/
FLMUINT FLMAPI f_encodeSEN(
	FLMUINT64		ui64Value,
	FLMBYTE **		ppucBuffer,
	FLMUINT			uiSizeWanted)
{
	FLMBYTE *		pucBuffer = *ppucBuffer;
	FLMUINT			uiSenLen = f_getSENByteCount( ui64Value);

	flmAssert( uiSizeWanted <= FLM_MAX_SEN_LEN && 
				  (!uiSizeWanted || uiSizeWanted >= uiSenLen));

	uiSenLen = uiSizeWanted > uiSenLen ? uiSizeWanted : uiSenLen;

	if( uiSenLen == 1)
	{
		*pucBuffer++ = (FLMBYTE)ui64Value;
	}
	else
	{
		FLMUINT			uiTmp = (uiSenLen - 1) << 3;

		*pucBuffer++ = ucSENPrefixArray[ uiSenLen] + 
							f_shiftRightRetByte( ui64Value, (FLMBYTE)uiTmp);
		while( uiTmp)
		{
			uiTmp -= 8;
			*pucBuffer++ = f_shiftRightRetByte( ui64Value, (FLMBYTE)uiTmp);
		}
	}

	*ppucBuffer = pucBuffer;
	return( uiSenLen);
}

/****************************************************************************
Desc:		Encodes a number as a SEN
****************************************************************************/
RCODE FLMAPI f_encodeSEN(
	FLMUINT64		ui64Value,
	FLMBYTE **		ppucBuffer,
	FLMBYTE *		pucEnd)
{
	RCODE				rc = NE_FLM_OK;
	FLMBYTE *		pucBuffer = *ppucBuffer;
	FLMUINT			uiSenLen = f_getSENByteCount( ui64Value);
	
	if( *ppucBuffer + uiSenLen > pucEnd)
	{
		rc = RC_SET_AND_ASSERT( NE_FLM_CONV_DEST_OVERFLOW);
		goto Exit;
	}

	if( uiSenLen == 1)
	{
		*pucBuffer++ = (FLMBYTE)ui64Value;
	}
	else
	{
		FLMUINT			uiTmp = (uiSenLen - 1) << 3;

		*pucBuffer++ = ucSENPrefixArray[ uiSenLen] + 
							f_shiftRightRetByte( ui64Value, (FLMBYTE)uiTmp);
		while( uiTmp)
		{
			uiTmp -= 8;
			*pucBuffer++ = f_shiftRightRetByte( ui64Value, (FLMBYTE)uiTmp);
		}
	}

	*ppucBuffer = pucBuffer;
	
Exit:

	return( rc);
}

/****************************************************************************
Desc:		Encodes a number as a SEN
****************************************************************************/
FLMUINT FLMAPI f_encodeSENKnownLength(
	FLMUINT64		ui64Value,
	FLMUINT			uiSenLen,
	FLMBYTE **		ppucBuffer)
{
	FLMBYTE *			pucBuffer = *ppucBuffer;

	if( uiSenLen == 1)
	{
		*pucBuffer++ = (FLMBYTE)ui64Value;
	}
	else
	{
		FLMUINT			uiTmp = (uiSenLen - 1) << 3;

		*pucBuffer++ = ucSENPrefixArray[ uiSenLen] + 
							f_shiftRightRetByte( ui64Value, (FLMBYTE)uiTmp);
		while( uiTmp)
		{
			uiTmp -= 8;
			*pucBuffer++ = f_shiftRightRetByte( ui64Value, (FLMBYTE)uiTmp);
		}
	}

	*ppucBuffer = pucBuffer;
	return( uiSenLen);
}

/****************************************************************************
Desc:
****************************************************************************/
RCODE FLMAPI FlmGetXMLObject(
	IF_XML **				ppXmlObject)
{
	*ppXmlObject = gv_pXml;
	(*ppXmlObject)->AddRef();
	
	return( NE_FLM_OK);
}

/****************************************************************************
Desc:
****************************************************************************/
IF_XML * f_getXmlObjPtr( void)
{
	return( gv_pXml);
}

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT32 FLMAPI f_getRandomUINT32(
	FLMUINT32		ui32Low,
	FLMUINT32		ui32High)
{
	return( gv_pRandomGenerator->getUINT32( ui32Low, ui32High));
}