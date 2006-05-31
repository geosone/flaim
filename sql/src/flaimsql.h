//------------------------------------------------------------------------------
// Desc:	FLAIM SQL public definitions and interfaces
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

/// \file

#ifndef FLAIMSQL_H
#define FLAIMSQL_H

/// \defgroup errors Error Codes

	/// \defgroup ftk_errors Toolkit Error Codes
	/// \ingroup errors
	
	/// \defgroup flaim_errors FLAIM Error Codes
	/// \ingroup errors
	

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "ftk.h"

// Forward declarations

class F_Db;
class F_Row;

/// Create options for creating a database.
typedef struct SFLM_CREATE_OPTS
{
	FLMUINT		uiBlockSize;				///< Size of blocks in the database.
#define SFLM_DEFAULT_BLKSIZ					4096

	FLMUINT		uiVersionNum;				///< Database version number.
#define SFLM_VER_6_00							600
#define SFLM_CURRENT_VERSION_NUM				SFLM_VER_6_00
#define SFLM_CURRENT_VER_STR					"6.00"

	FLMUINT		uiMinRflFileSize;			///< Minimum bytes per RFL file.
#define SFLM_DEFAULT_MIN_RFL_FILE_SIZE			((FLMUINT)100 * (FLMUINT)1024 * (FLMUINT)1024)
	FLMUINT		uiMaxRflFileSize;			///< Maximum bytes per RFL file.
#define SFLM_DEFAULT_MAX_RFL_FILE_SIZE			SFLM_MAXIMUM_FILE_SIZE
	FLMBOOL		bKeepRflFiles;				///< Keep RFL files?
#define SFLM_DEFAULT_KEEP_RFL_FILES_FLAG		FALSE
	FLMBOOL		bLogAbortedTransToRfl;	///< Log aborted transactions to RFL?
#define SFLM_DEFAULT_LOG_ABORTED_TRANS_FLAG	FALSE

	FLMUINT		uiDefaultLanguage;		///< Default language for database.\  See \ref flm_languages "here" for
													///< valid languages.
#define SFLM_DEFAULT_LANG							FLM_US_LANG

} SFLM_CREATE_OPTS;

/// Database header.
typedef struct SFLM_DB_HDR
{
	// The following items can only be changed by a convert operation, which
	// always converts out to a different file.  A convert is NEVER done
	// "in-place."  Hence, once a database is opened, it can be assumed that
	// these items are stable - they will not change.  This is an important
	// thing for the read transaction, because we do not want to be copying
	// this entire structure on read transactions.  We only want to do that on
	// update transactions.  It will be safe for the read transaction to assume
	// that its copy of these items inside the FDB structure are current.

	FLMBYTE		szSignature[ 8];					// Contains the string "SFLMDB "
#define SFLM_DB_SIGNATURE	"SFLMDB "
	FLMUINT8		ui8IsLittleEndian;				// Non-zero if DB is little-endian
#ifdef FLM_BIG_ENDIAN
	#define SFLM_NATIVE_IS_LITTLE_ENDIAN	0
#else
	#define SFLM_NATIVE_IS_LITTLE_ENDIAN	1
#endif
	FLMUINT8		ui8DefaultLanguage;				///< Default language for the database.
	FLMUINT16	ui16BlockSize;						///< Database block size.

	// The following items can be changed by an update transaction.
	// NOTE: The following items should always be accessed from the database
	// object.  The database object should have a copy of them that it takes
	// at the beginning of the transaction:
	//		ui64CurrTransID
	//		ui32LogicalEOF
	//		ui32FirstAvailBlkAddr	
	// IMPORTANT NOTE: The following items cannot currently be changed
	// during an update transaction:
	//		ui32DbVersion
	//		ui16BlockSize
	//		ui8DefaultLanguage
	//		ui32MaxFileSize
	//		ui32FirstLFBlkAddr
	// This is because they are always accessed from the "last committed"
	// database header, regardless of transaction type.  If we ever want to
	// change them in an update transaction, we will need to copy them
	// into the database object at the beginning of a transaction, and then
	// only access them from within the database object.

	FLMUINT32	ui32DbVersion;					///< Database version.
	FLMUINT8		ui8BlkChkSummingEnabled;	///< Is block checksumming enabled?
	FLMUINT8		ui8RflKeepFiles;				///< Keep RFL files flag - 1=Keep, 0=Dont Keep.
	FLMUINT8		ui8RflAutoTurnOffKeep;		///< Automatically turn off keeping of RFL files when RFL disk is full.\  1=Turn off
														///< 0=Don't automatically turn off.
	FLMUINT8		ui8RflKeepAbortedTrans;		///< Keep aborted transactions in RFL.\  1=Keep, 0=Don't keep.
	FLMUINT32	ui32RflCurrFileNum;			///< Current RFL log file number.
	FLMUINT64	ui64LastRflCommitID;			///< Transaction ID of last transaction that was committed in RFL.
	FLMUINT32	ui32RflLastFileNumDeleted;	///< Last RFL file that was deleted.
	FLMUINT32	ui32RflLastTransOffset;		///< Offset in RFL file where the last committed transaction ends.
	FLMUINT32	ui32RflLastCPFileNum;		///< RFL file number where last check-pointed transaction ends.
	FLMUINT32	ui32RflLastCPOffset;			///< Offset in RFL file where last check-pointed transaction ends.
	FLMUINT64	ui64RflLastCPTransID;		///< Transaction Id of the last check-pointed transaction.
	FLMUINT32	ui32RflMinFileSize;			///< Minimum RFL file size.
	FLMUINT32	ui32RflMaxFileSize;			///< Maximum RFL file size.
	FLMUINT64	ui64CurrTransID;				///< Current update transaction ID.
	FLMUINT64	ui64TransCommitCnt;			///< Total update transactions that have committed.
	FLMUINT32	ui32RblEOF;						///< Offset where the rollback log ends.
	FLMUINT32	ui32RblFirstCPBlkAddr;		///< Block address in rollback log of the first block modified after the last checkpoint.
	FLMUINT32	ui32FirstAvailBlkAddr;		///< Block address of the first block in the avail list.
	FLMUINT32	ui32FirstLFBlkAddr;			///< Block address of the first block in the list of logical file blocks.
	FLMUINT32	ui32LogicalEOF;				///< Address where the last block in the database ends.
	FLMUINT32	ui32MaxFileSize;				///< Maximum file size for database files.
	FLMUINT64	ui64LastBackupTransID;		///< Transaction ID when last backup was taken.
	FLMUINT32	ui32IncBackupSeqNum;			///< Sequence number of last incremental backup that was taken.
	FLMUINT32	ui32BlksChangedSinceBackup;///< Total blocks that have changed since last backup was taken.

#define SFLM_SERIAL_NUM_SIZE				16

	FLMBYTE		ucDbSerialNum[ SFLM_SERIAL_NUM_SIZE];
														///< Database serial number.
	FLMBYTE		ucLastTransRflSerialNum[ SFLM_SERIAL_NUM_SIZE];
														///< Serial number of RFL file that holds the last committed transaction.
	FLMBYTE		ucNextRflSerialNum[ SFLM_SERIAL_NUM_SIZE];
														///< Serial number for the next RFL file that will be created.
	FLMBYTE		ucIncBackupSerialNum[ SFLM_SERIAL_NUM_SIZE];
														///< Serial number for the next incremental backup to be taken.
	FLMUINT32	ui32DbKeyLen;					///< Length of database encryption key.
	
	// IMPORTANT NOTE: If anything is changed in here, need to make
	// corresponding changes to convertDbHdr routine and
	// flmVerifyDiskStructOffsets routine.

	FLMBYTE		ucReserved[ 64];					// Reserved for future
															// Always initialized to zero

	// Checksum should ALWAYS be last

	FLMUINT32	ui32HdrCRC;							///< Checksum for database header.

	// Encryption Key stuff
	
	#define SFLM_MAX_ENC_KEY_SIZE						256

	FLMBYTE			ucDbKey[ SFLM_MAX_ENC_KEY_SIZE];
															///< Database encryption key.
															
	// Offsets of variables in the structure

	#define SFLM_DB_HDR_szSignature_OFFSET							0
	#define SFLM_DB_HDR_ui8IsLittleEndian_OFFSET					8
	#define SFLM_DB_HDR_ui8DefaultLanguage_OFFSET				9
	#define SFLM_DB_HDR_ui16BlockSize_OFFSET						10
	#define SFLM_DB_HDR_ui32DbVersion_OFFSET						12
	#define SFLM_DB_HDR_ui8BlkChkSummingEnabled_OFFSET			16
	#define SFLM_DB_HDR_ui8RflKeepFiles_OFFSET					17
	#define SFLM_DB_HDR_ui8RflAutoTurnOffKeep_OFFSET			18
	#define SFLM_DB_HDR_ui8RflKeepAbortedTrans_OFFSET			19
	#define SFLM_DB_HDR_ui32RflCurrFileNum_OFFSET				20
	#define SFLM_DB_HDR_ui64LastRflCommitID_OFFSET				24
	#define SFLM_DB_HDR_ui32RflLastFileNumDeleted_OFFSET		32
	#define SFLM_DB_HDR_ui32RflLastTransOffset_OFFSET			36
	#define SFLM_DB_HDR_ui32RflLastCPFileNum_OFFSET				40
	#define SFLM_DB_HDR_ui32RflLastCPOffset_OFFSET				44
	#define SFLM_DB_HDR_ui64RflLastCPTransID_OFFSET				48
	#define SFLM_DB_HDR_ui32RflMinFileSize_OFFSET				56
	#define SFLM_DB_HDR_ui32RflMaxFileSize_OFFSET				60
	#define SFLM_DB_HDR_ui64CurrTransID_OFFSET					64
	#define SFLM_DB_HDR_ui64TransCommitCnt_OFFSET				72
	#define SFLM_DB_HDR_ui32RblEOF_OFFSET							80
	#define SFLM_DB_HDR_ui32RblFirstCPBlkAddr_OFFSET			84
	#define SFLM_DB_HDR_ui32FirstAvailBlkAddr_OFFSET			88
	#define SFLM_DB_HDR_ui32FirstLFBlkAddr_OFFSET				92
	#define SFLM_DB_HDR_ui32LogicalEOF_OFFSET						96
	#define SFLM_DB_HDR_ui32MaxFileSize_OFFSET					100
	#define SFLM_DB_HDR_ui64LastBackupTransID_OFFSET			104
	#define SFLM_DB_HDR_ui32IncBackupSeqNum_OFFSET				112
	#define SFLM_DB_HDR_ui32BlksChangedSinceBackup_OFFSET		116
	#define SFLM_DB_HDR_ucDbSerialNum_OFFSET						120
	#define SFLM_DB_HDR_ucLastTransRflSerialNum_OFFSET			136
	#define SFLM_DB_HDR_ucNextRflSerialNum_OFFSET				152
	#define SFLM_DB_HDR_ucIncBackupSerialNum_OFFSET				168
	#define SFLM_DB_HDR_ui32DbKeyLen									184
	#define SFLM_DB_HDR_ucReserved_OFFSET							188
	#define SFLM_DB_HDR_ui32HdrCRC_OFFSET							252
	#define SFLM_DB_HDR_ucDbKey											256
} SFLM_DB_HDR;

// Flags used by openDb method

#define SFLM_DONT_REDO_LOG				0x0001
#define SFLM_DONT_RESUME_THREADS		0x0002
#define SFLM_ALLOW_LIMITED_MODE		0x0004

// Maximum file size

#define SFLM_MAXIMUM_FILE_SIZE		0xFFFC0000

// Maximum key size

#define SFLM_MAX_KEY_SIZE				1024

// Extra retrieval flag - others are defined in ftk.h.  Make sure this bit
// doesn't collide with FLM_EXCL, FLM_INCL, FLM_FIRST, FLM_LAST, FLM_EXACT,
// FLM_KEY_EXACT, etc.

#define FLM_MATCH_ROW_ID				0x0800

/// Transaction types	
typedef enum
{
	SFLM_NO_TRANS = 0,
	SFLM_READ_TRANS,			///< 1 = Read transaction
	SFLM_UPDATE_TRANS			///< 2 = Update transaction
} eDbTransType;

// Transaction flags

#define SFLM_DONT_KILL_TRANS		0x1
#define SFLM_DONT_POISON_CACHE	0x2

