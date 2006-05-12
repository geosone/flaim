//------------------------------------------------------------------------------
// Desc: Rebuild a corrupted database
//
// Tabs:	3
//
//		Copyright (c) 1992-2006 Novell, Inc. All Rights Reserved.
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
// $Id: rebuild.cpp 3129 2006-01-25 11:46:17 -0700 (Wed, 25 Jan 2006) ahodgkinson $
//------------------------------------------------------------------------------

#include "xflaim.h"
#include "sharutil.h"

#define UTIL_ID					"REBUILD"

/* Columns/Rows where things go on the screen. */

#define LABEL_COLUMN    		5
#define VALUE_COLUMN    		35

#define PARAM_ROW					1
#define SOURCE_ROW				(PARAM_ROW + 1)
#define SOURCE_DATA_DIR_ROW	(SOURCE_ROW + 1)
#define DEST_ROW					(SOURCE_DATA_DIR_ROW + 1)
#define DEST_DATA_DIR_ROW		(DEST_ROW + 1)
#define DEST_RFL_ROW				(DEST_DATA_DIR_ROW + 1)
#define DICT_ROW					(DEST_RFL_ROW + 1)
#define CACHE_ROW					(DICT_ROW + 1)
#define LOG_FILE_ROW				(CACHE_ROW + 1)
#define DOING_ROW					(LOG_FILE_ROW + 1)
#define TOTAL_REC_ROW			(DOING_ROW + 1)
#define RECOV_ROW					(TOTAL_REC_ROW + 1)
#define DICT_RECOV_ROW			(RECOV_ROW + 1)
#define DISCARD_ROW				(DICT_RECOV_ROW + 1)

#define MAX_LOG_BUFF       2048

// Local class definitions
class F_LocalRebuildStatus : public IF_DbRebuildStatus
{
public:

	F_LocalRebuildStatus()
	{
	}

	RCODE FLMAPI reportRebuild(
		XFLM_REBUILD_INFO *	pRebuild);
	
	RCODE FLMAPI reportRebuildErr(
		XFLM_CORRUPT_INFO *		pCorruptInfo);

private:

};

// Local function prototypes

FSTATIC FLMBOOL bldDoRebuild( void);

FSTATIC void bldShowResults(
	const char *	pszFuncName,
	RCODE				rc,
	FLMUINT64		ui64TotalNodes,
	FLMUINT64		ui64NodesRecovered,
	FLMUINT64		ui64DictNodesRecovered,
	FLMUINT64		ui64DiscardedDocs);

FSTATIC void bldShowHelp( void);

FSTATIC FLMBOOL bldGetParams(
	FLMINT			iArgC,
	char **			ppszArgV);

FSTATIC FLMBOOL bldParseHdrInfo(
	char *			pszBuffer);

FSTATIC void bldOutLabel(
	FLMUINT			uiCol,
	FLMUINT			uiRow,
	const char *	pszLabel,
	const char *	pszValue,
	FLMUINT64		ui64NumValue,
	FLMBOOL			bLogIt);

FSTATIC void bldLogFlush( void);

FSTATIC void bldLogString(
	const char *	pszStr);

FSTATIC void bldOutValue(
	FLMUINT			uiRow,
	const char *	pszValue);

FSTATIC void bldOutNumValue(
	FLMUINT		uiRow,
	FLMUINT64	ui64Number);

FSTATIC RCODE bldGetUserInput( void);

FSTATIC void bldLogStr(
	FLMUINT			uiIndent,
	const char *	pszStr);

FSTATIC void bldLogCorruptError(
	XFLM_CORRUPT_INFO *	pCorruptInfo);

FSTATIC void bldShowError(
	const char *	pszMessage);

#ifdef FLM_NLM
FLMBOOL						gv_bSynchronized = FALSE;
FSTATIC void bldCleanup( void);
#endif

#ifdef FLM_NLM
#define bldGiveUpCPU()     f_yieldCPU()
#else
#define bldGiveUpCPU()     f_sleep( 0)
#endif

FLMBOOL						gv_bShutdown = FALSE;
static char *				gv_pszLogBuffer = NULL;
static FLMUINT				gv_uiLogBufferCount = 0;
static FLMBOOL				gv_bBatchMode;
static FLMINT				gv_iLastDoing;
static FLMUINT64			gv_ui64BytesDone;
static FLMUINT64			gv_ui64TotalNodes;
static FLMUINT64			gv_ui64NodesRecovered;
static FLMUINT64			gv_ui64DictNodesRecovered;
static FLMUINT64			gv_ui64DiscardedDocs;
static char					gv_szPassword[ 100];
static char					gv_szSrcFileName[ F_PATH_MAX_SIZE];
static char					gv_szSrcDataDir [F_PATH_MAX_SIZE];
static char					gv_szDestFileName[ F_PATH_MAX_SIZE];
static char					gv_szDestDataDir [F_PATH_MAX_SIZE];
static char					gv_szDestRflDir [F_PATH_MAX_SIZE];
static char					gv_szDictFileName[ F_PATH_MAX_SIZE];
static char					gv_szLogFileName[ F_PATH_MAX_SIZE];
static FLMUINT				gv_uiCacheSize = 30000;
static IF_FileHdl *		gv_pLogFile;
static FLMBOOL				gv_bLoggingEnabled;
static char *				gv_pszDictPath;
static XFLM_CREATE_OPTS		gv_DefaultCreateOpts;
static FLMBOOL				gv_bFixHdrInfo;
static FLMBOOL				gv_bRunning;
static FLMBOOL				gv_bPauseBeforeExiting = FALSE;
static IF_DbSystem *		gv_pDbSystem = NULL;


