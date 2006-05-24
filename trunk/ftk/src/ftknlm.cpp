//-------------------------------------------------------------------------
// Desc:	I/O for Netware OS
// Tabs:	3
//
//		Copyright (c) 1998-2003,2005-2006 Novell, Inc. All Rights Reserved.
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
// $Id: fnlm.cpp 12362 2006-03-09 12:11:37 -0700 (Thu, 09 Mar 2006) dsanders $
//-------------------------------------------------------------------------

#include "ftksys.h"
#include "ftknlm.h"

#if defined( FLM_NLM)

#if defined( FLM_RING_ZERO_NLM)
	#define zMAX_COMPONENT_NAME						256
	#define zGET_INFO_VARIABLE_DATA_SIZE			(zMAX_COMPONENT_NAME * 2)
	
	#define zOK												0			// the operation succeeded
	#define zERR_NO_MEMORY								20000 	// insufficent memory to complete the request
	#define zERR_NOT_SUPPORTED							20011 	// the operation is not supported
	#define zERR_CONNECTION_NOT_LOGGED_IN			20007 	// the connection has not been logged in
	#define zERR_END_OF_FILE							20100 	// read past the end of file
	#define zERR_OUT_OF_SPACE							20103 	// no available disk space is left
	#define zERR_BAD_FILE_HANDLE						20401 	// the file handle is out of range, bad instance, or doesn't exist
	#define zERR_INVALID_NAME							20403		// path name is invalid -- bad syntax
	#define zERR_INVALID_CHAR_IN_NAME				20404 	// path name had an invalid character
	#define zERR_INVALID_PATH							20405 	// the path is syntactically incorrect
	#define zERR_NAME_NOT_FOUND_IN_DIRECTORY		20407 	// name does not exist in the direcory being searched
	#define zERR_NO_NAMES_IN_PATH						20409 	// a NULL file name was given
	#define zERR_NO_MORE_NAMES_IN_PATH 				20410 	// doing a wild search but ran out of names to search
	#define zERR_PATH_MUST_BE_FULLY_QUALIFIED		20411 	// path name must be fully qualified in this context
	#define zERR_FILE_ALREADY_EXISTS					20412 	// the given file already exists
	#define zERR_NAME_NO_LONGER_VALID				20413 	// the dir/file name is no longer valid
	#define zERR_DIRECTORY_NOT_EMPTY					20417 	// the directory still has files in it
	#define zERR_NO_FILES_FOUND						20424 	// no files matched the given wildcard pattern
	#define zERR_DIR_CANNOT_BE_OPENED				20435 	// the requested parent was not found
	#define zERR_NO_OPEN_PRIVILEGE					20438 	// no the right privileges to open the file
	#define zERR_NO_MORE_CONTEXT_HANDLE_IDS		20439 	// there are no more available context handle IDs
	#define zERR_INVALID_PATH_FORMAT					20441 	// the pathFormat is either invalid or unsupported
	#define zERR_ALL_FILES_IN_USE						20500 	// all files were in use
	#define zERR_SOME_FILES_IN_USE					20501 	// some of the files were in use
	#define zERR_ALL_FILES_READ_ONLY					20502 	// all files were READONLY
	#define zERR_SOME_FILES_READ_ONLY				20503 	// some of the files were READONLY
	#define zERR_ALL_NAMES_EXIST						20504 	// all of the names already existed
	#define zERR_SOME_NAMES_EXIST						20505 	// some of the names already existed
	#define zERR_NO_RENAME_PRIVILEGE					20506 	// you do not have privilege to rename the file
	#define zERR_RENAME_DIR_INVALID					20507 	// the selected directory may not be renamed
	#define zERR_RENAME_TO_OTHER_VOLUME				20508 	// a rename/move may not move the beast to a different volume
	#define zERR_CANT_RENAME_DATA_STREAMS			20509 	// not allowed to rename a data stream
	#define zERR_FILE_RENAME_IN_PROGRESS			20510 	// the file is already being renamed by a different process
	#define zERR_CANT_RENAME_TO_DELETED				20511 	// only deleted files may be renamed to a deleted state
	#define zERR_HOLE_IN_DIO_FILE  	            20651 	// DIO files cannot have holes
	#define zERR_BEYOND_EOF  	            		20652 	// DIO files cannot be read beyond EOF
	#define zERR_INVALID_PATH_SEPARATOR				20704 	// the name space does not support the requested path separator type
	#define zERR_VOLUME_SEPARATOR_NOT_SUPPORTED	20705 	// the name space does not support volume separators
	#define zERR_BAD_VOLUME_NAME   					20800 	// the given volume name is syntactically incorrect
	#define zERR_VOLUME_NOT_FOUND  					20801 	// the given volume name could not be found
	#define zERR_NO_SET_PRIVILEGE  					20850 	// does not have rights to modify metadata
	#define zERR_NO_CREATE_PRIVILEGE					20851		// does not have rights to create an object
	#define zERR_ACCESS_DENIED							20859 	// authorization/attributes denied access
	#define zERR_NO_WRITE_PRIVILEGE					20860 	// no granted write privileges
	#define zERR_NO_READ_PRIVILEGE					20861 	// no granted read privileges
	#define zERR_NO_DELETE_PRIVILEGE					20862 	// no delete privileges
	#define zERR_SOME_NO_DELETE_PRIVILEGE			20863 	// on wildcard some do not have delete privileges
	#define zERR_NO_SUCH_OBJECT						20867 	// no such object in the naming services
	#define zERR_CANT_DELETE_OPEN_FILE				20868 	// cant delete an open file without rights
	#define zERR_NO_CREATE_DELETE_PRIVILEGE		20869 	// no delete on create privileges
	#define zERR_NO_SALVAGE_PRIVILEGE				20870 	// no privileges to salvage this file
	#define zERR_FILE_READ_LOCKED						20905 	// cant grant read access to the file
	#define zERR_FILE_WRITE_LOCKED					20906 	// cant grant write access to the file
	
	#define zRR_READ_ACCESS								0x00000001
	#define zRR_WRITE_ACCESS							0x00000002
	#define zRR_DENY_READ								0x00000004
	#define zRR_DENY_WRITE								0x00000008
	#define zRR_SCAN_ACCESS								0x00000010
	#define zRR_ENABLE_IO_ON_COMPRESSED_DATA		0x00000100
	#define zRR_LEAVE_FILE_COMPRESSED	        	0x00000200
	#define zRR_DELETE_FILE_ON_CLOSE					0x00000400
	#define zRR_FLUSH_ON_CLOSE							0x00000800
	#define zRR_PURGE_IMMEDIATE_ON_CLOSE			0x00001000
	#define zRR_DIO_MODE									0x00002000
	#define zRR_ALLOW_SECURE_DIRECTORY_ACCESS		0x00020000
	#define zRR_TRANSACTION_ACTIVE					0x00100000
	#define zRR_READ_ACCESS_TO_SNAPSHOT				0x04000000
	#define zRR_DENY_RW_OPENER_CAN_REOPEN			0x08000000
	#define zRR_CREATE_WITHOUT_READ_ACCESS			0x10000000
	#define zRR_OPENER_CAN_DELETE_WHILE_OPEN		0x20000000
	#define zRR_CANT_DELETE_WHILE_OPEN				0x40000000
	#define zRR_DONT_UPDATE_ACCESS_TIME				0x80000000
	
	#define zFA_READ_ONLY		 						0x00000001
	#define zFA_HIDDEN 									0x00000002
	#define zFA_SYSTEM 									0x00000004
	#define zFA_EXECUTE									0x00000008
	#define zFA_SUBDIRECTORY	 						0x00000010
	#define zFA_ARCHIVE									0x00000020
	#define zFA_SHAREABLE		 						0x00000080
	#define zFA_SMODE_BITS		 						0x00000700
	#define zFA_NO_SUBALLOC								0x00000800
	#define zFA_TRANSACTION								0x00001000
	#define zFA_NOT_VIRTUAL_FILE						0x00002000
	#define zFA_IMMEDIATE_PURGE						0x00010000
	#define zFA_RENAME_INHIBIT	 						0x00020000
	#define zFA_DELETE_INHIBIT	 						0x00040000
	#define zFA_COPY_INHIBIT	 						0x00080000
	#define zFA_IS_ADMIN_LINK							0x00100000
	#define zFA_IS_LINK									0x00200000
	#define zFA_REMOTE_DATA_INHIBIT					0x00800000
	#define zFA_COMPRESS_FILE_IMMEDIATELY 			0x02000000
	#define zFA_DATA_STREAM_IS_COMPRESSED 			0x04000000
	#define zFA_DO_NOT_COMPRESS_FILE	  				0x08000000
	#define zFA_CANT_COMPRESS_DATA_STREAM 			0x20000000
	#define zFA_ATTR_ARCHIVE	 						0x40000000
	#define zFA_VOLATILE									0x80000000
	
	#define zNSPACE_DOS									0
	#define zNSPACE_MAC									1
	#define zNSPACE_UNIX									2
	#define zNSPACE_LONG									4
	#define zNSPACE_DATA_STREAM						6
	#define zNSPACE_EXTENDED_ATTRIBUTE				7
	#define zNSPACE_INVALID								(-1)
	#define zNSPACE_DOS_MASK							(1 << zNSPACE_DOS)
	#define zNSPACE_MAC_MASK							(1 << zNSPACE_MAC)
	#define zNSPACE_UNIX_MASK							(1 << zNSPACE_UNIX)
	#define zNSPACE_LONG_MASK							(1 << zNSPACE_LONG)
	#define zNSPACE_DATA_STREAM_MASK					(1 << zNSPACE_DATA_STREAM)
	#define zNSPACE_EXTENDED_ATTRIBUTE_MASK 		(1 << zNSPACE_EXTENDED_ATTRIBUTE)
	#define zNSPACE_ALL_MASK							(0xffffffff)
	
	#define zMODE_VOLUME_ID								0x80000000
	#define zMODE_UTF8									0x40000000
	#define zMODE_DELETED								0x20000000
	#define zMODE_LINK									0x10000000
	
	#define zCREATE_OPEN_IF_THERE						0x00000001
	#define zCREATE_TRUNCATE_IF_THERE				0x00000002
	#define zCREATE_DELETE_IF_THERE					0x00000004
	
	#define zMATCH_ALL_DERIVED_TYPES					0x00000001
	#define zMATCH_HIDDEN								0x1
	#define zMATCH_NON_HIDDEN							0x2
	#define zMATCH_DIRECTORY							0x4
	#define zMATCH_NON_DIRECTORY						0x8
	#define zMATCH_SYSTEM								0x10
	#define zMATCH_NON_SYSTEM							0x20
	#define zMATCH_ALL									(~0)
	
	#define zSETSIZE_NON_SPARSE_FILE					0x00000001
	#define zSETSIZE_NO_ZERO_FILL						0x00000002
	#define zSETSIZE_UNDO_ON_ERR	 					0x00000004
	#define zSETSIZE_PHYSICAL_ONLY	 				0x00000008
	#define zSETSIZE_LOGICAL_ONLY	 					0x00000010
	#define zSETSIZE_COMPRESSED      				0x00000020
	
	#define zMOD_FILE_ATTRIBUTES						0x00000001
	#define zMOD_CREATED_TIME							0x00000002
	#define zMOD_ARCHIVED_TIME							0x00000004
	#define zMOD_MODIFIED_TIME							0x00000008
	#define zMOD_ACCESSED_TIME							0x00000010
	#define zMOD_METADATA_MODIFIED_TIME				0x00000020
	#define zMOD_OWNER_ID								0x00000040
	#define zMOD_ARCHIVER_ID							0x00000080
	#define zMOD_MODIFIER_ID							0x00000100
	#define zMOD_METADATA_MODIFIER_ID				0x00000200
	#define zMOD_PRIMARY_NAMESPACE					0x00000400
	#define zMOD_DELETED_INFO							0x00000800
	#define zMOD_MAC_METADATA							0x00001000
	#define zMOD_UNIX_METADATA							0x00002000
	#define zMOD_EXTATTR_FLAGS							0x00004000
	#define zMOD_VOL_ATTRIBUTES						0x00008000
	#define zMOD_VOL_NDS_OBJECT_ID					0x00010000
	#define zMOD_VOL_MIN_KEEP_SECONDS				0x00020000
	#define zMOD_VOL_MAX_KEEP_SECONDS				0x00040000
	#define zMOD_VOL_LOW_WATER_MARK					0x00080000
	#define zMOD_VOL_HIGH_WATER_MARK					0x00100000
	#define zMOD_POOL_ATTRIBUTES						0x00200000
	#define zMOD_POOL_NDS_OBJECT_ID					0x00400000
	#define zMOD_VOL_DATA_SHREDDING_COUNT			0x00800000
	#define zMOD_VOL_QUOTA								0x01000000
	
	/***************************************************************************
	Desc:
	***************************************************************************/
	enum zGetInfoMask_t
	{
		zGET_STD_INFO										= 0x1,
		zGET_NAME											= 0x2,
		zGET_ALL_NAMES										= 0x4,
		zGET_PRIMARY_NAMESPACE							= 0x8,
		zGET_TIMES_IN_SECS								= 0x10,
		zGET_TIMES_IN_MICROS								= 0x20,
		zGET_IDS												= 0x40,
		zGET_STORAGE_USED									= 0x80,
		zGET_BLOCK_SIZE									= 0x100,
		zGET_COUNTS											= 0x200,
		zGET_EXTENDED_ATTRIBUTE_INFO					= 0x400,
		zGET_DATA_STREAM_INFO							= 0x800,
		zGET_DELETED_INFO									= 0x1000,
		zGET_MAC_METADATA									= 0x2000,
		zGET_UNIX_METADATA								= 0x4000,
		zGET_EXTATTR_FLAGS								= 0x8000,
		zGET_VOLUME_INFO									= 0x10000,
		zGET_VOL_SALVAGE_INFO							= 0x20000,
		zGET_POOL_INFO										= 0x40000
	};
	
	/***************************************************************************
	Desc:
	***************************************************************************/
	enum
	{
		zINFO_VERSION_A = 1
	};
	
	/***************************************************************************
	Desc:
	***************************************************************************/
	typedef enum FileType_t
	{
		zFILE_UNKNOWN,
		zFILE_REGULAR,
		zFILE_EXTENDED_ATTRIBUTE,
		zFILE_NAMED_DATA_STREAM,
		zFILE_PIPE,
		zFILE_VOLUME,
		zFILE_POOL,
		zFILE_MAX
	} FileType_t;
	
	/***************************************************************************
	Desc:
	***************************************************************************/
	typedef struct	GUID_t
	{
		LONG					timeLow;
		WORD					timeMid;
		WORD					timeHighAndVersion;
		BYTE					clockSeqHighAndReserved;
		BYTE					clockSeqLow;
		BYTE					node[ 6];
	} GUID_t;
	
	/***************************************************************************
	Desc:
	***************************************************************************/
	typedef struct zMacInfo_s
	{
		BYTE 					finderInfo[32];
		BYTE 					proDOSInfo[6];
		BYTE					filler[2];
		LONG					dirRightsMask;
	} zMacInfo_s;
	
	/***************************************************************************
	Desc:
	***************************************************************************/
	typedef struct zUnixInfo_s
	{
		LONG					fMode;
		LONG					rDev;
		LONG					myFlags;
		LONG					nfsUID;
		LONG 					nfsGID;
		LONG					nwUID;
		LONG					nwGID;
		LONG					nwEveryone;
		LONG					nwUIDRights;
		LONG					nwGIDRights;
		LONG					nwEveryoneRights;
		BYTE					acsFlags;
		BYTE					firstCreated;
		FLMINT16				variableSize;
	} zUnixInfo_s;
	
	typedef struct zVolumeInfo_s
	{
		GUID_t				volumeID;
		GUID_t				ndsObjectID;
		LONG					volumeState;
		LONG					nameSpaceMask;
		
		struct
		{
			FLMUINT64		enabled;
			FLMUINT64		enableModMask;
			FLMUINT64		supported;
		} features;
		
		FLMUINT64			maximumFileSize;	
		FLMUINT64			totalSpaceQuota;
		FLMUINT64			numUsedBytes;
		FLMUINT64			numObjects;
		FLMUINT64			numFiles;
		LONG					authModelID;
		LONG					dataShreddingCount;
		
		struct
		{
			FLMUINT64		purgeableBytes;
			FLMUINT64		nonPurgeableBytes;
			FLMUINT64		numDeletedFiles;
			FLMUINT64		oldestDeletedTime;
			LONG				minKeepSeconds;
			LONG				maxKeepSeconds;
			LONG				lowWaterMark;
			LONG				highWaterMark;
		} salvage;
		
		struct
		{
			FLMUINT64		numCompressedFiles;
			FLMUINT64		numCompDelFiles;
			FLMUINT64		numUncompressibleFiles;
			FLMUINT64		numPreCompressedBytes;
			FLMUINT64		numCompressedBytes;
		} comp;
		 
	} zVolumeInfo_s;
	
	/***************************************************************************
	Desc:
	***************************************************************************/
	typedef struct zPoolInfo_s
	{
		GUID_t				poolID;
		GUID_t				ndsObjectID;
		LONG					poolState;
		LONG					nameSpaceMask;
		
		struct 
		{
			FLMUINT64		enabled;
			FLMUINT64		enableModMask;
			FLMUINT64		supported;
		} features;
		
		FLMUINT64			totalSpace;
		FLMUINT64			numUsedBytes;
		FLMUINT64			purgeableBytes;
		FLMUINT64			nonPurgeableBytes;
	} zPoolInfo_s;
	
	/***************************************************************************
	Desc:
	***************************************************************************/
	typedef struct zInfo_s
	{
		LONG					infoVersion;
		FLMINT				totalBytes;
		FLMINT				nextByte;
		LONG					padding;
		FLMUINT64			retMask;
		
		struct 
		{
			FLMUINT64		zid;
			FLMUINT64		dataStreamZid;
			FLMUINT64		parentZid;
			FLMUINT64		logicalEOF;
			GUID_t			volumeID;
			LONG				fileType;
			LONG				fileAttributes;
			LONG				fileAttributesModMask;
			LONG				padding;
		} std;
	
		struct
		{
			FLMUINT64		physicalEOF;
			FLMUINT64		dataBytes;
			FLMUINT64		metaDataBytes;
		} storageUsed;
	
		LONG					primaryNameSpaceID;
		LONG 					nameStart;
	
		struct 
		{
			LONG 				numEntries;
			LONG 				fileNameArray;
		} names;
	
		struct
		{
			FLMUINT64		created;
			FLMUINT64		archived;
			FLMUINT64		modified;
			FLMUINT64		accessed;
			FLMUINT64		metaDataModified;
		} time;
	
		struct 
		{
			GUID_t 			owner;
			GUID_t 			archiver;
			GUID_t 			modifier;
			GUID_t 			metaDataModifier;
		} id;
	
		struct 
		{
			LONG	 			size;
			LONG	 			sizeShift;
		} blockSize;
	
		struct 
		{
			LONG	 			open;
			LONG	 			hardLink;
		} count;
	
		struct 
		{
			LONG	 			count;
			LONG	 			totalNameSize;
			FLMUINT64		totalDataSize;
		} dataStream;
	
		struct 
		{
			LONG	 			count;
			LONG	 			totalNameSize;
			FLMUINT64		totalDataSize;
		} extAttr;
	
		struct 
		{
			FLMUINT64		time;
			GUID_t 			id;
		} deleted;
	
		struct 
		{
			zMacInfo_s 		info;
		} macNS;
	
		struct 
		{
			zUnixInfo_s 	info;
			LONG				offsetToData;
		} unixNS;
	
		zVolumeInfo_s		vol;
		zPoolInfo_s			pool;
		LONG					extAttrUserFlags;
		BYTE					variableData[zGET_INFO_VARIABLE_DATA_SIZE];
	
	} zInfo_s;
	
	RCODE DfsMapError(
		LONG					lResult,
		RCODE					defaultRc);
	
	FLMUINT f_getNSSOpenFlags(
		FLMUINT				uiAccess,
		FLMBOOL				bDoDirectIo);
	
	typedef FLMINT (* zROOT_KEY_FUNC)(
		FLMUINT				connectionID,
		FLMINT64 *			retRootKey);
	
	typedef FLMINT (* zCLOSE_FUNC)(
		FLMINT64				key);
	
	typedef FLMINT (* zCREATE_FUNC)(
		FLMINT64				key,
		FLMUINT				taskID,	
		FLMUINT64			xid,
		FLMUINT				nameSpace,
		const void *		path,
		FLMUINT				fileType,
		FLMUINT64			fileAttributes,
		FLMUINT				createFlags,
		FLMUINT				requestedRights,
		FLMINT64 *			retKey);
	
	typedef FLMINT (* zOPEN_FUNC)(
		FLMINT64				key,
		FLMUINT				taskID,
		FLMUINT				nameSpace,
		const void *		path,
		FLMUINT				requestedRights,
		FLMINT64 *			retKey);
	
	typedef FLMINT (* zDELETE_FUNC)(
		FLMINT64				key,
		FLMUINT64			xid,
		FLMUINT				nameSapce,
		const void *		path,
		FLMUINT				match,
		FLMUINT				deleteFlags);
	
	typedef FLMINT (* zREAD_FUNC)(
		FLMINT64				key,
		FLMUINT64			xid,	
		FLMUINT64			startingOffset,
		FLMUINT				bytesToRead,
		void *				retBuffer,
		FLMUINT *			retBytesRead);
	
	typedef FLMINT (* zDIO_READ_FUNC)(
		FLMINT64				key,
		FLMUINT64			unitOffset,
		FLMUINT				unitsToRead,
		FLMUINT				callbackData,
		void					(*dioReadCallBack)(
									FLMUINT	reserved,
									FLMUINT	callbackData,
									FLMUINT 	retStatus),
		void *				retBuffer);
	
	typedef FLMINT (* zGET_INFO_FUNC)(
		FLMINT64				key,
		FLMUINT64			getInfoMask,
		FLMUINT				sizeRetGetInfo,
		FLMUINT				infoVersion,
		zInfo_s *			retGetInfo);
	
	typedef FLMINT (* zMODIFY_INFO_FUNC)(
		FLMINT64				key,
		FLMUINT64			xid,
		FLMUINT64			modifyInfoMask,
		FLMUINT				sizeModifyInfo,
		FLMUINT				infoVersion,
		const zInfo_s *	modifyInfo);
	
	typedef FLMINT (* zSET_EOF_FUNC)(
		FLMINT64				key,
		FLMUINT64			xid,	
		FLMUINT64			startingOffset,
		FLMUINT				flags);
	
	typedef FLMINT (* zWRITE_FUNC)(
		FLMINT64				key,
		FLMUINT64			xid,	
		FLMUINT64			startingOffset,
		FLMUINT				bytesToWrite,
		const void *		buffer,
		FLMUINT *			retBytesWritten);
	
	typedef FLMINT (* zDIO_WRITE_FUNC)(
		FLMINT64				key,
		FLMUINT64			unitOffset,
		FLMUINT				unitsToWrite,
		FLMUINT				callbackData,
		void					(*dioWriteCallBack)(
									FLMUINT	reserved,
									FLMUINT	callbackData,
									FLMUINT	retStatus),
		const void *		buffer);
	
	typedef FLMINT (* zRENAME_FUNC)(
		FLMINT64				key,
		FLMUINT64			xid,
		FLMUINT				srcNameSpace,
		const void *		srcPath,
		FLMUINT				srcMatchAttributes,
		FLMUINT				dstNameSpace,
		const void *		dstPath,
		FLMUINT				renameFlags);
	
	typedef BOOL (* zIS_NSS_VOLUME_FUNC)(
		const char *		path);
	
	FSTATIC zIS_NSS_VOLUME_FUNC		gv_zIsNSSVolumeFunc = NULL;
	FSTATIC zROOT_KEY_FUNC				gv_zRootKeyFunc = NULL;
	FSTATIC zCLOSE_FUNC					gv_zCloseFunc = NULL;
	FSTATIC zCREATE_FUNC					gv_zCreateFunc = NULL;
	FSTATIC zOPEN_FUNC					gv_zOpenFunc = NULL;
	FSTATIC zDELETE_FUNC					gv_zDeleteFunc = NULL;
	FSTATIC zREAD_FUNC					gv_zReadFunc = NULL;
	FSTATIC zDIO_READ_FUNC				gv_zDIOReadFunc = NULL;
	FSTATIC zGET_INFO_FUNC				gv_zGetInfoFunc = NULL;
	FSTATIC zMODIFY_INFO_FUNC			gv_zModifyInfoFunc = NULL;
	FSTATIC zSET_EOF_FUNC				gv_zSetEOFFunc = NULL;
	FSTATIC zWRITE_FUNC					gv_zWriteFunc = NULL;
	FSTATIC zDIO_WRITE_FUNC				gv_zDIOWriteFunc = NULL;
	FSTATIC zRENAME_FUNC					gv_zRenameFunc = NULL;
	
	static unsigned long					yieldTimeSlice = 0;
	static unsigned long					stackFrame[0x100] = {0};
	static unsigned long *				ThreadFeederQue = 0;
	static struct OldPerCpuStruct *	pPerCpuDataArea = 0;
	
	extern "C" unsigned long 			CpuCurrentProcessor;
	extern "C" unsigned long			WorkToDoListHead;
	static void *							gv_MyModuleHandle = NULL;
	static FLMATOMIC						gv_NetWareStartupCount = 0;
	rtag_t									gv_lAllocRTag = 0;
	
	struct OldPerCpuStruct
	{
		unsigned long reserved0[24];
		unsigned long reserved1[3];
		unsigned long PSD_ThreadStartClocks;
		unsigned long reserved2[4];
		unsigned long reserved3[40];
		unsigned long PSD_LocalWTDHead;
	};
	
	#if !defined( __MWERKS__)
		extern unsigned long ReadInternalClock(void);
	#else
		unsigned long ReadInternalClock(void);
	#endif
	
	FSTATIC void ConvertToQualifiedNWPath(
		const char *		pInputPath,
		char *				pQualifiedPath);
	
	FSTATIC RCODE nssTurnOffRenameInhibit(
		const char *		pszFileName);
	
	FSTATIC LONG ConvertPathToLNameFormat(
		const char *		pPath,
		LONG *				plVolumeID,
		FLMBOOL *			pbNssVolume,
		FLMBYTE *			pLNamePath,
		LONG *				plLNamePathCount);
	
	FSTATIC void DirectIONoWaitCallBack(
		LONG					unknownAlwaysZero,
		LONG					callbackData,
		LONG 					completionCode);
	
	FSTATIC void nssDioCallback(
		FLMUINT				reserved,
		FLMUINT				UserData,
		FLMUINT				retStatus);
	
	FSTATIC RCODE MapNSSError(
		FLMINT				lStatus,
		RCODE					defaultRc);
	
	FLMINT64		gv_NssRootKey;
	FLMBOOL		gv_bNSSKeyInitialized = FALSE;

