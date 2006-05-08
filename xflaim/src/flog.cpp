//------------------------------------------------------------------------------
// Desc:	Contains routines for logging messages from within FLAIM.
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
// $Id: flog.cpp 3113 2006-01-19 13:20:35 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------

#include "flaimsys.h"

/****************************************************************************
Desc:	Returns an IF_LogMessageClient object if logging is enabled for the
		specified message type
****************************************************************************/
IF_LogMessageClient * flmBeginLogMessage(
	eLogMessageType	eMsgType)
{
	IF_LogMessageClient *		pNewMsg = NULL;

	f_mutexLock( gv_XFlmSysData.hLoggerMutex);
	
	if( !gv_XFlmSysData.pLogger)
	{
		goto Exit;
	}
		
	if( (pNewMsg = gv_XFlmSysData.pLogger->beginMessage( eMsgType)) != NULL)
	{
		gv_XFlmSysData.uiPendingLogMessages++;
	}
	
Exit:

	f_mutexUnlock( gv_XFlmSysData.hLoggerMutex);
	return( pNewMsg);
}

/****************************************************************************
Desc:		Logs information about an error
****************************************************************************/
void flmLogError(
	RCODE				rc,
	const char *	pszDoing,
	const char *	pszFileName,
	FLMINT			iLineNumber)
{
	FLMBYTE *					pszMsgBuf = NULL;
	IF_LogMessageClient *	pLogMsg = NULL;

	if( (pLogMsg = flmBeginLogMessage( XFLM_GENERAL_MESSAGE)) != NULL)
	{
		if( RC_OK( f_alloc( 512, &pszMsgBuf)))
		{
			if( pszFileName)
			{
				f_sprintf( (char *)pszMsgBuf,
					"Error %s: %e, File=%s, Line=%d.",
					pszDoing, rc, pszFileName, (int)iLineNumber);
			}
			else
			{
				f_sprintf( (char *)pszMsgBuf, "Error %s: %e.", pszDoing, rc);
			}

			pLogMsg->changeColor( FLM_YELLOW, FLM_BLACK);
			pLogMsg->appendString( (char *)pszMsgBuf);
		}
		flmEndLogMessage( &pLogMsg);
	}

	if( pszMsgBuf)
	{
		f_free( &pszMsgBuf);
	}
}

/****************************************************************************
Desc:	Ends a logging message
****************************************************************************/
void flmEndLogMessage(
	IF_LogMessageClient **		ppLogMessage)
{
	if( *ppLogMessage)
	{
		f_mutexLock( gv_XFlmSysData.hLoggerMutex);
		flmAssert( gv_XFlmSysData.uiPendingLogMessages);
		
		(*ppLogMessage)->endMessage();
		(*ppLogMessage)->Release();
		*ppLogMessage = NULL;
		
		gv_XFlmSysData.uiPendingLogMessages--;
		f_mutexUnlock( gv_XFlmSysData.hLoggerMutex);
	}
}