/// Backup types.
typedef enum
{
	// These values are stored in the header of the
	// backup, so do not change their values.
	SFLM_FULL_BACKUP = 0,		///< 0 = Full backup.
	SFLM_INCREMENTAL_BACKUP		///< 1 = Incremental backup.
} eDbBackupType;

/// Data types.
typedef enum
{
	SFLM_UNKNOWN_TYPE = 0,
	SFLM_STRING_TYPE,				///< 1 = UTF8 string
	SFLM_NUMBER_TYPE,				///< 2 = 64 bit integer number
	SFLM_BINARY_TYPE,				///< 3 = Binary
	SFLM_NUM_OF_TYPES
} eDataType;

/// Encryption algorithms.
typedef enum
{
	SFLM_AES_ENCRYPTION = 0,	///< 0 = AES encryption
	SFLM_DES3_ENCRYPTION			///< 1 = DES3 encryption
} eEncAlgorithm;

// Supported encryption key sizes

#define SFLM_AES128_KEY_SIZE		128
#define SFLM_AES192_KEY_SIZE		192
#define SFLM_AES256_KEY_SIZE		256
#define SFLM_DES3_168_KEY_SIZE			168

// Maximums for table, index, column, and encryption definition names.

#define MAX_TABLE_NAME_LEN			256
#define MAX_INDEX_NAME_LEN			256
#define MAX_COLUMN_NAME_LEN		256
#define MAX_ENCDEF_NAME_LEN		256

// Predefined table names and numbers

#define SFLM_TBLNUM_ENCDEFS								1
#define SFLM_TBLNAM_ENCDEFS								"flmtbl_encdefs"
#define SFLM_TBLNUM_TABLES									2
#define SFLM_TBLNAM_TABLES									"flmtbl_tables"
#define SFLM_TBLNUM_COLUMNS								3
#define SFLM_TBLNAM_COLUMNS								"flmtbl_columns"
#define SFLM_TBLNUM_INDEXES								4
#define SFLM_TBLNAM_INDEXES								"flmtbl_indexes"
#define SFLM_TBLNUM_INDEX_COMPONENTS					5
#define SFLM_TBLNAM_INDEX_COMPONENTS					"flmtbl_index_components"
#define SFLM_TBLNUM_BLOCK_CHAINS							6
#define SFLM_TBLNAM_BLOCK_CHAINS							"flmtbl_block_chains"

// Predefined index names and numbers

#define SFLM_IXNUM_ENCDEF_NAME							1
#define SFLM_IXNAM_ENCDEF_NAME							"flmix_encdef_name"
#define SFLM_IXNUM_TABLE_NAME								2
#define SFLM_IXNAM_TABLE_NAME								"flmix_table_name"
#define SFLM_IXNUM_TABLE_ENCDEF_NUM						3
#define SFLM_IXNAM_TABLE_ENCDEF_NUM						"flmix_table_encdef_num"
#define SFLM_IXNUM_COLUMN_TABLE_NUM						4
#define SFLM_IXNAM_COLUMN_TABLE_NUM						"flmix_column_table_num"
#define SFLM_IXNUM_COLUMN_ENCDEF_NUM					5
#define SFLM_IXNAM_COLUMN_ENCDEF_NUM					"flmix_column_encdef_num"
#define SFLM_IXNUM_INDEX_NAME								6
#define SFLM_IXNAM_INDEX_NAME								"flmix_index_name"
#define SFLM_IXNUM_INDEX_TABLE_NUM						7
#define SFLM_IXNAM_INDEX_TABLE_NUM						"flmix_index_table_num"
#define SFLM_IXNUM_INDEX_ENCDEF_NUM						8
#define SFLM_IXNAM_INDEX_ENCDEF_NUM						"flmix_index_encdef_num"
#define SFLM_IXNUM_INDEX_COMP_INDEX_NUM				9
#define SFLM_IXNAM_INDEX_COMP_INDEX_NUM				"flmix_index_comp_index_num"

// Column names and numbers for the encryption definitions table

#define SFLM_COLNUM_ENCDEFS_ENCDEF_NAME				1
#define SFLM_COLNAM_ENCDEFS_ENCDEF_NAME				"encdef_name"
#define SFLM_COLNUM_ENCDEFS_ENCDEF_NUM					2
#define SFLM_COLNAM_ENCDEFS_ENCDEF_NUM					"encdef_num"
#define SFLM_COLNUM_ENCDEFS_ENC_ALGORITHM				3
#define SFLM_COLNAM_ENCDEFS_ENC_ALGORITHM				"enc_algorithm"
#define SFLM_COLNUM_ENCDEFS_ENC_KEY_SIZE				4
#define SFLM_COLNAM_ENCDEFS_ENC_KEY_SIZE				"enc_key_size"
#define SFLM_COLNUM_ENCDEFS_ENC_KEY						5
#define SFLM_COLNAM_ENCDEFS_ENC_KEY						"enc_key"
#define SFLM_ENCDEFS_NUM_COLUMNS							5

// Column names and numbers for the tables table

#define SFLM_COLNUM_TABLES_TABLE_NAME					1
#define SFLM_COLNAM_TABLES_TABLE_NAME					"table_name"
#define SFLM_COLNUM_TABLES_TABLE_NUM					2
#define SFLM_COLNAM_TABLES_TABLE_NUM					"table_num"
#define SFLM_COLNUM_TABLES_ENCDEF_NUM					3
#define SFLM_COLNAM_TABLES_ENCDEF_NUM					"encdef_num"
#define SFLM_COLNUM_TABLES_NUM_COLUMNS					4
#define SFLM_COLNAM_TABLES_NUM_COLUMNS					"num_columns"
#define SFLM_TABLES_NUM_COLUMNS							4

// Column names and numbers for the columns table

#define SFLM_COLNUM_COLUMNS_TABLE_NUM					1
#define SFLM_COLNAM_COLUMNS_TABLE_NUM					"table_num"
#define SFLM_COLNUM_COLUMNS_COLUMN_NAME				2
#define SFLM_COLNAM_COLUMNS_COLUMN_NAME				"column_name"
#define SFLM_COLNUM_COLUMNS_COLUMN_NUM					3
#define SFLM_COLNAM_COLUMNS_COLUMN_NUM					"column_num"
#define SFLM_COLNUM_COLUMNS_DATA_TYPE					4
#define SFLM_COLNAM_COLUMNS_DATA_TYPE					"data_type"
#define SFLM_COLNUM_COLUMNS_ENCDEF_NUM					5
#define SFLM_COLNAM_COLUMNS_ENCDEF_NUM					"encdef_num"
#define SFLM_COLNUM_COLUMNS_READ_ONLY					6
#define SFLM_COLNAM_COLUMNS_READ_ONLY					"read_only"
#define SFLM_COLNUM_COLUMNS_NULL_ALLOWED				7
#define SFLM_COLNAM_COLUMNS_NULL_ALLOWED				"null_allowed"
#define SFLM_COLUMNS_NUM_COLUMNS							7

// Column names and numbers for the indexes table

#define SFLM_COLNUM_INDEXES_INDEX_NAME					1
#define SFLM_COLNAM_INDEXES_INDEX_NAME					"index_name"
#define SFLM_COLNUM_INDEXES_INDEX_NUM					2
#define SFLM_COLNAM_INDEXES_INDEX_NUM					"index_num"
#define SFLM_COLNUM_INDEXES_TABLE_NUM					3
#define SFLM_COLNAM_INDEXES_TABLE_NUM					"table_num"
#define SFLM_COLNUM_INDEXES_ENCDEF_NUM					4
#define SFLM_COLNAM_INDEXES_ENCDEF_NUM					"encdef_num"
#define SFLM_COLNUM_INDEXES_LANGUAGE					5
#define SFLM_COLNAM_INDEXES_LANGUAGE					"language"
#define SFLM_COLNUM_INDEXES_NUM_KEY_COMPONENTS		6
#define SFLM_COLNAM_INDEXES_NUM_KEY_COMPONENTS		"num_key_components"
#define SFLM_COLNUM_INDEXES_NUM_DATA_COMPONENTS		7
#define SFLM_COLNAM_INDEXES_NUM_DATA_COMPONENTS		"num_data_components"
#define SFLM_COLNUM_INDEXES_LAST_ROW_INDEXED			8
#define SFLM_COLNAM_INDEXES_LAST_ROW_INDEXED			"last_row_indexed"
#define SFLM_COLNUM_INDEXES_INDEX_STATE				9
#define SFLM_COLNAM_INDEXES_INDEX_STATE				"index_state"
#define SFLM_INDEXES_NUM_COLUMNS							9

// Column names and numbers for the index components table

#define SFLM_COLNUM_INDEX_COMP_INDEX_NUM				1
#define SFLM_COLNAM_INDEX_COMP_INDEX_NUM				"index_num"
#define SFLM_COLNUM_INDEX_COMP_COLUMN_NUM				2
#define SFLM_COLNAM_INDEX_COMP_COLUMN_NUM				"column_num"
#define SFLM_COLNUM_INDEX_COMP_KEY_COMPONENT			3
#define SFLM_COLNAM_INDEX_COMP_KEY_COMPONENT			"key_component"
#define SFLM_COLNUM_INDEX_COMP_DATA_COMPONENT		4
#define SFLM_COLNAM_INDEX_COMP_DATA_COMPONENT		"data_component"
#define SFLM_COLNUM_INDEX_COMP_INDEX_ON				5
#define SFLM_COLNAM_INDEX_COMP_INDEX_ON				"index_on"
#define SFLM_COLNUM_INDEX_COMP_COMPARE_RULES			6
#define SFLM_COLNAM_INDEX_COMP_COMPARE_RULES			"compare_rules"
#define SFLM_COLNUM_INDEX_COMP_SORT_DESCENDING		7
#define SFLM_COLNAM_INDEX_COMP_SORT_DESCENDING		"sort_descending"
#define SFLM_COLNUM_INDEX_COMP_SORT_MISSING_HIGH	8
#define SFLM_COLNAM_INDEX_COMP_SORT_MISSING_HIGH	"sort_missing_high"
#define SFLM_COLNUM_INDEX_COMP_LIMIT					9
#define SFLM_COLNAM_INDEX_COMP_LIMIT					"limit"
#define SFLM_INDEX_COMP_NUM_COLUMNS						9

// Column names and numbers for the block chains table

#define SFLM_COLNUM_BLOCK_CHAINS_BLOCK_ADDRESS		1
#define SFLM_COLNAM_BLOCK_CHAINS_BLOCK_ADDRESS		"block_address"
#define SFLM_BLOCK_CHAINS_NUM_COLUMNS					1

#define SFLM_STRING_OPTION_STR						"string"
#define SFLM_INTEGER_OPTION_STR						"integer"
#define SFLM_BINARY_OPTION_STR						"binary"
#define SFLM_CHECKING_OPTION_STR						"checking"
#define SFLM_PURGE_OPTION_STR							"purge"
#define SFLM_ACTIVE_OPTION_STR						"active"
#define SFLM_INDEX_SUSPENDED_STR						"suspended"
#define SFLM_INDEX_OFFLINE_STR						"offline"
#define SFLM_INDEX_ONLINE_STR							"online"
#define SFLM_CASE_INSENSITIVE_OPTION_STR			"caseinsensitive"
#define SFLM_MINSPACES_OPTION_STR					"minspaces"
#define SFLM_WHITESPACE_AS_SPACE_STR				"whitespaceasspace"
#define SFLM_IGNORE_LEADINGSPACES_OPTION_STR		"ignoreleadingspaces"
#define SFLM_IGNORE_TRAILINGSPACES_OPTION_STR	"ignoretrailingspaces"
#define SFLM_NOUNDERSCORE_OPTION_STR				"nounderscores"
#define SFLM_NOSPACE_OPTION_STR						"nospaces"
#define SFLM_NODASH_OPTION_STR						"nodashes"
#define SFLM_VALUE_OPTION_STR							"value"
#define SFLM_PRESENCE_OPTION_STR						"presence"
#define SFLM_SUBSTRING_OPTION_STR					"substring"
#define SFLM_EACHWORD_OPTION_STR						"eachword"
#define SFLM_ABS_POS_OPTION_STR						"abspos"
#define SFLM_METAPHONE_OPTION_STR					"metaphone"

// Encryption Schemes

