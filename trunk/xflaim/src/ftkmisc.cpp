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

#include "flaimsys.h"

// Global data

FLMUINT						gv_uiSerialInitCount = 0;
F_MUTEX						gv_hSerialMutex = F_MUTEX_NULL;

#ifdef FLM_UNIX
	F_RandomGenerator		gv_SerialRandom;
#endif

#ifdef FLM_NLM
	void f_sleep( 
		FLMUINT	uiMilliseconds)
	{
		if( ! uiMilliseconds )
		{
			kYieldThread();
		}
		else
		{
			kDelayThread( uiMilliseconds);
		}
	}
	
#endif

#if defined( FLM_UNIX)

	#ifdef FLM_AIX
		#ifndef nsleep
			extern "C"
			{
				extern int nsleep( struct timestruc_t *, struct timestruc_t *);
			}
		#endif
	#endif

/****************************************************************************
Desc: This routine causes the calling process to delay the given number
		of milliseconds.  Due to the nature of the call, the actual sleep
		time is almost guaranteed to be different from requested sleep time.
In:   milliseconds - the number of milliseconds to delay
****************************************************************************/
void f_sleep(
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

/***************************************************************************
Desc:   Map POSIX errno to Flaim IO errors.
***************************************************************************/
#if defined( FLM_UNIX) || defined( FLM_NLM)
RCODE MapErrnoToFlaimErr(
	int	err,
	RCODE	defaultRc)
{
	/* Switch on passed in error code value */

	switch (err)
	{
		case 0:
			return( NE_XFLM_OK);

		case ENOENT:
			return( RC_SET( NE_XFLM_IO_PATH_NOT_FOUND));

		case EACCES:
		case EEXIST:
			return( RC_SET( NE_XFLM_IO_ACCESS_DENIED));

		case EINVAL:
			return( RC_SET( NE_XFLM_IO_PATH_TOO_LONG));

		case EIO:
			return( RC_SET( NE_XFLM_IO_DISK_FULL));

		case ENOTDIR:
			return( RC_SET( NE_XFLM_IO_DIRECTORY_ERR));

#ifdef EBADFD
		case EBADFD:
			return( RC_SET( NE_XFLM_IO_BAD_FILE_HANDLE));
#endif

#ifdef EOF
		case EOF:
			return( RC_SET( NE_XFLM_IO_END_OF_FILE));
#endif
			
		case EMFILE:
			return( RC_SET( NE_XFLM_IO_NO_MORE_FILES));

		default:
			return( RC_SET( defaultRc));
	}
}
#endif

/****************************************************************************
Desc:		This routine initializes the serial number generator.  If the O/S
			does not provide support for GUID generation or if the GUID
			routines fail for some reason, a pseudo-GUID will be generated.
Notes:	This routine should only be called once by the process.
****************************************************************************/
RCODE f_initSerialNumberGenerator( void)
{
	FLMUINT					uiTime;
	RCODE						rc = NE_XFLM_OK;

	if (++gv_uiSerialInitCount > 1)
	{
		goto Exit;
	}

	if( RC_BAD( rc = f_mutexCreate( &gv_hSerialMutex)))
	{
		goto Exit;
	}

	f_timeGetSeconds( &uiTime );

#if defined( FLM_UNIX)
	gv_SerialRandom.randomSetSeed( (FLMUINT32)(uiTime ^ (FLMUINT)getpid()));
#endif

Exit:

	return( rc);
}

/****************************************************************************
Desc:		This routine will use the operating system calls to generate a
			"globally unique" identifier.  Typically, this is based on the
			MAC address of an ethernet card installed in the machine.  If the
			machine does not have an ethernet card, or if the OS does not
			support generating GUIDs, this routine will generate a pseudo-GUID
			using a random number generator.  A serial number is 16-bytes.
****************************************************************************/
RCODE f_createSerialNumber(
	FLMBYTE *		pszSerialNum)
{
	RCODE						rc = NE_XFLM_OK;

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

#elif defined( FLM_NLM)

	NWGUID	guidVal;
	int		err = SGUIDCreate( &guidVal);

	if( !err || err == 1) // NOTE: 1 == SGUID_WARN_RANDOM_NODE
	{
		UD2FBA( guidVal.time_low, &pszSerialNum[ 0]);
		UW2FBA( guidVal.time_mid, &pszSerialNum[ 4]);
		UW2FBA( guidVal.time_hi_and_version, &pszSerialNum[ 6]);
		pszSerialNum[ 8] = guidVal.clk_seq_hi_res;
		pszSerialNum[ 9] = guidVal.clk_seq_low;
		f_memcpy( &pszSerialNum[ 10], (FLMBYTE *)guidVal.node, 6);
		goto Exit;
	}

#elif defined( FLM_UNIX)

	// Generate a pseudo GUID value

	flmAssert( gv_hSerialMutex != F_MUTEX_NULL);

	f_mutexLock( gv_hSerialMutex);

	UD2FBA( (FLMUINT32)gv_SerialRandom.randomLong(), &pszSerialNum[ 0]);
	UD2FBA( (FLMUINT32)gv_SerialRandom.randomLong(), &pszSerialNum[ 4]);
	UD2FBA( (FLMUINT32)gv_SerialRandom.randomLong(), &pszSerialNum[ 8]);
	UD2FBA( (FLMUINT32)gv_SerialRandom.randomLong(), &pszSerialNum[ 12]);

	f_mutexUnlock( gv_hSerialMutex);

#endif

#if defined( FLM_WIN) || defined( FLM_NLM)
Exit:
#endif

	return( rc);
}

/****************************************************************************
Notes:	This routine should only be called once by the process.
****************************************************************************/
void f_freeSerialNumberGenerator( void)
{
	if( (--gv_uiSerialInitCount) > 0)
	{
		return;
	}

	if( gv_hSerialMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_hSerialMutex);
	}
}

