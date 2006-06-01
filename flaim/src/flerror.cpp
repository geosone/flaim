//-------------------------------------------------------------------------
// Desc:	Error routines.
// Tabs:	3
//
//		Copyright (c) 1997-2000,2002-2006 Novell, Inc. All Rights Reserved.
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
// $Id: flerror.cpp 12262 2006-01-19 14:42:10 -0700 (Thu, 19 Jan 2006) dsanders $
//-------------------------------------------------------------------------

#include "flaimsys.h"

// WARNING:	ANY CHANGES MADE TO THE FlmCorruptStrings TABLE MUST BE
// 			REFLECTED IN THE CHECK CODE DEFINES FOUND IN flaim.h

const char * FlmCorruptStrings[ FLM_LAST_CORRUPT_ERROR] =
{
	"OK",										// 0
	"BAD_CHAR",								// 1
	"BAD_ASIAN_CHAR",						// 2
	"BAD_CHAR_SET",						// 3
	"BAD_TEXT_FIELD",						// 4
	"BAD_NUMBER_FIELD",					// 5
	"BAD_CONTEXT_FIELD",					// 6
	"BAD_FIELD_TYPE",						// 7
	"BAD_IX_DEF",							// 8
	"MISSING_REQ_KEY_FIELD",			// 9
	"BAD_TEXT_KEY_COLL_CHAR",			// 10
	"BAD_TEXT_KEY_CASE_MARKER",		// 11
	"BAD_NUMBER_KEY",						// 12
	"BAD_CONTEXT_KEY",					// 13
	"BAD_BINARY_KEY",						// 14
	"BAD_DRN_KEY",							// 15
	"BAD_KEY_FIELD_TYPE",				// 16
	"BAD_KEY_COMPOUND_MARKER",			// 17
	"BAD_KEY_POST_MARKER",				// 18
	"BAD_KEY_POST_BYTE_COUNT",			// 19
	"BAD_KEY_LEN",							// 20
	"BAD_LFH_LIST_PTR",					// 21
	"BAD_LFH_LIST_END",					// 22
	"BAD_PCODE_LIST_END",				// 23
	"BAD_BLK_END",							// 24
	"KEY_COUNT_MISMATCH",				// 25
	"REF_COUNT_MISMATCH",				// 26
	"BAD_CONTAINER_IN_KEY",				// 27
	"BAD_BLK_HDR_ADDR",					// 28
	"BAD_BLK_HDR_LEVEL",					// 29
	"BAD_BLK_HDR_PREV",					// 30
	"BAD_BLK_HDR_NEXT",					// 31
	"BAD_BLK_HDR_TYPE",					// 32
	"BAD_BLK_HDR_ROOT_BIT",				// 33
	"BAD_BLK_HDR_BLK_END",				// 34
	"BAD_BLK_HDR_LF_NUM",				// 35
	"BAD_AVAIL_LIST_END",				// 36
	"BAD_PREV_BLK_NEXT",					// 37
	"BAD_FIRST_ELM_FLAG",				// 38
	"BAD_LAST_ELM_FLAG",					// 39
	"BAD_LEM",								// 40
	"BAD_ELM_LEN",							// 41
	"BAD_ELM_KEY_SIZE",					// 42
	"BAD_ELM_PKC_LEN",					// 43
	"BAD_ELM_KEY_ORDER",					// 44
	"BAD_ELM_KEY_COMPRESS",				// 45
	"BAD_CONT_ELM_KEY",					// 46
	"NON_UNIQUE_FIRST_ELM_KEY",		// 47
	"BAD_ELM_FLD_OVERHEAD",				// 48
	"BAD_ELM_FLD_LEVEL_JUMP",			// 49
	"BAD_ELM_FLD_NUM",					// 50
	"BAD_ELM_FLD_LEN",					// 51
	"BAD_ELM_FLD_TYPE",					// 52
	"BAD_ELM_END",							// 53
	"BAD_PARENT_KEY",						// 54
	"BAD_ELM_DOMAIN_SEN",				// 55
	"BAD_ELM_BASE_SEN",					// 56
	"BAD_ELM_IX_REF",						// 57
	"BAD_ELM_ONE_RUN_SEN",				// 58
	"BAD_ELM_DELTA_SEN",					// 59
	"BAD_ELM_DOMAIN",						// 60
	"BAD_LAST_BLK_NEXT",					// 61
	"BAD_FIELD_PTR",						// 62
	"REBUILD_REC_EXISTS",				// 63
	"REBUILD_KEY_NOT_UNIQUE",			// 64
	"NON_UNIQUE_ELM_KEY_REF",			// 65
	"OLD_VIEW",								// 66
	"COULD_NOT_SYNC_BLK",				// 67
	"IX_REF_REC_NOT_FOUND",				// 68
	"IX_KEY_NOT_FOUND_IN_REC",			// 69
	"DRN_NOT_IN_KEY_REFSET",			// 70
	"BAD_BLK_CHECKSUM",					// 71
	"BAD_LAST_DRN",						// 72
	"BAD_FILE_SIZE",						// 73
	"BAD_AVAIL_BLOCK_COUNT",			// 74
	"BAD_DATE_FIELD",						// 75
	"BAD_TIME_FIELD",						// 76
	"BAD_TMSTAMP_FIELD",					// 77
	"BAD_DATE_KEY",    					// 78
	"BAD_TIME_KEY",  						// 79
	"BAD_TMSTAMP_KEY", 					// 80
	"BAD_BLOB_FIELD",						// 81
	"BAD_PCODE_IXD_TBL",					// 82
	"DICT_REC_ADD_ERR",					// 83
	"FLM_BAD_FIELD_FLAG",				// 84
	"FLM_BAD_FOP",							// 85
};