#define SFLM_ENC_AES_OPTION_STR						"aes"
#define SFLM_ENC_DES3_OPTION_STR						"des3"

/// Actions that can be returned from an application during database
/// restore operations.
typedef enum
{
	SFLM_RESTORE_ACTION_CONTINUE = 0,	///< 0 = Continue recovery
	SFLM_RESTORE_ACTION_STOP,				///< 1 = Stop recovery
	SFLM_RESTORE_ACTION_SKIP,				///< 2 = Skip operation (future)
	SFLM_RESTORE_ACTION_RETRY				///< 3 = Retry the operation
} eRestoreAction;

/// Event categories an application may subscribe to.
typedef enum
{
	SFLM_EVENT_LOCKS,					///< Lock events.
	SFLM_EVENT_UPDATES,				///< Update events.
	SFLM_MAX_EVENT_CATEGORIES		// Should always be last.
} eEventCategory;

/// Events an application may receive.
typedef enum
{
	SFLM_EVENT_LOCK_WAITING,			///< 0 = Thread is waiting to obtain database lock.
	SFLM_EVENT_LOCK_GRANTED,			///< 1 = Thread was granted database lock.
	SFLM_EVENT_LOCK_SUSPENDED,			///< 2 = Thread is suspended waiting to obtain database lock.
	SFLM_EVENT_LOCK_RESUMED,			///< 3 = Thread was granted database lock and resumed.
	SFLM_EVENT_LOCK_RELEASED,			///< 4 = Thread that had the database lock released it.
	SFLM_EVENT_LOCK_TIMEOUT,			///< 5 = Thread that was waiting for a database lock timed out.
	SFLM_EVENT_BEGIN_TRANS,				///< 6 = Transaction was started.
	SFLM_EVENT_COMMIT_TRANS,			///< 7 = Transaction was committed.
	SFLM_EVENT_ABORT_TRANS,				///< 8 = Transaction was aborted.
	SFLM_EVENT_CREATE_TABLE,			///< 9 = New table created.
	SFLM_EVENT_CREATE_INDEX,			///< 10 = New index created.
	SFLM_EVENT_DROP_TABLE,				///< 11 = Table dropped.
	SFLM_EVENT_DROP_INDEX,				///< 12 = Index dropped.
	SFLM_EVENT_INSERT_ROW,				///< 13 = Row inserted into table.
	SFLM_EVENT_DELETE_ROW,				///< 14 = Row deleted from table.
	SFLM_EVENT_MODIFY_ROW,				///< 15 = Row modified in table.
	SFLM_EVENT_INDEXING_PROGRESS,		///< 16 = Background indexing progress.
	SFLM_MAX_EVENT_TYPES					// Should always be last.
} eEventType;

/// Types of logical files in the database.
typedef enum
{
	SFLM_LF_INVALID = 0,		///< 0 = Invalid
	SFLM_LF_TABLE,				///< 1 = Table
	SFLM_LF_INDEX				///< 2 = Index
} eLFileType;

/// Types of messages logged by the database system.
typedef enum
{
	SFLM_QUERY_MESSAGE,			///< Query
	SFLM_TRANSACTION_MESSAGE,	///< Transaction
	SFLM_GENERAL_MESSAGE,		///< General messages
	SFLM_NUM_MESSAGE_TYPES
} eLogMessageType;

/// Reasons the checkpoint thread forces a checkpoint.	
typedef enum
{
	SFLM_CP_NO_REASON = 0,
	SFLM_CP_TIME_INTERVAL_REASON = 1,	///< Maximum time interval since last completed checkpoint reached.
	SFLM_CP_SHUTTING_DOWN_REASON,			///< Database is shutting down.
	SFLM_CP_RFL_VOLUME_PROBLEM				///< Had a problem writing to the RFL disk volume.
} eForceCPReason;

/// Structure that returns information about the current activity of the
/// checkpoint thread.
typedef struct
{
	FLMBOOL			bRunning;								///< Is checkpoint thread currently running?
	FLMUINT			uiRunningTime;							///< Milliseconds checkpoint thread has been running.
	FLMBOOL			bForcingCheckpoint;					///< Is the checkpoint currently forcing a checkpoint?
	FLMUINT			uiForceCheckpointRunningTime;		///< Milliseconds checkpoint thread has been in the forcing checkpoint state.
	eForceCPReason	eForceCheckpointReason;			///< Reason checkpoint is being forced.
	FLMBOOL			bWritingDataBlocks;					///< Is the checkpoint thread currently writing out data blocks?
	FLMUINT			uiLogBlocksWritten;					///< Total blocks written to the rollback log.
	FLMUINT			uiDataBlocksWritten;					///< Total data blocks written.
	FLMUINT			uiDirtyCacheBytes;					///< Total bytes in cache that are dirty data blocks.
	FLMUINT			uiBlockSize;							///< Database block size.
	FLMUINT			uiWaitTruncateTime;					///< Milliseconds checkpoint thread has been waiting to truncate the rollback log. 
} SFLM_CHECKPOINT_INFO;

/// Structure for reporting cache usage and statistics.
typedef struct
{
	FLMUINT				uiByteCount;					///< Total bytes used in cache.
	FLMUINT				uiCount;							///< Total objects (blocks or rows) cached.
	FLMUINT				uiOldVerBytes;					///< Total bytes allocated to old versions of objects (blocks or rows).
	FLMUINT				uiOldVerCount;					///< Total objects (blocks or rows) in cache that are old versions. 
	FLMUINT				uiCacheHits;					///< Total cache hits.
	FLMUINT				uiCacheHitLooks;				///< Total objects (blocks or rows) examined to get cache hits.
	FLMUINT				uiCacheFaults;					///< Total cache faults.
	FLMUINT				uiCacheFaultLooks;			///< Total objects (blocks or rows) examined to get cache faults.
	FLM_SLAB_USAGE		slabUsage;						///< Slab usage for this particular cache.
} SFLM_CACHE_USAGE;

/// Structure used to return cache information and statistics.
typedef struct
{
	FLMUINT				uiMaxBytes;						///< Current cache maximum (in bytes).
	FLMUINT				uiTotalBytesAllocated;		///< Total cache bytes allocated.
	FLMBOOL				bDynamicCacheAdjust;			///< Is cache maximum being dynamically adjusted?
	FLMUINT				uiCacheAdjustPercent;		///< Cache adjust percent - only valid if bDynamicCacheAdjust is TRUE.
	FLMUINT				uiCacheAdjustMin;				///< Cache adjust minimum (bytes) - only valid if bDynamicCacheAdjust is TRUE.
	FLMUINT				uiCacheAdjustMax;				///< Cache adjust maximum (bytes) - only valid if bDynamicCacheAdjust is TRUE.
	FLMUINT				uiCacheAdjustMinToLeave;	///< Cache adjust minimum to leave (bytes) - only valid if bDynamicCacheAdjust is TRUE.
	FLMUINT				uiDirtyCount;					///< Blocks in block cache that are dirty.
	FLMUINT				uiDirtyBytes;					///< Total bytes in block cache that are dirty.
	FLMUINT				uiNewCount;						///< Blocks in block cache that are "new" - meaning they are newly created blocks that
																///< are beyond what was the database EOF at the beginning of the current update
																///< transaction. 
	FLMUINT				uiNewBytes;						///< Total bytes in block cache that are "new" blocks (see uiNewCount).
	FLMUINT				uiLogCount;						///< Total blocks in cache that need to be written to the rollback log.
	FLMUINT				uiLogBytes;						///< Total bytes in block cache that need to be written to the rollback log.
	FLMUINT				uiFreeCount;					///< Total blocks in block cache that are free blocks - not associated with any
																///< database.
	FLMUINT				uiFreeBytes;					///< Total bytes in block cache that are free blocks - not associated with any database.
	FLMUINT				uiReplaceableCount;			///< Total blocks in block cache that can be replaced.\  These are blocks that aren't
																///< flagged in some way that would prevent them from being replaced in cache.
	FLMUINT				uiReplaceableBytes;			///< Total bytes in replaceable blocks (see uiReplaceableCount).
	FLMBOOL				bPreallocatedCache;			///< Was cache pre-allocated?
	SFLM_CACHE_USAGE	BlockCache;						///< Information about block cache usage.
	SFLM_CACHE_USAGE	RowCache;						///< Information about row cache usage.			
} SFLM_CACHE_INFO;

/// Structure used in gathering statistics to hold a operation count, a byte count, and an elapsed time.  This
/// is typically used for I/O operations where it is useful to know the number of bytes that were read or
/// written by the operation.
typedef struct
{
	FLMUINT64	ui64Count;			///< Number of times operation was performed.
	FLMUINT64	ui64TotalBytes;	///< Total number of bytes involved in the operations.\  This usually represents
											///< bytes read from or written to disk.
	FLMUINT64	ui64ElapMilli;		///< Total elapsed time (milliseconds) for the operations.
} SFLM_DISKIO_STAT;

/// Statistics for read transactions.
typedef struct
{
	F_COUNT_TIME_STAT	CommittedTrans;	///< Statistics for read transactions committed.
	F_COUNT_TIME_STAT	AbortedTrans;		///< Statistics for read transactions aborted.
} SFLM_RTRANS_STATS;

/// Statistics for update transactions.
typedef struct
{
	F_COUNT_TIME_STAT	CommittedTrans;	///< Statistics for update transactions committed.
	F_COUNT_TIME_STAT	GroupCompletes;	///< Statistics for number of times multiple transactions were committed together.
	FLMUINT64			ui64GroupFinished;///< Total update transactions that were committed in a group.
	F_COUNT_TIME_STAT	AbortedTrans;		///< Statistics for update transactions aborted.
} SFLM_UTRANS_STATS;

/// Statistics for block reads and writes.
typedef struct
{
	SFLM_DISKIO_STAT	BlockReads;						///< Statistics on block reads.
	SFLM_DISKIO_STAT	OldViewBlockReads;			///< Statistics on block reads that resulted in an old view error.
	FLMUINT				uiBlockChkErrs;				///< Number of times we had errors checking the block after it
																///< was read in - either checksum errors or other problems
																///< validating data in the block.
	FLMUINT				uiOldViewBlockChkErrs;		///< Number of times we had errors checking the block after it
																///< was read in - either checksum errors or other problems
																///< validating data in the block.\  This statistic is for
																///< older versions of a block as opposed to the current version.
	FLMUINT				uiOldViewErrors;				///< Number of times we had an old view error when reading blocks.
	SFLM_DISKIO_STAT	BlockWrites;					///< Statistics on Block writes.
} SFLM_BLOCKIO_STATS;

/// Statistics gathered for a particular logical file (index or table).
typedef struct
{
	FLMBOOL					bHaveStats;						///< Flag indicating whether or not there are statistics
																	///< for this logical file.
	FLMUINT					uiLFileNum;						///< Logical file number.
	eLFileType				eLfType;							///< Type of logical file.
	SFLM_BLOCKIO_STATS	RootBlockStats;				///< Block I/O statistics for the logical file's root blocks.
	SFLM_BLOCKIO_STATS	MiddleBlockStats;				///< Block I/O statistics for for the blocks in the logical file that are not
																	///< root blocks or leaf blocks.
	SFLM_BLOCKIO_STATS	LeafBlockStats;				///< Block I/O statistics for the logical file's leaf blocks.
	FLMUINT64				ui64BlockSplits;				///< Number of block splits that have occurred in this logical file.
	FLMUINT64				ui64BlockCombines;			///< Number of block combines that have occurred in this logical file.
} SFLM_LFILE_STATS;