#ifdef FLM_NLM
typedef LONG (* RBLD_VOID_FUNC_p )(void);
#endif

#ifdef FLM_WATCOM_NLM
	#define main		nlm_main
#endif

/********************************************************************
Desc: ?
*********************************************************************/
extern "C" int main(
	int				iArgC,
	char **			ppszArgV)
{
	int				iRetCode = 0;
	F_Pool			logPool;

	logPool.poolInit( 1024);
	gv_bBatchMode = FALSE;
	gv_bRunning = TRUE;

#ifdef FLM_NLM

	// Setup the routines to be called when the NLM exits itself
	
	atexit( bldCleanup);

#endif

	if( RC_BAD( FlmAllocDbSystem( &gv_pDbSystem)))
	{
		goto Exit;
	}

	WpsInit( 0xFFFF, 0xFFFF, "XFLAIM Database Rebuild");
	WpsOptimize();
	
	if (RC_BAD( logPool.poolAlloc( MAX_LOG_BUFF, (void **)&gv_pszLogBuffer)))
	{
		WpsStrOut(
			"\nCould not allocate log buffer\n");
		goto Exit;
	}
	if (bldGetParams( iArgC, (char **)ppszArgV))
	{
		if (!bldDoRebuild())
		{
			iRetCode = 1;
		}
	}
	
Exit:

	if (gv_bPauseBeforeExiting && !gv_bShutdown)
	{
		WpsStrOut( "\nPress any character to exit REBUILD: ");
		for (;;)
		{
			if (gv_bShutdown)
			{
				break;
			}
			if (WpkTestKB())
			{
				(void)WpkIncar();
				break;
			}
			bldGiveUpCPU();
		}
	}
	
	logPool.poolFree();

	WpsExit();
	
	if( gv_pDbSystem)
	{
		gv_pDbSystem->Release();
	}

#ifdef FLM_NLM
	if (!gv_bSynchronized)
	{
		SynchronizeStart();
		gv_bSynchronized = TRUE;
	}
#endif
	gv_bRunning = FALSE;
	return( iRetCode);
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC FLMBOOL bldDoRebuild( void)
{
	FLMBOOL							bOk = TRUE;
	char								szErrMsg[ 100];
	char *							pszDestRflDir;
	RCODE								rc;
	F_LocalRebuildStatus			dbRebuildStatus;
	IF_FileSystem *				pFileSystem = NULL;
	
	if( RC_BAD( rc = FlmGetFileSystem( &pFileSystem)))
	{
		goto Exit;
	}

	gv_ui64BytesDone = 0;
	gv_ui64DictNodesRecovered = 0;
	gv_ui64DiscardedDocs = 0;
	gv_iLastDoing = -1;
	gv_ui64TotalNodes = 0;
	gv_ui64NodesRecovered = 0;

	WpsScrBackFor (FLM_BLACK, FLM_LIGHTGRAY);
	WpsScrClr( 0, 0);

	gv_bLoggingEnabled = FALSE;
	gv_uiLogBufferCount = 0;

	if( gv_szLogFileName[ 0])
	{
		pFileSystem->deleteFile( gv_szLogFileName);
		if (RC_OK( rc = pFileSystem->createFile( gv_szLogFileName, FLM_IO_RDWR,
									&gv_pLogFile)))
		{
			gv_bLoggingEnabled = TRUE;
		}
		else
		{
			f_sprintf( szErrMsg, "Error creating log file: 0x%04X",
				(unsigned)rc);
			bldShowError( szErrMsg);
			bOk = FALSE;
			goto Exit;
		}
	}

	/* Configure FLAIM */

	if (RC_BAD( rc = gv_pDbSystem->setHardMemoryLimit(
								0, FALSE, 0, gv_uiCacheSize * 1024, 0, FALSE)))
	{
		f_sprintf( szErrMsg, "Error setting cache size for FLAIM share: 0x%04X",
				(unsigned)rc);
		bldShowError( szErrMsg);
		bOk = FALSE;
		goto Exit;
	}

	WpsScrBackFor( FLM_BLACK, FLM_WHITE);
	if( gv_bLoggingEnabled)
	{
		bldLogString( NULL);
		bldLogString( NULL);
		bldLogString( NULL);
		bldLogString( 
"==========================================================================");
		bldLogString( "REBUILD PARAMETERS:");
	}
	WpsScrClr( 0, PARAM_ROW);
	WpsStrOutXY( "REBUILD PARAMETERS:", LABEL_COLUMN, PARAM_ROW);
	bldOutLabel( LABEL_COLUMN + 2, SOURCE_ROW, "Source DB",
					 gv_szSrcFileName, 0, TRUE);
	bldOutLabel( LABEL_COLUMN + 2, SOURCE_DATA_DIR_ROW,
		"Src. Data Dir",
		(gv_szSrcDataDir [0])
		? &gv_szSrcDataDir [0]
		: "<NONE>", 0, TRUE);
	bldOutLabel( LABEL_COLUMN + 2, DEST_ROW, "Destination DB",
		gv_szDestFileName, 0, TRUE);
	bldOutLabel( LABEL_COLUMN + 2, DEST_DATA_DIR_ROW,
		"Dest. Data Dir",
		(gv_szDestDataDir [0])
		? &gv_szDestDataDir [0]
		: "<NONE>", 0, TRUE);
	bldOutLabel( LABEL_COLUMN + 2, DEST_RFL_ROW, "Dest. RFL Dir",
		(gv_szDestRflDir [0])
		? &gv_szDestRflDir [0]
		: "<NONE>", 0, TRUE);
	bldOutLabel( LABEL_COLUMN + 2, DICT_ROW, "Dictionary File",
		(gv_szDictFileName [0])
		? &gv_szDictFileName [0]
		: "<NONE>", 0, TRUE);
	bldOutLabel( LABEL_COLUMN + 2, CACHE_ROW, "Cache (kb)", NULL,
		gv_uiCacheSize, TRUE);
	bldOutLabel( LABEL_COLUMN + 2, LOG_FILE_ROW, "Log File",
		(gv_szLogFileName [0])
		? &gv_szLogFileName [0]
		: "<NONE>", 0, TRUE);
	bldOutLabel( LABEL_COLUMN, DOING_ROW, "Current Action",
		"Startup                  ", 0L, FALSE);
	bldOutLabel( LABEL_COLUMN, TOTAL_REC_ROW, "Total Nodes",
		NULL, gv_ui64TotalNodes, FALSE);
	bldOutLabel( LABEL_COLUMN, RECOV_ROW, "Nodes Recovered",
		NULL, gv_ui64NodesRecovered, FALSE);
	bldOutLabel( LABEL_COLUMN, DICT_RECOV_ROW, "Dict Items Recov",
		NULL, gv_ui64DictNodesRecovered, FALSE);
	bldOutLabel( LABEL_COLUMN, DISCARD_ROW, "Discarded Documents",
		NULL, gv_ui64DiscardedDocs, FALSE);

	if( gv_szDictFileName [0])
	{
		gv_pszDictPath = &gv_szDictFileName [0];
	}
	else
	{
		gv_pszDictPath = NULL;
	}

	pszDestRflDir = ((gv_szDestRflDir [0])
										 ? &gv_szDestRflDir [0]
										 : NULL);

	//VISIT: Implement a proper IF_DbRebuildClient!!!
	rc = gv_pDbSystem->dbRebuild( gv_szSrcFileName,
										 gv_szSrcDataDir,
										 gv_szDestFileName,
										 gv_szDestDataDir,
										 pszDestRflDir,
										 gv_pszDictPath,
										 gv_szPassword,
										 NULL,
										 &gv_ui64TotalNodes,
										 &gv_ui64NodesRecovered,
										 &gv_ui64DiscardedDocs,
										 &dbRebuildStatus);

	bldShowResults( "DbRebuild",
						 rc,
						 gv_ui64TotalNodes,
						 gv_ui64NodesRecovered,
						 gv_ui64DictNodesRecovered,
						 gv_ui64DiscardedDocs);

Exit:

	if( gv_bLoggingEnabled)
	{
		bldLogFlush();
		gv_pLogFile->close();
		gv_pLogFile->Release();
		gv_pLogFile = NULL;
	}
	
	if( pFileSystem)
	{
		pFileSystem->Release();
	}

	return( bOk);
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldShowResults(
	const char *	pszFuncName,
	RCODE				rc,
	FLMUINT64		ui64TotalNodes,
	FLMUINT64		ui64NodesRecovered,
	FLMUINT64		ui64DictNodesRecovered,
	FLMUINT64		ui64DiscardedDocs)
{
	char				szErrMsg[ 100];

	if( RC_BAD( rc))
	{
		if( rc != NE_FLM_FAILURE)
		{
			f_strcpy( szErrMsg, "Error calling ");
			f_strcpy( &szErrMsg[ f_strlen( szErrMsg)], pszFuncName);
			f_sprintf( &szErrMsg[ f_strlen( szErrMsg)], ": 0x%04X",
				(unsigned)rc);
			bldShowError( szErrMsg);
			if( gv_bLoggingEnabled)
			{
				bldLogString( szErrMsg);
			}
		}
		else if( gv_bLoggingEnabled)
		{
			bldLogString( "REBUILD HALTED BY USER");
			gv_bShutdown = TRUE;
		}
	}
	else
	{
		bldOutNumValue( TOTAL_REC_ROW, ui64TotalNodes);
		bldOutNumValue( RECOV_ROW, ui64NodesRecovered);
		bldOutNumValue( DICT_RECOV_ROW, ui64DictNodesRecovered);
		bldOutNumValue( DISCARD_ROW, ui64DiscardedDocs);
		if( gv_bLoggingEnabled)
		{
			f_sprintf( szErrMsg, "Total Nodes:      %u", (unsigned)ui64TotalNodes);
			bldLogString( szErrMsg);
			f_sprintf( szErrMsg, "Nodes Recovered:  %u", (unsigned)ui64NodesRecovered);
			bldLogString( szErrMsg);
			f_sprintf( szErrMsg, "Dict Items Recovered: %u",
					(unsigned)ui64DictNodesRecovered);
			f_sprintf( szErrMsg, "Discarded Documents: %u",
					(unsigned)ui64DiscardedDocs);
			bldLogString( szErrMsg);
		}
		f_strcpy( szErrMsg, "Recovery completed successfully");
		bldShowError( szErrMsg);
		if( gv_bLoggingEnabled)
		{
			bldLogString( szErrMsg);
		}
	}
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldShowHelp(
	void
	)
{
	WpsStrOut( "\n");
	WpsStrOut( 
"Parameters: <SourceName> <DestName> [Options]\n\n");
	WpsStrOut( 
"SourceName = Name of database which is to be recovered.\n");
	WpsStrOut( 
"DestName   = Name of destination database to recover data to.  Recovered\n");
	WpsStrOut( 
"             records are put in this database.\n");
	WpsStrOut( 
"Options    = (may be specified anywhere on command line): \n");
	WpsStrOut( 
"  -c<n>         = Cache (kilobytes) to use.\n");
	WpsStrOut( 
"  -sd<DirName>  = Data directory for source DB.\n");
	WpsStrOut( 
"  -dc<DictName> = Dictionary file to use to create destination DB.\n");
	WpsStrOut( 
"  -dd<DirName>  = Data directory for destination DB.\n");
	WpsStrOut( 
"  -dr<DirName>  = RFL directory for destination DB.\n");
	WpsStrOut( 
"  -l<FileName>  = Log detailed information to <FileName>.\n");
	WpsStrOut( 
"  -w<Password>  = Specifies a Security Password to be used.\n");
	WpsStrOut( 
"  -b            = Run in Batch Mode.\n");
	WpsStrOut( 
"  -h<HdrInfo>   = Fix file header information. HdrInfo is in the format\n");
	WpsStrOut( 
"                  BlkSiz:MinRfl:MaxRfl:Lang:FlmVer\n");
	WpsStrOut( 
"  -q<FileName>  = Output binary log information to <FileName>.\n");
	WpsStrOut( 
"  -v<FileName>  = Verify binary log information in <FileName>.  NOTE: The\n");
	WpsStrOut( 
"                  -v and -q options cannot both be specified.\n");
	WpsStrOut( 
"  -p            = Pause before exiting.\n");
#ifdef FLM_NLM
	WpsStrOut( 
"  -w            = Wait to end to synchronize\n");
#endif
	WpsStrOut( 
"  -?           = A '?' anywhere in the command line will cause this help\n");
	WpsStrOut( 
"                 screen to be displayed, with or without the leading '-'.\n");
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC FLMBOOL bldGetParams(
	FLMINT		iArgC,
	char **		ppszArgV)
{
#define MAX_ARGS     30
	FLMUINT		uiLoop;
	char			szErrMsg [100];
	char *		pszPtr;
	char *		ppszArgs[ MAX_ARGS];
	char			szCommandBuffer [300];
#ifdef FLM_NLM
	FLMBOOL		bWaitToSync = FALSE;
#endif

	gv_szSrcFileName [0] = 0;
	gv_szSrcDataDir [0] = 0;
	gv_szDestFileName [0] = 0;
	gv_szDestDataDir [0] = 0;
	gv_szDestRflDir [0] = 0;
	gv_szDictFileName [0] = 0;
	gv_szLogFileName [0] = 0;
	gv_bFixHdrInfo = FALSE;
	gv_DefaultCreateOpts.uiBlockSize = XFLM_DEFAULT_BLKSIZ;
	gv_DefaultCreateOpts.uiMinRflFileSize = XFLM_DEFAULT_MIN_RFL_FILE_SIZE;
	gv_DefaultCreateOpts.uiMaxRflFileSize = XFLM_DEFAULT_MAX_RFL_FILE_SIZE;
	gv_DefaultCreateOpts.bKeepRflFiles = XFLM_DEFAULT_KEEP_RFL_FILES_FLAG;
	gv_DefaultCreateOpts.bLogAbortedTransToRfl = XFLM_DEFAULT_LOG_ABORTED_TRANS_FLAG;
	gv_DefaultCreateOpts.uiDefaultLanguage = XFLM_DEFAULT_LANG;
	gv_DefaultCreateOpts.uiVersionNum = XFLM_CURRENT_VERSION_NUM;
	gv_uiCacheSize = 30000;
	gv_bBatchMode = FALSE;
	gv_szPassword [0] = 0;

	// Ask the user to enter parameters if none were entered on the command
	// line.

	if( iArgC < 2)
	{
		for (;;)
		{
			WpsStrOut( "\nRebuild Params (enter ? for help): ");
			szCommandBuffer[ 0] = 0;
			WpsLineEd( szCommandBuffer, sizeof( szCommandBuffer) - 1,
							&gv_bShutdown);
			if( gv_bShutdown)
			{
				return( FALSE);
			}

			if( f_stricmp( szCommandBuffer, "?") == 0)
			{
				bldShowHelp();
			}
			else
			{
				break;
			}
		}
		flmUtilParseParams( szCommandBuffer, MAX_ARGS, &iArgC, &ppszArgs [1]);
		ppszArgs[ 0] = ppszArgV[ 0];
		iArgC++;
		ppszArgV = &ppszArgs[ 0];
	}

	uiLoop = 1;
	while (uiLoop < (FLMUINT)iArgC)
	{
		pszPtr = ppszArgV [uiLoop];

		// See if they specified an option

#ifdef FLM_UNIX
		if (*pszPtr == '-')
#else
		if (*pszPtr == '-' || *pszPtr == '/')
#endif
		{
			pszPtr++;
			if (*pszPtr == 'c' || *pszPtr == 'C')
			{
				gv_uiCacheSize = f_atoi( pszPtr + 1);
			}
			else if (*pszPtr == 'd' || *pszPtr == 'D')
			{
				pszPtr++;
				if (*pszPtr == 'r' || *pszPtr == 'R')
				{
					pszPtr++;
					if (*pszPtr)
					{
						f_strcpy( gv_szDestRflDir, pszPtr);
					}
					else
					{
						bldShowError(
							"Destination RFL directory not specified");
						return( FALSE);
					}
				}
				else if (*pszPtr == 'd' || *pszPtr == 'D')
				{
					pszPtr++;
					if (*pszPtr)
					{
						f_strcpy( gv_szDestDataDir, pszPtr);
					}
					else
					{
						bldShowError(
							"Destination data directory not specified");
						return( FALSE);
					}
				}
				else if (*pszPtr == 'c' || *pszPtr == 'C')
				{
					pszPtr++;
					if (*pszPtr)
					{
						f_strcpy( gv_szDictFileName, pszPtr);
					}
					else
					{
						bldShowError(
							"Dictionary file name not specified");
						return( FALSE);
					}
				}
				else
				{
					f_sprintf( szErrMsg, "Invalid option %s", pszPtr-1);
					bldShowError( szErrMsg);
					return( FALSE);
				}
			}
			else if (*pszPtr == 's' || *pszPtr == 'S')
			{
				pszPtr++;
				if (*pszPtr == 'd' || *pszPtr == 'D')
				{
					pszPtr++;
					if (*pszPtr)
					{
						f_strcpy( gv_szSrcDataDir, pszPtr);
					}
					else
					{
						bldShowError(
							"Source data directory not specified");
						return( FALSE);
					}
				}
				else
				{
					f_sprintf( szErrMsg, "Invalid option %s", pszPtr-1);
					bldShowError( szErrMsg);
					return( FALSE);
				}
			}
			else if (*pszPtr == 'h' || *pszPtr == 'H')
			{
				pszPtr++;
				if( *pszPtr)
				{
					if( !bldParseHdrInfo( pszPtr))
					{
						return( FALSE);
					}
				}
				else
				{
					bldShowError( "Block sizes not specified");
					return( FALSE);
				}
			}
			else if (*pszPtr == 'l' || *pszPtr == 'L')
			{
				pszPtr++;
				if (*pszPtr)
				{
					f_strcpy( gv_szLogFileName, pszPtr);
				}
				else
				{
					bldShowError( "Log file name not specified");
					return( FALSE);
				}
			}
			else if (*pszPtr == 'w' || *pszPtr == 'W')
			{
				pszPtr++;
				if (*pszPtr)
				{
					f_strcpy( gv_szPassword, pszPtr);
				}
				else
				{
					bldShowError( "Password not specified");
					return( FALSE);
				}
			}
			else if (f_stricmp( pszPtr, "P") == 0)
			{
				gv_bPauseBeforeExiting = TRUE;
			}
			else if (f_stricmp( pszPtr, "B") == 0)
			{
				gv_bBatchMode = TRUE;
			}
#ifdef FLM_NLM
			else if (f_stricmp( pszPtr, "W") == 0)
			{
				bWaitToSync = TRUE;
			}
#endif
			else if (f_stricmp( pszPtr, "?") == 0)
			{
				goto Show_Help;
			}
			else
			{
				f_sprintf( szErrMsg, "Invalid option %s", pszPtr);
				bldShowError( szErrMsg);
				return( FALSE);
			}
		}
		else if (f_stricmp( pszPtr, "?") == 0)
		{
Show_Help:
#ifdef FLM_NLM
			if (!gv_bSynchronized)
			{
				SynchronizeStart();
				gv_bSynchronized = TRUE;
			}
#endif
			bldShowHelp();
			gv_bPauseBeforeExiting = TRUE;
			return( FALSE);
		}
		else if (!gv_szSrcFileName[ 0])
		{
			f_strcpy( gv_szSrcFileName, pszPtr);
		}
		else if (!gv_szDestFileName[ 0])
		{
			f_strcpy( gv_szDestFileName, pszPtr);
		}
		uiLoop++;
	}

#ifdef FLM_NLM
	if (!bWaitToSync && !gv_bSynchronized)
	{
		SynchronizeStart();
		gv_bSynchronized = TRUE;
	}
#endif

	if (!gv_szSrcFileName [0] || !gv_szDestFileName [0])
	{
		goto Show_Help;
	}
	
	return( TRUE);
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC FLMBOOL bldParseHdrInfo(
	char *	pszBuffer)
{
	FLMUINT				uiNum;
	FLMBOOL				bHaveParam;
	XFLM_CREATE_OPTS	CreateOpts;
	FLMUINT				uiFieldNum;

	f_memcpy( &CreateOpts, &gv_DefaultCreateOpts, sizeof( XFLM_CREATE_OPTS));
	uiFieldNum = 1;
	for (;;)
	{
		uiNum = 0;
		bHaveParam = FALSE;
		while ((*pszBuffer == ' ') ||
				 (*pszBuffer == ':') ||
				 (*pszBuffer == ',') ||
				 (*pszBuffer == ';') ||
				 (*pszBuffer == '\t'))
		{
			pszBuffer++;
		}

		if( uiFieldNum == 4)	// Language
		{
			char		szTmpBuf[ 100];
			FLMUINT	uiTmpLen = 0;

			while ((*pszBuffer) &&
					 (*pszBuffer != ':') &&
					 (*pszBuffer != ',') &&
					 (*pszBuffer != ';') &&
					 (*pszBuffer != ' ') &&
					 (*pszBuffer != '\t'))
			{
				szTmpBuf[ uiTmpLen++] = *pszBuffer++;
			}

			szTmpBuf[ uiTmpLen] = 0;
			if( uiTmpLen)
			{
				uiNum = f_languageToNum( szTmpBuf);
				if( (!uiNum) && (f_stricmp( szTmpBuf, "US") != 0))
				{
					bldShowError( "Illegal language in header information");
					return( FALSE);
				}
				bHaveParam = TRUE;
			}
		}
		else
		{
			while( (*pszBuffer >= '0') && (*pszBuffer <= '9'))
			{
				uiNum *= 10;
				uiNum += (FLMUINT)(*pszBuffer - '0');
				pszBuffer++;
				bHaveParam = TRUE;
			}
		}

		if( ((*pszBuffer != 0) &&
			  (*pszBuffer != ' ') &&
			  (*pszBuffer != ':') &&
			  (*pszBuffer != ',') &&
			  (*pszBuffer != ';') &&
			  (*pszBuffer != '\t')))
		{
			bldShowError( "Illegal value in header information");
			return( FALSE);
		}

		if( bHaveParam)
		{
			switch( uiFieldNum)
			{
				case 1:
					if( uiNum != 0 && uiNum != 4096 && uiNum != 8192)
					{
						bldShowError( "Illegal block size");
						return( FALSE);
					}
					CreateOpts.uiBlockSize = uiNum;
					break;
				case 2:
					CreateOpts.uiMinRflFileSize = uiNum;
					break;
				case 3:
					CreateOpts.uiMaxRflFileSize = uiNum;
					break;
				case 4:
					CreateOpts.uiDefaultLanguage = uiNum;
					break;
				case 5:
					CreateOpts.uiVersionNum = uiNum;
					break;
				default:
					bldShowError( "Too many parameters in header information");
					return( FALSE);
			}
		}

		if( !(*pszBuffer))
		{
			break;
		}

		uiFieldNum++;
	}

	gv_bFixHdrInfo = TRUE;
	f_memcpy( &gv_DefaultCreateOpts, &CreateOpts, sizeof( XFLM_CREATE_OPTS));
	return( TRUE);
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldOutLabel(
	FLMUINT			uiCol,
	FLMUINT			uiRow,
	const char *	pszLabel,
	const char *	pszValue,
	FLMUINT64		ui64NumValue,
	FLMBOOL			bLogIt)
{
	char		szMsg[ 100];
	FLMUINT	uiLen = (FLMUINT)(VALUE_COLUMN - uiCol - 1);

	f_memset( szMsg, '.', uiLen);
	szMsg[ uiLen] = 0;
	WpsScrBackFor (FLM_BLACK, FLM_LIGHTGRAY);
	WpsStrOutXY( szMsg, uiCol, uiRow);
	WpsStrOutXY( pszLabel, uiCol, uiRow);

	if( pszValue != NULL)
	{
		bldOutValue( uiRow, pszValue);
	}
	else
	{
		bldOutNumValue( uiRow, ui64NumValue);
	}

	if( (bLogIt) && (gv_bLoggingEnabled))
	{
		f_strcpy( szMsg, pszLabel);
		f_strcpy( &szMsg[ f_strlen( szMsg)], ": ");
		if( pszValue != NULL)
		{
			f_strcpy( &szMsg[ f_strlen( szMsg)], pszValue);
		}
		else
		{
			f_sprintf( (&szMsg[ f_strlen( szMsg)]), "%I64u",
				ui64NumValue);
		}
		bldLogString( szMsg);
	}
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldOutValue(
	FLMUINT			uiRow,
	const char *	pszValue)
{
	WpsScrBackFor (FLM_BLACK, FLM_LIGHTGRAY);
	WpsStrOutXY( pszValue, VALUE_COLUMN, uiRow);
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldOutNumValue(
	FLMUINT		uiRow,
	FLMUINT64	ui64Number
	)
{
	char		szMsg[ 80];

	f_sprintf( szMsg, "%-20I64u  (0x%16I64X)", 
		ui64Number, ui64Number);
	bldOutValue( uiRow, szMsg);
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC RCODE bldGetUserInput(
	void
	)
{
	FLMUINT		uiChar;

	WpsStrOutXY( "Q,ESC=Quit, Other=Continue: ", 0, 23);
	for (;;)
	{
		if( gv_bShutdown)
		{
			uiChar = FKB_ESCAPE;
			break;
		}
		else if( WpkTestKB())
		{
			uiChar = WpkIncar();
			if( uiChar)
			{
				break;
			}
		}
		bldGiveUpCPU();
	}

	WpsScrBackFor (FLM_BLACK, FLM_LIGHTGRAY);
	WpsScrClr( 0, 22);
	switch( uiChar)
	{
		case 'q':
		case 'Q':
		case FKB_ESCAPE:
			return( RC_SET( NE_XFLM_USER_ABORT));
		default:
			break;
	}
	return( NE_XFLM_OK);
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldLogStr(
	FLMUINT			uiIndent,
	const char *	pszStr)
{
	FLMUINT		uiLoop;

	if( gv_bLoggingEnabled)
	{
		for( uiLoop = 0; uiLoop < uiIndent; uiLoop++)
		{
			gv_pszLogBuffer[ gv_uiLogBufferCount++] = ' ';
			if( gv_uiLogBufferCount == MAX_LOG_BUFF)
			{
				bldLogFlush();
			}
		}
		bldLogString( pszStr);
	}
}

/********************************************************************
Desc: Log a corruption error to file.
*********************************************************************/
FSTATIC void bldLogCorruptError(
	XFLM_CORRUPT_INFO *	pCorruptInfo)
{
	char			szBuf[ 100];

	// Log the container number

	bldLogString( NULL);
	bldLogString( "ERROR IN DATABASE");
	f_sprintf( szBuf, "Collection Number: %u",
					(unsigned)pCorruptInfo->uiErrLfNumber);
	bldLogStr( 2, szBuf);

	// Log the block address, if known

	if (pCorruptInfo->uiErrBlkAddress)
	{
		f_sprintf( szBuf, "Block Address: 0x%08X (%u)",
						 (unsigned)pCorruptInfo->uiErrBlkAddress,
						 (unsigned)pCorruptInfo->uiErrBlkAddress);
		bldLogStr( 2, szBuf);
	}

	// Log the parent block address, if known

	if (pCorruptInfo->uiErrParentBlkAddress)
	{
		f_sprintf( szBuf, "Parent Block Address: 0x%08X (%u)",
						 (unsigned)pCorruptInfo->uiErrParentBlkAddress,
						 (unsigned)pCorruptInfo->uiErrParentBlkAddress);
		bldLogStr( 2, szBuf);
	}

	// Log the element offset, if known

	if (pCorruptInfo->uiErrElmOffset)
	{
		f_sprintf( szBuf, "Offset of Element within Block: %u",
						 (unsigned)pCorruptInfo->uiErrElmOffset);
		bldLogStr( 2, szBuf);
	}

	// Log the elment node Id, if known

	if (pCorruptInfo->ui64ErrNodeId)
	{
		f_sprintf( szBuf, "Node Id: %u",
						 (unsigned)pCorruptInfo->ui64ErrNodeId);
		bldLogStr( 2, szBuf);
	}

	// Log the error message

	f_strcpy( szBuf, gv_pDbSystem->checkErrorToStr( pCorruptInfo->iErrCode));
	f_sprintf( (&szBuf [f_strlen( szBuf)]), " (%d)",
		(int)pCorruptInfo->iErrCode);
	bldLogStr( 2, szBuf);
	bldLogStr( 0, NULL);
}

/********************************************************************
Desc: ?
*********************************************************************/

RCODE F_LocalRebuildStatus::reportRebuild(
	XFLM_REBUILD_INFO *	pRebuild)
{
	RCODE		rc = NE_XFLM_OK;

	// First update the display
	if( gv_iLastDoing != pRebuild->iDoingFlag)
	{
		gv_iLastDoing = pRebuild->iDoingFlag;

		if( gv_iLastDoing == REBUILD_GET_BLK_SIZ)
		{
			bldOutValue( DOING_ROW, "Determining Block Size   ");
		}
		else if( gv_iLastDoing == REBUILD_RECOVER_DICT)
		{
			bldOutValue( DOING_ROW, "Recovering Dictionaries  ");
		}
		else
		{
			bldOutValue( DOING_ROW, "Recovering Data          ");
		}
	}

	if( gv_iLastDoing != REBUILD_GET_BLK_SIZ)
	{
		if( gv_ui64TotalNodes != pRebuild->ui64TotNodes)
		{
			gv_ui64TotalNodes = pRebuild->ui64TotNodes;
			bldOutNumValue( TOTAL_REC_ROW, gv_ui64TotalNodes);
		}

		if( gv_iLastDoing == REBUILD_RECOVER_DICT)
		{
			if( gv_ui64DictNodesRecovered != pRebuild->ui64NodesRecov)
			{
				gv_ui64DictNodesRecovered = pRebuild->ui64NodesRecov;
				bldOutNumValue( DICT_RECOV_ROW, gv_ui64DictNodesRecovered);
				gv_ui64DiscardedDocs = pRebuild->ui64DiscardedDocs;
				bldOutNumValue( DISCARD_ROW, gv_ui64DiscardedDocs);
			}
		}
		else
		{
			if( gv_ui64NodesRecovered != pRebuild->ui64NodesRecov)
			{
				gv_ui64NodesRecovered = pRebuild->ui64NodesRecov;
				bldOutNumValue( RECOV_ROW, gv_ui64NodesRecovered);
				gv_ui64DiscardedDocs = pRebuild->ui64DiscardedDocs;
				bldOutNumValue( DISCARD_ROW, gv_ui64DiscardedDocs);
			}
		}
	}

	// See if they pressed an ESC character
	if ((WpkTestKB()) && (WpkIncar() == FKB_ESCAPE))
	{
		WpsScrBackFor (FLM_BLACK, FLM_LIGHTGRAY);
		WpsScrClr( 0, 22);
		WpsScrBackFor (FLM_RED, FLM_WHITE);
		WpsStrOutXY( "ESCAPE key pressed", 0, 22);
		rc = bldGetUserInput();
		goto Exit;
	}
	bldGiveUpCPU();
Exit:
	return( rc);
}

RCODE F_LocalRebuildStatus::reportRebuildErr(
	XFLM_CORRUPT_INFO *			pCorruptInfo)
{
	RCODE		rc = NE_XFLM_OK;

	bldLogCorruptError( pCorruptInfo);

	// See if they pressed an ESC character

	if ((WpkTestKB()) && (WpkIncar() == FKB_ESCAPE))
	{
		WpsScrBackFor (FLM_BLACK, FLM_LIGHTGRAY);
		WpsScrClr( 0, 22);
		WpsScrBackFor (FLM_RED, FLM_WHITE);
		WpsStrOutXY( "ESCAPE key pressed", 0, 22);
		rc = bldGetUserInput();
		goto Exit;
	}

	bldGiveUpCPU();

Exit:

	return( rc);
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldShowError(
	const char * 	pszMessage)
{

	if( !gv_bBatchMode)
	{
		WpsScrBackFor (FLM_BLACK, FLM_LIGHTGRAY);
		WpsScrClr( 0, 22);
		WpsScrBackFor (FLM_RED, FLM_WHITE);
		WpsStrOutXY( pszMessage, 0, 22);
		WpsStrOutXY( "Press any character to continue, ESCAPE to quit: ", 0, 23);
		for (;;)
		{
			if (gv_bShutdown)
				break;
			else if (WpkTestKB())
			{
				if (WpkIncar() == FKB_ESCAPE)
					gv_bShutdown = TRUE;
				break;
			}
			bldGiveUpCPU();
		}
		WpsScrBackFor (FLM_BLACK, FLM_LIGHTGRAY);
		WpsScrClr( 0, 22);
	}
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldLogFlush(
	void
	)
{
	FLMUINT		uiBytesWritten;

	if( gv_uiLogBufferCount)
	{
		gv_pLogFile->write( FLM_IO_CURRENT_POS,
			gv_uiLogBufferCount, gv_pszLogBuffer, &uiBytesWritten);
		gv_uiLogBufferCount = 0;
	}
}

/********************************************************************
Desc: ?
*********************************************************************/
FSTATIC void bldLogString(
	const char *	pszStr)
{
	FLMUINT		uiLen;
	FLMUINT		uiLoop;

	if( (gv_bLoggingEnabled) && (gv_pszLogBuffer != NULL))
	{
		uiLen = (FLMUINT)((pszStr != NULL) ? (FLMUINT)f_strlen( pszStr) : 0);
		for( uiLoop = 0; uiLoop < uiLen; uiLoop++)
		{
			gv_pszLogBuffer[ gv_uiLogBufferCount++] = *pszStr++;
			if( gv_uiLogBufferCount == MAX_LOG_BUFF)
			{
				bldLogFlush();
			}
		}
		gv_pszLogBuffer[ gv_uiLogBufferCount++] = '\r';
		if( gv_uiLogBufferCount == MAX_LOG_BUFF)
		{
			bldLogFlush();
		}
		gv_pszLogBuffer[ gv_uiLogBufferCount++] = '\n';
		if( gv_uiLogBufferCount == MAX_LOG_BUFF)
		{
			bldLogFlush();
		}
	}
}

#ifdef FLM_NLM
/****************************************************************************
Desc: This routine shuts down all threads in the NLM.
****************************************************************************/
FSTATIC void bldCleanup(
	void
	)
{
	gv_bShutdown = TRUE;
	while( gv_bRunning)
	{
		bldGiveUpCPU();
	}
}
#endif