/****************************************************************************
Desc:	The primary purpose of this function is to provide a way to easily
		trap errors when they occur.  Just put a breakpoint in this function
		to catch them.
Note:	Some of the most common errors will be coded so the use can set a 
		break point.
****************************************************************************/
#ifdef FLM_DEBUG
RCODE flmMakeErr(
	RCODE				rc,
	const char *	pszFile,
	int				iLine,
	FLMBOOL			bAssert)
{
	if( rc == FERR_OK)
	{
		return FERR_OK;
	}
		
	// Switch on warning type return codes
	
	if( rc <= FERR_NOT_FOUND)
	{
		switch(rc)
		{
			case FERR_BOF_HIT:
				break;
			case FERR_EOF_HIT:
				break;
			case FERR_END:
				break;
			case FERR_EXISTS:
				break;
			case FERR_FAILURE:
				break;
			case FERR_NOT_FOUND:
				break;
			default:
				break;
		}
		
		return( rc);
	}

	switch(rc)
	{
		case FERR_DATA_ERROR:
		case FERR_BAD_RFL_PACKET:
			flmAssert( 0);
			flmLogError( rc, "", pszFile, iLine);
			break;
		case FERR_BTREE_ERROR:
			flmLogError( rc, "", pszFile, iLine);
			break;
		case FERR_MEM:
			break;
		case FERR_OLD_VIEW:
			break;
		case FERR_SYNTAX:
			break;
		case FERR_BLOCK_CHECKSUM:
			flmLogError( rc, "", pszFile, iLine);
			break;
		case FERR_CACHE_ERROR:
			flmLogError( rc, "", pszFile, iLine);
			break;
		case FERR_BLOB_MISSING_FILE:
			break;
		case FERR_CONV_BAD_DIGIT:
			break;
		case FERR_NOT_IMPLEMENTED:
			break;
		case FERR_BAD_REFERENCE:
			break;
		case FERR_IO_ACCESS_DENIED:
			break;
		case FERR_IO_PATH_NOT_FOUND:
			break;
		case FERR_UNSUPPORTED_FEATURE:
			break;
		case FERR_ENCRYPTION_UNAVAILABLE:
			break;
		default:
			rc = rc;
			break;
	}

	if( bAssert)
	{
		flmAssert( 0);
	}

	return( rc);
}
#endif