#endif

/***************************************************************************
Desc:
***************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
#if !defined( __MWERKS__)
	#pragma aux ReadInternalClock = \
	0x0F 0x31 \
	modify exact [EAX EDX];
#else
	unsigned long ReadInternalClock(void)
	{
		__asm
		{
			rdtsc
			ret
		}
	}
#endif
#endif

/***************************************************************************
Desc:	Initialize the root NSS key.
***************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE f_nssInitialize( void)
{
	RCODE		rc = NE_FLM_OK;
	FLMINT	lStatus;

	if (!gv_bNSSKeyInitialized)
	{
		// Import the required NSS functions

		if( (gv_zIsNSSVolumeFunc = (zIS_NSS_VOLUME_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x0C" "zIsNSSVolume")) == NULL)
		{
			// NSS is not available on this server.  Jump to exit.
			goto Exit;
		}
		
		if( (gv_zRootKeyFunc = (zROOT_KEY_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x08" "zRootKey")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}

		if( (gv_zCloseFunc = (zCLOSE_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x06" "zClose")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}

		if( (gv_zCreateFunc = (zCREATE_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x07" "zCreate")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}

		if( (gv_zOpenFunc = (zOPEN_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x05" "zOpen")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		
		if( (gv_zDeleteFunc = (zDELETE_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x07" "zDelete")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		
		if( (gv_zReadFunc = (zREAD_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x05" "zRead")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		
		if( (gv_zDIOReadFunc = (zDIO_READ_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x08" "zDIORead")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		
		if( (gv_zGetInfoFunc = (zGET_INFO_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x08" "zGetInfo")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		
		if( (gv_zModifyInfoFunc = (zMODIFY_INFO_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x0B" "zModifyInfo")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		
		if( (gv_zSetEOFFunc = (zSET_EOF_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x07" "zSetEOF")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		
		if( (gv_zWriteFunc = (zWRITE_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x06" "zWrite")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		
		if( (gv_zDIOWriteFunc = (zDIO_WRITE_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x09" "zDIOWrite")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}

		if( (gv_zRenameFunc = (zRENAME_FUNC)ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x07" "zRename")) == NULL)
		{
			flmAssert( 0);
			rc = RC_SET( NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}

		// Get the NSS root key

		if ((lStatus = gv_zRootKeyFunc( 0, &gv_NssRootKey)) != zOK)
		{
			rc = MapNSSError( lStatus, NE_FLM_INITIALIZING_IO_SYSTEM);
			goto Exit;
		}
		gv_bNSSKeyInitialized = TRUE;
	}

Exit:

	return( rc);
}
#endif

/***************************************************************************
Desc:	Close the root NSS key.
***************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
void f_nssUninitialize( void)
{
	if (gv_bNSSKeyInitialized)
	{
		(void)gv_zCloseFunc( gv_NssRootKey);
		gv_bNSSKeyInitialized = FALSE;
	}
}
#endif

/***************************************************************************
Desc:	Maps NSS errors to IO errors.
***************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FSTATIC RCODE MapNSSError(
	FLMINT	lStatus,
	RCODE		defaultRc)
{
	RCODE		rc;

	switch (lStatus)
	{
		case zERR_FILE_ALREADY_EXISTS:
		case zERR_DIRECTORY_NOT_EMPTY:
		case zERR_DIR_CANNOT_BE_OPENED:
		case zERR_NO_SET_PRIVILEGE:
		case zERR_NO_CREATE_PRIVILEGE:
		case zERR_ACCESS_DENIED:
		case zERR_NO_WRITE_PRIVILEGE:
		case zERR_NO_READ_PRIVILEGE:
		case zERR_NO_DELETE_PRIVILEGE:
		case zERR_SOME_NO_DELETE_PRIVILEGE:
		case zERR_CANT_DELETE_OPEN_FILE:
		case zERR_NO_CREATE_DELETE_PRIVILEGE:
		case zERR_NO_SALVAGE_PRIVILEGE:
		case zERR_FILE_READ_LOCKED:
		case zERR_FILE_WRITE_LOCKED:
			rc = RC_SET( NE_FLM_IO_ACCESS_DENIED);
			break;

		case zERR_BAD_FILE_HANDLE:
			rc = RC_SET( NE_FLM_IO_BAD_FILE_HANDLE);
			break;

		case zERR_OUT_OF_SPACE:
			rc = RC_SET( NE_FLM_IO_DISK_FULL);
			break;

		case zERR_NO_OPEN_PRIVILEGE:
			rc = RC_SET( NE_FLM_IO_OPEN_ERR);
			break;

		case zERR_NAME_NOT_FOUND_IN_DIRECTORY:
		case zERR_NO_FILES_FOUND:
		case zERR_VOLUME_NOT_FOUND:
		case zERR_NO_SUCH_OBJECT:
		case zERR_INVALID_NAME:
		case zERR_INVALID_CHAR_IN_NAME:
		case zERR_INVALID_PATH:
		case zERR_NO_NAMES_IN_PATH:
		case zERR_NO_MORE_NAMES_IN_PATH:
		case zERR_PATH_MUST_BE_FULLY_QUALIFIED:
		case zERR_NAME_NO_LONGER_VALID:
		case zERR_INVALID_PATH_FORMAT:
		case zERR_INVALID_PATH_SEPARATOR:
		case zERR_VOLUME_SEPARATOR_NOT_SUPPORTED:
		case zERR_BAD_VOLUME_NAME:
			rc = RC_SET( NE_FLM_IO_PATH_NOT_FOUND);
			break;

		case zERR_NO_MORE_CONTEXT_HANDLE_IDS:
			rc = RC_SET( NE_FLM_IO_TOO_MANY_OPEN_FILES);
			break;
		case zERR_ALL_FILES_IN_USE:
		case zERR_SOME_FILES_IN_USE:
		case zERR_ALL_FILES_READ_ONLY:
		case zERR_SOME_FILES_READ_ONLY:
		case zERR_ALL_NAMES_EXIST:
		case zERR_SOME_NAMES_EXIST:
		case zERR_NO_RENAME_PRIVILEGE:
		case zERR_RENAME_DIR_INVALID:
		case zERR_RENAME_TO_OTHER_VOLUME:
		case zERR_CANT_RENAME_DATA_STREAMS:
		case zERR_FILE_RENAME_IN_PROGRESS:
		case zERR_CANT_RENAME_TO_DELETED:
			rc = RC_SET( NE_FLM_IO_RENAME_FAILURE);
			break;

		case zERR_CONNECTION_NOT_LOGGED_IN:
			rc = RC_SET( NE_FLM_IO_CONNECT_ERROR);
			break;
		case zERR_NO_MEMORY:
			rc = RC_SET( NE_FLM_MEM);
			break;
		case zERR_NOT_SUPPORTED:
			rc = RC_SET( NE_FLM_NOT_IMPLEMENTED);
			break;
		case zERR_END_OF_FILE:
		case zERR_BEYOND_EOF:
			rc = RC_SET( NE_FLM_IO_END_OF_FILE);
			break;

		default:
			rc = RC_SET( defaultRc);
			break;
	}
	
	return( rc );
}
#endif

/***************************************************************************
Desc:	Maps direct IO errors to IO errors.
fix: we shouldn't have 2 copies of this function.  this is just temporary.
      long term, we need to make the FDFS.CPP version public.
***************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE DfsMapError(
	LONG		lResult,
	RCODE		defaultRc)
{
	switch (lResult)
	{
		case DFSHoleInFileError:
		case DFSOperationBeyondEndOfFile:
			return( RC_SET( NE_FLM_IO_END_OF_FILE));
		case DFSHardIOError:
		case DFSInvalidFileHandle:
			return( RC_SET( NE_FLM_IO_BAD_FILE_HANDLE));
		case DFSNoReadPrivilege:
			return( RC_SET( NE_FLM_IO_ACCESS_DENIED));
		case DFSInsufficientMemory:
			return( RC_SET( NE_FLM_MEM));
		default:
			return( RC_SET( defaultRc));
	}
}
#endif

/****************************************************************************
Desc:		Map flaim I/O flags to NDS I/O flags for NSS volumes
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FLMUINT f_getNSSOpenFlags(
   FLMUINT	uiAccess,
	FLMBOOL	bDoDirectIo)
{
	FLMUINT	lFlags = zRR_ALLOW_SECURE_DIRECTORY_ACCESS |
						zRR_CANT_DELETE_WHILE_OPEN;

	if (uiAccess & (FLM_IO_RDONLY | FLM_IO_RDWR))
	{
		lFlags |= zRR_READ_ACCESS;
	}
	if (uiAccess & FLM_IO_RDWR)
	{
		lFlags |= zRR_WRITE_ACCESS;
	}

	if (uiAccess & FLM_IO_SH_DENYRW)
	{
		lFlags |= zRR_DENY_READ;
	}
	if (uiAccess & (FLM_IO_SH_DENYWR | FLM_IO_SH_DENYRW))
	{
		lFlags |= zRR_DENY_WRITE;
	}
	if (bDoDirectIo)
	{
		lFlags |= zRR_DIO_MODE;
	}
	return( lFlags );
}
#endif

/****************************************************************************
Desc:		Map flaim I/O flags to NetWare I/O flags
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
LONG f_getNWOpenFlags(
   FLMUINT		uiAccess,
	FLMBOOL		bDoDirectIo)
{
	LONG	lFlags = 0;

	if (uiAccess & (FLM_IO_RDONLY | FLM_IO_RDWR))
	{
		lFlags |= READ_ACCESS_BIT;
	}
	
	if (uiAccess & FLM_IO_RDWR)
	{
		lFlags |= WRITE_ACCESS_BIT;
	}

	if (uiAccess & FLM_IO_SH_DENYRW )
	{
		lFlags |= DENY_READ_BIT;
	}
	
	if (uiAccess & (FLM_IO_SH_DENYWR | FLM_IO_SH_DENYRW))
	{
		lFlags |= DENY_WRITE_BIT;
	}
	
	if (bDoDirectIo)
	{
		lFlags |= NEVER_READ_AHEAD_BIT;
	}
	
	return( lFlags );
}
#endif

/****************************************************************************
Desc:		Default Constructor for F_FileHdl class
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
F_FileHdl::F_FileHdl()
{
	// Should call the base class constructor automatically.

	m_lFileHandle = -1;
	m_lOpenAttr = 0;
	m_uiCurrentPos = 0;
	m_lVolumeID = F_NW_DEFAULT_VOLUME_NUMBER;
	m_bDoSuballocation = FALSE;
	m_lLNamePathCount = 0;
	m_pszIoPath = NULL;
	m_uiExtendSize = 0;
	m_uiMaxFileSize = f_getMaxFileSize();
	m_uiMaxAutoExtendSize = m_uiMaxFileSize;

	// Direct IO members
	m_bDoDirectIO = FALSE;	// TRUE = do direct IO-style read/writes
	m_lSectorsPerBlock = 0;
	m_lMaxBlocks = 0;

	m_bNSS = FALSE;
	m_bNSSFileOpen = FALSE;
}
#endif

/***************************************************************************
Desc:
***************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
F_FileHdl::~F_FileHdl( void)
{
	if( m_bFileOpened)
	{
		(void)close();
	}
}
#endif

/***************************************************************************
Desc:		Open or create a file.
***************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::openOrCreate(
	const char *		pFileName,
   FLMUINT				uiAccess,
	FLMBOOL				bCreateFlag)
{
	RCODE					rc = NE_FLM_OK;
	LONG					unused;
	void *				unused2;
	char *				pszQualifiedPath = NULL;
	LONG					lErrorCode;
	FLMBYTE *			pTmpLNamePath;
	char *				pSaveFileName;
	FLMBYTE *			pLNamePath;
	LONG *				plLNamePathCount;
	LONG					LNamePathCount;
	struct VolumeInformationStructure *
							pVolumeInfo;
	char *				pszTemp;
	char *				pIoDirPath;
	FLMBOOL				bNssVolume = FALSE;
	IF_FileSystem *	pFileSystem = f_getFileSysPtr();

	m_bDoDirectIO = (uiAccess & FLM_IO_DIRECT) ? TRUE : FALSE;

	if( uiAccess & FLM_IO_DELETE_ON_RELEASE)
	{
		if( !m_pszIoPath)
		{
			if( RC_BAD( rc = f_alloc( F_PATH_MAX_SIZE, &m_pszIoPath)))
			{
				goto Exit;
			}
		}
		
		f_strcpy( m_pszIoPath, pFileName);
		m_bDeleteOnClose = TRUE;
	}
	else
	{
		m_bDeleteOnClose = FALSE;
	}

	if (RC_BAD( rc = f_alloc(
								(FLMUINT)(F_PATH_MAX_SIZE +
											 F_PATH_MAX_SIZE +
											 F_PATH_MAX_SIZE +
											 F_PATH_MAX_SIZE +
											 sizeof( struct VolumeInformationStructure) +
											 F_PATH_MAX_SIZE),
											 &pszQualifiedPath)))
	{
		goto Exit;
	}

	pTmpLNamePath = (((FLMBYTE *)pszQualifiedPath) + F_PATH_MAX_SIZE);
	pSaveFileName = (((char *)pTmpLNamePath) + F_PATH_MAX_SIZE);
	pIoDirPath = (((char *)pSaveFileName) + F_PATH_MAX_SIZE);
	pVolumeInfo = (struct VolumeInformationStructure *)
								(((char *)pIoDirPath) + F_PATH_MAX_SIZE);
	pszTemp = (char *)(((char *)(pVolumeInfo)) +
									sizeof( struct VolumeInformationStructure));


	// Save the file name in case we have to create the directory

	if((bCreateFlag) && (uiAccess & FLM_IO_CREATE_DIR))
	{
		f_strcpy( pSaveFileName, pFileName);
	}

	if( !m_pszIoPath)
	{
		pLNamePath = pTmpLNamePath;
		plLNamePathCount = &LNamePathCount;
	}
	else
	{
		pLNamePath = (FLMBYTE *)m_pszIoPath;
		plLNamePathCount = &m_lLNamePathCount;
	}
	
	ConvertToQualifiedNWPath( pFileName, pszQualifiedPath);

	lErrorCode = ConvertPathToLNameFormat(
		pszQualifiedPath,
		&m_lVolumeID,
		&bNssVolume,
		pLNamePath,
		plLNamePathCount);
	if( lErrorCode != 0)
	{
      rc = f_mapPlatformError( lErrorCode, NE_FLM_PARSING_FILE_NAME);
		goto Exit;
   }

	// Determine if the volume is NSS or not

	if (gv_bNSSKeyInitialized && bNssVolume)
	{
		m_bNSS = TRUE;
	}

	if ( m_bDoDirectIO )
	{
		if (!m_bNSS)
		{
			lErrorCode = 
				ReturnVolumeMappingInformation( m_lVolumeID, pVolumeInfo);
			if (lErrorCode != 0)
			{
				rc = DfsMapError( lErrorCode, NE_FLM_INITIALIZING_IO_SYSTEM);
				goto Exit;
			}
			
			m_lSectorsPerBlock = (LONG)(pVolumeInfo->VolumeAllocationUnitSizeInBytes /
										FLM_NLM_SECTOR_SIZE);
			m_lMaxBlocks = (LONG)(m_uiMaxFileSize /
								(FLMUINT)pVolumeInfo->VolumeAllocationUnitSizeInBytes);
		}
		else
		{
			m_lMaxBlocks = (LONG)(m_uiMaxFileSize / (FLMUINT)65536);
		}
	}

	// Set up the file characteristics requested by caller.

	if (bCreateFlag)
	{

		// File is to be created

		if (f_netwareTestIfFileExists( pszQualifiedPath ) == NE_FLM_OK)
		{
			if (uiAccess & FLM_IO_EXCL)
			{
				rc = RC_SET( NE_FLM_IO_ACCESS_DENIED);
				goto Exit;
			}
			
			(void)f_netwareDeleteFile( pszQualifiedPath);
		}
	}

	// Try to create or open the file

	if (m_bNSS)
	{
		FLMINT	lStatus;

		if (bCreateFlag)
		{
			FLMUINT64	qFileAttr;

			qFileAttr = (FLMUINT64)(((m_bDoSuballocation)
									 ? (FLMUINT)(zFA_DO_NOT_COMPRESS_FILE)
									 : (FLMUINT)(zFA_NO_SUBALLOC |
												 zFA_DO_NOT_COMPRESS_FILE)) |
									zFA_IMMEDIATE_PURGE);

Retry_NSS_Create:

			m_lOpenAttr = f_getNSSOpenFlags( uiAccess, m_bDoDirectIO);
			if ((lStatus = gv_zCreateFunc( gv_NssRootKey, 1, 0,
				zNSPACE_LONG | zMODE_UTF8, pszQualifiedPath, zFILE_REGULAR,
				qFileAttr, zCREATE_DELETE_IF_THERE, (FLMUINT)m_lOpenAttr,
				&m_NssKey)) != zOK)
			{
				if (uiAccess & FLM_IO_CREATE_DIR)
				{
					uiAccess &= ~FLM_IO_CREATE_DIR;

					// Remove the file name for which we are creating the directory.

					if( pFileSystem->pathReduce( pSaveFileName, 
						pIoDirPath, pszTemp) == NE_FLM_OK)
					{
						if (RC_OK( pFileSystem->createDir( pIoDirPath)))
						{
							goto Retry_NSS_Create;
						}
					}
				}
				
				rc = MapNSSError( lStatus,
							(RCODE)(m_bDoDirectIO
									  ? (RCODE)NE_FLM_DIRECT_CREATING_FILE
									  : (RCODE)NE_FLM_CREATING_FILE));
				goto Exit;
			}
		}
		else
		{
			m_lOpenAttr = f_getNSSOpenFlags( uiAccess, m_bDoDirectIO);
			if ((lStatus = gv_zOpenFunc( gv_NssRootKey, 1,
				zNSPACE_LONG | zMODE_UTF8, pszQualifiedPath, (FLMUINT)m_lOpenAttr,
				&m_NssKey)) != zOK)
			{
				rc = MapNSSError( lStatus,
							(RCODE)(m_bDoDirectIO
									  ? (RCODE)NE_FLM_DIRECT_OPENING_FILE
									  : (RCODE)NE_FLM_OPENING_FILE));
				goto Exit;
			}
		}
		m_bNSSFileOpen = TRUE;
	}
	else
	{
		if (bCreateFlag)
		{
			m_lOpenAttr = (LONG)(((m_bDoSuballocation)
											 ? (LONG)(DO_NOT_COMPRESS_FILE_BIT)
											 : (LONG)(NO_SUBALLOC_BIT |
														 DO_NOT_COMPRESS_FILE_BIT)) | 
										IMMEDIATE_PURGE_BIT);

Retry_Create:
			lErrorCode = CreateFile( 0, 1, m_lVolumeID, 0, (BYTE *)pLNamePath,
				*plLNamePathCount, LONGNameSpace, m_lOpenAttr, 0xff,
				PrimaryDataStream, &m_lFileHandle, &unused, &unused2
				);

			if ((lErrorCode != 0) && (uiAccess & FLM_IO_CREATE_DIR))
			{
				uiAccess &= ~FLM_IO_CREATE_DIR;

				// Remove the file name for which we are creating the directory

				if( pFileSystem->pathReduce( pSaveFileName, 
					pIoDirPath, pszTemp) == NE_FLM_OK)
				{
					if( RC_OK( pFileSystem->createDir( pIoDirPath)))
					{
						goto Retry_Create;
					}
				}
			}

			// Too many error codes map to 255, so we put in a special
			// case check here.

			if( lErrorCode == 255)
			{
				rc = RC_SET( NE_FLM_IO_ACCESS_DENIED);
				goto Exit;
			}
		}
		else
		{
			m_lOpenAttr = f_getNWOpenFlags(uiAccess, m_bDoDirectIO);
			lErrorCode = OpenFile( 0, 1, m_lVolumeID, 0, (BYTE *)pLNamePath,
				*plLNamePathCount, LONGNameSpace, 0, m_lOpenAttr,
				PrimaryDataStream, &m_lFileHandle, &unused, &unused2);

			// Too many error codes map to 255, so we put in a special
			// case check here.

			if( lErrorCode == 255)
			{
				rc = RC_SET( NE_FLM_IO_PATH_NOT_FOUND);
				goto Exit;
			}
		}

		// Check if the file operation was successful

		if ( lErrorCode != 0)
		{
			rc = f_mapPlatformError( lErrorCode,
						(RCODE)(bCreateFlag
								  ? (RCODE)(m_bDoDirectIO
												? (RCODE)NE_FLM_DIRECT_CREATING_FILE
												: (RCODE)NE_FLM_CREATING_FILE)
								  : (RCODE)(m_bDoDirectIO
												? (RCODE)NE_FLM_DIRECT_OPENING_FILE
												: (RCODE)NE_FLM_OPENING_FILE)));
			goto Exit;
		}

		if (bCreateFlag)
		{
			// Revoke the file handle rights and close the file
			// (signified by passing 2 for the QueryFlag parameter).
			// If this call fails and returns a 255 error, it may
			// indicate that the FILESYS.NLM being used on the server
			// does not implement option 2 for the QueryFlag parameter.
			// In this case, we will default to our old behavior
			// and simply call CloseFile.  This, potentially, will
			// not free all of the lock objects and could result in
			// a memory leak in filesys.nlm.  However, we want to
			// at least make sure that there is a corresponding
			// RevokeFileHandleRights or CloseFile call for every
			// file open / create call.

			if( (lErrorCode = RevokeFileHandleRights( 0, 1, 
					m_lFileHandle, 2, m_lOpenAttr & 0x0000000F, &unused)) == 0xFF)
			{
				lErrorCode = CloseFile( 0, 1, m_lFileHandle);
			}
			m_lOpenAttr = 0;

			if ( lErrorCode != 0 )
			{
				rc = f_mapPlatformError(lErrorCode, NE_FLM_CLOSING_FILE);
				goto Exit;
			}

			m_lOpenAttr = f_getNWOpenFlags(uiAccess, m_bDoDirectIO);
			lErrorCode = OpenFile( 0, 1, m_lVolumeID, 0, (BYTE *)pLNamePath,
				*plLNamePathCount, LONGNameSpace, 0, m_lOpenAttr,
				PrimaryDataStream, &m_lFileHandle, &unused, &unused2);

			if ( lErrorCode != 0 )
			{
				// Too many error codes map to 255, so we put in a special
				// case check here.

				if( lErrorCode == 255)
				{
					rc = RC_SET( NE_FLM_IO_PATH_NOT_FOUND);
				}
				else
				{
					rc = f_mapPlatformError( lErrorCode,
								(RCODE)(m_bDoDirectIO
										 ? (RCODE)NE_FLM_DIRECT_OPENING_FILE
										 : (RCODE)NE_FLM_OPENING_FILE));
				}
				goto Exit;
			}
		}
		
		if ( m_bDoDirectIO)
		{
			lErrorCode = SwitchToDirectFileMode(0, m_lFileHandle);
			if (lErrorCode != 0)
			{
				if (RevokeFileHandleRights( 0, 1, 
					m_lFileHandle, 2, m_lOpenAttr & 0x0000000F, &unused) == 0xFF)
				{
					(void)CloseFile( 0, 1, m_lFileHandle);
				}
				rc = f_mapPlatformError( lErrorCode,
						(RCODE)(bCreateFlag
								  ? (RCODE)NE_FLM_DIRECT_CREATING_FILE
								  : (RCODE)NE_FLM_DIRECT_OPENING_FILE));
				goto Exit;
			}
		}
	}

Exit:

	if (RC_BAD( rc))
	{
		m_lFileHandle = -1;
		m_lOpenAttr = 0;
		m_bNSSFileOpen = FALSE;
	}

	if (pszQualifiedPath)
	{
		f_free( &pszQualifiedPath);
	}

   return( rc );
}
#endif

/****************************************************************************
Desc:		Create a file 
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::create(
	const char *		pIoPath,
	FLMUINT				uiIoFlags)
{
	RCODE			rc = NE_FLM_OK;

	flmAssert( m_bFileOpened == FALSE);

	if( RC_BAD( rc = openOrCreate( pIoPath, uiIoFlags, TRUE)))
	{
		goto Exit;
	}

	m_bFileOpened = TRUE;
	m_uiCurrentPos = 0;
	m_bOpenedExclusive = (uiIoFlags & FLM_IO_SH_DENYRW) ? TRUE : FALSE;

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::createUnique(
	char *				pIoPath,
	const char *		pszFileExtension,
	FLMUINT				uiIoFlags)
{
	RCODE					rc = NE_FLM_OK;
	char *				pszTmp;
	FLMBOOL				bModext = TRUE;
	FLMUINT				uiBaseTime = 0;
	FLMBYTE				ucHighByte = 0;
	char					ucFileName[ F_FILENAME_SIZE];
	char					szDirPath[ F_PATH_MAX_SIZE];
	char					szTmpPath[ F_PATH_MAX_SIZE];
	FLMUINT				uiCount;
	IF_FileSystem *	pFileSystem = f_getFileSysPtr();

	f_memset( ucFileName, 0, sizeof( ucFileName));

	flmAssert( m_bFileOpened == FALSE);

	f_strcpy( szDirPath, pIoPath);

   // Search backwards replacing trailing spaces with NULLs.

	pszTmp = szDirPath;
	pszTmp += (f_strlen( pszTmp) - 1);
	while ((*pszTmp == 0x20) && pszTmp >= szDirPath)
	{
		*pszTmp = 0;
		pszTmp--;
	}

	// Append a backslash if one isn't already there

	if (pszTmp >= szDirPath && *pszTmp != '\\')
	{
		pszTmp++;
		*pszTmp++ = '\\';
	}
	else
	{
		pszTmp++;
	}
	*pszTmp = 0;

	if ((pszFileExtension) && (f_strlen( pszFileExtension) >= 3))
	{
		bModext = FALSE;
	}

	uiCount = 0;
	do
	{
		pFileSystem->pathCreateUniqueName( &uiBaseTime, ucFileName,
				pszFileExtension, &ucHighByte, bModext);

		f_strcpy( szTmpPath, szDirPath);
		pFileSystem->pathAppend( szTmpPath, ucFileName);

		rc = create( szTmpPath, uiIoFlags | FLM_IO_EXCL);

		if (rc == NE_FLM_IO_DISK_FULL)
		{
			(void)f_netwareDeleteFile( szTmpPath);
			goto Exit;
		}

		if ((rc == NE_FLM_IO_PATH_NOT_FOUND) || (rc == NE_FLM_IO_INVALID_PASSWORD))
		{
			goto Exit;
		}
	} while ((rc != NE_FLM_OK) && (uiCount++ < 10));

   // Check if the path was created

   if ((uiCount >= 10) && (rc != NE_FLM_OK))
   {
		rc = RC_SET( NE_FLM_IO_PATH_CREATE_FAILURE);
		goto Exit;
   }

	m_bFileOpened = TRUE;
	m_bOpenedExclusive = (uiIoFlags & FLM_IO_SH_DENYRW) ? TRUE : FALSE;

	// Created file name needs to be returned.

	f_strcpy( pIoPath, szTmpPath);

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Open a file
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::open(
	const char *	pIoPath,
	FLMUINT			uiIoFlags)
{
	RCODE				rc = NE_FLM_OK;
	FLMUINT			uiStartTime;
	FLMUINT			ui15Secs;
	FLMUINT			uiCurrTime;

	flmAssert( m_bFileOpened == FALSE);

	ui15Secs = FLM_SECS_TO_TIMER_UNITS( 15);
	uiStartTime = FLM_GET_TIMER();

	do
	{
		if( RC_OK( rc = openOrCreate( pIoPath, uiIoFlags, FALSE)))
		{
			break;
		}

		if( rc != NE_FLM_IO_TOO_MANY_OPEN_FILES)
		{
			goto Exit;
		}

		f_sleep( 50);
		uiCurrTime = FLM_GET_TIMER();
	} while( FLM_ELAPSED_TIME( uiCurrTime, uiStartTime) < ui15Secs);
	
	if ( RC_BAD(rc) )
	{
		goto Exit;
	}

	m_bFileOpened = TRUE;
	m_uiCurrentPos = 0;
	m_bOpenedReadOnly = (uiIoFlags & FLM_IO_RDONLY) ? TRUE : FALSE;
	m_bOpenedExclusive = (uiIoFlags & FLM_IO_SH_DENYRW) ? TRUE : FALSE;

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Close a file
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::close()
{
	LONG			unused;
	FLMBOOL		bDeleteAllowed = TRUE;
	RCODE			rc = NE_FLM_OK;

	if( !m_bFileOpened)
	{
		goto Exit;
	}

	if (m_bNSS)
	{
		if (m_bNSSFileOpen)
		{
			(void)gv_zCloseFunc( m_NssKey);
			m_bNSSFileOpen = FALSE;
		}
	}
	else
	{
		// Revoke the file handle rights and close the file
		// (signified by passing 2 for the QueryFlag parameter).
		// If this call fails and returns a 255 error, it may
		// indicate that the FILESYS.NLM being used on the server
		// does not implement option 2 for the QueryFlag parameter.
		// In this case, we will default to our old behavior
		// and simply call CloseFile.  This, potentially, will
		// not free all of the lock objects and could result in
		// a memory leak in filesys.nlm.  However, we want to
		// at least make sure that there is a corresponding
		// RevokeFileHandleRights or CloseFile call for every
		// file open / create call.

		if( RevokeFileHandleRights( 0, 1, 
				m_lFileHandle, 2, m_lOpenAttr & 0x0000000F, &unused) == 0xFF)
		{
			(void)CloseFile( 0, 1, m_lFileHandle);
		}
	}

	m_lOpenAttr = 0;
	m_lFileHandle = -1;
	m_bFileOpened = m_bOpenedReadOnly = m_bOpenedExclusive = FALSE;

	if( m_bDeleteOnClose)
	{
		if( bDeleteAllowed)
		{
			if (m_bNSS)
			{
				(void)gv_zDeleteFunc( gv_NssRootKey, 0, zNSPACE_LONG | zMODE_UTF8,
									m_pszIoPath, zMATCH_ALL, 0);
			}
			else
			{
				(void)EraseFile( 0, 1, m_lVolumeID, 0, (BYTE *)m_pszIoPath,
					m_lLNamePathCount, LONGNameSpace, 0);
			}
		}

		m_bDeleteOnClose = FALSE;
		m_lLNamePathCount = 0;
	}

	if( m_pszIoPath)
	{
		f_free( &m_pszIoPath);
	}

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Read from a file
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::read(
	FLMUINT64			ui64Offset,
	FLMUINT				uiLength,
	void *				pvBuffer,
	FLMUINT *			puiBytesRead)
{
	RCODE					rc = NE_FLM_OK;

	if ( m_bDoDirectIO)
	{
		if( RC_BAD( rc = _directIORead( (FLMUINT)ui64Offset, uiLength, 
					pvBuffer, puiBytesRead)))
		{
			goto Exit;
		}
	}
	else
	{
		if( RC_BAD( rc = _read( (FLMUINT)ui64Offset, uiLength, 
			pvBuffer, puiBytesRead)))
		{
			goto Exit;
		}
	}

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Read from a file
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::_read(
	FLMUINT		uiReadOffset,
	FLMUINT		uiBytesToRead,	
   void *		pvBuffer,
   FLMUINT *	puiBytesReadRV)
{
	RCODE			rc = NE_FLM_OK;
	FCBType *	fcb;
	LONG			lBytesRead;
	LONG			lErr;
	
	flmAssert( m_bFileOpened == TRUE);

	if( puiBytesReadRV)
	{
		*puiBytesReadRV = 0;
	}

	if (uiReadOffset == FLM_IO_CURRENT_POS)
		uiReadOffset = m_uiCurrentPos;

	if (m_bNSS)
	{
		FLMINT	lStatus;
		FLMUINT	nBytesRead;

		if ((lStatus = gv_zReadFunc( m_NssKey, 0, (FLMUINT64)uiReadOffset,
									(FLMUINT)uiBytesToRead, pvBuffer,
									&nBytesRead)) != zOK)
		{
			rc = MapNSSError( lStatus, NE_FLM_READING_FILE);
			goto Exit;
		}

		if( puiBytesReadRV)
		{
			*puiBytesReadRV = (FLMUINT)nBytesRead;
		}

		if ((FLMUINT)nBytesRead < uiBytesToRead)
		{
			rc = RC_SET( NE_FLM_IO_END_OF_FILE);
		}
		m_uiCurrentPos = uiReadOffset + (FLMUINT)nBytesRead;
	}
	else
	{
		lErr = MapFileHandleToFCB( m_lFileHandle, &fcb );
		if ( lErr != 0 )
		{
			rc = f_mapPlatformError( lErr, NE_FLM_SETTING_UP_FOR_READ);
			goto Exit;
		}
		lErr = ReadFile( fcb->Station, m_lFileHandle, uiReadOffset,
					uiBytesToRead, &lBytesRead, pvBuffer);
		if ( lErr == 0 )
		{
			if( puiBytesReadRV)
			{
				*puiBytesReadRV = (FLMUINT) lBytesRead;
			}

			if (lBytesRead < (LONG)uiBytesToRead)
			{
				rc = RC_SET( NE_FLM_IO_END_OF_FILE);
			}
			m_uiCurrentPos = uiReadOffset + lBytesRead;
		}
		else
		{
			rc = f_mapPlatformError(lErr, NE_FLM_READING_FILE);
		}
	}

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Returns m_uiCurrentPos
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::tell(
	FLMUINT64 *			pui64Offset)
{
	*pui64Offset = m_uiCurrentPos;
	return( NE_FLM_OK);
}
#endif

/****************************************************************************
Desc:		Calls the Direct IO-style read routine
Note:		Where possible, the caller should attempt to read on sector
			boundaries and full sectors.  This routine will do the
			necessary work if this is not done, but it will be less
			efficient.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::_directIORead(
	FLMUINT		uiReadOffset,
	FLMUINT		uiBytesToRead,	
   void *		pvBuffer,
   FLMUINT *	puiBytesReadRV)
{
	RCODE			rc = NE_FLM_OK;
	FLMBYTE *	pucBuffer = (FLMBYTE *)pvBuffer;
	LONG			lStartSector;
	LONG			lSectorCount;
	LONG			lResult;
	BYTE			ucSectorBuf [FLM_NLM_SECTOR_SIZE];
	FLMUINT		uiBytesToCopy;
	FLMUINT		uiSectorOffset;
	FLMUINT		uiTotal;
	FLMINT		lStatus;

	flmAssert( m_bFileOpened == TRUE);

	if( puiBytesReadRV)
	{
		*puiBytesReadRV = 0;
	}

	if (uiReadOffset == FLM_IO_CURRENT_POS)
		uiReadOffset = m_uiCurrentPos;

	// Calculate the starting sector.

	lStartSector = uiReadOffset / FLM_NLM_SECTOR_SIZE;

	// See if the offset is on a sector boundary.  If not, we must read
	// into the local sector buffer and then copy into the buffer.
	// We must also read into the local buffer if our read size is less
	// than the sector size.

	if ((uiReadOffset % FLM_NLM_SECTOR_SIZE != 0) ||
		 (uiBytesToRead < FLM_NLM_SECTOR_SIZE))
	{
		if (m_bNSS)
		{
			if ((lStatus = gv_zDIOReadFunc( m_NssKey, 
									(FLMUINT64)lStartSector, 1,
									(FLMUINT)0, NULL, ucSectorBuf)) != zOK)
			{
				rc = MapNSSError( lStatus, NE_FLM_DIRECT_READING_FILE);
				goto Exit;
			}
		}
		else
		{
			lResult = DirectReadFile(
				0,
				m_lFileHandle,
				lStartSector,
				1,
				ucSectorBuf
				);
			if (lResult != 0)
			{
				rc = DfsMapError( lResult, NE_FLM_DIRECT_READING_FILE);
				goto Exit;
			}
		}

		// Copy the part of the sector that was requested into the buffer.

		uiSectorOffset = uiReadOffset % FLM_NLM_SECTOR_SIZE;

		if ((uiBytesToCopy = uiBytesToRead) > FLM_NLM_SECTOR_SIZE - uiSectorOffset)
			uiBytesToCopy = FLM_NLM_SECTOR_SIZE - uiSectorOffset;
		f_memcpy( pucBuffer, &ucSectorBuf [uiSectorOffset], uiBytesToCopy);
		pucBuffer += uiBytesToCopy;
		uiBytesToRead -= (FLMUINT)uiBytesToCopy;
		m_uiCurrentPos += (FLMUINT)uiBytesToCopy;

		if( puiBytesReadRV)
		{
   		(*puiBytesReadRV) += (FLMUINT)uiBytesToCopy;
		}

		// See if we got everything we wanted to with this read.

		if (!uiBytesToRead)
			goto Exit;

		// Go to the next sector boundary

		lStartSector++;
	}

	// At this point, we are poised to read on a sector boundary.  See if we
	// have at least one full sector to read.  If so, we can read it directly
	// into the provided buffer.  If not, we must use the temporary sector
	// buffer.

	if (uiBytesToRead >= FLM_NLM_SECTOR_SIZE)
	{
		lSectorCount = (LONG)(uiBytesToRead / FLM_NLM_SECTOR_SIZE);
Try_Read:
		if (m_bNSS)
		{
			if ((lStatus = gv_zDIOReadFunc( m_NssKey,
									(FLMUINT64)lStartSector,
									(FLMUINT)lSectorCount,
									(FLMUINT)0, NULL, pucBuffer)) != zOK)
			{
				if ((lStatus == zERR_END_OF_FILE || lStatus == zERR_BEYOND_EOF) &&
					 (lSectorCount > 1))
				{

					// See if we can read one less sector.  We will return
					// NE_FLM_IO_END_OF_FILE in this case.

					lSectorCount--;
					rc = RC_SET( NE_FLM_IO_END_OF_FILE);
					goto Try_Read;
				}
				rc = MapNSSError( lStatus, NE_FLM_DIRECT_READING_FILE);
				goto Exit;
			}
		}
		else
		{
			lResult = DirectReadFile( 0, m_lFileHandle, lStartSector,
				lSectorCount, pucBuffer);
				
			if (lResult != 0)
			{
				if ((lResult == DFSOperationBeyondEndOfFile) &&
					 (lSectorCount > 1))
				{

					// See if we can read one less sector.  We will return
					// NE_FLM_IO_END_OF_FILE in this case.

					lSectorCount--;
					rc = RC_SET( NE_FLM_IO_END_OF_FILE);
					goto Try_Read;
				}
				rc = DfsMapError( lResult, NE_FLM_DIRECT_READING_FILE);
				goto Exit;
			}
		}
		
		uiTotal = (FLMUINT)(lSectorCount * FLM_NLM_SECTOR_SIZE);
		pucBuffer += uiTotal;
		m_uiCurrentPos += uiTotal;

		if( puiBytesReadRV)
		{
   		(*puiBytesReadRV) += uiTotal;
		}
		uiBytesToRead -= uiTotal;

		// See if we got everything we wanted to or could with this read.

		if ((!uiBytesToRead) || (rc == NE_FLM_IO_END_OF_FILE))
			goto Exit;

		// Go to the next sector after the ones we just read

		lStartSector += lSectorCount;
	}

	// At this point, we have less than a sector's worth to read, so we must
	// read it into a local buffer.

	if (m_bNSS)
	{
		if ((lStatus = gv_zDIOReadFunc( m_NssKey, 
								(FLMUINT64)lStartSector, 1,
								(FLMUINT)0, NULL, ucSectorBuf)) != zOK)
		{
			rc = MapNSSError( lStatus, NE_FLM_DIRECT_READING_FILE);
			goto Exit;
		}
	}
	else
	{
		lResult = DirectReadFile( 0, m_lFileHandle, lStartSector, 1, ucSectorBuf);
		if (lResult != 0)
		{
			rc = DfsMapError( lResult, NE_FLM_DIRECT_READING_FILE);
			goto Exit;
		}
	}

	// Copy the part of the sector that was requested into the buffer.

	m_uiCurrentPos += uiBytesToRead;

	if( puiBytesReadRV)
	{
  		(*puiBytesReadRV) += uiBytesToRead;
	}

	f_memcpy( pucBuffer, ucSectorBuf, uiBytesToRead);

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Might call the direct IO routine in the future
Note:		This function assumes that the pvBuffer that is passed in is
			a multiple of a sector size (512 bytes).
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::sectorRead(
	FLMUINT64		ui64ReadOffset,
	FLMUINT			uiBytesToRead,
	void *			pvBuffer,
	FLMUINT *		puiBytesReadRV)
{
	RCODE				rc = NE_FLM_OK;

	if( m_bDoDirectIO)
	{
		if( RC_BAD( rc = _directIOSectorRead( (FLMUINT)ui64ReadOffset, 
			uiBytesToRead, pvBuffer, puiBytesReadRV)))
		{
			goto Exit;
		}
	}
	else
	{
		if( RC_BAD( rc = _read( (FLMUINT)ui64ReadOffset, uiBytesToRead, 
			pvBuffer, puiBytesReadRV)))
		{
			goto Exit;
		}
	}

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Calls the direct IO Read routine
Note:		This function assumes that the pvBuffer that is passed in is
			a multiple of a sector size (512 bytes).
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::_directIOSectorRead(
	FLMUINT		uiReadOffset,
	FLMUINT		uiBytesToRead,	
   void *		pvBuffer,
   FLMUINT *	puiBytesReadRV)
{
	RCODE		rc = NE_FLM_OK;
	LONG		lStartSector;
	LONG		lSectorCount;
	LONG		lResult;
	FLMINT	lStatus;

	flmAssert( m_bFileOpened == TRUE);

	if (uiReadOffset == FLM_IO_CURRENT_POS)
		uiReadOffset = m_uiCurrentPos;

	if (uiReadOffset % FLM_NLM_SECTOR_SIZE != 0)
	{
		rc = _read( uiReadOffset, uiBytesToRead, pvBuffer, puiBytesReadRV);
		goto Exit;
	}

	// Calculate the starting sector

	lStartSector = uiReadOffset / FLM_NLM_SECTOR_SIZE;
	lSectorCount = (LONG)(uiBytesToRead / FLM_NLM_SECTOR_SIZE);
	if (uiBytesToRead % FLM_NLM_SECTOR_SIZE != 0)
		lSectorCount++;

Try_Read:

	if (m_bNSS)
	{
		if ((lStatus = gv_zDIOReadFunc( m_NssKey,
								(FLMUINT64)lStartSector,
								(FLMUINT)lSectorCount,
								(FLMUINT)0, NULL, pvBuffer)) != zOK)
		{
			if ((lStatus == zERR_END_OF_FILE || lStatus == zERR_BEYOND_EOF) &&
				 (lSectorCount > 1))
			{

				// See if we can read one less sector.  We will return
				// NE_FLM_IO_END_OF_FILE in this case.

				lSectorCount--;
				uiBytesToRead = (FLMUINT)(lSectorCount * FLM_NLM_SECTOR_SIZE);
				rc = RC_SET( NE_FLM_IO_END_OF_FILE);
				goto Try_Read;
			}
			uiBytesToRead = 0;
			rc = MapNSSError( lStatus, NE_FLM_DIRECT_READING_FILE);
			goto Exit;
		}
	}
	else
	{
		lResult = DirectReadFile( 0, m_lFileHandle,
										lStartSector, lSectorCount,(BYTE *)pvBuffer);
		if (lResult != 0)
		{
			if ((lResult == DFSOperationBeyondEndOfFile) &&
					(lSectorCount > 1))
			{
				// See if we can read one less sector.  We will return
				// NE_FLM_IO_END_OF_FILE in this case.

				lSectorCount--;
				uiBytesToRead = (FLMUINT)(lSectorCount * FLM_NLM_SECTOR_SIZE);
				rc = RC_SET( NE_FLM_IO_END_OF_FILE);
				goto Try_Read;
			}
			uiBytesToRead = 0;
			rc = DfsMapError( lResult, NE_FLM_DIRECT_READING_FILE);
			goto Exit;
		}
	}
	m_uiCurrentPos += uiBytesToRead;

Exit:

	if( puiBytesReadRV)
	{
  		*puiBytesReadRV = uiBytesToRead;
	}

	return( rc);
}
#endif

/****************************************************************************
Desc:		Sets current position of file.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::seek(
	FLMUINT64		ui64Offset,
	FLMINT			iWhence,
	FLMUINT64 *		pui64NewOffset)
{
	RCODE			rc = NE_FLM_OK;

	switch (iWhence)
	{
		case FLM_IO_SEEK_CUR:
			m_uiCurrentPos += (FLMUINT)ui64Offset;
			break;
			
		case FLM_IO_SEEK_SET:
			m_uiCurrentPos = (FLMUINT)ui64Offset;
			break;
			
		case FLM_IO_SEEK_END:
			if( RC_BAD( rc = size( &ui64Offset)))
			{
				goto Exit;
			}
			m_uiCurrentPos = (FLMUINT)ui64Offset;
			break;
			
		default:
			rc = RC_SET( NE_FLM_NOT_IMPLEMENTED);
			goto Exit;
	}
	
	*pui64NewOffset = m_uiCurrentPos;
	
Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Return the size of the file
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::size(
	FLMUINT64 *		pui64Size)
{
	RCODE				rc = NE_FLM_OK;
	LONG				lErr;
	LONG				lSize;

	if (m_bNSS)
	{
		FLMINT	lStatus;
		zInfo_s	Info;

		if ((lStatus = gv_zGetInfoFunc( m_NssKey,
								zGET_STORAGE_USED,
								sizeof( Info), zINFO_VERSION_A,
								&Info)) != zOK)
		{
			rc = MapNSSError( lStatus, NE_FLM_GETTING_FILE_INFO);
			goto Exit;
		}
		flmAssert( Info.infoVersion == zINFO_VERSION_A);
		*pui64Size = (FLMUINT64)Info.std.logicalEOF;
	}
	else
	{
		lErr = GetFileSize( 0, m_lFileHandle, &lSize);
		if ( lErr != 0 )
		{
			rc = f_mapPlatformError( lErr, NE_FLM_GETTING_FILE_SIZE);
		}
		*pui64Size = (FLMUINT64)lSize;
	}

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Truncate the file to the indicated size
WARNING: Direct IO methods are calling this method.  Make sure that all
			changes to this method work in direct IO mode.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::truncate(
	FLMUINT64		ui64Size)
{
	RCODE				rc = NE_FLM_OK;
	LONG				lErr;

	flmAssert( m_bFileOpened == TRUE);

	if (m_bNSS)
	{
		FLMINT	lStatus;
		
		if ((lStatus = gv_zSetEOFFunc( m_NssKey, 0, ui64Size,
								zSETSIZE_NON_SPARSE_FILE |
								zSETSIZE_NO_ZERO_FILL |
								zSETSIZE_UNDO_ON_ERR)) != zOK)
		{
			rc = MapNSSError( lStatus, NE_FLM_TRUNCATING_FILE);
			goto Exit;
		}
	}
	else
	{
		if ((lErr = SetFileSize( 0, m_lFileHandle, (FLMUINT)ui64Size, TRUE)) != 0)
		{
			rc = f_mapPlatformError( lErr, NE_FLM_TRUNCATING_FILE);
			goto Exit;
		}
	}
	
	if (m_uiCurrentPos > ui64Size)
	{
		m_uiCurrentPos = (FLMUINT)ui64Size;
	}
	
Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Write to a file
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::write(
	FLMUINT64		ui64Offset,
	FLMUINT			uiLength,
	const void *	pvBuffer,
	FLMUINT *		puiBytesWritten)
{
	RCODE				rc = NE_FLM_OK;

	if( m_bDoDirectIO)
	{
		if( RC_BAD( rc = _directIOWrite( (FLMUINT)ui64Offset, uiLength, 
					pvBuffer, puiBytesWritten)))
		{
			goto Exit;
		}
	}
	else
	{
		if( RC_BAD( rc = _write( (FLMUINT)ui64Offset, uiLength, 
					pvBuffer, puiBytesWritten)))
		{
			goto Exit;
		}
	}

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Write to a file
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::_write(
	FLMUINT			uiWriteOffset,
	FLMUINT			uiBytesToWrite,
	const void *	pvBuffer,
	FLMUINT *		puiBytesWrittenRV)
{
	RCODE				rc = NE_FLM_OK;
	LONG				lErr;
	FCBType *		fcb;

	flmAssert( m_bFileOpened == TRUE);

	*puiBytesWrittenRV = 0;

	if (uiWriteOffset == FLM_IO_CURRENT_POS)
	{
		uiWriteOffset = m_uiCurrentPos;
	}
	else
	{
		m_uiCurrentPos = uiWriteOffset;
	}
	
	if( m_bNSS)
	{
		FLMINT	lStatus;
		FLMUINT	nBytesWritten;

		if( (lStatus = gv_zWriteFunc( m_NssKey, 0, (FLMUINT64)uiWriteOffset,
			(FLMUINT)uiBytesToWrite, pvBuffer, &nBytesWritten)) != zOK)
		{
			rc = MapNSSError( lStatus, NE_FLM_WRITING_FILE);
			goto Exit;
		}
		
		if( nBytesWritten != (FLMUINT)uiBytesToWrite)
		{
			rc = RC_SET( NE_FLM_IO_DISK_FULL);
			goto Exit;
		}
	}
	else
	{
		if( (lErr = MapFileHandleToFCB( m_lFileHandle, &fcb )) != 0)
		{
			rc = f_mapPlatformError( lErr, NE_FLM_SETTING_UP_FOR_WRITE);
			goto Exit;
		}

		if( (lErr = WriteFile( fcb->Station, m_lFileHandle, uiWriteOffset,
					uiBytesToWrite, (void *)pvBuffer)) != 0)
		{
			rc = f_mapPlatformError( lErr, NE_FLM_WRITING_FILE);
			goto Exit;
		}
	}
	
	*puiBytesWrittenRV = uiBytesToWrite;
	m_uiCurrentPos = uiWriteOffset + uiBytesToWrite;
	
Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Calls the direct IO Write routine.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::_directIOWrite(
	FLMUINT			uiWriteOffset,
	FLMUINT			uiBytesToWrite,
	const void *	pvBuffer,
	FLMUINT *		puiBytesWrittenRV)
{
	RCODE				rc = NE_FLM_OK;
	FLMBYTE *		pucBuffer = (FLMBYTE *)pvBuffer;
	LONG				lStartSector;
	LONG				lSectorCount;
	LONG				lResult;
	BYTE				ucSectorBuf[ FLM_NLM_SECTOR_SIZE];
	FLMUINT			uiBytesToCopy;
	FLMUINT			uiSectorOffset;
	FLMUINT			uiTotal;
	FLMINT			lStatus;

	flmAssert( m_bFileOpened == TRUE);

	*puiBytesWrittenRV = 0;

	if( uiWriteOffset == FLM_IO_CURRENT_POS)
	{
		uiWriteOffset = m_uiCurrentPos;
	}
	else
	{
		m_uiCurrentPos = uiWriteOffset;
	}
	
	// Calculate the starting sector

	lStartSector = uiWriteOffset / FLM_NLM_SECTOR_SIZE;

	// See if the offset is on a sector boundary.  If not, we must first read
	// the sector into memory, overwrite it with data from the input
	// buffer and write it back out again.

	if( (uiWriteOffset % FLM_NLM_SECTOR_SIZE != 0) || 
		 (uiBytesToWrite < FLM_NLM_SECTOR_SIZE))
	{
		if( m_bNSS)
		{
			if( (lStatus = gv_zDIOReadFunc( m_NssKey, 
				(FLMUINT64)lStartSector,
				(FLMUINT)1, (FLMUINT)0, NULL, ucSectorBuf)) != zOK)
			{
				if (lStatus == zERR_END_OF_FILE || lStatus == zERR_BEYOND_EOF)
				{
					f_memset( ucSectorBuf, 0, sizeof( ucSectorBuf));

					// Expand the file

					if( RC_BAD( rc = expand( lStartSector, 1)))
					{
						goto Exit;
					}
				}
				else
				{
					rc = MapNSSError( lStatus, NE_FLM_DIRECT_READING_FILE);
					goto Exit;
				}
			}
		}
		else
		{
			lResult = DirectReadFile( 0, m_lFileHandle, lStartSector, 
												1, ucSectorBuf);
												
			if( lResult == DFSHoleInFileError || 
				 lResult == DFSOperationBeyondEndOfFile )
			{
				f_memset( ucSectorBuf, 0, sizeof( ucSectorBuf));

				// Expand the file

				if( RC_BAD( rc = expand( lStartSector, 1)))
				{
					goto Exit;
				}
			}
			else if( lResult != 0)
			{
				rc = DfsMapError( lResult, NE_FLM_DIRECT_READING_FILE);
				goto Exit;
			}
		}

		// Copy the part of the buffer that is being written back into
		// the sector buffer.

		uiSectorOffset = uiWriteOffset % FLM_NLM_SECTOR_SIZE;

		if( (uiBytesToCopy = uiBytesToWrite) > FLM_NLM_SECTOR_SIZE - uiSectorOffset)
		{
			uiBytesToCopy = FLM_NLM_SECTOR_SIZE - uiSectorOffset;
		}
		
		f_memcpy( &ucSectorBuf [uiSectorOffset], pucBuffer, uiBytesToCopy);
		pucBuffer += uiBytesToCopy;
		uiBytesToWrite -= (FLMUINT)uiBytesToCopy;
		m_uiCurrentPos += (FLMUINT)uiBytesToCopy;
		(*puiBytesWrittenRV) += (FLMUINT)uiBytesToCopy;

		// Write the sector buffer back out

		if( RC_BAD( rc = writeSectors( &ucSectorBuf [0], lStartSector, 1, NULL)))
		{
			goto Exit;
		}

		// See if we wrote everything we wanted to with this write

		if (!uiBytesToWrite)
		{
			goto Exit;
		}

		// Go to the next sector boundary

		lStartSector++;
	}

	// At this point, we are poised to write on a sector boundary.  See if we
	// have at least one full sector to write.  If so, we can write it directly
	// from the provided buffer.  If not, we must use the temporary sector
	// buffer.

	if( uiBytesToWrite >= FLM_NLM_SECTOR_SIZE)
	{
		lSectorCount = (LONG)(uiBytesToWrite / FLM_NLM_SECTOR_SIZE);
		
		if( RC_BAD( rc = writeSectors( (void *)pucBuffer, lStartSector,
									lSectorCount, NULL)))
		{
			goto Exit;
		}
		
		uiTotal = (FLMUINT)(lSectorCount * FLM_NLM_SECTOR_SIZE);
		pucBuffer += uiTotal;
		m_uiCurrentPos += uiTotal;
		(*puiBytesWrittenRV) += uiTotal;
		uiBytesToWrite -= uiTotal;

		// See if we wrote everything we wanted to with this write

		if( !uiBytesToWrite)
		{
			goto Exit;
		}

		// Go to the next sector after the ones we just wrote

		lStartSector += lSectorCount;
	}

	// At this point, we have less than a sector's worth to write, so we must
	// first read the sector from disk, alter it, and then write it back out.

	if( m_bNSS)
	{
		if( (lStatus = gv_zDIOReadFunc( m_NssKey, (FLMUINT64)lStartSector,
			(FLMUINT)1, (FLMUINT)0, NULL, ucSectorBuf)) != zOK)
		{
			if( lStatus == zERR_END_OF_FILE || lStatus == zERR_BEYOND_EOF)
			{
				f_memset( ucSectorBuf, 0, sizeof( ucSectorBuf));

				// Expand the file

				if( RC_BAD( rc = expand( lStartSector, 1)))
				{
					goto Exit;
				}
			}
			else
			{
				rc = MapNSSError( lStatus, NE_FLM_DIRECT_READING_FILE);
				goto Exit;
			}
		}
	}
	else
	{
		lResult = DirectReadFile( 0, m_lFileHandle, lStartSector,
											1, ucSectorBuf);
											
		if( lResult == DFSHoleInFileError)
		{
			f_memset( ucSectorBuf, 0, sizeof( ucSectorBuf));

			// Expand the file

			if( RC_BAD( rc = expand( lStartSector, 1)))
			{
				goto Exit;
			}
		}
		else if( lResult != 0)
		{
			rc = DfsMapError( lResult, NE_FLM_DIRECT_READING_FILE);
			goto Exit;
		}
	}

	// Copy the rest of the output buffer into the sector buffer

	f_memcpy( ucSectorBuf, pucBuffer, uiBytesToWrite);

	// Write the sector back to disk

	if( RC_BAD( rc = writeSectors( &ucSectorBuf [0], lStartSector, 1, NULL)))
	{
		goto Exit;
	}

	m_uiCurrentPos += uiBytesToWrite;
	(*puiBytesWrittenRV) += uiBytesToWrite;
	
Exit:

	return( rc);
}
#endif

/***************************************************************************
Desc:		Expand a file for writing.
***************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::expand(
	LONG		lStartSector,
	LONG		lSectorsToAlloc)
{
	RCODE			rc = NE_FLM_OK;
	LONG			lResult;
	LONG			lBlockNumber;
	LONG			lStartBlockNumber;
	LONG			lNumBlocksToAlloc;
	LONG			lNumBlocksAllocated;
	LONG			lMinToAlloc;
	LONG			lLastBlockNumber;
	LONG			lTotalToAlloc;
	LONG			lExtendSize;
	FLMUINT		uiFileSize;
	FLMUINT		uiRequestedExtendSize = m_uiExtendSize;
	FLMBOOL		bVerifyFileSize = FALSE;

	// If the requested extend size is the "special" value of ~0,
	// we will set the requested size to 0, so that we will use the
	// minimum default below.  This allows us to somewhat emulate what
	// the Window's code does.

	if( uiRequestedExtendSize == (FLMUINT)(~0))
	{
		uiRequestedExtendSize = 0;
	}

	if( m_bNSS)
	{
		lStartBlockNumber = lStartSector / (65536 / FLM_NLM_SECTOR_SIZE);
		lLastBlockNumber = (lStartSector + lSectorsToAlloc) / (65536 / FLM_NLM_SECTOR_SIZE);
		lExtendSize = uiRequestedExtendSize / 65536;
	}
	else
	{
		lStartBlockNumber = lStartSector / m_lSectorsPerBlock;
		lLastBlockNumber = (lStartSector + lSectorsToAlloc) / m_lSectorsPerBlock;
		lExtendSize = uiRequestedExtendSize / (m_lSectorsPerBlock * FLM_NLM_SECTOR_SIZE);
	}

	// Last block number better be greater than or equal to
	// start block number.

	flmAssert( lLastBlockNumber >= lStartBlockNumber);
	lMinToAlloc = (lLastBlockNumber - lStartBlockNumber) + 1;

	if( lExtendSize < 5)
	{
		lExtendSize = 5;
	}

	// Allocate up to lExtendSize blocks at a time - hopefully this will be
	// more efficient.

	if( lMinToAlloc < lExtendSize)
	{
		lTotalToAlloc = lExtendSize;
	}
	else if( lMinToAlloc % lExtendSize == 0)
	{
		lTotalToAlloc = lMinToAlloc;
	}
	else
	{
		// Keep the total blocks to allocate a multiple of lExtendSize.

		lTotalToAlloc = lMinToAlloc - 
			(lMinToAlloc % lExtendSize) + lExtendSize;
	}
	
	lNumBlocksToAlloc = lTotalToAlloc;
	lBlockNumber = lStartBlockNumber;
	lNumBlocksAllocated = 0;

	// Must not go over maximum file size.

	if( lStartBlockNumber + lTotalToAlloc > m_lMaxBlocks)
	{
		lNumBlocksToAlloc = lTotalToAlloc = m_lMaxBlocks - lStartBlockNumber;
		
		if( lTotalToAlloc < lMinToAlloc)
		{
			rc = RC_SET( NE_FLM_IO_DISK_FULL);
			goto Exit;
		}
	}

	if( m_bNSS)
	{
		FLMINT	lStatus;

		for( ;;)
		{
			if( (lStatus = gv_zSetEOFFunc( m_NssKey, 0,
				(FLMUINT64)lBlockNumber * 65536 + lNumBlocksToAlloc * 65536,
				zSETSIZE_NO_ZERO_FILL | zSETSIZE_NON_SPARSE_FILE)) != zOK)
			{
				if( lStatus == zERR_OUT_OF_SPACE)
				{
					if( lNumBlocksToAlloc > lMinToAlloc)
					{
						lNumBlocksToAlloc--;
						continue;
					}
				}
				
				rc = MapNSSError( lStatus, NE_FLM_EXPANDING_FILE);
				goto Exit;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		for (;;)
		{
			lResult = ExpandFileInContiguousBlocks( 0, m_lFileHandle, 
									lBlockNumber, lNumBlocksToAlloc, -1, -1);

			// If we couldn't allocate space, see if we can free some of
			// the limbo space on the volume.

			if( lResult == DFSInsufficientSpace || lResult == DFSBoundryError)
			{
				// May not have been able to get contiguous space for
				// multiple blocks.  If we were asking for more than
				// one, reduce the number we are asking for and try
				// again.

				if( lNumBlocksToAlloc > 1)
				{
					lNumBlocksToAlloc--;
					continue;
				}

				// If we could not even get one block, it is time to
				// try and free some limbo space.

				lResult = FreeLimboVolumeSpace( (LONG)m_lVolumeID, 1);
				if( lResult == DFSInsufficientLimboFileSpace)
				{
					// It is not an error to be out of space if
					// we successfully allocated at least the minimum
					// number of blocks needed.

					if( lNumBlocksAllocated >= lMinToAlloc)
					{
						break;
					}
					else
					{
						rc = RC_SET( NE_FLM_IO_DISK_FULL);
						goto Exit;
					}
				}
				
				continue;
			}
			else if( lResult == DFSOverlapError)
			{
				lResult = 0;
				bVerifyFileSize = TRUE;

				// If lNumBlocksToAlloc is greater than one, we
				// don't know exactly where the hole is, so we need
				// to try filling exactly one block right where
				// we are at.

				// If lNumBlocksToAlloc is exactly one, we know that
				// we have a block right where we are at, so we let
				// the code fall through as if the expand had
				// succeeded.

				if( lNumBlocksToAlloc > 1)
				{
					// If we have an overlap, try getting one block at
					// the current block number - need to make sure this
					// is not where the hole is at.

					lNumBlocksToAlloc = 1;
					continue;
				}
			}
			else if (lResult != 0)
			{
				rc = DfsMapError( lResult, NE_FLM_EXPANDING_FILE);
				goto Exit;
			}
			
			lNumBlocksAllocated += lNumBlocksToAlloc;
			lBlockNumber += lNumBlocksToAlloc;
			
			if( lNumBlocksAllocated >= lTotalToAlloc)
			{
				break;
			}
			else if( lNumBlocksToAlloc > lTotalToAlloc - lNumBlocksAllocated)
			{
				lNumBlocksToAlloc = lTotalToAlloc - lNumBlocksAllocated;
			}
		}

		// If bVerifyFileSize is TRUE, we had an overlap error, which means
		// that we may have had a hole in the file.  In that case, we
		// do NOT want to truncate the file to an incorrect size, so we
		// get the current file size to make sure we are not reducing it
		// down inappropriately.  NOTE: This is not foolproof - if we have
		// a hole that is exactly the size we asked for, we will not verify
		// the file size.

		uiFileSize = (FLMUINT)(lStartBlockNumber + lNumBlocksAllocated) *
				(FLMUINT)m_lSectorsPerBlock * (FLMUINT)FLM_NLM_SECTOR_SIZE;
				
		if( bVerifyFileSize)
		{
			LONG	lCurrFileSize;

			lResult = GetFileSize( 0, m_lFileHandle, &lCurrFileSize);
			
			if( lResult != DFSNormalCompletion)
			{
				rc = DfsMapError( lResult, NE_FLM_GETTING_FILE_SIZE);
				goto Exit;
			}
			
			if( (FLMUINT)lCurrFileSize > uiFileSize)
			{
				uiFileSize = (FLMUINT)lCurrFileSize;
			}
		}

		// This call of SetFileSize is done to force the directory entry file size
		// to account for the newly allocated blocks.  It also forces the directory
		// entry to be updated on disk.  If we didn't do this here, the directory
		// entry's file size on disk would not account for this block.
		// Thus, if we crashed after writing data to this
		// newly allocated block, we would lose the data in the block.

		lResult = SetFileSize( 0, m_lFileHandle, uiFileSize, FALSE);
		
		if( lResult != DFSNormalCompletion)
		{
			rc = DfsMapError( lResult, NE_FLM_TRUNCATING_FILE);
			goto Exit;
		}
	}
	
Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Calls the direct IO Write routine.  Handles both asynchronous writes
			and synchronous writes.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::writeSectors(
	void *			pvBuffer,
	LONG				lStartSector,
	LONG				lSectorCount,
	IF_IOBuffer *	pBufferObj,
	FLMBOOL *		pbDidAsync)
{
	RCODE				rc = NE_FLM_OK;
	LONG				lResult;
	FLMBOOL			bAlreadyExpanded = FALSE;
	FLMINT			lStatus;
	FLMBOOL			bMadePending;

	// Keep trying write until we succeed or get an error we can't deal with.
	// Actually, this will NOT loop forever.  At most, it will try twice - 
	// and then it is only when we get a hole in the file error.

	bMadePending = FALSE;
	for (;;)
	{
		if (m_bNSS)
		{
			if( pBufferObj)
			{
				if (!bMadePending)
				{
					flmAssert( pbDidAsync);
					pBufferObj->makePending();
					bMadePending = TRUE;
				}
				lStatus = gv_zDIOWriteFunc( m_NssKey,
									(FLMUINT64)lStartSector,
									(FLMUINT)lSectorCount,
									(FLMUINT)pBufferObj,
									nssDioCallback,
									pvBuffer);
			}
			else
			{
				lStatus = gv_zDIOWriteFunc( m_NssKey,
									(FLMUINT64)lStartSector,
									(FLMUINT)lSectorCount, (FLMUINT)0, NULL, pvBuffer);
			}

			// We may need to allocate space to do this write

			if (lStatus == zERR_END_OF_FILE ||
				 lStatus == zERR_BEYOND_EOF || 
				 lStatus == zERR_HOLE_IN_DIO_FILE)
			{
				if (bAlreadyExpanded)
				{
					flmAssert( 0);
					rc = MapNSSError( lStatus, NE_FLM_DIRECT_WRITING_FILE);
					goto Exit;
				}

				// Expand the file

				if (RC_BAD( rc = expand( lStartSector, lSectorCount)))
				{
					goto Exit;
				}
				bAlreadyExpanded = TRUE;
				continue;
			}
			else if (lStatus != 0)
			{
				rc = MapNSSError( lStatus, NE_FLM_DIRECT_WRITING_FILE);
				goto Exit;
			}
			else
			{
				if (pBufferObj)
				{
					*pbDidAsync = TRUE;
				}
				break;
			}
		}
		else
		{
			LONG		lSize;
			FLMBOOL	bNeedToWriteEOF;

			// Determine if this write will change the EOF.  If so, pre-expand
			// the file.

			lResult = GetFileSize( 0, m_lFileHandle, &lSize);
			if (lResult != 0)
			{
				rc = f_mapPlatformError( lResult, NE_FLM_GETTING_FILE_SIZE);
				goto Exit;
			}
			
			bNeedToWriteEOF = (lSize < (lStartSector + lSectorCount) * FLM_NLM_SECTOR_SIZE)
											? TRUE
											: FALSE;
											
			if( pBufferObj)
			{
				if (!bMadePending)
				{
					flmAssert( pbDidAsync);
					pBufferObj->makePending();
					bMadePending = TRUE;
				}
				
				lResult = DirectWriteFileNoWait( 0, m_lFileHandle,
									lStartSector,lSectorCount,
									(BYTE *)pvBuffer, DirectIONoWaitCallBack, 
									(LONG)pBufferObj);
			}
			else
			{
				lResult = DirectWriteFile( 0, m_lFileHandle,
									lStartSector, lSectorCount, (BYTE *)pvBuffer);
			}

			// We may need to allocate space to do this write

			if( lResult == DFSHoleInFileError || 
				 lResult == DFSOperationBeyondEndOfFile)
			{
				if( bAlreadyExpanded)
				{
					flmAssert( 0);
					rc = DfsMapError( lResult, NE_FLM_DIRECT_WRITING_FILE);
					goto Exit;
				}

				// Expand the file

				if( RC_BAD( rc = expand( lStartSector, lSectorCount)))
				{
					goto Exit;
				}
				
				bAlreadyExpanded = TRUE;

				// The Expand method forces the file EOF in the directory
				// entry to be written to disk.

				bNeedToWriteEOF = FALSE;
				continue;
			}
			else if (lResult != 0)
			{
				rc = DfsMapError( lResult, NE_FLM_DIRECT_WRITING_FILE);
				goto Exit;
			}
			else
			{
				if( pBufferObj)
				{
					*pbDidAsync = TRUE;
				}

				// If bNeedToWriteEOF is TRUE, we need to force EOF to disk.

				if( bNeedToWriteEOF)
				{
					LONG	lFileSizeInSectors;
					LONG	lExtraSectors;

					// Set the EOF to the nearest block boundary - so we don't
					// have to do this very often.

					lFileSizeInSectors = lStartSector + lSectorCount;
					lExtraSectors = lFileSizeInSectors % m_lSectorsPerBlock;
					
					if (lExtraSectors)
					{
						lFileSizeInSectors += (m_lSectorsPerBlock - lExtraSectors);
					}
					
					if ((lResult = SetFileSize( 0, m_lFileHandle,
											(FLMUINT)lFileSizeInSectors * FLM_NLM_SECTOR_SIZE,
											FALSE)) != 0)
					{
						rc = DfsMapError( lResult, NE_FLM_TRUNCATING_FILE);
						goto Exit;
					}
				}

				break;
			}
		}
	}
	
Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc: Legacy async I/O completion callback
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FSTATIC void DirectIONoWaitCallBack(
	LONG		unknownAlwaysZero,
	LONG		callbackData,
	LONG 		completionCode)
{
	IF_IOBuffer *	pIOBuffer = (IF_IOBuffer *)callbackData;

	F_UNREFERENCED_PARM( unknownAlwaysZero);

	pIOBuffer->notifyComplete(
		(RCODE)(completionCode == DFSNormalCompletion
				  ? NE_FLM_OK
				  : DfsMapError( completionCode, NE_FLM_DIRECT_WRITING_FILE)));
}
#endif

/****************************************************************************
Desc: NSS async I/O completion callback
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FSTATIC void nssDioCallback(
	FLMUINT	reserved,
	FLMUINT	callbackData,
	FLMUINT	completionCode)
{
	IF_IOBuffer *	pIOBuffer = (IF_IOBuffer *)callbackData;

	F_UNREFERENCED_PARM( reserved);

	pIOBuffer->notifyComplete( 
		(RCODE)(completionCode == zOK
				  ? NE_FLM_OK
				  : MapNSSError( completionCode, NE_FLM_DIRECT_WRITING_FILE)));
}
#endif

/****************************************************************************
Desc:		Might call the direct IO Write routine in the future
Note:		This routine assumes that the size of pvBuffer is a multiple of
			sector size (512 bytes) and can be used to write out full
			sectors.  Even if uiBytesToWrite does not account for full sectors,
			data from the buffer will still be written out - a partial sector
			on disk will not be preserved.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::sectorWrite(
	FLMUINT64			ui64WriteOffset,
	FLMUINT				uiBytesToWrite,
	const void *		pvBuffer,
	FLMUINT				uiBufferSize,
	void *				pvBufferObj,
	FLMUINT *			puiBytesWrittenRV,
	FLMBOOL				bZeroFill)
{
	RCODE					rc = NE_FLM_OK;
	IF_IOBuffer *		pBufferObj = (IF_IOBuffer *)pvBufferObj;

	F_UNREFERENCED_PARM( uiBufferSize);

	if ( m_bDoDirectIO)
	{
		if( RC_BAD( rc = _directIOSectorWrite( (FLMUINT)ui64WriteOffset, 
			uiBytesToWrite, pvBuffer, pBufferObj, puiBytesWrittenRV, bZeroFill)))
		{
			goto Exit;
		}
	}
	else
	{
		flmAssert( pBufferObj == NULL);
		
		if( RC_BAD( rc = _write( (FLMUINT)ui64WriteOffset, 
			uiBytesToWrite, pvBuffer, puiBytesWrittenRV)))
		{
			goto Exit;
		}
	}

Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:		Calls the direct IO-style write routine.
Note:		This routine assumes that the size of pvBuffer is a multiple of
			sector size (512 bytes) and can be used to write out full
			sectors.  Even if uiBytesToWrite does not account for full sectors,
			data from the buffer will still be written out - a partial sector
			on disk will not be preserved.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::_directIOSectorWrite(
	FLMUINT			uiWriteOffset,
	FLMUINT			uiBytesToWrite,
	const void *	pvBuffer,
	IF_IOBuffer *	pBufferObj,
	FLMUINT *		puiBytesWrittenRV,
	FLMBOOL			bZeroFill)
{
	RCODE		rc = NE_FLM_OK;
	LONG		lStartSector;
	LONG		lSectorCount;
	FLMBOOL	bDidAsync = FALSE;

	flmAssert( m_bFileOpened == TRUE);

	if (uiWriteOffset == FLM_IO_CURRENT_POS)
	{
		uiWriteOffset = m_uiCurrentPos;
	}
	else
	{
		m_uiCurrentPos = uiWriteOffset;
	}
	
	if (uiWriteOffset % FLM_NLM_SECTOR_SIZE != 0)
	{
		rc = write( uiWriteOffset, uiBytesToWrite, pvBuffer,
							puiBytesWrittenRV);
		goto Exit;
	}

	// Calculate the starting sector and number of sectors to write

	lStartSector = uiWriteOffset / FLM_NLM_SECTOR_SIZE;
	lSectorCount = (LONG)(uiBytesToWrite / FLM_NLM_SECTOR_SIZE);
	if (uiBytesToWrite % FLM_NLM_SECTOR_SIZE != 0)
	{
		FLMBYTE *	pucBuffer = (FLMBYTE *)pvBuffer;

		lSectorCount++;

		if (bZeroFill)
		{
			// Zero out the part of the buffer that was not included in
			// uiBytesToWrite - because it will still be written to disk.

			f_memset( &pucBuffer [uiBytesToWrite], 0,
						(FLMUINT)(FLM_NLM_SECTOR_SIZE - (uiBytesToWrite % FLM_NLM_SECTOR_SIZE)));
		}
	}

	if( RC_BAD( rc = writeSectors( (void *)pvBuffer, lStartSector, lSectorCount,
											pBufferObj, &bDidAsync)))
	{
		goto Exit;
	}

	m_uiCurrentPos += uiBytesToWrite;

	if( puiBytesWrittenRV)
	{
		*puiBytesWrittenRV = uiBytesToWrite;
	}

Exit:

	if( !bDidAsync && pBufferObj)
	{
		pBufferObj->notifyComplete( rc);
	}
	
	return( rc);
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE F_FileHdl::flush( void)
{
	return( NE_FLM_OK);
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FLMBOOL F_FileHdl::canDoAsync( void)
{
	return( m_bDoDirectIO);
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::lock( void)
{
	return( RC_SET_AND_ASSERT( NE_FLM_NOT_IMPLEMENTED));
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI F_FileHdl::unlock( void)
{
	return( RC_SET_AND_ASSERT( NE_FLM_NOT_IMPLEMENTED));
}
#endif

/****************************************************************************
Desc:	Determine if a file or directory exists
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE f_netwareTestIfFileExists(
	const char *	pPath)
{
	RCODE			rc = NE_FLM_OK;
	LONG			unused;
	FLMBYTE		ucPseudoLNamePath[ F_PATH_MAX_SIZE + 1];
	FLMBYTE		ucLNamePath[ F_PATH_MAX_SIZE];
	LONG			lVolumeID;
	LONG			lPathID;
	LONG			lLNamePathCount;
	LONG			lDirectoryID;
	LONG			lErrorCode;

	f_strcpy( (char *)&ucPseudoLNamePath[1], pPath);
	ucPseudoLNamePath[0] = (char)f_strlen( pPath);
	
	if( (lErrorCode = ConvertPathString( 0, 0, ucPseudoLNamePath, &lVolumeID,		
		&lPathID, ucLNamePath, &lLNamePathCount)) != 0)
	{
		goto Exit;
	}

	if( (lErrorCode = MapPathToDirectoryNumber( 0, lVolumeID, 0, ucLNamePath,
		lLNamePathCount, LONGNameSpace, &lDirectoryID, &unused)) != 0)
	{
		goto Exit;
	}

Exit:

	if( lErrorCode == 255 || lErrorCode == 156)
	{
		// Too many error codes map to 255, so we put in a special
		// case check here

		rc = RC_SET( NE_FLM_IO_PATH_NOT_FOUND);
	}
	else if( lErrorCode )
	{
		rc = f_mapPlatformError( lErrorCode, NE_FLM_CHECKING_FILE_EXISTENCE);
	}
	
	return( rc);
}
#endif

/****************************************************************************
Desc:		Delete a file
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE f_netwareDeleteFile(
	const char *	pPath)
{
	RCODE				rc = NE_FLM_OK;
	LONG				lErrorCode;
	char				pszQualifiedPath[ F_PATH_MAX_SIZE];
	FLMBYTE			ucLNamePath[ F_PATH_MAX_SIZE + 1];
	LONG				lLNamePathCount;
	LONG				lVolumeID;
	FLMBOOL			bNssVolume = FALSE;

	ConvertToQualifiedNWPath( pPath, pszQualifiedPath);

	if( (lErrorCode = ConvertPathToLNameFormat( pszQualifiedPath, &lVolumeID,
			&bNssVolume, ucLNamePath, &lLNamePathCount)) != 0)
	{
		rc = f_mapPlatformError( lErrorCode, NE_FLM_RENAMING_FILE);
		goto Exit;
	}

	if( gv_bNSSKeyInitialized && bNssVolume)
	{
		if( (lErrorCode = gv_zDeleteFunc( gv_NssRootKey, 0,
								zNSPACE_LONG | zMODE_UTF8,
								pszQualifiedPath, zMATCH_ALL, 0)) != zOK)
		{
			rc = MapNSSError( lErrorCode, NE_FLM_IO_DELETING_FILE);
			goto Exit;
		}
	}
	else
	{
		if( (lErrorCode = EraseFile( 0, 1, lVolumeID, 0, ucLNamePath,
			lLNamePathCount, LONGNameSpace, 0)) != 0)
		{
			// Too many error codes map to 255, so we put in a special
			// case check here.

			if( lErrorCode == 255)
			{
				rc = RC_SET( NE_FLM_IO_PATH_NOT_FOUND);
			}
			else
			{
				rc = f_mapPlatformError( lErrorCode, NE_FLM_IO_DELETING_FILE);
			}
			
			goto Exit;
		}
	}
	
Exit:

	return( rc);
}
#endif

/****************************************************************************
Desc:	Turn off the rename inhibit bit for a file in an NSS volume.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FSTATIC RCODE nssTurnOffRenameInhibit(
	const char *	pszFileName)
{
	RCODE				rc = NE_FLM_OK;
	zInfo_s			Info;
	FLMINT64			NssKey;
	FLMBOOL			bFileOpened = FALSE;
	FLMINT			lStatus;
	FLMUINT			nOpenAttr;

	nOpenAttr = f_getNSSOpenFlags( (FLMUINT)(FLM_IO_RDWR |
															FLM_IO_SH_DENYNONE), FALSE);
															
	if( (lStatus = gv_zOpenFunc( gv_NssRootKey, 1, zNSPACE_LONG | zMODE_UTF8,
					pszFileName, nOpenAttr, &NssKey)) != zOK)
	{
		rc = MapNSSError( lStatus, NE_FLM_OPENING_FILE);
		goto Exit;
	}
	
	bFileOpened = TRUE;

	// Get the file attributes.

	if( (lStatus = gv_zGetInfoFunc( NssKey, zGET_STD_INFO, sizeof( Info),
				zINFO_VERSION_A, &Info)) != zOK)
	{
		rc = MapNSSError( lStatus, NE_FLM_GETTING_FILE_INFO);
		goto Exit;
	}
	
	flmAssert( Info.infoVersion == zINFO_VERSION_A);

	// See if the rename inhibit bit is set.

	if( Info.std.fileAttributes & zFA_RENAME_INHIBIT)
	{
		// Turn bit off

		Info.std.fileAttributes = 0;

		// Specify which bits to modify - only rename inhibit in this case

		Info.std.fileAttributesModMask = zFA_RENAME_INHIBIT;

		if( (lStatus = gv_zModifyInfoFunc( NssKey, 0, zMOD_FILE_ATTRIBUTES,
			sizeof( Info), zINFO_VERSION_A, &Info)) != zOK)
		{
			rc = MapNSSError( lStatus, NE_FLM_SETTING_FILE_INFO);
			goto Exit;
		}
	}
	
Exit:

	if( bFileOpened)
	{
		(void)gv_zCloseFunc( NssKey);
	}
	
	return( rc);
}
#endif

/****************************************************************************
Desc:		Rename a file
Notes:	Currently, this function doesn't support moving the file from one
			volume to another.  (There is a CopyFileToFile function that could
			be used to do the move.)  The toolkit function does appear to 
			support moving (copy/delete) the file.
			
			This function does support renaming directories.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE f_netwareRenameFile(
	const char *	pOldFilePath,
	const char *	pNewFilePath)
{
	RCODE				rc = NE_FLM_OK;
	LONG				unused;
	FLMBYTE			ucOldLNamePath[ F_PATH_MAX_SIZE + 1];
	LONG				lOldLNamePathCount;
	FLMBYTE			ucNewLNamePath[ F_PATH_MAX_SIZE + 1];
	LONG				lNewLNamePathCount;
	LONG				lVolumeID;
	LONG				lErrorCode;
	FLMBYTE			ucPseudoLNamePath[ F_PATH_MAX_SIZE + 1];
	LONG				lPathID;
	LONG				lIsFile;
	FLMBOOL			bIsDirectory;
	struct 			ModifyStructure modifyStruct;
	LONG				lDirectoryID;
	LONG				lFileAttributes;
	LONG				lMatchBits;
	FLMBOOL			bNssVolume =
							(FLMBOOL)(gv_zIsNSSVolumeFunc
								? (gv_zIsNSSVolumeFunc( (const char *)pOldFilePath)
									 ? TRUE
									 : FALSE)
								: FALSE);

	if( gv_bNSSKeyInitialized && bNssVolume)
	{
		FLMINT	lStatus;
		FLMBOOL	bTurnedOffRenameInhibit = FALSE;

Retry_Nss_Rename:

		if( (lStatus = gv_zRenameFunc( gv_NssRootKey, 0,
			zNSPACE_LONG | zMODE_UTF8, pOldFilePath, zMATCH_ALL,
			zNSPACE_LONG | zMODE_UTF8, pNewFilePath, 0)) != zOK)
		{
			if( lStatus == zERR_NO_RENAME_PRIVILEGE && !bTurnedOffRenameInhibit)
			{
				// Attempt to turn off rename inhibit.  This isn't always the
				// reason for zERR_NO_RENAME_PRIVILEGE, but it is one we
				// definitely need to take care of.

				if( RC_BAD( rc = nssTurnOffRenameInhibit( pOldFilePath)))
				{
					goto Exit;
				}
				
				bTurnedOffRenameInhibit = TRUE;
				goto Retry_Nss_Rename;
			}
			
			rc = MapNSSError( lStatus, NE_FLM_RENAMING_FILE);
			goto Exit;
		}
	}
	else
	{
		f_strcpy( (char *)&ucPseudoLNamePath[1], pOldFilePath);
		ucPseudoLNamePath[0] = (char)f_strlen( (const char *)&ucPseudoLNamePath[1] );
		
		if( (lErrorCode = ConvertPathString( 0, 0, ucPseudoLNamePath, &lVolumeID,		
			&lPathID, (BYTE *)ucOldLNamePath, &lOldLNamePathCount)) != 0)
		{
			goto Exit;
		}

		if( (lErrorCode = MapPathToDirectoryNumber( 0, lVolumeID, 0,
			(BYTE *)ucOldLNamePath, lOldLNamePathCount, LONGNameSpace,
			&lDirectoryID, &lIsFile)) != 0)
		{
			goto Exit;
		}
		
		if( lIsFile)
		{
			bIsDirectory = FALSE;
			lMatchBits = 0;
		}
		else
		{
			bIsDirectory = TRUE;
			lMatchBits = SUBDIRECTORY_BIT;
		}
		
		f_strcpy( (char *)&ucPseudoLNamePath[1], pNewFilePath);
		ucPseudoLNamePath[0] = (char)f_strlen( (const char *)&ucPseudoLNamePath[1]);
		
		if( (lErrorCode = ConvertPathString( 0, 0, ucPseudoLNamePath, &unused,
			&lPathID, (BYTE *)ucNewLNamePath, &lNewLNamePathCount)) != 0)
		{
			goto Exit;
		}

		{
			struct DirectoryStructure * pFileInfo;

			if( (lErrorCode = VMGetDirectoryEntry( lVolumeID, 
				lDirectoryID & 0x00ffffff, &pFileInfo)) != 0)
			{
				goto Exit;
			}
			
			lFileAttributes = pFileInfo->DFileAttributes;
		}
		
		if( lFileAttributes & RENAME_INHIBIT_BIT )
		{
			f_memset(&modifyStruct, 0, sizeof(modifyStruct));
			modifyStruct.MFileAttributesMask = RENAME_INHIBIT_BIT;
			
			if( (lErrorCode = ModifyDirectoryEntry( 0, 1, lVolumeID, 0,
				(BYTE *)ucOldLNamePath, lOldLNamePathCount, LONGNameSpace, 
				lMatchBits, LONGNameSpace, &modifyStruct,
				MFileAttributesBit, 0)) != 0)
			{
				goto Exit;
			}
		}

		lErrorCode = RenameEntry( 0, 1, lVolumeID, 0, ucOldLNamePath,
			lOldLNamePathCount, LONGNameSpace, lMatchBits,
			(BYTE)bIsDirectory ? 1 : 0, 0, ucNewLNamePath, lNewLNamePathCount,
			TRUE, TRUE);

		if( lFileAttributes & RENAME_INHIBIT_BIT )
		{
			FLMBYTE *		pFileName;

			if( lErrorCode )
			{
				pFileName = ucOldLNamePath;
				lNewLNamePathCount = lOldLNamePathCount;
			}
			else
			{
				pFileName = ucNewLNamePath;
			}
				
			// Turn the RENAME_INHIBIT_BIT back on
			
			f_memset(&modifyStruct, 0, sizeof(modifyStruct));
			modifyStruct.MFileAttributes = RENAME_INHIBIT_BIT;
			modifyStruct.MFileAttributesMask = RENAME_INHIBIT_BIT;

			(void)ModifyDirectoryEntry( 0, 1, lVolumeID, 0, (BYTE *)pFileName,
				lNewLNamePathCount, LONGNameSpace, lMatchBits, LONGNameSpace,
				&modifyStruct, MFileAttributesBit, 0);
		}
	}

Exit:

	if( !gv_bNSSKeyInitialized || !bNssVolume)
	{
		if( lErrorCode )
		{
			// Too many error codes map to 255, so we put in a special
			// case check here.

			if( lErrorCode == 255)
			{
				rc = RC_SET( NE_FLM_IO_PATH_NOT_FOUND);
			}
			else
			{
				rc = f_mapPlatformError( lErrorCode, NE_FLM_RENAMING_FILE);
			}
		}
	}
	
	return( rc);
}
#endif

/****************************************************************************
Desc:		Convert the given path to NetWare LName format.
Input:	pPath = qualified netware path of the format:
						volume:directory_1\...\directory_n\filename.ext
Output:	plVolumeID = NetWare volume ID
			pLNamePath = NetWare LName format path
											 
				Netware expects paths to be in LName format:
					<L1><C1><L2><C2>...<Ln><Cn>
					where <Lx> is a one-byte length and <Cx> is a path component.
					
					Example: 6SYSTEM4Fred
						note that the 6 and 4 are binary, not ASCII

			plLNamePathCount = number of path components in pLNamePath
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FSTATIC LONG ConvertPathToLNameFormat(
	const char *	pPath,
	LONG *			plVolumeID,
	FLMBOOL *		pbNssVolume,
	FLMBYTE *		pLNamePath,
	LONG *			plLNamePathCount)
{
	FLMBYTE			ucPseudoLNamePath[ F_PATH_MAX_SIZE + 1];
	LONG				lPathID;
	LONG				lErrorCode = 0;

	*pLNamePath = 0;
	*plLNamePathCount = 0;

	*pbNssVolume = (FLMBOOL)(gv_zIsNSSVolumeFunc
									? (gv_zIsNSSVolumeFunc( (const char *)pPath)
										? TRUE
										: FALSE)
									: FALSE);

	if( gv_bNSSKeyInitialized && *pbNssVolume)
	{
		f_strcpy( (char *)pLNamePath, pPath);
		*plLNamePathCount = 1;
	}
	else
	{
		f_strcpy( (char *)&ucPseudoLNamePath[1], pPath);
		ucPseudoLNamePath[0] = (FLMBYTE)f_strlen( (const char *)&ucPseudoLNamePath[1]);
		
		if( (lErrorCode = ConvertPathString( 0, 0, ucPseudoLNamePath, plVolumeID,		
			&lPathID, (BYTE *)pLNamePath, plLNamePathCount)) != 0)
		{
			goto Exit;
		}
	}

Exit:

	return( lErrorCode );
}
#endif

/****************************************************************************
Desc:		Convert the given path to a NetWare format.  The format isn't 
			critical, it just needs to be consistent.  See below for a 
			description of the format chosen.
Input:	pInputPath = a path to a file
Output:	pszQualifiedPath = qualified netware path of the format:
									volume:directory_1\...\directory_n\filename.ext
									
			If no volume is given, "SYS:" is the default.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FSTATIC void ConvertToQualifiedNWPath(
	const char *		pInputPath,
	char *				pszQualifiedPath)
{
	char					ucFileName [F_FILENAME_SIZE];
	char					ucVolume [MAX_NETWARE_VOLUME_NAME+1];
	char					ucPath [F_PATH_MAX_SIZE + 1];
	IF_FileSystem *	pFileSystem = f_getFileSysPtr();

	// Separate path into its components: volume, path...

	pFileSystem->pathParse( pInputPath, NULL, ucVolume, ucPath, ucFileName);

	// Rebuild path to a standard, fully-qualified format, defaulting the
	// volume if one isn't specified.

	*pszQualifiedPath = 0;
	if( ucVolume [0])
	{
		// Append the volume specified by the user.

		f_strcat( pszQualifiedPath, ucVolume );
	}
	else
	{
		// No volume specified, use the default

		f_strcat( pszQualifiedPath, "SYS:");
	}
	
	if( ucPath [0])
	{
		// User specified a path...

		if( ucPath[0] == '\\' || ucPath[0] == '/' )
		{
			// Append the path to the volume without the leading slash

			f_strcat( pszQualifiedPath, &ucPath [1]);
		}
		else
		{
			// Append the path to the volume

			f_strcat( pszQualifiedPath, ucPath);
		}
	}

	if( ucFileName [0])
	{
		// Append the file name to the path

		pFileSystem->pathAppend( pszQualifiedPath, ucFileName);
	}
}
#endif

/****************************************************************************
Desc:	This API is called once the first time NWYieldIfTime() is called if
		kYieldIfTimeSliceUp isn't supported by the host OS.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FSTATIC int LocalYieldInit(void)
{
	unsigned char *	buffer;
	unsigned char *	pointer;
	unsigned long		ccode;
	unsigned long		address;
	unsigned long		(*ConvertMicroSecondsToClocks)(unsigned long data);

	// Fixup ThreadFeededQue

	buffer = (unsigned char *)"SERVER.NLM|ThreadFeederQue";

	// Get the address from the debugger

	ccode = CEvaluateExpression( &buffer, &stackFrame[0], &address);
	if (ccode != 0)
	{
		return( -1);
	}

	ThreadFeederQue = (unsigned long *)address;

	// Setup yieldTimeSlice
	// First get a pointer to ConvertMicroSecondsToClocks and then verify it

	pointer = (unsigned char *)(&StopBell);
	pointer = pointer + 0x1ea;
	if (pointer[0] != 0x8b ||
			pointer[1] != 0x44 ||
			pointer[2] != 0x24 ||
			pointer[3] != 0x04 ||
			pointer[4] != 0x33 ||
			pointer[5] != 0xd2 ||
			pointer[6] != 0xf7)
	{
		return( -1);
	}

	*(unsigned long *)(&ConvertMicroSecondsToClocks) = (unsigned long)pointer;

	// Now calculate our desired time slice in clocks

	yieldTimeSlice = (*ConvertMicroSecondsToClocks)(200);

	// Get a pointer to the per-CPU data area

	buffer = (unsigned char *)"LOADER.NLM|PerCpuDataArea";
	ccode = CEvaluateExpression( &buffer, &stackFrame[0], &address);
	if (ccode != 0)
	{
		return( -1);
	}

	pPerCpuDataArea = (struct OldPerCpuStruct *)address;
	return( 0);
}
#endif

/****************************************************************************
Desc:	This routine does everything that kYieldIfTimeSliceUp() does, and it is
		compatible with 5.0 SP4 and 5.1 release.  It shouldn't be used on host
		OSs that support kYieldIfTimeSliceUp().
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
FSTATIC void OldOSCompatibleYield(void)
{
	unsigned long	timeStamp;
	unsigned long	threadStartTime;

	if ((CpuCurrentProcessor == 0) && (WorkToDoListHead != 0))
	{

		// If we are P0 and there is legacy WTD waiting, then yield

		kYieldThread();
		return;
	}

	if ((pPerCpuDataArea->PSD_LocalWTDHead != 0) ||
		 (kQueCount( ThreadFeederQue[CpuCurrentProcessor]) != 0))
	{

		// If there is MPK WTD or a feederQ thread waiting, then yield

		kYieldThread();
		return;
	}

	timeStamp = ReadInternalClock();
	threadStartTime = pPerCpuDataArea->PSD_ThreadStartClocks;

	// Note 32 bit arithmetic is sufficient and less overhead

	if ((timeStamp - threadStartTime) > yieldTimeSlice)
	{
		kYieldThread();
	}
}
#endif

/****************************************************************************
Desc: This is the routine to call for time sensitive yielding.  The first
		time it is called, it will change itself to a JMP to the appropriate
		yield procedure.  If it is on 5.1 SP1 or greater, it will jump to
		kYieldIfTimeSliceUp().  If it is on 5.0 SP4 or 5.1 release, it will
		jump to OldOSCompatibleYield().  Otherwise it will jump to
		kYieldThread().
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
void NWYieldIfTime( void)
{
	unsigned char *	fixup;
	unsigned long		address;

	*(unsigned long *)(&address) = ImportPublicSymbol(
			(unsigned long)f_getNLMHandle(),
			(unsigned char *)"\x13" "kYieldIfTimeSliceUp");

	fixup = (unsigned char *)(&NWYieldIfTime);

	if (address == 0)
	{
		// We couldn't import the procedure, so see if our local routine 
		// is compatible.

		if (LocalYieldInit() == 0)
		{
			address = (unsigned long)(&OldOSCompatibleYield);
		}
		else
		{
			address = (unsigned long)(&kYieldThread);
		}
	}

	fixup[0] = 0xE9;
	++fixup;
	address = address - (4 + (unsigned long)fixup);
	*(unsigned long *)fixup = address;
}
#endif

/****************************************************************************
Desc:
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
void FLMAPI f_yieldCPU( void)
{
	NWYieldIfTime();
}
#endif

/****************************************************************************
Desc: 	Function that must be called within a NLM's startup routine.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE f_netwareStartup( void)
{
	RCODE		rc = NE_FLM_OK;

	if( f_atomicInc( &gv_NetWareStartupCount) != 1)
	{
		goto Exit;
	}

	gv_MyModuleHandle = CFindLoadModuleHandle( (void *)f_netwareShutdown);

	// Allocate the needed resource tags

	if( (gv_lAllocRTag = AllocateResourceTag(
						gv_MyModuleHandle,
						"NOVDB Memory", AllocSignature)) == NULL)
	{
		rc = RC_SET( NE_FLM_MEM);
		goto Exit;
	}
	
	// Initialize NSS
	
	if( RC_BAD( rc = f_nssInitialize()))
	{
		goto Exit;
	}


Exit:

	if( RC_BAD( rc))
	{
		f_netwareShutdown();
	}

	return( rc);
}
#endif

/****************************************************************************
Desc: 	Closes (Frees) any resources used by FLAIM's clib patches layer.
****************************************************************************/
#if defined( FLM_RING_ZERO_NLM)
void f_netwareShutdown( void)
{
	// Call exit function.

	if( f_atomicDec( &gv_NetWareStartupCount) != 0)
	{
		goto Exit;
	}
	
	f_nssShutdown();

	if( gv_lAllocRTag)
	{
		ReturnResourceTag( gv_lAllocRTag, 1);
		gv_lAllocRTag = 0;
	}

	gv_MyModuleHandle = NULL;

Exit:

	return;
}
#endif

/****************************************************************************
Desc: 	
****************************************************************************/
void * f_getNLMHandle( void)
{
#if defined( FLM_RING_ZERO_NLM)
	return( gv_MyModuleHandle);
#else
	return( getnlmhandle());
#endif
}

/**********************************************************************
Desc:
**********************************************************************/
#if defined( FLM_RING_ZERO_NLM)
RCODE FLMAPI f_chdir(
	const char *		pszDir)
{
	F_UNREFERENCED_PARM( pszDir);
	return( RC_SET( NE_FLM_NOT_IMPLEMENTED));
}
#endif
	
#endif // FLM_NLM

/****************************************************************************
Desc:
****************************************************************************/
#if defined( FLM_OSX)
void gv_fnlm()
{
}
#endif
