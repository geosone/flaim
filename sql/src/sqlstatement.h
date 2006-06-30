//------------------------------------------------------------------------------
// Desc:	This file contains SQL statement class.
//
// Tabs:	3
//
//		Copyright (c) 2006 Novell, Inc. All Rights Reserved.
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
// $Id$
//------------------------------------------------------------------------------

#ifndef SQLSTATEMENT_H
#define SQLSTATEMENT_H

// Maximum name length for table names, column names, and index names

#define MAX_SQL_NAME_LEN	128

typedef enum
{
	SQL_PARSE_STATS
} eSQLStatus;

typedef RCODE (* SQL_STATUS_HOOK)(
	eSQLStatus		eStatusType,
	void *			pvArg1,
	void *			pvArg2,
	void *			pvArg3,
	void *			pvUserData);

/*============================================================================
Desc:	SQL statement class.  Parses and executes SQL statements.
============================================================================*/
class SQLStatement : public F_Object
{
public:

#define MAX_SQL_TOKEN_SIZE		80

	SQLStatement();

	virtual ~SQLStatement();

	RCODE setupStatement( void);

	void resetStatement( void);

	RCODE executeSQL(
		IF_IStream *	pStream,
		F_Db *			pDb,
		SQL_STATS *		pSQLStats);

private:

	// Methods

	RCODE getByte(
		FLMBYTE *	pucByte);
		
	FINLINE void ungetByte(
		FLMBYTE	ucByte)
	{
		// Can only unget a single byte.
		
		flmAssert( !m_ucUngetByte);
		m_ucUngetByte = ucByte;
		m_sqlStats.uiChars--;
	}
		
	RCODE getLine( void);
	
	FINLINE FLMBYTE getChar( void)
	{
		if (m_uiCurrLineOffset == m_uiCurrLineBytes)
		{
			return( (FLMBYTE)0);
		}
		else
		{
			FLMBYTE	ucChar = m_pucCurrLineBuf [m_uiCurrLineOffset++];
			return( ucChar);
		}
	}
	
	FINLINE FLMBYTE peekChar( void)
	{
		if (m_uiCurrLineOffset == m_uiCurrLineBytes)
		{
			return( (FLMBYTE)0);
		}
		else
		{
			return( m_pucCurrLineBuf [m_uiCurrLineOffset]);
		}
	}
	
	FINLINE void ungetChar( void)
	{
		
		// There should never be a reason to unget past the beginning of the current
		// line.
		
		flmAssert( m_uiCurrLineOffset);
		m_uiCurrLineOffset--;
	}

	RCODE skipWhitespace(
		FLMBOOL	bRequired);

	RCODE haveToken(
		const char *	pszToken,
		FLMBOOL			bEofOK,
		SQLParseError	eNotHaveErr = SQL_NO_ERROR);

	RCODE getToken(
		char *		pszToken,
		FLMUINT		uiTokenBufSize,
		FLMBOOL		bEofOK,
		FLMUINT *	puiTokenLineOffset);
		
	FINLINE void setErrInfo(
		FLMUINT			uiErrLineNum,
		FLMUINT			uiErrLineOffset,
		SQLParseError	eErrorType,
		FLMUINT			uiErrLineFilePos,
		FLMUINT			uiErrLineBytes)
	{
		m_sqlStats.uiErrLineNum = uiErrLineNum;
		m_sqlStats.uiErrLineOffset = uiErrLineOffset;
		m_sqlStats.eErrorType = eErrorType;
		m_sqlStats.uiErrLineFilePos = uiErrLineFilePos;
		m_sqlStats.uiErrLineBytes = uiErrLineBytes;
	}

	RCODE getUTF8String(
		FLMBOOL		bMustHaveEqual,
		FLMBYTE *	pszStr,
		FLMUINT		uiStrBufSize,
		FLMUINT *	puiStrLen);
		
	RCODE getNumber(
		FLMBOOL		bMustHaveEqual,
		FLMUINT64 *	pui64Num,
		FLMBOOL *	pbNeg,
		FLMBOOL		bNegAllowed);
		
	RCODE getBool(
		FLMBOOL		bMustHaveEqual,
		FLMBOOL *	pbBool);
		
	RCODE getUINT(
		FLMBOOL		bMustHaveEqual,
		FLMUINT *	puiNum);
		
	RCODE getName(
		char *		pszName,
		FLMUINT		uiNameBufSize,
		FLMUINT *	puiNameLen);
		
	RCODE getEncDefName(
		FLMBOOL		bMustExist,
		char *		pszEncDefName,
		FLMUINT		uiEncDefNameBufSize,
		FLMUINT *	puiEncDefNameLen,
		F_ENCDEF **	ppEncDef);

	RCODE getTableName(
		FLMBOOL		bMustExist,
		char *		pszTableName,
		FLMUINT		uiTableNameBufSize,
		FLMUINT *	puiTableNameLen,
		F_TABLE **	ppTable);

	RCODE getIndexName(
		FLMBOOL		bMustExist,
		char *		pszIndexName,
		FLMUINT		uiIndexNameBufSize,
		FLMUINT *	puiIndexNameLen,
		F_INDEX **	ppIndex);
		
	RCODE getStringValue(
		F_COLUMN *			pColumn,
		F_COLUMN_VALUE *	pColumnValue);

	RCODE getNumberValue(
		F_COLUMN_VALUE *	pColumnValue);

	RCODE getBinaryValue(
		F_COLUMN *			pColumn,
		F_COLUMN_VALUE *	pColumnValue);
		
	RCODE getValue(
		F_COLUMN *			pColumn,
		F_COLUMN_VALUE *	pColumnValue);
		
	RCODE insertRow( void);

	RCODE processCreateDatabase( void);
	
	RCODE processOpenDatabase( void);
	
	RCODE processDropDatabase( void);
	
	RCODE getDataType(
		eDataType *	peDataType,
		FLMUINT *	puiMax,
		FLMUINT *	puiEncDefNum);
		
	RCODE processCreateTable( void);
	
	RCODE processDropTable( void);
	
	RCODE processCreateIndex(
		FLMBOOL	bUnique);
	
	RCODE processDropIndex( void);
	
	RCODE processInsertRow( void);
	
	// Data

	F_Db *						m_pDb;
	IF_XML *						m_pXml;
	FLMBYTE						m_ucUngetByte;
	FLMBYTE *					m_pucCurrLineBuf;
	FLMUINT						m_uiCurrLineBufMaxBytes;
	FLMUINT						m_uiCurrLineOffset;
	FLMUINT						m_uiCurrLineNum;
	FLMUINT						m_uiCurrLineFilePos;
	FLMUINT						m_uiCurrLineBytes;
	IF_IStream *				m_pStream;
	FLMUINT						m_uiFlags;
	SQL_STATUS_HOOK			m_fnStatus;
	void *						m_pvCallbackData;
	SQL_STATS					m_sqlStats;
	F_Pool						m_tmpPool;

friend class F_Db;
friend class F_Database;
};

#endif // SQLSTATEMENT_H