/****************************************************************************
Desc : Returns a pointer to the string representation of a corruption
		 error code.
****************************************************************************/
const char * FlmVerifyErrToStr(
	eCorruptionType	eCorruption)
{
	return( FlmCorruptStrings [eCorruption]);
}

/****************************************************************************
Desc:	Returns a pointer to the ASCII string representation of a FLAIM
		return code.
****************************************************************************/
FLMEXP const char * FLMAPI FlmErrorString(
	RCODE			rc)
{
	const char *	pszStr;

	if( (pszStr = flmErrorString( rc)) == NULL)
	{
		pszStr = "Unknown Error";
	}

	return( pszStr);
}

/****************************************************************************
Desc: Returns a pointer to a static RCODE string or NULL if the RCODE
		cannot be mapped
****************************************************************************/
const char * flmErrorString(
	RCODE			rc)
{
	if( RC_OK( rc))
	{
		return( "FERR_OK");
	}

#define	CASE_RET( c) \
	case c: return( #c )
	
	switch( rc)
	{
		CASE_RET( FERR_BOF_HIT);
		CASE_RET( FERR_EOF_HIT);
		CASE_RET( FERR_END);
		CASE_RET( FERR_EXISTS);
		CASE_RET( FERR_FAILURE);
		CASE_RET( FERR_NOT_FOUND);
		CASE_RET( FERR_BAD_DICT_ID);
		CASE_RET( FERR_BAD_CONTAINER);
		CASE_RET( FERR_NO_ROOT_BLOCK);
		CASE_RET( FERR_BAD_DRN);
		CASE_RET( FERR_BAD_FIELD_NUM);
		CASE_RET( FERR_BAD_FIELD_TYPE);
		CASE_RET( FERR_BAD_HDL);
		CASE_RET( FERR_BAD_IX);
		CASE_RET( FERR_BACKUP_ACTIVE);
		CASE_RET( FERR_SERIAL_NUM_MISMATCH);
		CASE_RET( FERR_BAD_RFL_DB_SERIAL_NUM);
		CASE_RET( FERR_BTREE_ERROR);
		CASE_RET( FERR_BTREE_FULL);
		CASE_RET( FERR_BAD_RFL_FILE_NUMBER);
		CASE_RET( FERR_CANNOT_DEL_ITEM);
		CASE_RET( FERR_CANNOT_MOD_FIELD_TYPE);
		CASE_RET( FERR_CONV_BAD_DEST_TYPE);
		CASE_RET( FERR_CONV_BAD_DIGIT);
		CASE_RET( FERR_CONV_BAD_SRC_TYPE);
		CASE_RET( FERR_RFL_FILE_NOT_FOUND);
		CASE_RET( FERR_CONV_DEST_OVERFLOW);
		CASE_RET( FERR_CONV_ILLEGAL);
		CASE_RET( FERR_CONV_NULL_SRC);
		CASE_RET( FERR_CONV_NULL_DEST);
		CASE_RET( FERR_CONV_NUM_OVERFLOW);
		CASE_RET( FERR_CONV_NUM_UNDERFLOW);
		CASE_RET( FERR_DATA_ERROR);
		CASE_RET( FERR_DD_ERROR);
		CASE_RET( FERR_INVALID_FILE_SEQUENCE);
		CASE_RET( FERR_ILLEGAL_OP);
		CASE_RET( FERR_DUPLICATE_DICT_REC);
		CASE_RET( FERR_CANNOT_CONVERT);
		CASE_RET( FERR_UNSUPPORTED_VERSION);
		CASE_RET( FERR_FILE_ER);
		CASE_RET( FERR_BAD_FIELD_LEVEL);
		CASE_RET( FERR_GED_BAD_RECID);
		CASE_RET( FERR_GED_BAD_VALUE);
		CASE_RET( FERR_GED_MAXLVLNUM);
		CASE_RET( FERR_GED_SKIP_LEVEL);
		CASE_RET( FERR_ILLEGAL_TRANS);
		CASE_RET( FERR_ILLEGAL_TRANS_OP);
		CASE_RET( FERR_INCOMPLETE_LOG);
		CASE_RET( FERR_INVALID_BLOCK_LENGTH);
		CASE_RET( FERR_INVALID_TAG);
		CASE_RET( FERR_KEY_NOT_FOUND);
		CASE_RET( FERR_VALUE_TOO_LARGE);
		CASE_RET( FERR_MEM);
		CASE_RET( FERR_BAD_RFL_SERIAL_NUM);
		CASE_RET( FERR_NEWER_FLAIM);
		CASE_RET( FERR_CANNOT_MOD_FIELD_STATE);
		CASE_RET( FERR_NO_MORE_DRNS);
		CASE_RET( FERR_NO_TRANS_ACTIVE);
		CASE_RET( FERR_NOT_UNIQUE);
		CASE_RET( FERR_NOT_FLAIM);
		CASE_RET( FERR_NULL_RECORD);
		CASE_RET( FERR_NO_HTTP_STACK);
		CASE_RET( FERR_OLD_VIEW);
		CASE_RET( FERR_PCODE_ERROR);
		CASE_RET( FERR_PERMISSION);
		CASE_RET( FERR_SYNTAX);
		CASE_RET( FERR_CALLBACK_FAILURE);
		CASE_RET( FERR_TRANS_ACTIVE);
		CASE_RET( FERR_RFL_TRANS_GAP);
		CASE_RET( FERR_BAD_COLLATED_KEY);
		CASE_RET( FERR_UNSUPPORTED_FEATURE);
		CASE_RET( FERR_MUST_DELETE_INDEXES);
		CASE_RET( FERR_RFL_INCOMPLETE);
		CASE_RET( FERR_CANNOT_RESTORE_RFL_FILES);
		CASE_RET( FERR_INCONSISTENT_BACKUP);
		CASE_RET( FERR_BLOCK_CHECKSUM);
		CASE_RET( FERR_ABORT_TRANS);
		CASE_RET( FERR_NOT_RFL);
		CASE_RET( FERR_BAD_RFL_PACKET);
		CASE_RET( FERR_DATA_PATH_MISMATCH);
		CASE_RET( FERR_HTTP_REGISTER_FAILURE);
		CASE_RET( FERR_HTTP_DEREG_FAILURE);
		CASE_RET( FERR_IX_FAILURE);
		CASE_RET( FERR_HTTP_SYMS_EXIST);
		CASE_RET( FERR_FILE_EXISTS);
		CASE_RET( FERR_SYM_RESOLVE_FAIL);
		CASE_RET( FERR_BAD_SERVER_CONNECTION);
		CASE_RET( FERR_CLOSING_DATABASE);
		CASE_RET( FERR_INVALID_CRC);
		CASE_RET( FERR_KEY_OVERFLOW);
		CASE_RET( FERR_NOT_IMPLEMENTED);
		CASE_RET( FERR_MUTEX_OPERATION_FAILED);
		CASE_RET( FERR_MUTEX_UNABLE_TO_LOCK);
		CASE_RET( FERR_SEM_OPERATION_FAILED);
		CASE_RET( FERR_SEM_UNABLE_TO_LOCK);
		CASE_RET( FERR_BAD_REFERENCE);
		CASE_RET( FERR_UNALLOWED_UPGRADE);
		CASE_RET( FERR_ID_RESERVED);
		CASE_RET( FERR_CANNOT_RESERVE_ID);
		CASE_RET( FERR_DUPLICATE_DICT_NAME);
		CASE_RET( FERR_CANNOT_RESERVE_NAME);
		CASE_RET( FERR_BAD_DICT_DRN);
		CASE_RET( FERR_CANNOT_MOD_DICT_REC_TYPE);
		CASE_RET( FERR_PURGED_FLD_FOUND);
		CASE_RET( FERR_DUPLICATE_INDEX);
		CASE_RET( FERR_TOO_MANY_OPEN_DBS);
		CASE_RET( FERR_ACCESS_DENIED);
		CASE_RET( FERR_CACHE_ERROR);
		CASE_RET( FERR_BLOB_MISSING_FILE);
		CASE_RET( FERR_NO_REC_FOR_KEY);
		CASE_RET( FERR_DB_FULL);
		CASE_RET( FERR_TIMEOUT);
		CASE_RET( FERR_CURSOR_SYNTAX);
		CASE_RET( FERR_THREAD_ERR);
		CASE_RET( FERR_UNIMPORT_SYMBOL);
		CASE_RET( FERR_EMPTY_QUERY);
		CASE_RET( FERR_INDEX_OFFLINE);
		CASE_RET( FERR_TRUNCATED_KEY);
		CASE_RET( FERR_INVALID_PARM);
		CASE_RET( FERR_USER_ABORT);
		CASE_RET( FERR_RFL_DEVICE_FULL);
		CASE_RET( FERR_MUST_WAIT_CHECKPOINT);
		CASE_RET( FERR_NAMED_SEMAPHORE_ERR);
		CASE_RET( FERR_LOAD_LIBRARY);
		CASE_RET( FERR_UNLOAD_LIBRARY);
		CASE_RET( FERR_IMPORT_SYMBOL);
		CASE_RET( FERR_BLOCK_FULL);
		CASE_RET( FERR_BAD_BASE64_ENCODING);
		CASE_RET( FERR_MISSING_FIELD_TYPE);
		CASE_RET( FERR_BAD_DATA_LENGTH);
		CASE_RET( FERR_IO_ACCESS_DENIED);
		CASE_RET( FERR_IO_BAD_FILE_HANDLE);
		CASE_RET( FERR_IO_COPY_ERR);
		CASE_RET( FERR_IO_DISK_FULL);
		CASE_RET( FERR_IO_END_OF_FILE);
		CASE_RET( FERR_IO_OPEN_ERR);
		CASE_RET( FERR_IO_SEEK_ERR);
		CASE_RET( FERR_IO_MODIFY_ERR);
		CASE_RET( FERR_IO_PATH_NOT_FOUND);
		CASE_RET( FERR_IO_TOO_MANY_OPEN_FILES);
		CASE_RET( FERR_IO_PATH_TOO_LONG);
		CASE_RET( FERR_IO_NO_MORE_FILES);
		CASE_RET( FERR_DELETING_FILE);
		CASE_RET( FERR_IO_FILE_LOCK_ERR);
		CASE_RET( FERR_IO_FILE_UNLOCK_ERR);
		CASE_RET( FERR_IO_PATH_CREATE_FAILURE);
		CASE_RET( FERR_IO_RENAME_FAILURE);
		CASE_RET( FERR_IO_INVALID_PASSWORD);
		CASE_RET( FERR_SETTING_UP_FOR_READ);
		CASE_RET( FERR_SETTING_UP_FOR_WRITE);
		CASE_RET( FERR_IO_AT_PATH_ROOT);
		CASE_RET( FERR_INITIALIZING_IO_SYSTEM);
		CASE_RET( FERR_FLUSHING_FILE);
		CASE_RET( FERR_IO_INVALID_PATH);
		CASE_RET( FERR_IO_CONNECT_ERROR);
		CASE_RET( FERR_OPENING_FILE);
		CASE_RET( FERR_DIRECT_OPENING_FILE);
		CASE_RET( FERR_CREATING_FILE);
		CASE_RET( FERR_DIRECT_CREATING_FILE);
		CASE_RET( FERR_READING_FILE);
		CASE_RET( FERR_DIRECT_READING_FILE);
		CASE_RET( FERR_WRITING_FILE);
		CASE_RET( FERR_DIRECT_WRITING_FILE);
		CASE_RET( FERR_POSITIONING_IN_FILE);
		CASE_RET( FERR_GETTING_FILE_SIZE);
		CASE_RET( FERR_TRUNCATING_FILE);
		CASE_RET( FERR_PARSING_FILE_NAME);
		CASE_RET( FERR_CLOSING_FILE);
		CASE_RET( FERR_GETTING_FILE_INFO);
		CASE_RET( FERR_EXPANDING_FILE);
		CASE_RET( FERR_GETTING_FREE_BLOCKS);
		CASE_RET( FERR_CHECKING_FILE_EXISTENCE);
		CASE_RET( FERR_RENAMING_FILE);
		CASE_RET( FERR_SETTING_FILE_INFO);
		CASE_RET( FERR_NICI_CONTEXT);
		CASE_RET( FERR_NICI_FIND_INIT);
		CASE_RET( FERR_NICI_FIND_OBJECT);
		CASE_RET( FERR_NICI_WRAPKEY_NOT_FOUND);
		CASE_RET( FERR_NICI_ATTRIBUTE_VALUE);
		CASE_RET( FERR_NICI_BAD_ATTRIBUTE);
		CASE_RET( FERR_NICI_BAD_RANDOM);
		CASE_RET( FERR_NICI_WRAPKEY_FAILED);
		CASE_RET( FERR_NICI_GENKEY_FAILED);
		CASE_RET( FERR_REQUIRE_PASSWD);
		CASE_RET( FERR_NICI_SHROUDKEY_FAILED);
		CASE_RET( FERR_NICI_UNSHROUDKEY_FAILED);
		CASE_RET( FERR_NICI_UNWRAPKEY_FAILED);
		CASE_RET( FERR_NICI_ENC_INIT_FAILED);
		CASE_RET( FERR_NICI_ENCRYPT_FAILED);
		CASE_RET( FERR_NICI_DECRYPT_INIT_FAILED);
		CASE_RET( FERR_NICI_DECRYPT_FAILED);
		CASE_RET( FERR_NICI_INIT_FAILED);
		CASE_RET( FERR_NICI_KEY_NOT_FOUND);
		CASE_RET( FERR_NICI_INVALID_ALGORITHM);
		CASE_RET( FERR_FLD_NOT_ENCRYPTED);
		CASE_RET( FERR_BAD_ENCDEF_ID);
		CASE_RET( FERR_CANNOT_SET_KEY);
		CASE_RET( FERR_MISSING_ENC_TYPE);
		CASE_RET( FERR_CANNOT_MOD_ENC_TYPE);
		CASE_RET( FERR_MISSING_ENC_KEY);
		CASE_RET( FERR_CANNOT_CHANGE_KEY);
		CASE_RET( FERR_BAD_ENC_KEY);
		CASE_RET( FERR_CANNOT_MOD_ENC_STATE);
		CASE_RET( FERR_DATA_SIZE_MISMATCH);
		CASE_RET( FERR_ENCRYPTION_UNAVAILABLE);
		CASE_RET( FERR_PURGED_ENCDEF_FOUND);
		CASE_RET( FERR_FLD_NOT_DECRYPTED);
		CASE_RET( FERR_PBE_ENCRYPT_FAILED);
		CASE_RET( FERR_DIGEST_FAILED);
		CASE_RET( FERR_DIGEST_INIT_FAILED);
		CASE_RET( FERR_EXTRACT_KEY_FAILED);
		CASE_RET( FERR_INJECT_KEY_FAILED);
		CASE_RET( FERR_PBE_DECRYPT_FAILED);
		CASE_RET( FERR_PASSWD_INVALID);
		CASE_RET( FERR_SVR_NOIP_ADDR);
		CASE_RET( FERR_SVR_SOCK_FAIL);
		CASE_RET( FERR_SVR_CONNECT_FAIL);
		CASE_RET( FERR_SVR_BIND_FAIL);
		CASE_RET( FERR_SVR_LISTEN_FAIL);
		CASE_RET( FERR_SVR_ACCEPT_FAIL);
		CASE_RET( FERR_SVR_SELECT_ERR);
		CASE_RET( FERR_SVR_SOCKOPT_FAIL);
		CASE_RET( FERR_SVR_DISCONNECT);
		CASE_RET( FERR_SVR_READ_FAIL);
		CASE_RET( FERR_SVR_WRT_FAIL);
		CASE_RET( FERR_SVR_READ_TIMEOUT);
		CASE_RET( FERR_SVR_WRT_TIMEOUT);
		CASE_RET( FERR_SVR_ALREADY_CLOSED);
		default:
			return( NULL);
	}
}