/// Database statistics.
typedef struct
{
	const char *			pszDbName;					///< Name of database these statistics are for.
	FLMBOOL					bHaveStats;					///< Flag indicating whether or not there are statistics for this database.
	SFLM_RTRANS_STATS		ReadTransStats;			///< Read transaction statistics for the database.
	SFLM_UTRANS_STATS		UpdateTransStats;			///< Update transaction statistics for the database.
	FLMUINT					uiLFileAllocSeq;			///< Allocation sequence number for pLFileStats array - used internally.
	SFLM_LFILE_STATS *	pLFileStats;				///< Logical file statistics for this database.
	FLMUINT					uiLFileStatArraySize;	///< Number of logical files in the pLFileStats array - used internally.
	FLMUINT					uiNumLFileStats;			///< Number of elements in the pLFileStats array currently in use.
	SFLM_BLOCKIO_STATS	LFHBlockStats;				///< Block I/O statistics for LFH blocks.
	SFLM_BLOCKIO_STATS	AvailBlockStats;			///< Block I/O statistics for AVAIL blocks.

	// Write statistics

	SFLM_DISKIO_STAT		DbHdrWrites;				///< Statistics for writes to the database header.
	SFLM_DISKIO_STAT		LogBlockWrites;			///< Statistics for writes of blocks to the rollback log.
	SFLM_DISKIO_STAT		LogBlockRestores;			///< Statistics for writing of blocks from the rollback log back into data files (done
																///< during database recovery or when aborting a transaction).
	// Read statistics

	SFLM_DISKIO_STAT		LogBlockReads;				///< Statistics on reading blocks from the rollback log.
	FLMUINT					uiLogBlockChkErrs;		///< Number of times we had checksum errors reading blocks from the rollback log.
	FLMUINT					uiReadErrors;				///< Number of times we got read errors.
	FLMUINT					uiWriteErrors;				///< Number of times we got write errors.

	// Lock statistics

	F_LOCK_STATS			LockStats;					///< Lock statistics.
	
	// Update statistics

	F_COUNT_TIME_STAT		RowInserts;					///< Number of row insert operations that have been performed on
																///< this database.
	F_COUNT_TIME_STAT		RowDeletes;					///< Number of row delete operations that have been performed
																///< on this database.
	F_COUNT_TIME_STAT		RowModifies;				///< Number of row modify operations that have been performed
																///< on this database.
} SFLM_DB_STATS;

/// Structure for return various database statistics.
typedef struct
{
	SFLM_DB_STATS *	pDbStats;				///< Pointer to array of database statistics.\  There will be an ::SFLM_DB_STATS
														///< structure for every database currently open.
	FLMUINT				uiDBAllocSeq;			///< Allocation sequence number for the pDbStats array - only used internally.
	FLMUINT				uiDbStatArraySize;	///< Size of the pDbStats array.
	FLMUINT				uiNumDbStats;			///< Number of elements in the pDbStats array that are currently in use.
	FLMBOOL				bCollectingStats;		///< Flag indicating whether or not we are currently collecting statistics.
	FLMUINT				uiStartTime;			///< Time we started collecting statistics.\  This is GMT time - seconds since
														///< January 1, 1970 midnight.
	FLMUINT				uiStopTime;				///< Time we stopped collecting statistics.\  This is GMT time - seconds since
														///< January 1, 1970 midnight.
} SFLM_STATS;

/// Index states.
typedef enum
{
	SFLM_INDEX_ONLINE = 0,				///< 0 = Index is on-line
	SFLM_INDEX_BRINGING_ONLINE,		///< 1 = Index is currently being brought on-line
	SFLM_INDEX_SUSPENDED					///< 2 = Index is suspended
} eSFlmIndexState;

/// Structure for returning information about an index.
typedef struct
{
	FLMUINT				uiIndexNum;					///< Index number
	eSFlmIndexState	eState;						///< Current index state.
	
	// Statistics when eState is SFLM_INDEX_BRINGING_ONLINE
	
	FLMUINT				uiStartTime;				///< Time when index started to be brought on-line.\  Only valid if the state is
															///< eSflmIndexState::SFLM_INDEX_BRINGING_ONLINE.
	FLMUINT64			ui64LastRowIndexed;		///< If ~0 then index is online, otherwises this is the row ID of the last row
															///< that was indexed.
	FLMUINT64			ui64KeysProcessed;		///< Total keys processed by the background indexing thread.
	FLMUINT64			ui64RowsProcessed;		///< total rows indexed by the background indexing thread.
	FLMUINT64			ui64Transactions;			///< Total transactions performed by the background indexing thread.
} SFLM_INDEX_STATUS;

/// Actions that the database rebuild operation performs.
typedef enum
{
	REBUILD_GET_BLK_SIZ = 0,		///< 0 = Rebuild is making a pass through the database to try and figure out the block size.
	REBUILD_RECOVER_DICT,			///< 1 = Rebuild is recovering dictionary information.
	REBUILD_RECOVER_DATA				///< 2 = Rebuild is recovering table rows.
} eRebuildActions;

/// Structure passed to the application during database rebuild operations.
typedef struct
{
	eRebuildActions	eAction;					///< Current action being taken by the database rebuild operation.
	FLMBOOL				bStartFlag;				///< Flag indicating if we are just starting the current operation.
	FLMUINT64			ui64DatabaseSize;		///< Total database size (bytes).		
	FLMUINT64			ui64BytesExamined;	///< Total bytes examined so far.
	FLMUINT64			ui64TotalRows;			///< Total rows examined so far.
	FLMUINT64			ui64RowsRecov;			///< Total rows recovered so far.
} SFLM_REBUILD_INFO;

/// Types of corruption
typedef enum
{
	
	// WARNING:	ANY CHANGES MADE TO THE CHECK ERROR CODE DEFINES MUST BE
	// REFLECTED IN THE FlmCorruptStrings TABLE FOUND IN flerrstr.cpp
	
	SFLM_NO_CORRUPTION = 0,
	SFLM_BAD_CHAR,									///< 1
	SFLM_BAD_ASIAN_CHAR,							///< 2
	SFLM_BAD_CHAR_SET,							///< 3
	SFLM_BAD_TEXT_FIELD,							///< 4
	SFLM_BAD_NUMBER_FIELD,						///< 5
	SFLM_BAD_FIELD_TYPE,							///< 6
	SFLM_BAD_IX_DEF,								///< 7
	SFLM_BAD_NUMBER_KEY,							///< 8
	SFLM_BAD_BINARY_KEY,							///< 9
	SFLM_BAD_KEY_FIELD_TYPE,					///< 10
	SFLM_BAD_KEY_LEN,								///< 11
	SFLM_BAD_LFH_LIST_PTR,						///< 12
	SFLM_BAD_LFH_LIST_END,						///< 13
	SFLM_BAD_BLK_END,								///< 14
	SFLM_KEY_COUNT_MISMATCH,					///< 15
	SFLM_REF_COUNT_MISMATCH,					///< 16
	SFLM_BAD_BLK_HDR_ADDR,						///< 17
	SFLM_BAD_BLK_HDR_LEVEL,						///< 18
	SFLM_BAD_BLK_HDR_PREV,						///< 19

	// WARNING:	ANY CHANGES MADE TO THE CHECK ERROR CODE DEFINES MUST BE
	// REFLECTED IN THE FlmCorruptStrings TABLE FOUND IN flerrstr.cpp

	SFLM_BAD_BLK_HDR_NEXT,						///< 20
	SFLM_BAD_BLK_HDR_TYPE,						///< 21
	SFLM_BAD_BLK_HDR_ROOT_BIT,					///< 22
	SFLM_BAD_BLK_HDR_BLK_END,					///< 23
	SFLM_BAD_BLK_HDR_LF_NUM,					///< 24
	SFLM_BAD_AVAIL_LIST_END,					///< 25
	SFLM_BAD_PREV_BLK_NEXT,						///< 26
	SFLM_BAD_FIRST_ELM_FLAG,					///< 27
	SFLM_BAD_LEM,									///< 28
	SFLM_BAD_ELM_LEN,								///< 29
	SFLM_BAD_ELM_KEY_SIZE,						///< 30
	SFLM_BAD_ELM_KEY,								///< 31
	SFLM_BAD_ELM_KEY_ORDER,						///< 32
	SFLM_BAD_CONT_ELM_KEY,						///< 34
	SFLM_NON_UNIQUE_FIRST_ELM_KEY,			///< 35
	SFLM_BAD_ELM_OFFSET,							///< 36
	SFLM_BAD_ELM_INVALID_LEVEL,				///< 37
	SFLM_BAD_ELM_FLD_NUM,						///< 38
	SFLM_BAD_ELM_FLD_LEN,						///< 39

	// WARNING:	ANY CHANGES MADE TO THE CHECK ERROR CODE DEFINES MUST BE
	// REFLECTED IN THE FlmCorruptStrings TABLE FOUND IN flerrstr.cpp

	SFLM_BAD_ELM_FLD_TYPE,						///< 40
	SFLM_BAD_ELM_END,								///< 41
	SFLM_BAD_PARENT_KEY,							///< 42
	SFLM_BAD_ELM_IX_REF,							///< 43
	SFLM_BAD_LAST_BLK_NEXT,						///< 44
	SFLM_REBUILD_REC_EXISTS,					///< 45
	SFLM_REBUILD_KEY_NOT_UNIQUE,				///< 46
	SFLM_NON_UNIQUE_ELM_KEY_REF,				///< 47
	SFLM_OLD_VIEW,									///< 48
	SFLM_COULD_NOT_SYNC_BLK,					///< 49
	SFLM_IX_KEY_NOT_FOUND_IN_REC,				///< 50
	SFLM_KEY_NOT_IN_KEY_REFSET,				///< 51
	SFLM_BAD_BLK_CHECKSUM,						///< 52
	SFLM_BAD_FILE_SIZE,							///< 53
	SFLM_BAD_BLK_TYPE,							///< 54
	SFLM_BAD_ELEMENT_CHAIN,						///< 55
	SFLM_BAD_ELM_EXTRA_DATA,					///< 56
	SFLM_BAD_BLOCK_STRUCTURE,					///< 57
	SFLM_BAD_DATA_BLOCK_COUNT,					///< 58
	SFLM_BAD_AVAIL_SIZE,							///< 59
	SFLM_BAD_CHILD_ELM_COUNT,					///< 60
	SFLM_NUM_CORRUPT_ERRORS
} eCorruptionType;

/// Locations in the database where corruption are reported.
typedef enum
{
	SFLM_LOCALE_NONE = 0,						///< 0 = None
	SFLM_LOCALE_LFH_LIST,						///< 1 = Corruption was found in logical file block list.
	SFLM_LOCALE_AVAIL_LIST,						///< 2 = Corruption was found in the avail block list.
	SFLM_LOCALE_B_TREE,							///< 3 = Corruption was found in an index or table b-tree.
	SFLM_LOCALE_INDEX								///< 4 = Corruption is a logical corruption of an index.
} eErrorLocation;

/// Structure for reporting a corruption when checking a database.
typedef struct
{
	eCorruptionType	eCorruption;			///< Type of corruption that was found.
	eErrorLocation		eErrLocale;				///< Location of the corruption.
	FLMUINT				uiErrLfNumber;			///< Logical file number where corruption occurred, if eErrLocal == eErrorLocation::SFLM_LOCALE_B_TREE.
	eLFileType			eErrLfType;				///< Logical file type, if eErrLocal == eErrorLocation::SFLM_LOCALE_B_TREE.
	FLMUINT				uiErrBTreeLevel;		///< Level in b-tree where corruption occurred.\  Zero if not applicable.
	FLMUINT				uiErrBlkAddress;		///< Address of block that had the corruption.\  Zero if not applicable.
	FLMUINT				uiErrParentBlkAddress;	///< Address of the parent block to the block that had the corruption.
	FLMUINT				uiErrElmOffset;		///< Offset of element in the block where the corruption was found.\  Zero if not applicable.
	FLMUINT64			ui64ErrRowId;			///< Row ID where corruption was detected.\  Zero if not applicable.
} SFLM_CORRUPT_INFO;

/// Phases that the database check operation goes through.
typedef enum
{
	SFLM_CHECK_LFH_BLOCKS = 1,					///< 1 = Checking logical file block list.
	SFLM_CHECK_B_TREE,							///< 2 = Checking physical structure of b-trees.
	SFLM_CHECK_AVAIL_BLOCKS,					///< 3 = Checking the available block list.
	SFLM_CHECK_RS_SORT							///< 4 = Sorting index keys result set.
} eCheckPhase;

