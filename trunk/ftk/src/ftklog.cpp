//------------------------------------------------------------------------------
// Desc:	Contains routines for logging messages from within FLAIM.
// Tabs:	3
//
// Copyright (c) 2001-2007 Novell, Inc. All Rights Reserved.
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

#include "ftksys.h"

static F_MUTEX					gv_hLoggerMutex = F_MUTEX_NULL;
static FLMUINT					gv_uiPendingLogMessages = 0;
static IF_LoggerClient *	gv_pLogger = NULL;

/****************************************************************************
Desc:	Main entry point for printf functionality.
****************************************************************************/
void f_logPrintf(
	IF_LogMessageClient *	pLogMessage,
	const char *				pszFormatStr, ...)
{
	f_va_list	args;
	F_Printf		formatter;

	f_va_start( args, pszFormatStr);
	formatter.logvPrintf( pLogMessage, pszFormatStr, &args);
	f_va_end( args);
}

/****************************************************************************
Desc:	Printf routine that accepts a va_list argument
****************************************************************************/
void FLMAPI f_logVPrintf(
	IF_LogMessageClient *	pLogMessage,
	const char *				pszFormatStr,
	f_va_list *					args)
{
	F_Printf		formatter;

	formatter.logvPrintf( pLogMessage, pszFormatStr, args);
}

/****************************************************************************
Desc:	Returns an IF_LogMessageClient object if logging is enabled for the
		specified message type
****************************************************************************/
IF_LogMessageClient * FLMAPI f_beginLogMessage(
	FLMUINT						uiMsgType,
	eLogMessageSeverity		eMsgSeverity)
{
	IF_LogMessageClient *		pNewMsg = NULL;

	f_mutexLock( gv_hLoggerMutex);
	
	if( !gv_pLogger)
	{
		goto Exit;
	}
		
	if( (pNewMsg = gv_pLogger->beginMessage( uiMsgType, eMsgSeverity)) != NULL)
	{
		gv_uiPendingLogMessages++;
	}
	
Exit:

	f_mutexUnlock( gv_hLoggerMutex);
	return( pNewMsg);
}

/****************************************************************************
Desc:		Logs information about an error
****************************************************************************/
void FLMAPI f_logError(
	RCODE				rc,
	const char *	pszDoing,
	const char *	pszFileName,
	FLMINT			iLineNumber)
{
	IF_LogMessageClient *	pLogMsg = NULL;

	if( (pLogMsg = f_beginLogMessage( 0, F_ERR_MESSAGE)) != NULL)
	{
		pLogMsg->changeColor( FLM_YELLOW, FLM_BLACK);
		if( pszFileName)
		{
			f_logPrintf( pLogMsg, 
				"Error %s: %e, File=%s, Line=%d.",
				pszDoing, rc, pszFileName, (int)iLineNumber);
		}
		else
		{
			f_logPrintf( pLogMsg, "Error %s: %e.", pszDoing, rc);
		}
		
		f_endLogMessage( &pLogMsg);
	}
}

/****************************************************************************
Desc:	Ends a logging message
****************************************************************************/
void FLMAPI f_endLogMessage(
	IF_LogMessageClient **		ppLogMessage)
{
	if( *ppLogMessage)
	{
		f_mutexLock( gv_hLoggerMutex);
		f_assert( gv_uiPendingLogMessages);
		
		(*ppLogMessage)->endMessage();
		(*ppLogMessage)->Release();
		*ppLogMessage = NULL;
		
		gv_uiPendingLogMessages--;
		f_mutexUnlock( gv_hLoggerMutex);
	}
}

/****************************************************************************
Desc:	Initialize the toolkit logger
****************************************************************************/
RCODE f_loggerInit( void)
{
	RCODE	rc = NE_FLM_OK;

	if (RC_BAD( rc = f_mutexCreate( &gv_hLoggerMutex)))
	{
		goto Exit;
	}

Exit:

	return( rc);
}

/****************************************************************************
Desc:	Shutdown the toolkit logger
****************************************************************************/
void f_loggerShutdown( void)
{
	if (gv_pLogger)
	{
		gv_pLogger->Release();
		gv_pLogger = NULL;
	}
	if (gv_hLoggerMutex != F_MUTEX_NULL)
	{
		f_mutexDestroy( &gv_hLoggerMutex);
	}
}

/****************************************************************************
Desc:	Set the toolkit logger client
****************************************************************************/
void f_setLoggerClient(
	IF_LoggerClient *	pLogger)
{
	f_mutexLock( gv_hLoggerMutex);
	if (gv_pLogger)
	{
		gv_pLogger->Release();
	}
	if ((gv_pLogger = pLogger) != NULL)
	{
		gv_pLogger->AddRef();
	}
	f_mutexUnlock( gv_hLoggerMutex);
}