/****************************************************************************
Desc: Generates a table of remainders for each 8-bit byte.  The resulting
		table is used by flmUpdateCRC to calculate a CRC value.  The table
		must be freed via a call to f_freeCRCTable.
*****************************************************************************/
RCODE f_initCRCTable(
	FLMUINT32 **	ppui32CRCTbl)
{
	FLMUINT32 *		pTable;
	FLMUINT32		ui32Val;
	FLMUINT32		ui32Loop;
	FLMUINT32		ui32SubLoop;
	RCODE				rc = NE_XFLM_OK;

	// Use the standard degree-32 polynomial used by
	// Ethernet, PKZIP, etc. for computing the CRC of
	// a data stream.  This is the little-endian
	// representation of the polynomial.  The big-endian
	// representation is 0x04C11DB7.

#define CRC_POLYNOMIAL		((FLMUINT32)0xEDB88320)

	*ppui32CRCTbl = NULL;

	if( RC_BAD( rc = f_alloc( 256 * sizeof( FLMUINT32), &pTable)))
	{
		goto Exit;
	}

	for( ui32Loop = 0; ui32Loop < 256; ui32Loop++)
	{
		ui32Val = ui32Loop;
		for( ui32SubLoop = 0; ui32SubLoop < 8; ui32SubLoop++)
		{
			if( ui32Val & 0x00000001)
			{
				ui32Val = CRC_POLYNOMIAL ^ (ui32Val >> 1);
			}
			else
			{
				ui32Val >>= 1;
			}
		}

		pTable[ ui32Loop] = ui32Val;
	}

	*ppui32CRCTbl = pTable;
	pTable = NULL;

Exit:

	if( pTable)
	{
		f_free( &pTable);
	}

	return( rc);
}

/****************************************************************************
Desc: Computes the CRC of the passed-in data buffer.  Multiple calls can
		be made to this routine to build a CRC over multiple data buffers.
		On the first call, *pui32CRC must be initialized to something
		(0, etc.).  For generating CRCs that are compatible with PKZIP,
		*pui32CRC should be initialized to 0xFFFFFFFF and the ones complement
		of the resulting CRC should be computed.
*****************************************************************************/
void f_updateCRC(
	FLMUINT32 *		pui32CRCTbl,
	FLMBYTE *		pucBlk,
	FLMUINT			uiBlkSize,
	FLMUINT32 *		pui32CRC)
{
	FLMUINT32		ui32CRC = *pui32CRC;
	FLMUINT			uiLoop;

	for( uiLoop = 0; uiLoop < uiBlkSize; uiLoop++)
	{
		ui32CRC = (ui32CRC >> 8) ^ pui32CRCTbl[
			((FLMBYTE)(ui32CRC & 0x000000FF)) ^ pucBlk[ uiLoop]];
	}

	*pui32CRC = ui32CRC;
}

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT f_breakpoint(
	FLMUINT		uiBreakFlag)
{
	if( uiBreakFlag)
	{
#ifdef FLM_NLM
		EnterDebugger();
#else
		flmAssert( 0);
#endif
	}

	return( 0);
}

/****************************************************************************
Desc: 	
****************************************************************************/
void f_getenv(
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
		f_strcpy( pszBuffer, pszValue);
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

/****************************************************************************
Desc:
****************************************************************************/
#if defined( FLM_SOLARIS) && defined( FLM_SPARC) && !defined( FLM_GNUC)
static void sparc_asm_code( void)
{
	asm( ".align 8");
	asm( ".global sparc_atomic_add_32");
	asm( ".type sparc_atomic_add_32, #function");
	asm( "sparc_atomic_add_32:");
	asm( "    membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad");
	asm( "    ld [%o0], %l0");
	asm( "    add %l0, %o1, %l2");
	asm( "    cas [%o0], %l0, %l2");
	asm( "    cmp %l0, %l2");
	asm( "    bne sparc_atomic_add_32");
	asm( "    nop");
	asm( "    add %l2, %o1, %o0");
	asm( "    membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad");
	asm( "retl");
	asm( "nop");
	
	asm( ".align 8");
	asm( ".global sparc_atomic_xchg_32");
	asm( ".type sparc_atomic_xchg_32, #function");
	asm( "sparc_atomic_xchg_32:");
	asm( "    membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad");
	asm( "    ld [%o0], %l0");
	asm( "    mov %o1, %l1");
	asm( "    cas [%o0], %l0, %l1");
	asm( "    cmp %l0, %l1");
	asm( "    bne sparc_atomic_xchg_32");
	asm( "    nop");
	asm( "    mov %l0, %o0");
	asm( "    membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad");
	asm( "retl");
	asm( "nop");
}
#endif