/// Structure for reporting the status and progress of a database check operation.
typedef struct
{
	eCheckPhase		ePhase;						///< Current phase the database check operation is currently in.
	FLMBOOL			bStartFlag;					///< Is the current check phase just starting?
	FLMUINT64		ui64DatabaseSize;			///< Database size (in bytes).		
	FLMUINT64		ui64BytesExamined;		///< Total bytes checked so far in the database.
	FLMUINT			uiNumLFs;					///< Total logical files (tables and indexes) in the database.
	FLMUINT			uiCurrLF;					///< Count of the logical file (table or index) currently being checked.
	FLMUINT			uiLfNumber;					///< Logical file number (table or index) currently being checked.
	eLFileType		eLfType;						///< Type of logical file currently being checked.		
	FLMUINT			uiNumProblemsFixed;		///< Total number of corruptions fixed.

	// Index check progress

	FLMUINT64		ui64NumKeys;				///< Number of index keys found in rows.
	FLMUINT64		ui64NumDuplicateKeys;	///< Number of duplicate index keys found.
	FLMUINT64		ui64NumKeysExamined;		///< Number of index keys checked.
	FLMUINT64		ui64NumKeysNotFound;		///< Number of keys found in index, but not found in the row they reference.
	FLMUINT64		ui64NumRowKeysNotFound;	///< Number of keys found in rows, but not found in the index.
	FLMUINT64		ui64NumConflicts;			///< Number of corruptions that turned out not to be corruptions when an attempt was made to fix them.
} SFLM_PROGRESS_CHECK_INFO;

/// Query operators.
typedef enum
{
	SQL_UNKNOWN_OP						= 0,

	// NOTE: These operators MUST stay in this order - this order is assumed
	// by the precedence table - see sqlparse.cpp

	SQL_AND_OP							= 1,	///< 1 = and
	SQL_OR_OP							= 2,	///< 2 = or
	SQL_NOT_OP							= 3,	///< 3 = ! (not)
	SQL_EQ_OP							= 4,	///< 4 = equals
	SQL_NE_OP							= 5,	///< 5 = not equal
	SQL_APPROX_EQ_OP					= 6,	///< 6 = approximate equal
	SQL_LT_OP							= 7,	///< 7 = less than
	SQL_LE_OP							= 8,	///< 8 = less than or equal
	SQL_GT_OP							= 9,	///< 9 = greater than
	SQL_GE_OP							= 10,	///< 10 = greater than or equal
	SQL_BITAND_OP						= 11,	///< 11 = bit and
	SQL_BITOR_OP						= 12,	///< 12 = bit or
	SQL_BITXOR_OP						= 13,	///< 13 = bit exclusive or
	SQL_MULT_OP							= 14,	///< 14 = multiply
	SQL_DIV_OP							= 15,	///< 15 = divide
	SQL_MOD_OP							= 16,	///< 16 = modulo
	SQL_PLUS_OP							= 17,	///< 17 = plus
	SQL_MINUS_OP						= 18,	///< 18 = minus
	SQL_NEG_OP							= 19,	///< 19 = negative
	SQL_LPAREN_OP						= 20,	///< 20 = left parenthesis
	SQL_RPAREN_OP						= 21,	///< 21 = right parenthesis

	// IMPORTANT NOTE: If operators are added after this point,
	// modify the isLegalOperator method below.

	// The following operators are only used internally.  They
	// may NOT be passed into the addOperator method.

	SQL_EXISTS_OP						= 25,	///< 25 = exists
	SQL_RANGE_OP						= 26,	///< 26 = range
	SQL_MATCH_OP						= 27	///< 27 = match
} eSQLQueryOperators;

#define SQL_FIRST_ARITH_OP			SQL_BITAND_OP
#define SQL_LAST_ARITH_OP			SQL_NEG_OP

/// States of a query.
typedef enum
{
	SQL_QUERY_NOT_POSITIONED,		///< Query has not been positioned yet
	SQL_QUERY_AT_BOF,					///< Positioned at the beginning of result set (before the first row).
	SQL_QUERY_AT_FIRST,				///< Positioned on the first row in result set.
	SQL_QUERY_AT_FIRST_AND_LAST,	///< Positioned on the first and last row in the result set.
	SQL_QUERY_POSITIONED,			///< Positioned in the middle of the result set - not on first or last row.
	SQL_QUERY_AT_LAST,				///< Positioned on the last row in the result set.
	SQL_QUERY_AT_EOF					///< Positioned at the end of the result set (after the last row).
} eQueryStates;

/// Boolean values - tri-state.
typedef enum 
{
	SQL_FALSE = 0,						///< 0 = FALSE
	SQL_TRUE,							///< 1 = TRUE
	SQL_UNKNOWN							///< 2 = UNKNOWN
} SQLBoolType;

/// Value types
typedef enum
{
	SQL_MISSING_VAL = 0,

	// WARNING: Don't renumber below _VAL enums without 
	// re-doing gv_DoValAndDictTypesMatch table

	SQL_BOOL_VAL,						///< 1 = Boolean (::SQLBoolType)
	SQL_UINT_VAL,						///< 2 = Unsigned integer (FLMUINT) 
	SQL_UINT64_VAL,					///< 3 = Unsigned 64 bit integer (FLMUINT64)
	SQL_INT_VAL,						///< 4 = Signed integer (FLMINT)
	SQL_INT64_VAL,						///< 5 = Signed 64 bit integer (FLMINT64)
	SQL_BINARY_VAL,					///< 6 = Binary
	SQL_UTF8_VAL,						///< 7 = UTF8 string
} eSQLValTypes;

/// Optimization types.	
typedef enum
{
	SQL_OPT_NONE = 0,					///< 0 = None
	SQL_OPT_USING_INDEX,				///< 1 = Using an index.
	SQL_OPT_FULL_TABLE_SCAN,		///< 2 = Full table scan.
	SQL_OPT_SINGLE_ROW_ID,			///< 3 = Fetch single row using row ID.
	SQL_OPT_ROW_ID_RANGE				///< 4 = Range of row IDs.
} eSQLOptTypes;

/// Query optimization structure.
typedef struct
{
	eSQLOptTypes	eOptType;						///< Type of optimization done.
	FLMUINT			uiCost;							///< Cost calculated for sub-query.
	FLMUINT64		ui64RowId;						///< Only valid if eOptType is
															///< eSQLOptTypes::SQL_OPT_SINGLE_ROW_ID or
															///< eSQLOptTypes::SQL_OPT_ROW_ID_RANGE
	FLMUINT64		ui64EndRowId;					///< Only valid if eOptType is
															///< eSQLOptTypes::SQL_OPT_ROW_ID_RANGE
	FLMUINT			uiIxNum;							///< Index used to execute query if
															///< eOptType == SQL_OPT_USING_INDEX
	FLMBYTE			szIxName[ 80];					///< Name of index corresponding to uiIxNum.
	FLMBOOL			bDoRowMatch;					///< Node must be retrieved to exe
															///< query.  Only valid if eOptType
															///< is eSQLOptTypes::SQL_OPT_USING_INDEX.
	FLMUINT			bCanCompareOnKey;				///< Can we compare using only the data from index keys?\  Only
															///< valid if eOptType == eSQLOptTypes::SQL_OPT_USING_INDEX.
	FLMUINT64		ui64KeysRead;					///< Total index keys read.
	FLMUINT64		ui64KeyHadDupRow;				///< Total index keys that pointed to a row we had already passed.
	FLMUINT64		ui64KeysPassed;				///< Total keys that passed query criteria.
	FLMUINT64		ui64RowsRead;					///< Total rows read.
	FLMUINT64		ui64RowsPassed;				///< Total rows that passed the query criteria.
	FLMUINT64		ui64DupRowsEliminated;		///< Total rows that were duplicates that we eliminated.
	FLMUINT64		ui64RowsFailedValidation;	///< Total rows that failed validation callback.
} SQL_OPT_INFO;

//*************************************************************************
//                  Dictionary Tag Numbers
//
// NOTES:
//		1) These numbers cannot be changed for backward compatibility reasons.
//		2) IF ANY NEW TAGS ARE INSERTED - Then you MUST change the database
//			version number, because old databases will become invalid.....
//
//**************************************************************************

// Special purpose tables
// NOTE: Do not change the order of these definitions.  The
// getNextTable routine assumes they are in this order.

// We have reserved from 65501 to 65535 for internal collections
// These should be allocated starting from 65535 and going down.

#define SFLM_MAX_TABLE_NUM				65500

#define SFLM_MAINT_TABLE				65530
#define SFLM_ENCDEF_TABLE				65531
#define SFLM_INDEX_COLUMNS_TABLE		65532
#define SFLM_INDEX_TABLE				65533
#define SFLM_TABLE_COLUMNS_TABLE		65534
#define SFLM_TABLE_TABLE				65535

// Special purpose indexes
// NOTE: Do not change the order of these definitions.  The
// getNextIndex routine assumes they are in this order.

// We have reserved from 65501 to 65535 for internal indexes
// These should be allocated starting from 65535 and going down.

#define SFLM_MAX_INDEX_NUM					65500
#define SFLM_INDEX_COLUMN_NAME_INDEX	65530
#define SFLM_INDEX_NUMBER_INDEX			65531
#define SFLM_INDEX_NAME_INDEX				65532
#define SFLM_TABLE_COLUMN_NAME_INDEX	65533
#define SFLM_TABLE_NUMBER_INDEX			65534
#define SFLM_TABLE_NAME_INDEX				65535

#define SFLM_MAX_ENCDEF_NUM				65500

//***************************************************************************
//							Dictionary Identifiers
//***************************************************************************

#define SFLM_INI_CACHE							"cache"
#define SFLM_INI_CACHE_ADJUST_INTERVAL		"cacheadjustinterval"
#define SFLM_INI_CACHE_CLEANUP_INTERVAL	"cachecleanupinterval"
#define SFLM_INI_MAX_DIRTY_CACHE				"maxdirtycache"
#define SFLM_INI_LOW_DIRTY_CACHE				"lowdirtycache"

// Defaults for certain settable items in the database system

#define SFLM_DEFAULT_MAX_CP_INTERVAL					180
#define SFLM_DEFAULT_CACHE_ADJUST_PERCENT				70
#define SFLM_DEFAULT_CACHE_ADJUST_MIN					(16 * 1024 * 1024)
#define SFLM_DEFAULT_CACHE_ADJUST_MAX					0xE0000000
#define SFLM_DEFAULT_CACHE_ADJUST_MIN_TO_LEAVE		0
#define SFLM_DEFAULT_CACHE_ADJUST_INTERVAL			15
#define SFLM_DEFAULT_CACHE_CLEANUP_INTERVAL			15
#define SFLM_DEFAULT_UNUSED_CLEANUP_INTERVAL			2
#define SFLM_DEFAULT_MAX_UNUSED_TIME					120
#define SFLM_DEFAULT_FILE_EXTEND_SIZE					(8192 * 1024)
#define SFLM_MIN_BLOCK_SIZE								4096
#define SFLM_MAX_BLOCK_SIZE								8192
#define SFLM_DEFAULT_OPEN_THRESHOLD						100		// 100 file handles to cache
#define SFLM_DEFAULT_MAX_AVAIL_TIME						900		// 15 minutes
#define SFLM_DEFAULT_REHASH_BACKOFF_INTERVAL			60			// 1 minute


// Error codes

/// \addtogroup flaim_errors
/// @{
#define NE_SFLM_OK				 						NE_FLM_OK
#define NE_SFLM_NOT_IMPLEMENTED						0xE000	///< 0xE000 = Attempt was made to use a feature that is not implemented.
#define NE_SFLM_MEM										0xE001	///< 0xE001 = Attempt to allocate memory failed.
#define NE_SFLM_INVALID_PARM							0xE002	///< 0xD002 = Invalid parameter passed into a function.
#define NE_SFLM_TIMEOUT									0xE003	///< 0xE003 = Operation timed out (usually a query operation).
#define NE_SFLM_NOT_FOUND								0xE004	///< 0xE004 = An object was not found.
#define NE_SFLM_EXISTS									0xE005	///< 0xE005 = Object already exists.
#define NE_SFLM_USER_ABORT								0xE006	///< 0xE006 = User or application aborted (canceled) the operation.
#define NE_SFLM_FAILURE									0xE007	///< 0xE007 = Internal failure.
#define NE_SFLM_BOF_HIT									0xE008	///< 0xE008 = Beginning of results encountered.\  This error is may be returned when reading query results in reverse order (from last to first).
#define NE_SFLM_EOF_HIT									0xE009	///< 0xE009 = End of results encountered.\  This error may be returned when reading query results in forward order (first to last).
#define NE_SFLM_END										0xE00A	///< 0xE00A = End of roll-forward log packets encountered.\  NOTE: This error code should never be returned to an application.
#define NE_SFLM_BAD_TABLE								0xE00B	///< 0xE00B = Invalid table number specified.\  Table is not defined.
#define NE_SFLM_DATABASE_LOCK_REQ_TIMEOUT			0xE00C	///< 0xE00C = Request to lock the database timed out.
#define NE_SFLM_BAD_DATA_TYPE							0xE00D	///< 0xE00D = Attempt to set/get data on a column using a data type that is incompatible with the data type specified in the dictionary.
#define NE_SFLM_BAD_IX									0xE00E	///< 0xE00E = Invalid index number specified.\  Index is not defined.
#define NE_SFLM_BACKUP_ACTIVE							0xE00F	///< 0xE00F = Operation could not be performed because a backup is currently in progress.
#define NE_SFLM_SERIAL_NUM_MISMATCH					0xE010	///< 0xE010 = Serial number on backup file does not match the serial number that is expected.
#define NE_SFLM_BAD_RFL_DB_SERIAL_NUM				0xE011	///< 0xE011 = Bad database serial number in roll-forward log file header.
#define NE_SFLM_BTREE_ERROR							0xE012	///< 0xE012 = A B-Tree in the database has a corruption.
#define NE_SFLM_BTREE_FULL								0xE013	///< 0xE013 = A B-tree in the database is full, or a b-tree being used for a temporary result set is full.
#define NE_SFLM_BAD_RFL_FILE_NUMBER					0xE014	///< 0xE014 = Bad roll-forward log file number in roll-forward log file header.
#define NE_SFLM_CANNOT_MOD_DATA_TYPE				0xE015	///< 0xE015 = Cannot modify the data type for a column definition in the dictionary.
#define NE_SFLM_CANNOT_INDEX_DATA_TYPE				0xE016	///< 0xE016 = Data type of column is not one that can be indexed.
#define NE_SFLM_CONV_BAD_DIGIT						0xE017	///< 0xE017 = Non-numeric digit found in text to numeric conversion.
#define NE_SFLM_CONV_DEST_OVERFLOW					0xE018	///< 0xE018 = Destination buffer not large enough to hold data.
#define NE_SFLM_CONV_ILLEGAL							0xE019	///< 0xE019 = Attempt to convert between data types is an unsupported conversion.
#define NE_SFLM_CONV_NULL_SRC							0xE01A	///< 0xE01A = Data source cannot be NULL when doing data conversion.
#define NE_SFLM_CONV_NUM_OVERFLOW					0xE01B	///< 0xE01B = Numeric overflow (> upper bound) converting to numeric type.
#define NE_SFLM_CONV_NUM_UNDERFLOW					0xE01C	///< 0xE01C = Numeric underflow (< lower bound) converting to numeric type.
#define NE_SFLM_BAD_COLUMN_NUM						0xE01D	///< 0xE01D = Bad column number specified - column not defined in dictionary.
#define NE_SFLM_DATA_ERROR								0xE01E	///< 0xE01E = Encountered data in the database that was corrupted.
#define NE_SFLM_INVALID_FILE_SEQUENCE				0xE01F	///< 0xE01F = Incremental backup file number provided during a restore is invalid.
#define NE_SFLM_ILLEGAL_OP								0xE020	///< 0xE020 = Attempt to perform an illegal operation.
#define NE_SFLM_ILLEGAL_TRANS_TYPE					0xE021	///< 0xE021 = Illegal transaction type specified for transaction begin operation.
#define NE_SFLM_UNSUPPORTED_VERSION					0xE022	///< 0xE022 = Version of database found in database header is not supported.
#define NE_SFLM_ILLEGAL_TRANS_OP						0xE023	///< 0xE023 = Illegal operation for transaction type.
#define NE_SFLM_INCOMPLETE_LOG						0xE024	///< 0xE024 = Incomplete rollback log.
#define NE_SFLM_BAD_RFL_SERIAL_NUM					0xE025	///< 0xE025 = Serial number in roll-forward log file header does not match expected serial number.
#define NE_SFLM_NEWER_FLAIM							0xE026	///< 0xE026 = Running old code on a newer version of database.\  Newer code must be used.
#define NE_SFLM_NO_TRANS_ACTIVE						0xE027	///< 0xE027 = Operation must be performed inside a database transaction.
#define NE_SFLM_NOT_UNIQUE								0xE028	///< 0xE028 = Attempt was made to insert a key into a b-tree that was already in the b-tree.
#define NE_SFLM_NOT_FLAIM								0xE029	///< 0xE029 = The file specified is not a FLAIM database.
#define NE_SFLM_OLD_VIEW								0xE02A	///< 0xE02A = Unable to maintain read transaction's view of the database.
#define NE_SFLM_SHARED_LOCK							0xE02B	///< 0xE02B = Attempted to perform an operation on the database that requires exclusive access, but cannot because there is a shared lock.
#define NE_SFLM_TRANS_ACTIVE							0xE02C	///< 0xE02C = Operation cannot be performed while a transaction is active.
#define NE_SFLM_RFL_TRANS_GAP							0xE02D	///< 0xE02D = A gap was found in the transaction sequence in the roll-forward log.
#define NE_SFLM_BAD_COLLATED_KEY						0xE02E	///< 0xE02E = Something in collated key is bad.
#define NE_SFLM_UNSUPPORTED_FEATURE					0xE02F	///< 0xE02F = Attempting to use a feature for which full support has been disabled.
#define NE_SFLM_MUST_DELETE_INDEXES					0xE030	///< 0xE030 = Attempting to drop a table that has indexes defined for it.\  Associated indexes must be deleted before the table can be dropped.
#define NE_SFLM_RFL_INCOMPLETE						0xE031	///< 0xE031 = Roll-forward log file is incomplete.
#define NE_SFLM_CANNOT_RESTORE_RFL_FILES			0xE032	///< 0xE032 = Cannot restore roll-forward log files - not using multiple roll-forward log files.
#define NE_SFLM_INCONSISTENT_BACKUP					0xE033	///< 0xE033 = A problem (corruption, etc.\ ) was detected in a backup set.
#define NE_SFLM_BLOCK_CRC								0xE034	///< 0xE034 = CRC for database block was invalid.\  May indicate problems in reading from or writing to disk.
#define NE_SFLM_ABORT_TRANS							0xE035	///< 0xE035 = Attempted operation after a critical error - transaction should be aborted.
#define NE_SFLM_NOT_RFL									0xE036	///< 0xE036 = File was not a roll-forward log file as expected.
#define NE_SFLM_BAD_RFL_PACKET						0xE037	///< 0xE037 = Roll-forward log file packet was bad.
#define NE_SFLM_DATA_PATH_MISMATCH					0xE038	///< 0xE038 = Bad data path specified to open database.\  Does not match data path specified for prior opens of the database.
#define NE_SFLM_DATABASE_EXISTS						0xE039	///< 0xE039 = Attempt to create a database, but the database already exists.
#define NE_SFLM_COULD_NOT_CREATE_SEMAPHORE		0xE03A	///< 0xE03A = Could not create a semaphore.
#define NE_SFLM_MUST_CLOSE_DATABASE					0xE03B	///< 0xE03B = Database must be closed due to a critical error.
#define NE_SFLM_INVALID_ENCKEY_CRC					0xE03C	///< 0xE03C = Encryption key CRC could not be verified.
#define NE_SFLM_BAD_UTF8								0xE03D	///< 0xE03D = An invalid byte sequence was found in a UTF-8 string.
#define NE_SFLM_COULD_NOT_CREATE_MUTEX				0xE03E	///< 0xE03E = Could not create a mutex.
#define NE_SFLM_ERROR_WAITING_ON_SEMPAHORE		0xE03F	///< 0xE03F = Error occurred while waiting on a sempahore.
#define NE_SFLM_BAD_PLATFORM_FORMAT					0xE040	///< 0xE040 = Cannot support platform format.\  NOTE: This error is internal only.
#define NE_SFLM_HDR_CRC									0xE041	///< 0xE041 = Database header has a bad CRC.
#define NE_SFLM_NO_NAME_TABLE							0xE042	///< 0xE042 = No name table was set up for the database.
#define NE_SFLM_UNALLOWED_UPGRADE					0xE043	///< 0xE043 = Cannot upgrade database from one version to another.
#define NE_SFLM_BTREE_BAD_STATE						0xE044	///< 0xE044 = Btree function called before proper setup steps taken.
#define NE_SFLM_TOO_MANY_OPEN_DATABASES			0xE045	///< 0xE045 = Too many open databases, cannot open another one.
#define NE_SFLM_DATABASE_OPEN							0xE046	///< 0xE046 = Operation cannot be performed because the database is currently open.
#define NE_SFLM_CACHE_ERROR							0xE047	///< 0xE047 = Cached database block has been compromised while in cache.
#define NE_SFLM_BTREE_KEY_SIZE						0xE048	///< 0xE048 = Key too large to insert/lookup in a b-tree.
#define NE_SFLM_DB_FULL									0xE049	///< 0xE049 = Database is full, cannot create more blocks.
#define NE_SFLM_QUERY_SYNTAX							0xE04A	///< 0xE04A = Query expression had improper syntax.
#define NE_SFLM_COULD_NOT_START_THREAD				0xE04B	///< 0xE04B = Error occurred while attempting to start a thread.
#define NE_SFLM_INDEX_OFFLINE							0xE04C	///< 0xE04C = Index is offline, cannot be used in a query.
#define NE_SFLM_RFL_DISK_FULL							0xE04D	///< 0xE04D = Disk which contains roll-forward log is full.
#define NE_SFLM_MUST_WAIT_CHECKPOINT				0xE04E	///< 0xE04E = Must wait for a checkpoint before starting transaction - due to disk problems - usually in disk containing roll-forward log files.
#define NE_SFLM_BAD_SEN									0xE04F	///< 0xE04F = Invalid simple encoded number.
#define NE_SFLM_RFL_FILE_NOT_FOUND					0xE050	///< 0xE050 = Could not open a roll-forward log file - was not found in the roll-forward log directory.
#define NE_SFLM_BAD_RCODE_TABLE						0xE051	///< 0xE051 = The error code tables are incorrect.\  NOTE: This is an internal error only.
#define NE_SFLM_BUFFER_OVERFLOW						0xE052	///< 0xE052 = Buffer overflow.
#define NE_SFLM_KEY_OVERFLOW							0xE053	///< 0xE053 = Generated index key too large.
#define NE_SFLM_NO_MORE_ENCDEF_NUMS					0xE054	///< 0xE054 = The highest encryption definition number has already been used cannot create more.
#define NE_SFLM_TABLE_OFFLINE							0xE055	///< 0xE055 = Table is encrypted, cannot be accessed while in operating in limited mode.
#define NE_SFLM_READ_ONLY								0xE056	///< 0xE056 = Column is read-only and cannot be updated.
#define NE_SFLM_DELETE_NOT_ALLOWED					0xE057	///< 0xE057 = Row cannot be deleted.
#define NE_SFLM_RESET_NEEDED							0xE058	///< 0xE058 = Used during check operations to indicate we need to reset the view.\  NOTE: This is an internal error code.
#define NE_SFLM_ILLEGAL_LANGUAGE						0xE059	///< 0xE059 = Invalid language specified in an index definition.
#define NE_SFLM_ROW_DELETED							0xE05A	///< 0xE05A = Row being accessed has been deleted.
#define NE_SFLM_ROW_NOT_FOUND							0xE05B	///< 0xE05B - Row being retrieved does not exist.

// Dictionary definition errors.

#define NE_SFLM_ENCDEF_NAME_ALREADY_DEFINED		0xE100	///< 0xE100 = Encryption definition name already defined.\  Cannot use again.
#define NE_SFLM_NULL_ENCDEF_NAME						0xE101	///< 0xE101 = Encryption definition name may not be null.
#define NE_SFLM_NULL_ENCDEF_NUM						0xE102	///< 0xE102 = Encryption definition number is null.\  Dictionary is corrupt.
#define NE_SFLM_DUPLICATE_ENCDEF_NAME				0xE103	///< 0xE103 = Duplicate encryption definition name in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_DUPLICATE_ENCDEF_NUM				0xE104	///< 0xE104 = Duplicate encryption definition number in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_INVALID_ENCDEF_NUM					0xE105	///< 0xE105 = Invalid encryption definition number in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_NULL_ENC_ALGORITHM					0xE106	///< 0xE106 = Encryption algorithm may not be null.
#define NE_SFLM_INVALID_ENC_ALGORITHM				0xE107	///< 0xE107 = Invalid encryption algorithm specified in encryption definition.
#define NE_SFLM_NULL_ENC_KEY_SIZE					0xE108	///< 0xE108 = Encryption key size may not be null.
#define NE_SFLM_INVALID_ENC_KEY_SIZE				0xE109	///< 0xE109 = Invalid key size specified in encryption definition.
#define NE_SFLM_NULL_ENC_KEY							0xE10A	///< 0xE10A = Encryption key is null.\  Dictionary is corrupt.

#define NE_SFLM_TABLE_NAME_ALREADY_DEFINED		0xE10B	///< 0xE10B = Table name already defined.\  Cannot use again.
#define NE_SFLM_NULL_TABLE_NAME						0xE10C	///< 0xE10C = Table name may not be null.
#define NE_SFLM_NULL_TABLE_NUM						0xE10D	///< 0xE10D = Table number is null.\  Dictionary is corrupt.
#define NE_SFLM_DUPLICATE_TABLE_NAME				0xE10E	///< 0xE10E = Duplicate table name in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_DUPLICATE_TABLE_NUM					0xE10F	///< 0xE10F = Duplicate table number in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_INVALID_TABLE_NUM					0xE110	///< 0xE110 = Invalid table number in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_NULL_NUM_COLUMNS						0xE111	///< 0xE111 = Number of columns for table is null.\  Dictionary is corrupt.
#define NE_SFLM_INVALID_NUM_COLUMNS					0xE112	///< 0xE112 = Number of columns for table is invalid.\  Dictionary is corrupt.

#define NE_SFLM_COLUMN_NAME_ALREADY_DEFINED		0xE113	///< 0xE113 = Column name already defined in table.\  Cannot use again.
#define NE_SFLM_NULL_COLUMN_NAME						0xE114	///< 0xE114 = Column name may not be null.
#define NE_SFLM_NULL_COLUMN_NUM						0xE115	///< 0xE115 = Column number is null.\  Dictionary is corrupt.
#define NE_SFLM_DUPLICATE_COLUMN_NAME				0xE116	///< 0xE116 = Duplicate column name found for table in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_DUPLICATE_COLUMN_NUM				0xE117	///< 0xE117 = Duplicate column number found for table in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_INVALID_COLUMN_NUM					0xE118	///< 0xE118 = Invalid column number for table in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_UNDEFINED_COLUMN_NUM				0xE119	///< 0xE119 = Required column number for table not defined in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_NULL_DATA_TYPE						0xE11A	///< 0xE11A = Data type may not be null.
#define NE_SFLM_ILLEGAL_DATA_TYPE					0xE11B	///< 0xE11B = Data type specified for column definition is illegal.
#define NE_SFLM_INVALID_READ_ONLY_VALUE			0xE11C	///< 0xE11C = Read-only value specified for column definition is illegal.
#define NE_SFLM_INVALID_NULL_ALLOWED_VALUE		0xE11D	///< 0xE11D = Null-allowed value specified for colum definition is illegal.

#define NE_SFLM_INDEX_NAME_ALREADY_DEFINED		0xE11E	///< 0xE11E = Index name already defined.\  Cannot use again.
#define NE_SFLM_NULL_INDEX_NAME						0xE11F	///< 0xE11F = Index name may not be null.
#define NE_SFLM_NULL_INDEX_NUM						0xE120	///< 0xE120 = Index number is null.\  Dictionary is corrupt.
#define NE_SFLM_DUPLICATE_INDEX_NAME				0xE121	///< 0xE121 = Duplicate index name in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_DUPLICATE_INDEX_NUM					0xE122	///< 0xE122 = Duplicate index number in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_INVALID_INDEX_NUM					0xE123	///< 0xE123 = Invalid index number in dictionary.\  Dictionary is corrupt.
#define NE_SFLM_INVALID_COMPARE_RULE				0xE124	///< 0xE124 = Invalid comparison rule in index definition.\  Dictionary is corrupt.
#define NE_SFLM_INVALID_INDEX_OPTION				0xE125	///< 0xE125 = Invalid index option specified on index definition.
#define NE_SFLM_INVALID_INDEX_STATE					0xE126	///< 0xE126 = Invalid index state specified for an index.
#define NE_SFLM_DUPLICATE_KEY_COMPONENT			0xE127	///< 0xE127 = Duplicate key component in internal index definition.\  Data dictionary is corrupt.
#define NE_SFLM_INVALID_KEY_COMPONENT				0xE128	///< 0xE128 = Invalid key component in internal index definition.\  Data dictionary is corrupt.
#define NE_SFLM_UNDEFINED_KEY_COMPONENT			0xE129	///< 0xE129 = Undefined key component in internal index definition.\  Data dictionary is corrupt.
#define NE_SFLM_DUPLICATE_DATA_COMPONENT			0xE12A	///< 0xE12A = Duplicate data component in internal index definition.\  Data dictionary is corrupt.
#define NE_SFLM_INVALID_DATA_COMPONENT				0xE12B	///< 0xE12B = Invalid data component in internal index definition.\  Data dictionary is corrupt.
#define NE_SFLM_UNDEFINED_DATA_COMPONENT			0xE12C	///< 0xE12C = Undefined data component in internal index definition.\  Data dictionary is corrupt.
#define NE_SFLM_NO_COMP_NUM_FOR_INDEX_COLUMN		0xE12D	///< 0xE12D = Neither key component or data component specified for index column.\  Data dictionary is corrupt.
#define NE_SFLM_INVALID_INDEX_ON_VALUE				0xE12E	///< 0xE12E = Invalid value for the "index on" option in an index definition.
#define NE_SFLM_INVALID_COMPARE_RULES_VALUE		0xE12F	///< 0xE12F = Invalid value for the "compare rules" option in an index definition.
#define NE_SFLM_INVALID_SORT_DESCENDING_VALUE	0xE130	///< 0xE130 = Invalid value for the "sort descending" option in an index definition.
#define NE_SFLM_INVALID_SORT_MISSING_HIGH_VALUE	0xE131	///< 0xE131 = Invalid value for the "sort missing high" option in an index definition.

// Query Errors

#define NE_SFLM_Q_UNMATCHED_RPAREN					0xE200	///< 0xE200 = Query syntax error: Unmatched right paren.
#define NE_SFLM_Q_UNEXPECTED_LPAREN					0xE201	///< 0xE201 = Query syntax error: Unexpected left paren.
#define NE_SFLM_Q_UNEXPECTED_RPAREN					0xE202	///< 0xE202 = Query syntax error: Unexpected right paren.
#define NE_SFLM_Q_EXPECTING_OPERAND					0xE203	///< 0xE203 = Query syntax error: Expecting an operand.
#define NE_SFLM_Q_EXPECTING_OPERATOR				0xE204	///< 0xE204 = Query syntax error: Expecting an operator.
#define NE_SFLM_Q_UNEXPECTED_VALUE					0xE205	///< 0xE205 = Query syntax error: Unexpected value.
#define NE_SFLM_Q_ILLEGAL_OPERAND					0xE206	///< 0xE206 = Query syntax error: Operand for some operator is not valid for that operator type.
#define NE_SFLM_Q_INCOMPLETE_QUERY_EXPR			0xE207	///< 0xE207 = Query syntax error: Query expression is incomplete.
#define NE_SFLM_Q_INVALID_ROW_ID_VALUE				0xE208	///< 0xE208 = Query syntax error: Invalid type of value constant used for row id value comparison.
#define NE_SFLM_Q_ALREADY_OPTIMIZED					0xE209	///< 0xE209 = Operation is illegal, cannot change certain things after query has been optimized.
#define NE_SFLM_Q_MISMATCHED_DB						0xE20A	///< 0xE20A = Database handle passed in does not match database associated with query.
#define NE_SFLM_Q_NOT_POSITIONED						0xE20B	///< 0xE20B = Query not positioned due to previous error, cannot call getNext, getPrev, or getCurrent.
#define NE_SFLM_Q_BAD_SEARCH_STRING					0xE20C	///< 0xE20C = String in query criteria is bad.
#define NE_SFLM_Q_COMPARE_OPERAND_TYPE_MISMATCH	0xE20D	///< 0xE20D = Types of operands in a comparison operator are incompatible, cannot be compared.
#define NE_SFLM_Q_INVALID_OPERATOR					0xE20E	///< 0xE20E = Invalid operator in query expression.
#define NE_Q_INVALID_ROW_ID_VALUE					0xE20F	///< 0xE20F = Could not get ROW ID from constant in query expression.
#define NE_SFLM_Q_EXPECTING_LPAREN					0xE210	///< 0xE210 = Expecting a left parenthesis in query expression.
#define NE_SFLM_Q_ILLEGAL_OPERATOR					0xE211	///< 0xE211 = Illegal operator specified in query expression.
#define NE_SFLM_Q_ILLEGAL_COMPARE_RULES			0xE212	///< 0xE212 = Illegal combination comparison rules specified in query expression.
#define NE_SFLM_Q_UNEXPECTED_COLUMN					0xE213	///< 0xE213 = Not expecting a column name in query expression.

// Desc:	NICI / Encryption Errors

#define NE_SFLM_NICI_CONTEXT							0xE300	///< 0xE300 = Error occurred while creating NICI context for encryption/decryption.
#define NE_SFLM_NICI_ATTRIBUTE_VALUE				0xE301	///< 0xE301 = Error occurred while accessing an attribute on a NICI encryption key.
#define NE_SFLM_NICI_BAD_ATTRIBUTE					0xE302	///< 0xE302 = Value retrieved from an attribute on a NICI encryption key was bad.
#define NE_SFLM_NICI_WRAPKEY_FAILED					0xE303	///< 0xE303 = Error occurred while wrapping a NICI encryption key in another NICI encryption key.
#define NE_SFLM_NICI_UNWRAPKEY_FAILED				0xE304	///< 0xE304 = Error occurred while unwrapping a NICI encryption key that is wrapped in another NICI encryption key.
#define NE_SFLM_NICI_INVALID_ALGORITHM				0xE305	///< 0xE305 = Attempt to use invalid NICI encryption algorithm. 
#define NE_SFLM_NICI_GENKEY_FAILED					0xE306	///< 0xE306 = Error occurred while attempting to generate a NICI encryption key.
#define NE_SFLM_NICI_BAD_RANDOM						0xE307	///< 0xE307 = Error occurred while generating random data using NICI.
#define NE_SFLM_PBE_ENCRYPT_FAILED					0xE308	///< 0xE308 = Error occurred while attempting to wrap a NICI encryption key in a password.
#define NE_SFLM_PBE_DECRYPT_FAILED					0xE309	///< 0xE309 = Error occurred while attempting to unwrap a NICI encryption key that was previously wrapped in a password.
#define NE_SFLM_DIGEST_INIT_FAILED					0xE30A	///< 0xE30A = Error occurred while attempting to initialize the NICI digest functionality.
#define NE_SFLM_DIGEST_FAILED							0xE30B	///< 0xE30B = Error occurred while attempting to create a NICI digest. 
#define NE_SFLM_INJECT_KEY_FAILED					0xE30C	///< 0xE30C = Error occurred while attempting to inject an encryption key into NICI. 
#define NE_SFLM_NICI_FIND_INIT						0xE30D	///< 0xE30D = Error occurred while attempting to initialize NICI to find information on a NICI encryption key.
#define NE_SFLM_NICI_FIND_OBJECT						0xE30E	///< 0xE30E = Error occurred while attempting to find information on a NICI encryption key.
#define NE_SFLM_NICI_KEY_NOT_FOUND					0xE30F	///< 0xE30F = Could not find the NICI encryption key or information on the NICI encryption key.
#define NE_SFLM_NICI_ENC_INIT_FAILED				0xE310	///< 0xE310 = Error occurred while initializing NICI to encrypt data.
#define NE_SFLM_NICI_ENCRYPT_FAILED					0xE311	///< 0xE311 = Error occurred while encrypting data.
#define NE_SFLM_NICI_DECRYPT_INIT_FAILED			0xE312	///< 0xE312 = Error occurred while initializing NICI to decrypt data.
#define NE_SFLM_NICI_DECRYPT_FAILED					0xE313	///< 0xE313 = Error occurred while decrypting data.
#define NE_SFLM_NICI_WRAPKEY_NOT_FOUND				0xE314	///< 0xE314 = Could not find the NICI encryption key used to wrap another NICI encryption key.
#define NE_SFLM_NOT_EXPECTING_PASSWORD				0xE315	///< 0xE315 = Password supplied when none was expected.
#define NE_SFLM_EXPECTING_PASSWORD					0xE316	///< 0xE316 = No password supplied when one was required.
#define NE_SFLM_EXTRACT_KEY_FAILED					0xE317	///< 0xE317 = Error occurred while attempting to extract a NICI encryption key.
#define NE_SFLM_NICI_INIT_FAILED						0xE318	///< 0xE318 = Error occurred while initializing NICI.
#define NE_SFLM_BAD_ENCKEY_SIZE						0xE319	///< 0xE319 = Bad encryption key size found in roll-forward log packet.
#define NE_SFLM_ENCRYPTION_UNAVAILABLE				0xE31A	///< 0xE31A = Attempt was made to encrypt data when NICI is unavailable.


// VARIOUS INTERFACES FOR CALLBACKS, ETC.

// Note:  Any interfaces ending in Client or Status are interfaces
// that FlaimSQL does not provide implementations of.  They exist to
// allow FlaimSQL to pass data back to the client.  Interfaces ending in
// Status are, generally, informational only, while interfaces ending
// in Client exist to allow the client to modify the data or take
// other action.

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_BackupClient : public F_Object
{
	virtual RCODE WriteData(
		const void *	pvBuffer,
		FLMUINT			uiBytesToWrite) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_BackupStatus : public F_Object
{
	virtual RCODE backupStatus(
		FLMUINT64	ui64BytesToDo,
		FLMUINT64	ui64BytesDone) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_RestoreStatus : public F_Object
{
	virtual RCODE reportProgress(
		eRestoreAction *		peAction,
		FLMUINT64				ui64BytesToDo,
		FLMUINT64				ui64BytesDone) = 0;

	virtual RCODE reportError(
		eRestoreAction *		peAction,
		RCODE						rcErr) = 0;

	virtual RCODE reportOpenRflFile(
		eRestoreAction *		peAction,
		FLMUINT					uiFileNum) = 0;

	virtual RCODE reportRflRead(
		eRestoreAction *		peAction,
		FLMUINT					uiFileNum,
		FLMUINT					uiBytesRead) = 0;

	virtual RCODE reportBeginTrans(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId) = 0;

	virtual RCODE reportCommitTrans(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId) = 0;

	virtual RCODE reportAbortTrans(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId) = 0;

	virtual RCODE reportBlockChainFree(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId,
		FLMUINT64				ui64MaintDocNum,
		FLMUINT					uiStartBlkAddr,
		FLMUINT					uiEndBlkAddr,
		FLMUINT					uiCount) = 0;

	virtual RCODE reportIndexSuspend(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId,
		FLMUINT					uiIndexNum) = 0;

	virtual RCODE reportIndexResume(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId,
		FLMUINT					uiIndexNum) = 0;

	virtual RCODE reportReduce(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId,
		FLMUINT					uiCount) = 0;

	virtual RCODE reportUpgrade(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId,
		FLMUINT					uiOldDbVersion,
		FLMUINT					uiNewDbVersion) = 0;

	virtual RCODE reportEnableEncryption(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId) = 0;

	virtual RCODE reportWrapKey(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId) = 0;
		
	virtual RCODE reportRollOverDbKey(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId) = 0;
		
	virtual RCODE reportSetNextRowId(
		eRestoreAction *		peAction,
		FLMUINT64				ui64TransId,
		FLMUINT					uiTableNum,
		FLMUINT64				ui64NextRowId) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_RestoreClient : public F_Object
{
	virtual RCODE openBackupSet( void) = 0;

	virtual RCODE openRflFile(					// Open an RFL file
		FLMUINT					uiFileNum) = 0;

	virtual RCODE openIncFile(					// Open an incremental backup file
		FLMUINT					uiFileNum) = 0;

	virtual RCODE read(
		FLMUINT					uiLength,					// Number of bytes to read
		void *					pvBuffer,					// Buffer to place read bytes into
		FLMUINT *				puiBytesRead) = 0;		// [out] Number of bytes read

	virtual RCODE close( void) = 0;			// Close the current file

	virtual RCODE abortFile( void) = 0;		// Abort processing the file
																	// and close file handles, etc.
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_UpgradeClient : public F_Object
{
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_DbCopyStatus : public F_Object
{
	virtual RCODE dbCopyStatus(
		FLMUINT64				ui64BytesToCopy,
		FLMUINT64				ui64BytesCopied,
		FLMBOOL					bNewSrcFile,
		const char *			pszSrcFileName,
		const char *			pszDestFileName) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_DbRebuildStatus : public F_Object
{
	virtual RCODE reportRebuild(
		SFLM_REBUILD_INFO *	pRebuild) = 0;
	
	virtual RCODE reportRebuildErr(
		SFLM_CORRUPT_INFO *	pCorruptInfo) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_DbCheckStatus : public F_Object
{
	virtual RCODE reportProgress(
		SFLM_PROGRESS_CHECK_INFO *	pProgCheck) = 0;
	
	virtual RCODE reportCheckErr(
		SFLM_CORRUPT_INFO *	pCorruptInfo,
		FLMBOOL *				pbFix) = 0;
		// [OUT] - If the client sets this to true, then FlaimSQL will
		// attempt to fix the problem.  NOTE: It is allowable for
		// FlaimSQL to pass in NULL here!! (This means that the client
		// doesn't have a choice regarding FlaimSQL's actions.)  The
		// client must check for NULL before attempting to assign a
		// value to this parameter!!
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_DbRenameStatus : public F_Object
{
	virtual RCODE dbRenameStatus(
		const char *	pszSrcFileName,
		const char *	pszDstFileName) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_IxStatus : public F_Object
{
	virtual RCODE reportIndex(
		FLMUINT64	ui64LastRowId) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_DbInfo : public F_Object
{
	virtual FLMUINT getNumTables( void) = 0;
	
	virtual FLMUINT getNumIndexes( void) = 0;

	virtual FLMUINT getNumLogicalFiles( void) = 0;

	virtual FLMUINT64 getFileSize( void) = 0;

	virtual const SFLM_DB_HDR * getDbHdr( void) = 0;

	virtual void getAvailBlockStats(
		FLMUINT64 *				pui64BytesUsed,
		FLMUINT *				puiBlockCount,
		FLMINT *					piLastError,
		FLMUINT *				puiNumErrors) = 0;

	virtual void getLFHBlockStats(
		FLMUINT64 *				pui64BytesUsed,
		FLMUINT *				puiBlockCount,
		FLMINT *					piLastError,
		FLMUINT *				puiNumErrors) = 0;

	virtual void getBTreeInfo(
		FLMUINT					uiNthLogicalFile,
		FLMUINT *				puiLfNum,
		eLFileType *			peLfType,
		FLMUINT *				puiRootBlkAddress,
		FLMUINT *				puiNumLevels) = 0;

	virtual void getBTreeBlockStats(
		FLMUINT					uiNthLogicalFile,
		FLMUINT					uiLevel,
		FLMUINT64 *				pui64KeyCount,
		FLMUINT64 *				pui64BytesUsed,
		FLMUINT64 *				pui64ElementCount,
		FLMUINT64 *				pui64ContElementCount,
		FLMUINT64 *				pui64ContElmBytes,
		FLMUINT *				puiBlockCount,
		FLMINT *					piLastError,
		FLMUINT *				puiNumErrors) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_QueryStatus : public F_Object
{
	virtual RCODE queryStatus(
		SQL_OPT_INFO *		pOptInfo) = 0;

	virtual RCODE newSource(
		SQL_OPT_INFO *		pOptInfo) = 0;
		
	virtual RCODE resultSetStatus(
		FLMUINT64	ui64TotalDocsRead,
		FLMUINT64	ui64TotalDocsPassed,
		FLMBOOL		bCanRetrieveDocs) = 0;
		
	virtual RCODE resultSetComplete(
		FLMUINT64	ui64TotalDocsRead,
		FLMUINT64	ui64TotalDocsPassed) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_CommitClient : public F_Object
{
	virtual void commit( 
		F_Db *	pDb) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_EventClient : public F_Object
{
	virtual void catchEvent(
		eEventType	eEvent,
		F_Db *		pDb,
		FLMUINT		uiThreadId,
		FLMUINT64	ui64TransID,
		FLMUINT		uiIndexOrTable,
		FLMUINT64	ui64RowId,
		RCODE			rc) = 0;
};

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_IxClient : public F_Object
{
	virtual RCODE doIndexing(
		F_Db *	pDb,
		FLMUINT	uiIndexNum,
		FLMUINT	uiTableNum,
		F_Row *	pRow) = 0;
};

/****************************************************************************
Desc:	Types of information that can be gathered about a B-Tree.
****************************************************************************/
typedef struct SFLM_BTREE_LEVEL_INFO
{
	FLMUINT64		ui64BlockCount;
	FLMUINT64		ui64BlockLength;
	FLMUINT64		ui64BlockFreeSpace;
	FLMUINT64		ui64ElmOffsetOverhead;
	FLMUINT64		ui64ElmCount;
	FLMUINT64		ui64ContElmCount;
	FLMUINT64		ui64ElmFlagOvhd;
	FLMUINT64		ui64ElmKeyLengthOvhd;
	FLMUINT64		ui64ElmCountsOvhd;
	FLMUINT64		ui64ElmChildAddrsOvhd;
	FLMUINT64		ui64ElmDataLenOvhd;
	FLMUINT64		ui64ElmOADataLenOvhd;
	FLMUINT64		ui64ElmKeyLength;
	FLMUINT64		ui64ElmDataLength;
	
	// The following three are how ui64ElmKeyLength is subdivided.
	// They are only applicable to index keys.
	
	FLMUINT64		ui64KeyDataSize;
	FLMUINT64		ui64KeyIdSize;
	FLMUINT64		ui64KeyComponentLengthsSize;
	
	// Data only blocks
	
	FLMUINT64		ui64DataOnlyBlockCount;
	FLMUINT64		ui64DataOnlyBlockLength;
	FLMUINT64		ui64DataOnlyBlockFreeSpace;
} SFLM_BTREE_LEVEL_INFO;

/****************************************************************************
Desc:
****************************************************************************/
flminterface IF_BTreeInfoStatus : public F_Object
{
	virtual RCODE infoStatus(
		FLMUINT		uiCurrLfNum,
		FLMBOOL		bIsTable,
		char *		pszCurrLfName,
		FLMUINT		uiCurrLevel,
		FLMUINT64	ui64CurrLfBlockCount,
		FLMUINT64	ui64CurrLevelBlockCount,
		FLMUINT64	ui64TotalBlockCount) = 0;
};

/****************************************************************************
Desc:	BTree Info. Gatherer
****************************************************************************/
flminterface IF_BTreeInfo : public F_Object
{
	virtual void clearBTreeInfo( void) = 0;
	
	virtual RCODE collectIndexInfo(
		F_Db *					pDb,
		FLMUINT					uiIndexNum,
		IF_BTreeInfoStatus *	pInfoStatus) = 0;
		
	virtual RCODE collectTableInfo(
		F_Db *					pDb,
		FLMUINT					uiTableNum,
		IF_BTreeInfoStatus *	pInfoStatus) = 0;
		
	virtual FLMUINT getNumIndexes( void) = 0;
		
	virtual FLMUINT getNumTables( void) = 0;

	virtual FLMBOOL getIndexInfo(
		FLMUINT		uiNthIndex,
		FLMUINT *	puiIndexNum,
		char **		ppszIndexName,
		FLMUINT *	puiNumLevels) = 0;
		
	virtual FLMBOOL getTableInfo(
		FLMUINT		uiNthCollection,
		FLMUINT *	puiTableNum,
		char **		ppszTableName,
		FLMUINT *	puiNumLevels) = 0;
		
	virtual FLMBOOL getIndexLevelInfo(
		FLMUINT						uiNthIndex,
		FLMUINT						uiBTreeLevel,
		SFLM_BTREE_LEVEL_INFO *	pLevelInfo) = 0;

	virtual FLMBOOL getTableLevelInfo(
		FLMUINT						uiNthTable,
		FLMUINT						uiBTreeLevel,
		SFLM_BTREE_LEVEL_INFO *	pLevelInfo) = 0;
};
	
#endif // FLAIMSQL_H
