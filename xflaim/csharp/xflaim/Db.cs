//------------------------------------------------------------------------------
// Desc:	Db Class
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
using System;
using System.Runtime.InteropServices;

namespace xflaim
{
	//-----------------------------------------------------------------------------
	// Element tags
	//-----------------------------------------------------------------------------

	/// <summary>
	/// Reserved dictionary tags for elements
	/// IMPORTANT NOTE: These need to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	public enum ReservedElmTag : uint
	{
		/// <summary>
		/// "element"
		/// </summary>
		ELM_ELEMENT_TAG = 0xFFFFFE00,
		/// <summary>
		/// "attribute"
		/// </summary>
		ELM_ATTRIBUTE_TAG = 0xFFFFFE01,
		/// <summary>
		/// "Index"
		/// </summary>
		ELM_INDEX_TAG = 0xFFFFFE02,
		/// <summary>
		/// "ElementComponent"
		/// </summary>
		ELM_ELEMENT_COMPONENT_TAG = 0xFFFFFE04,
		/// <summary>
		/// "AttributeComponent"
		/// </summary>
		ELM_ATTRIBUTE_COMPONENT_TAG = 0xFFFFFE05,
		/// <summary>
		/// "Collection"
		/// </summary>
		ELM_COLLECTION_TAG = 0xFFFFFE06,
		/// <summary>
		/// "Prefix"
		/// </summary>
		ELM_PREFIX_TAG = 0xFFFFFE07,
		/// <summary>
		/// "NextDictNums"
		/// </summary>
		ELM_NEXT_DICT_NUMS_TAG = 0xFFFFFE08,
		/// <summary>
		/// "DocumentTitle"
		/// </summary>
		ELM_DOCUMENT_TITLE_TAG = 0xFFFFFE09,
		/// <summary>
		/// "Invalid"
		/// </summary>
		ELM_INVALID_TAG = 0xFFFFFE0A,
		/// <summary>
		/// "Quarantined"
		/// </summary>
		ELM_QUARANTINED_TAG = 0xFFFFFE0B,
		/// <summary>
		/// "All"
		/// </summary>
		ELM_ALL_TAG = 0xFFFFFE0C,
		/// <summary>
		/// "Annotation"
		/// </summary>
		ELM_ANNOTATION_TAG = 0xFFFFFE0D,
		/// <summary>
		/// "Any"
		/// </summary>
		ELM_ANY_TAG = 0xFFFFFE0E,
		/// <summary>
		/// "AttributeGroup"
		/// </summary>
		ELM_ATTRIBUTE_GROUP_TAG = 0xFFFFFE0F,
		/// <summary>
		/// "Choice"
		/// </summary>
		ELM_CHOICE_TAG = 0xFFFFFE10,
		/// <summary>
		/// "ComplexContent"
		/// </summary>
		ELM_COMPLEX_CONTENT_TAG = 0xFFFFFE11,
		/// <summary>
		/// "ComplexType"
		/// </summary>
		ELM_COMPLEX_TYPE_TAG = 0xFFFFFE12,
		/// <summary>
		/// "Documentation"
		/// </summary>
		FLM_DOCUMENTATION_TAG = 0xFFFFFE13,
		/// <summary>
		/// "enumeration"
		/// </summary>
		ELM_ENUMERATION_TAG = 0xFFFFFE14,
		/// <summary>
		/// "extension"
		/// </summary>
		ELM_EXTENSION_TAG = 0xFFFFFE15,
		/// <summary>
		/// "Delete"
		/// </summary>
		ELM_DELETE_TAG = 0xFFFFFE16,
		/// <summary>
		/// "BlockChain"
		/// </summary>
		ELM_BLOCK_CHAIN_TAG = 0xFFFFFE17,
		/// <summary>
		/// "EncDef"
		/// </summary>
		ELM_ENCDEF_TAG = 0xFFFFFE18,
		/// <summary>
		/// "Sweep"
		/// </summary>
		ELM_SWEEP_TAG = 0xFFFFFE19
	}

	//-----------------------------------------------------------------------------
	// Attribute tags
	//-----------------------------------------------------------------------------

	/// <summary>
	/// Reserved dictionary tags for attributes
	/// IMPORTANT NOTE: These need to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	public enum ReservedAttrTag : uint
	{
		/// <summary>
		/// "DictNumber"
		/// </summary>
		ATTR_DICT_NUMBER_TAG = 0xFFFFFE00,
		/// <summary>
		/// "CollectionNumber"
		/// </summary>
		ATTR_COLLECTION_NUMBER_TAG = 0xFFFFFE01,
		/// <summary>
		/// "CollectionName"
		/// </summary>
		ATTR_COLLECTION_NAME_TAG = 0xFFFFFE02,
		/// <summary>
		/// "name"
		/// </summary>
		ATTR_NAME_TAG = 0xFFFFFE03,
		/// <summary>
		/// "targetNameSpace"
		/// </summary>
		ATTR_TARGET_NAMESPACE_TAG = 0xFFFFFE04,
		/// <summary>
		/// "type"
		/// </summary>
		ATTR_TYPE_TAG = 0xFFFFFE05,
		/// <summary>
		/// "State"
		/// </summary>
		ATTR_STATE_TAG = 0xFFFFFE06,
		/// <summary>
		/// "Language"
		/// </summary>
		ATTR_LANGUAGE_TAG = 0xFFFFFE07,
		/// <summary>
		/// "IndexOptions"
		/// </summary>
		ATTR_INDEX_OPTIONS_TAG = 0xFFFFFE08,
		/// <summary>
		/// "IndexOn"
		/// </summary>
		ATTR_INDEX_ON_TAG = 0xFFFFFE09,
		/// <summary>
		/// "Required"
		/// </summary>
		ATTR_REQUIRED_TAG = 0xFFFFFE0A,
		/// <summary>
		/// "Limit"
		/// </summary>
		ATTR_LIMIT_TAG = 0xFFFFFE0B,
		/// <summary>
		/// "CompareRules"
		/// </summary>
		ATTR_COMPARE_RULES_TAG = 0xFFFFFE0C,
		/// <summary>
		/// "KeyComponent"
		/// </summary>
		ATTR_KEY_COMPONENT_TAG = 0xFFFFFE0D,
		/// <summary>
		/// "DataComponent"
		/// </summary>
		ATTR_DATA_COMPONENT_TAG = 0xFFFFFE0E,
		/// <summary>
		/// "LastDocumentIndexed"
		/// </summary>
		ATTR_LAST_DOC_INDEXED_TAG = 0xFFFFFE0F,
		/// <summary>
		/// "NextElementNum"
		/// </summary>
		ATTR_NEXT_ELEMENT_NUM_TAG = 0xFFFFFE10,
		/// <summary>
		/// "NextAttributeNum"
		/// </summary>
		ATTR_NEXT_ATTRIBUTE_NUM_TAG = 0xFFFFFE11,
		/// <summary>
		/// "NextIndexNum"
		/// </summary>
		ATTR_NEXT_INDEX_NUM_TAG = 0xFFFFFE12,
		/// <summary>
		/// "NextCollectionNum"
		/// </summary>
		ATTR_NEXT_COLLECTION_NUM_TAG = 0xFFFFFE13,
		/// <summary>
		/// "NextPrefixNum"
		/// </summary>
		ATTR_NEXT_PREFIX_NUM_TAG = 0xFFFFFE14,
		/// <summary>
		/// "Source"
		/// </summary>
		ATTR_SOURCE_TAG = 0xFFFFFE15,
		/// <summary>
		/// "StateChangeCount"
		/// </summary>
		ATTR_STATE_CHANGE_COUNT_TAG = 0xFFFFFE16,
		/// <summary>
		/// "xmlns"
		/// </summary>
		ATTR_XMLNS_TAG = 0xFFFFFE17,
		/// <summary>
		/// "abstract"
		/// </summary>
		ATTR_ABSTRACT_TAG = 0xFFFFFE18,
		/// <summary>
		/// "base"
		/// </summary>
		ATTR_BASE_TAG = 0xFFFFFE19,
		/// <summary>
		/// "block"
		/// </summary>
		ATTR_BLOCK_TAG = 0xFFFFFE1A,
		/// <summary>
		/// "default"
		/// </summary>
		ATTR_DEFAULT_TAG = 0xFFFFFE1B,
		/// <summary>
		/// "final"
		/// </summary>
		ATTR_FINAL_TAG = 0xFFFFFE1C,
		/// <summary>
		/// "fixed"
		/// </summary>
		ATTR_FIXED_TAG = 0xFFFFFE1D,
		/// <summary>
		/// "itemtype"
		/// </summary>
		ATTR_ITEM_TYPE_TAG = 0xFFFFFE1E,
		/// <summary>
		/// "membertypes"
		/// </summary>
		ATTR_MEMBER_TYPES_TAG = 0xFFFFFE1F,
		/// <summary>
		/// "mixed"
		/// </summary>
		ATTR_MIXED_TAG = 0xFFFFFE20,
		/// <summary>
		/// "nillable"
		/// </summary>
		ATTR_NILLABLE_TAG = 0xFFFFFE21,
		/// <summary>
		/// "ref"
		/// </summary>
		ATTR_REF_TAG = 0xFFFFFE22,
		/// <summary>
		/// "use"
		/// </summary>
		ATTR_USE_TAG = 0xFFFFFE23,
		/// <summary>
		/// "value"
		/// </summary>
		ATTR_VALUE_TAG = 0xFFFFFE24,
		/// <summary>
		/// "address"
		/// </summary>
		ATTR_ADDRESS_TAG = 0xFFFFFE25,
		/// <summary>
		/// "xmlns:xflaim"
		/// </summary>
		ATTR_XMLNS_XFLAIM_TAG = 0xFFFFFE26,
		/// <summary>
		/// "Key"
		/// </summary>
		ATTR_ENCRYPTION_KEY_TAG = 0xFFFFFE27,
		/// <summary>
		/// "Transaction"
		/// </summary>
		ATTR_TRANSACTION_TAG = 0xFFFFFE28,
		/// <summary>
		/// "NextEncDefNum"
		/// </summary>
		ATTR_NEXT_ENCDEF_NUM_TAG = 0xFFFFFE29,
		/// <summary>
		/// "encId"
		/// </summary>
		ATTR_ENCRYPTION_ID_TAG = 0xFFFFFE2A,
		/// <summary>
		/// "keySize"
		/// </summary>
		ATTR_ENCRYPTION_KEY_SIZE_TAG = 0xFFFFFE2B,
		/// <summary>
		/// "UniqueSubElements"
		/// </summary>
		ATTR_UNIQUE_SUB_ELEMENTS_TAG = 0xFFFFFE2C
	}

	//-----------------------------------------------------------------------------
	// XML parse errors
	//-----------------------------------------------------------------------------

	/// <summary>
	/// IMPORTANT NOTE: These need to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	public enum XMLParseError : uint
	{
		/// <summary>
		/// No error
		/// </summary>
		XML_NO_ERROR = 0,
		/// <summary>
		/// Invalid element name - does not start with a valid 
		/// character for element names
		/// </summary>
		XML_ERR_BAD_ELEMENT_NAME,
		/// <summary>
		/// Element names cannot be "xmlns" or have "xmlns:" as a prefix
		/// </summary>
		XML_ERR_XMLNS_IN_ELEMENT_NAME,
		/// <summary>
		/// The element begin and end tags do not match
		/// </summary>
		XML_ERR_ELEMENT_NAME_MISMATCH,
		/// <summary>
		/// The prefix for the element or attribute has not been defined with 
		/// an "xmlns:prefix=" attribute somewhere
		/// </summary>
		XML_ERR_PREFIX_NOT_DEFINED,
		/// <summary>
		/// Expecting a right angle bracket
		/// </summary>
		XML_ERR_EXPECTING_GT,
		/// <summary>
		/// Expecting a left angle bracket to begin an element name
		/// </summary>
		XML_ERR_EXPECTING_ELEMENT_LT,
		/// <summary>
		/// Expecting a '=' after the attribute name
		/// </summary>
		XML_ERR_EXPECTING_EQ,
		/// <summary>
		/// Multiple "xmlns" default namespace declarations in an element
		/// </summary>
		XML_ERR_MULTIPLE_XMLNS_DECLS,
		/// <summary>
		/// Multiple definitions for the same prefix ("xmlns:prefix=...") in an element
		/// </summary>
		XML_ERR_MULTIPLE_PREFIX_DECLS,
		/// <summary>
		/// Invalid xml declaration terminator
		/// </summary>
		XML_ERR_EXPECTING_QUEST_GT,
		/// <summary>
		/// Invalid XML markup
		/// </summary>
		XML_ERR_INVALID_XML_MARKUP,
		/// <summary>
		/// Must have at least one attr def in an ATTRLIST markup
		/// </summary>
		XML_ERR_MUST_HAVE_ONE_ATT_DEF,
		/// <summary>
		/// Expecting "NDATA" keyword
		/// </summary>
		XML_ERR_EXPECTING_NDATA,
		/// <summary>
		/// Expecting "SYSTEM" or "PUBLIC" keyword in NOTATION declaration
		/// </summary>
		XML_ERR_EXPECTING_SYSTEM_OR_PUBLIC,
		/// <summary>
		/// Expecting "("
		/// </summary>
		XML_ERR_EXPECTING_LPAREN,
		/// <summary>
		/// Expecing ")" or "|"
		/// </summary>
		XML_ERR_EXPECTING_RPAREN_OR_PIPE,
		/// <summary>
		/// Expecting a name
		/// </summary>
		XML_ERR_EXPECTING_NAME,
		/// <summary>
		/// Invalid Attr type in ATTLIST
		/// </summary>
		XML_ERR_INVALID_ATT_TYPE,
		/// <summary>
		/// Invalid default decl, expecting #FIXED, #REQUIRED, #IMPLIED, or quoted attr value
		/// </summary>
		XML_ERR_INVALID_DEFAULT_DECL,
		/// <summary>
		/// Expecting PCDATA - only PCDATA allowed after #
		/// </summary>
		XML_ERR_EXPECTING_PCDATA,
		/// <summary>
		/// Expecting "*"
		/// </summary>
		XML_ERR_EXPECTING_ASTERISK,
		/// <summary>
		/// Empty content is invalid - must be parameters between parens
		/// </summary>
		XML_ERR_EMPTY_CONTENT_INVALID,
		/// <summary>
		/// Cannot mix choice items with sequenced items.
		/// </summary>
		XML_ERR_CANNOT_MIX_CHOICE_AND_SEQ,
		/// <summary>
		/// "XML" is not a legal name for a processing instruction
		/// </summary>
		XML_ERR_XML_ILLEGAL_PI_NAME,
		/// <summary>
		/// Illegal first character in name - must be an alphabetic letter or underscore
		/// </summary>
		XML_ERR_ILLEGAL_FIRST_NAME_CHAR,
		/// <summary>
		/// Illegal second ":" found in name.  Name already has a colon.
		/// </summary>
		XML_ERR_ILLEGAL_COLON_IN_NAME,
		/// <summary>
		/// Expecting "version"
		/// </summary>
		XML_ERR_EXPECTING_VERSION,
		/// <summary>
		/// Invalid version number - only 1.0 is supported.
		/// </summary>
		XML_ERR_INVALID_VERSION_NUM,
		/// <summary>
		/// Unsupported encoding - must be "UTF-8" or "us-ascii"
		/// </summary>
		XML_ERR_UNSUPPORTED_ENCODING,
		/// <summary>
		/// Expecting "yes" or "no"
		/// </summary>
		XML_ERR_EXPECTING_YES_OR_NO,
		/// <summary>
		/// Expecting quote character - unexpected end of line
		/// </summary>
		XML_ERR_EXPECTING_QUOTE_BEFORE_EOL,
		/// <summary>
		/// Expecting ";"
		/// </summary>
		XML_ERR_EXPECTING_SEMI,
		/// <summary>
		/// Unexpected end of line in entity reference, need proper 
		/// terminating character - ";"
		/// </summary>
		XML_ERR_UNEXPECTED_EOL_IN_ENTITY,
		/// <summary>
		/// Invalid numeric character entity.  Number is either too large, or zero,
		/// or illegal characters were used in the number.
		/// </summary>
		XML_ERR_INVALID_CHARACTER_NUMBER,
		/// <summary>
		/// Unsupported predefined entity reference.
		/// </summary>
		XML_ERR_UNSUPPORTED_ENTITY,
		/// <summary>
		/// Expecting single or double quote character.
		/// </summary>
		XML_ERR_EXPECTING_QUOTE,
		/// <summary>
		/// Invalid character in public id.
		/// </summary>
		XML_ERR_INVALID_PUBLIC_ID_CHAR,
		/// <summary>
		/// Whitespace required
		/// </summary>
		XML_ERR_EXPECTING_WHITESPACE,
		/// <summary>
		/// Expecting HEX digit for binary value
		/// </summary>
		XML_ERR_EXPECTING_HEX_DIGIT,
		/// <summary>
		/// Invalid binary value for attribute
		/// </summary>
		XML_ERR_INVALID_BINARY_ATTR_VALUE,
		/// <summary>
		/// Error returned from createNode in processCDATA
		/// </summary>
		XML_ERR_CREATING_CDATA_NODE,
		/// <summary>
		/// Error returned from createNode in processComment
		/// </summary>
		XML_ERR_CREATING_COMMENT_NODE,
		/// <summary>
		/// Error returned from createNode in processPI
		/// </summary>
		XML_ERR_CREATING_PI_NODE,
		/// <summary>
		/// Error returned from createNode in processPI
		/// </summary>
		XML_ERR_CREATING_DATA_NODE,
		/// <summary>
		/// Error returned from createRootElement in processSTag
		/// </summary>
		XML_ERR_CREATING_ROOT_ELEMENT,
		/// <summary>
		/// Error returned from createNode in processSTag
		/// </summary>
		XML_ERR_CREATING_ELEMENT_NODE
	}
	
	//-----------------------------------------------------------------------------
	// XML encoding
	//-----------------------------------------------------------------------------

	/// <summary>
	/// IMPORTANT NOTE: These need to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	public enum XMLEncoding : uint
	{
		/// <summary>
		/// UTF-8 encoding
		/// </summary>
		XFLM_XML_UTF8_ENCODING,
		/// <summary>
		/// US ASCII encoding
		/// </summary>
		XFLM_XML_USASCII_ENCODING
	}

	//-----------------------------------------------------------------------------
	// XML import stats
	//-----------------------------------------------------------------------------

	/// <summary>
	/// XML import stats
	/// IMPORTANT NOTE: This structure needs to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	public class XFLM_IMPORT_STATS
	{
		/// <summary>
		/// Lines processed
		/// </summary>
		public uint					uiLines;
		/// <summary>
		/// Characters processed
		/// </summary>
		public uint					uiChars;
		/// <summary>
		/// Attributes processed
		/// </summary>
		public uint					uiAttributes;
		/// <summary>
		/// Elements processed
		/// </summary>
		public uint					uiElements;
		/// <summary>
		/// Text nodes processed
		/// </summary>
		public uint					uiText;
		/// <summary>
		/// Documents processed
		/// </summary>
		public uint					uiDocuments;
		/// <summary>
		/// Line number where the parser encountered an error
		/// </summary>
		public uint					uiErrLineNum;
		/// <summary>
		/// Offset in the line where the parser encountered an error
		/// </summary>
		public uint					uiErrLineOffset;
		/// <summary>
		/// Type of error encountered
		/// </summary>
		public XMLParseError		eErrorType;
		/// <summary>
		/// Offset in the stream where the line containing the error starts
		/// </summary>
		public uint					uiErrLineFilePos;
		/// <summary>
		/// Offset in the stream where the error starts
		/// </summary>
		public uint					uiErrLineBytes;
		/// <summary>
		/// XML encoding (UTF-8, etc.)
		/// </summary>
		XMLEncoding					eXMLEncoding;
	}

	//-----------------------------------------------------------------------------
	// Database transaction types
	//-----------------------------------------------------------------------------

	/// <summary>
	/// Database transaction types.
	/// IMPORTANT NOTE: These need to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	public enum eDbTransType : uint
	{
		/// <summary>No transaction</summary>
		XFLM_NO_TRANS = 0,
		/// <summary>Read transaction</summary>
		XFLM_READ_TRANS,
		/// <summary>Update transaction</summary>
		XFLM_UPDATE_TRANS
	}

	//-----------------------------------------------------------------------------
	// Database transaction flags
	//-----------------------------------------------------------------------------

	/// <summary>
	/// Database transaction flags.
	/// IMPORTANT NOTE: These need to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	[Flags]
	public enum DbTransFlags : uint
	{
		/// <summary>
		/// Do not terminate the transaction, even if
		/// a checkpoint is waiting to complete
		/// </summary>
		XFLM_DONT_KILL_TRANS = 0x0001,
		/// <summary>
		/// Place all blocks and nodes read during the transaction
		/// at the least-recently used positions in the cache lists.
		/// </summary>
		XFLM_DONT_POISON_CACHE = 0x0002
	}

	//-----------------------------------------------------------------------------
	// Database lock types
	//-----------------------------------------------------------------------------

	/// <summary>
	/// Types of locks that may be requested.
	/// IMPORTANT NOTE: These need to be kept in sync with the corresponding
	/// enum in ftk.h
	/// </summary>
	public enum eLockType : uint
	{
		/// <summary>No lock</summary>
		FLM_LOCK_NONE = 0,
		/// <summary>Exclusive lock</summary>
		FLM_LOCK_EXCLUSIVE,
		/// <summary>Shared lock</summary>
		FLM_LOCK_SHARED
	}

	/// <summary>
	/// Types of locks that may be requested.
	/// IMPORTANT NOTE: These need to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	public enum eXFlmIndexState : uint
	{
		/// <summary>Index is on-line and available for use.</summary>
		XFLM_INDEX_ONLINE = 0,
		/// <summary>Index is being built and is unavailable.</summary>
		XFLM_INDEX_BRINGING_ONLINE,
		/// <summary>Index has been suspended and is unavailable.</summary>
		XFLM_INDEX_SUSPENDED
	}

	/// <summary>
	/// Index status object
	/// IMPORTANT NOTE: This structure needs to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	public class XFLM_INDEX_STATUS
	{
		/// <summary>
		/// If ~0 then index is online, otherwise this is the value of the 
		/// last document ID that was indexed.
		/// </summary>
		public long  				ui64LastDocumentIndexed;
		/// <summary>
		/// Keys processed by the background indexing thread.
		/// </summary>
		public long					ui64KeysProcessed;
		/// <summary>
		/// Documents processed by the background indexing thread.
		/// </summary>
		public long					ui64DocumentsProcessed;
		/// <summary>
		/// Number of transactions completed by the background indexing thread.
		/// </summary>
		public long					ui64Transactions;
		/// <summary>
		/// ID of the index.
		/// </summary>
		public uint					ui32IndexNum;
		/// <summary>
		/// Time the bacground indexing thread (if any) was started.
		/// </summary>
		public uint					ui32StartTime;
		/// <summary>
		/// State of the background indexing thread (if any).
		/// </summary>
		public eXFlmIndexState	eState;
	}

	//-----------------------------------------------------------------------------
	// RetrieveFlags
	//-----------------------------------------------------------------------------

	/// <summary>
	/// Flags used to specify items to be retrieved from a result set.
	/// The <see cref="Db.keyRetrieve"/> method also uses these flags
	/// to specify how keys from an index are to be retrieved.
	/// IMPORTANT NOTE: These flags need to be kept in sync with the corresponding
	/// definitions in xflaim.h
	/// </summary>
	[Flags]
	public enum RetrieveFlags : uint
	{
		/// <summary>Return item greater than or equal to the search key.</summary>
		XFLM_INCL			= 0x0010,
		/// <summary>Return item greater than the search key.</summary>
		XFLM_EXCL			= 0x0020,
		/// <summary>Return item that exactly matches the search key.</summary>
		XFLM_EXACT			= 0x0040,
		/// <summary>
		/// Used in conjunction with XFLM_EXCL.  Specifies that the item to be
		/// returned must match the key components, but the node ids may be
		/// different.
		/// </summary>
		XFLM_KEY_EXACT		= 0x0080,
		/// <summary>Retrieve the first key in the index or first item in a result set.</summary>
		XFLM_FIRST			= 0x0100,
		/// <summary>Retrieve the last key in the index or last item in a result set.</summary>
		XFLM_LAST			= 0x0200,
		/// <summary>Specifies whether to match node IDs in the search key.</summary>
		XFLM_MATCH_IDS		= 0x0400,
		/// <summary>Specifies whether to match the document ID in the search key.</summary>
		XFLM_MATCH_DOC_ID = 0x0800
	}

	/// <remarks>
	/// The Db class provides a number of methods that allow C#
	/// applications to access an XFLAIM database.  A Db object
	/// is obtained by calling <see cref="DbSystem.dbCreate"/> or
	/// <see cref="DbSystem.dbOpen"/>
	/// </remarks>
	public class Db
	{
		private ulong 		m_pDb;			// Pointer to IF_Db object in unmanaged space
		private DbSystem 	m_dbSystem;

		//-----------------------------------------------------------------------------
		// Reserved strings
		//-----------------------------------------------------------------------------

		const string ELM_ELEMENT_TAG_NAME = "element";
		const string ELM_ATTRIBUTE_TAG_NAME = "attribute";
		const string ELM_INDEX_TAG_NAME = "Index";
		const string ELM_ELEMENT_COMPONENT_TAG_NAME = "ElementComponent";
		const string ELM_ATTRIBUTE_COMPONENT_TAG_NAME = "AttributeComponent";
		const string ELM_COLLECTION_TAG_NAME = "Collection";
		const string ELM_PREFIX_TAG_NAME = "Prefix";
		const string ELM_NEXT_DICT_NUMS_TAG_NAME = "NextDictNums";
		const string ELM_DOCUMENT_TITLE_TAG_NAME = "DocumentTitle";
		const string ELM_INVALID_TAG_NAME = "Invalid";
		const string ELM_QUARANTINED_TAG_NAME = "Quarantined";
		const string ELM_ALL_TAG_NAME = "All";
		const string ELM_ANNOTATION_TAG_NAME = "Annotation";
		const string ELM_ANY_TAG_NAME = "Any";
		const string ELM_ATTRIBUTE_GROUP_TAG_NAME = "AttributeGroup";
		const string ELM_CHOICE_TAG_NAME = "Choice";
		const string ELM_COMPLEX_CONTENT_TAG_NAME = "ComplexContent";
		const string ELM_COMPLEX_TYPE_TAG_NAME = "ComplexType";
		const string ELM_DOCUMENTATION_TAG_NAME = "Documentation";
		const string ELM_ENUMERATION_TAG_NAME = "enumeration";
		const string ELM_EXTENSION_TAG_NAME = "extension";
		const string ELM_DELETE_TAG_NAME = "Delete";
		const string ELM_BLOCK_CHAIN_TAG_NAME = "BlockChain";
		const string ELM_ENCDEF_TAG_NAME = "EncDef";
		const string ELM_SWEEP_TAG_NAME = "Sweep";

		const string ATTR_DICT_NUMBER_TAG_NAME = "DictNumber";
		const string ATTR_COLLECTION_NUMBER_TAG_NAME = "CollectionNumber";
		const string ATTR_COLLECTION_NAME_TAG_NAME = "CollectionName";
		const string ATTR_NAME_TAG_NAME = "name";
		const string ATTR_TARGET_NAMESPACE_TAG_NAME = "targetNameSpace";
		const string ATTR_TYPE_TAG_NAME = "type";
		const string ATTR_STATE_TAG_NAME = "State";
		const string ATTR_LANGUAGE_TAG_NAME = "Language";
		const string ATTR_INDEX_OPTIONS_TAG_NAME = "IndexOptions";
		const string ATTR_INDEX_ON_TAG_NAME = "IndexOn";
		const string ATTR_REQUIRED_TAG_NAME = "Required";
		const string ATTR_LIMIT_TAG_NAME = "Limit";
		const string ATTR_COMPARE_RULES_TAG_NAME = "CompareRules";
		const string ATTR_KEY_COMPONENT_TAG_NAME = "KeyComponent";
		const string ATTR_DATA_COMPONENT_TAG_NAME = "DataComponent";
		const string ATTR_LAST_DOC_INDEXED_TAG_NAME = "LastDocumentIndexed";
		const string ATTR_NEXT_ELEMENT_NUM_TAG_NAME = "NextElementNum";
		const string ATTR_NEXT_ATTRIBUTE_NUM_TAG_NAME = "NextAttributeNum";
		const string ATTR_NEXT_INDEX_NUM_TAG_NAME = "NextIndexNum";
		const string ATTR_NEXT_COLLECTION_NUM_TAG_NAME = "NextCollectionNum";
		const string ATTR_NEXT_PREFIX_NUM_TAG_NAME = "NextPrefixNum";
		const string ATTR_SOURCE_TAG_NAME = "Source";
		const string ATTR_STATE_CHANGE_COUNT_TAG_NAME = "StateChangeCount";
		const string ATTR_XMLNS_TAG_NAME = "xmlns";
		const string ATTR_ABSTRACT_TAG_NAME = "abstract";
		const string ATTR_BASE_TAG_NAME = "base";
		const string ATTR_BLOCK_TAG_NAME = "block";
		const string ATTR_DEFAULT_TAG_NAME = "default";
		const string ATTR_FINAL_TAG_NAME = "final";
		const string ATTR_FIXED_TAG_NAME = "fixed";
		const string ATTR_ITEM_TYPE_TAG_NAME = "itemtype";
		const string ATTR_MEMBER_TYPES_TAG_NAME = "membertypes";
		const string ATTR_MIXED_TAG_NAME = "mixed";
		const string ATTR_NILLABLE_TAG_NAME = "nillable";
		const string ATTR_REF_TAG_NAME = "ref";
		const string ATTR_USE_TAG_NAME = "use";
		const string ATTR_VALUE_TAG_NAME = "value";
		const string ATTR_ADDRESS_TAG_NAME = "address";
		const string ATTR_XMLNS_XFLAIM_TAG_NAME = "xmlns:xflaim";
		const string ATTR_ENCRYPTION_KEY_TAG_NAME = "Key";
		const string ATTR_TRANSACTION_TAG_NAME = "Transaction";
		const string ATTR_NEXT_ENCDEF_NUM_TAG_NAME = "NextEncDefNum";
		const string ATTR_ENCRYPTION_ID_TAG_NAME = "encId";
		const string ATTR_ENCRYPTION_KEY_SIZE_TAG_NAME = "keySize";
		const string ATTR_UNIQUE_SUB_ELEMENTS_TAG_NAME = "UniqueSubElements";

		//-----------------------------------------------------------------------------
		// constructor
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Db constructor.
		/// </summary>
		/// <param name="pDb">
		/// Reference to an IF_Db object.
		/// </param>
		/// <param name="dbSystem">
		/// DbSystem object that this Db object is associated with.
		/// </param>
		internal Db(
			ulong		pDb,
			DbSystem	dbSystem)
		{
			if (pDb == 0)
			{
				throw new XFlaimException( "Invalid IF_Db reference");
			}
			
			m_pDb = pDb;

			if (dbSystem == null)
			{
				throw new XFlaimException( "Invalid DbSystem reference");
			}
			
			m_dbSystem = dbSystem;
			
			// Must call something inside of DbSystem.  Otherwise, the
			// m_dbSystem object gets a compiler warning on linux because
			// it is not used anywhere.  Other than that, there is really
			// no need to make the following call.
			if (m_dbSystem.getDbSystem() == 0)
			{
				throw new XFlaimException( "Invalid DbSystem.IF_DbSystem object");
			}
		}

		//-----------------------------------------------------------------------------
		// destructor
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Destructor.
		/// </summary>
		~Db()
		{
			close();
		}

		//-----------------------------------------------------------------------------
		// getDb
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Return the pointer to the IF_Db object.
		/// </summary>
		/// <returns>Returns a pointer to the IF_Db object.</returns>
		internal ulong getDb()
		{
			return( m_pDb);
		}

		//-----------------------------------------------------------------------------
		// getDbSystem
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Return the DbSystem object associated with this Db
		/// </summary>
		/// <returns>Returns the DbSystem object associated with this Db</returns>
		internal DbSystem getDbSystem()
		{
			return m_dbSystem;
		}

		//-----------------------------------------------------------------------------
		// close
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Close this database.
		/// </summary>
		public void close()
		{
			// Release the native pDb!
		
			if (m_pDb != 0)
			{
				xflaim_Db_Release( m_pDb);
				m_pDb = 0;
			}
		
			// Remove our reference to the dbSystem so it can be released.
		
			m_dbSystem = null;
		}

		[DllImport("xflaim")]
		private static extern void xflaim_Db_Release(
			ulong	pDb);

		//-----------------------------------------------------------------------------
		// transBegin
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Starts a transaction.
		/// </summary>
		/// <param name="eTransType">
		/// The type of transaction (<see cref="eDbTransType"/>)
		/// </param>
		/// <param name="uiMaxLockWait">
		/// Specifies the amount of time to wait for lock requests occuring
		/// during the transaction to be granted.  Valid values are 0 through
		/// 255 seconds.  Zero is used to specify no-wait locks.  255 specifies
		/// that there is no timeout.
		/// </param>
		/// <param name="uiFlags">
		/// Should be a logical OR'd combination of the members of
		/// <see cref="DbTransFlags"/>
		/// </param>
		/// <returns></returns>
		public void transBegin(
			eDbTransType	eTransType,
			uint				uiMaxLockWait,
			DbTransFlags	uiFlags)
		{
			RCODE				rc;

			if ((rc = xflaim_Db_transBegin( m_pDb,
				eTransType, uiMaxLockWait, uiFlags)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_transBegin(
			ulong				pDb,
			eDbTransType	eTransType,
			uint				uiMaxLockWait,
			DbTransFlags	uiFlags);

		//-----------------------------------------------------------------------------
		// transBegin
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Starts a transaction.  Transaction will be of the same type and same
		/// snapshot as the passed in Db object.  The passed in Db object should
		/// be running a read transaction.
		/// </summary>
		/// <param name="pDb">
		/// Database whose transaction is to be copied.
		/// </param>
		/// <returns></returns>
		public void transBegin(
			ulong				pDb)
		{
			RCODE				rc;

			if( (rc = xflaim_Db_transBeginClone( m_pDb, pDb)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_transBeginClone(
			ulong				pDb,
			ulong				pDbToClone);

		//-----------------------------------------------------------------------------
		// transCommit
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Commits an active transaction.  If no transaction is running, or the
		/// transaction commit fails, an exception will be thrown.
		/// </summary>
		/// <returns></returns>
		public void transCommit()
		{
			RCODE				rc;

			if( (rc = xflaim_Db_transCommit( m_pDb)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}
		
		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_transCommit(
			ulong				pDb);

		//-----------------------------------------------------------------------------
		// transAbort
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Aborts an active transaction.
		/// </summary>
		/// <returns></returns>
		public void transAbort()
		{
			RCODE				rc;

			if( (rc = xflaim_Db_transAbort( m_pDb)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}
		
		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_transAbort(
			ulong				pDb);

		//-----------------------------------------------------------------------------
		// getTransType
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Get the current transaction type.
		/// </summary>
		/// <returns><see cref="eDbTransType"/></returns>
		public eDbTransType getTransType()
		{
			return( xflaim_Db_getTransType( m_pDb));
		}

		[DllImport("xflaim")]
		private static extern eDbTransType xflaim_Db_getTransType(
			ulong				pDb);

		//-----------------------------------------------------------------------------
		// doCheckpoint
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Perform a checkpoint on the database.
		/// </summary>
		/// <param name="uiTimeout">
		/// Specifies the amount of time to wait for database lock.  
		/// Valid values are 0 through 255 seconds.  Zero is used to specify no-wait
		/// locks. 255 is used to specify that there is no timeout.
		/// </param>
		/// <returns></returns>
		public void doCheckpoint(
			uint				uiTimeout)
		{
			RCODE				rc;

			if( (rc = xflaim_Db_doCheckpoint( m_pDb, uiTimeout)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_doCheckpoint(
			ulong				pDb,
			uint				uiTimeout);

		//-----------------------------------------------------------------------------
		// dbLock
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Lock the database. 
		/// </summary>
		/// <param name="eLckType">
		/// Type of lock being requested.
		/// </param>
		/// <param name="iPriority">
		/// Priority of lock being requested.
		/// </param>
		/// <param name="uiTimeout">
		/// Lock wait time.  Specifies the amount of time to wait for 
		/// database lock.  Valid values are 0 through 255 seconds.
		/// Zero is used to specify no-wait locks. 255 is used to specify
		/// that there is no timeout.
		/// </param>
		/// <returns></returns>
		public void dbLock(
			eLockType		eLckType,
			int				iPriority,
			uint				uiTimeout)
		{
			RCODE				rc;

			if( (rc = xflaim_Db_dbLock( m_pDb, eLckType, 
				iPriority, uiTimeout)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_dbLock(
			ulong				pDb,
			eLockType		eLckType,
			int				iPriority,
			uint				uiTimeout);
	
		//-----------------------------------------------------------------------------
		// dbUnlock
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Unlocks the database.
		/// </summary>
		/// <returns></returns>
		public void dbUnlock()
		{
			RCODE			rc;

			if ((rc = xflaim_Db_dbUnlock( m_pDb)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_dbUnlock(
			ulong				pDb);

		//-----------------------------------------------------------------------------
		// getLockType
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Get the type of database lock current held.
		/// </summary>
		/// <returns></returns>
		public eLockType getLockType()
		{
			return( xflaim_Db_getLockType( m_pDb));
		}

		[DllImport("xflaim")]
		private static extern eLockType xflaim_Db_getLockType(
			ulong				pDb);

		//-----------------------------------------------------------------------------
		// getLockImplicit
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Determine if the database lock was implicitly obtained 
		/// (i.e., obtained when transBegin was called as opposed to dbLock).
		/// </summary>
		/// <returns></returns>
		public bool getLockImplicit()
		{
			return( xflaim_Db_getLockImplicit( m_pDb));
		}

		[DllImport("xflaim")]
		private static extern bool xflaim_Db_getLockImplicit(
			ulong				pDb);

		//-----------------------------------------------------------------------------
		// getLockThreadId
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Returns the thread id of the thread that currently holds the 
		/// database lock.
		/// </summary>
		/// <returns></returns>
		public uint getLockThreadId()
		{
			return( xflaim_Db_getLockThreadId( m_pDb));
		}

		[DllImport("xflaim")]
		private static extern uint xflaim_Db_getLockThreadId(
			ulong				pDb);

		//-----------------------------------------------------------------------------
		//	getLockNumExclQueued
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Returns the number of threads that are currently waiting to obtain
		/// an exclusive database lock.
		/// </summary>
		/// <returns></returns>
		public uint getLockNumExclQueued()
		{
			return( xflaim_Db_getLockNumExclQueued( m_pDb));
		}

		[DllImport("xflaim")]
		private static extern uint xflaim_Db_getLockNumExclQueued(
			ulong				pDb);

		//-----------------------------------------------------------------------------
		//	getLockNumSharedQueued
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Returns the number of threads that are currently waiting to obtain
		/// a shared database lock.
		/// </summary>
		/// <returns></returns>
		public uint getLockNumSharedQueued()
		{
			return (xflaim_Db_getLockNumSharedQueued(m_pDb));
		}

		[DllImport("xflaim")]
		private static extern uint xflaim_Db_getLockNumSharedQueued(
			ulong				pDb);
	
		//-----------------------------------------------------------------------------
		// getLockPriorityCount
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Returns the number of threads that are currently waiting to obtain
		/// a database lock whose priority is >= iPriority.
		/// </summary>
		/// <param name="iPriority">
		/// Priority to look for - a count of all waiting threads with a
		/// lock priority greater than or equal to this will be returned.
		/// </param>
		/// <returns>Returns number of threads waiting for a database lock whose
		/// priority is >= iPriority.</returns>
		public uint getLockPriorityCount(
			int			iPriority)
		{
			return( xflaim_Db_getLockPriorityCount( m_pDb, iPriority));
		}

		[DllImport("xflaim")]
		private static extern uint xflaim_Db_getLockPriorityCount(
			ulong			pDb,
			int			iPriority);

		//-----------------------------------------------------------------------------
		// indexSuspend
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Suspend indexing on the specified index.
		/// </summary>
		/// <param name="uiIndex">
		/// Index to be suspended.
		/// </param>
		/// <returns></returns>
		public void indexSuspend(
			uint			uiIndex)
		{
			RCODE			rc;

			if( (rc = xflaim_Db_indexSuspend( m_pDb, uiIndex)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_indexSuspend(
			ulong			pDb,
			uint			uiIndex);
	
		//-----------------------------------------------------------------------------
		// indexResume
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Resume indexing on the specified index.
		/// </summary>
		/// <param name="uiIndex">
		/// Index to be resumed.
		/// </param>
		/// <returns></returns>
		public void indexResume(
			uint			uiIndex)
		{
			RCODE			rc;

			if( (rc = xflaim_Db_indexResume( m_pDb, uiIndex)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_indexResume(
			ulong			pDb,
			uint			uiIndex);
	
		//-----------------------------------------------------------------------------
		// indexGetNext
		//-----------------------------------------------------------------------------

		/// <summary>
		/// This method provides a way to iterate through all of the indexes in the
		/// database.  It returns the index ID of the index that comes after the
		/// passed in index number.  The first index can be obtained by passing in a
		/// zero.
		/// </summary>
		/// <param name="uiCurrIndex">
		/// Current index number.  Index that comes after this one
		/// will be returned.	
		/// </param>
		/// <returns>
		/// Returns the index ID of the index that comes after uiCurrIndex.
		/// </returns>
		public uint indexGetNext(
			uint			uiCurrIndex)
		{
			RCODE			rc;
			uint			uiNextIndex = 0;

			if( (rc = xflaim_Db_indexGetNext( m_pDb, 
				uiCurrIndex, out uiNextIndex)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return( uiNextIndex);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_indexGetNext(
			ulong			pDb,
			uint			uiCurrIndex,
			out uint		uiNextIndex);

		//-----------------------------------------------------------------------------
		//	indexStatus
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Retrieves index status information
		/// </summary>
		/// <param name="uiIndex">
		/// Index whose status is to be returned
		/// </param>
		/// <returns>An instance of a <see cref="XFLM_INDEX_STATUS"/> object.</returns>
		public XFLM_INDEX_STATUS indexStatus(
			uint						uiIndex)
		{
			RCODE						rc;
			XFLM_INDEX_STATUS		pIndexStatus = null;

			pIndexStatus = new XFLM_INDEX_STATUS();

			if( (rc = xflaim_Db_indexStatus( m_pDb, 
				uiIndex, out pIndexStatus)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return( pIndexStatus);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_indexStatus(
			ulong							pDb,
			uint							uiCurrIndex,
			out XFLM_INDEX_STATUS	pIndexStatus);

		//-----------------------------------------------------------------------------
		// reduceSize
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Return unused blocks back to the file system.
		/// </summary>
		/// <param name="uiCount">
		/// Maximum number of blocks to be returned.
		/// </param>
		/// <returns>
		/// Returns the number of blocks that were actually returned to the
		/// file system.
		/// </returns>
		public uint reduceSize(
			uint			uiCount)
		{
			RCODE			rc;
			uint			uiNumReduced = 0;

			if( (rc = xflaim_Db_reduceSize( m_pDb, 
				uiCount, out uiNumReduced)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return( uiNumReduced);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_reduceSize(
			ulong			pDb,
			uint			uiCount,
			out uint		uiNumReduced);

		//-----------------------------------------------------------------------------
		// keyRetrieve
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Lookup/retrieve keys in an index. 
		/// </summary>
		/// <param name="uiIndex">
		/// The index that is being searched.
		/// </param>
		/// <param name="searchKey">
		/// The "from" key use for the search.
		/// </param>
		/// <param name="retrieveFlags">
		/// Search flags <see cref="RetrieveFlags"/>.
		/// </param>
		/// <param name="foundKey">
		/// Found key.
		/// </param>
		public void keyRetrieve(
			uint					uiIndex,
			DataVector			searchKey,
			RetrieveFlags		retrieveFlags,
			DataVector			foundKey)
		{
			RCODE rc;
			ulong pSearchKey = (searchKey == null ? 0 : searchKey.getDataVector());
			ulong	pFoundKey;

			if (foundKey == null)
			{
				foundKey = m_dbSystem.createDataVector();
			}

			pFoundKey = foundKey.getDataVector();

			if( (rc = xflaim_Db_keyRetrieve( m_pDb,
				uiIndex, pSearchKey, retrieveFlags, pFoundKey)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_keyRetrieve(
			ulong					pDb,
			uint					uiIndex,
			ulong					pSearchKey,
			RetrieveFlags		retrieveFlags,
			ulong					pFoundKey);

		//-----------------------------------------------------------------------------
		// createDocument
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Creates a new document node. 
		/// </summary>
		/// <param name="uiCollection">
		/// The collection to store the new document in.
		/// </param>
		/// <returns>
		/// An instance of a <see cref="DOMNode"/> object.
		/// </returns>
		 public DOMNode createDocument(
	 		uint			uiCollection)
		{
			RCODE rc;
			ulong pNewNode;

			if ((rc = xflaim_Db_createDocument(m_pDb, uiCollection, 
				out pNewNode)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (new DOMNode( pNewNode, this));
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_createDocument(
			ulong pDb,
			uint uiCollection,
			out ulong pNewNode);

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Creates a new root element node.  This is the root node of a document
		/// in the XFLAIM database.
		/// </summary>
		/// <param name="uiCollection">
		/// The collection to store the new node in.
		/// </param>
		/// <param name="uiElementNameId">
		/// Name of the element to be created.
		/// </param>
		/// <returns>
		/// An instance of a <see cref="DOMNode"/> object.
		/// </returns>
		public DOMNode createRootElement(
			uint		uiCollection,
			uint		uiElementNameId)
		{
			RCODE rc;
			ulong pNewNode;

			if ((rc = xflaim_Db_createRootElement(m_pDb, uiCollection,
				uiElementNameId, out pNewNode)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (new DOMNode(pNewNode, this));
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_createRootElement(
			ulong pDb,
			uint uiCollection,
			uint uiElementNameId,
			out ulong pNewNode);

		//-----------------------------------------------------------------------------
		// getFirstDocument
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Retrieve the first document in a specified collection. 
		/// </summary>
		/// <param name="uiCollection">
		/// The collection from which to retrieve the first document
		/// </param>
		/// <param name="nodeToReuse">
		/// An existing DOM node object can optionally be passed in.  It will
		/// be reused rather than allocating a new object.
		/// </param>
		/// <returns>
		/// Returns the root <see cref="DOMNode"/> of the document.
		/// </returns>
		public DOMNode getFirstDocument(
			uint			uiCollection,
			DOMNode		nodeToReuse)
		 {
			RCODE			rc;
			ulong			pNode = 0;
			
			if( nodeToReuse != null)
			{
				pNode = nodeToReuse.getNode();
			}
			
			if( (rc = xflaim_Db_getFirstDocument( m_pDb, uiCollection,
				ref pNode)) != 0)
			{
				throw new XFlaimException(rc);
			}

			if( nodeToReuse != null)
			{
				nodeToReuse.setNodePtr( pNode, this);
				return (nodeToReuse);
			}

			return( new DOMNode( pNode, this));
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getFirstDocument(
			ulong pDb,
			uint uiCollection,
			ref ulong pNode);

		//-----------------------------------------------------------------------------
		// getLastDocument
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Retrieves the last document in a specified collection. 
		/// </summary>
		/// <param name="uiCollection">
		/// The collection from which to retrieve the document
		/// </param>
		/// <param name="nodeToReuse">
		/// An existing DOM node object can optionally be passed in.  It will
		/// be reused rather than allocating a new object.
		/// </param>
		/// <returns>
		/// Returns the root <see cref="DOMNode"/> of the document.
		/// </returns>
		public DOMNode getLastDocument(
			uint uiCollection,
			DOMNode nodeToReuse)
		{
			RCODE rc;
			ulong pNode = 0;

			if (nodeToReuse != null)
			{
				pNode = nodeToReuse.getNode();
			}

			if ((rc = xflaim_Db_getLastDocument(m_pDb, uiCollection,
				ref pNode)) != 0)
			{
				throw new XFlaimException(rc);
			}

			if (nodeToReuse != null)
			{
				nodeToReuse.setNodePtr(pNode, this);
				return (nodeToReuse);
			}

			return (new DOMNode(pNode, this));
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getLastDocument(
			ulong pDb,
			uint uiCollection,
			ref ulong pNewNode);
 
		//-----------------------------------------------------------------------------
		// getDocument
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Retrieves a document from the specified collection. 
		/// </summary>
		/// <param name="uiCollection">
		/// The collection from which to retrieve the document
		/// </param>
		/// <param name="retrieveFlags">
		/// Search flags <see cref="RetrieveFlags"/>.
		/// </param>
		/// <param name="ulDocumentId">
		/// Document to retrieve.
		/// </param>
		/// <param name="nodeToReuse">
		/// An existing DOM node object can optionally be passed in.  It will
		/// be reused rather than allocating a new object.
		/// </param>
		/// <returns>
		/// Returns the root <see cref="DOMNode"/> of the document.
		/// </returns>
		public DOMNode getDocument(
			uint uiCollection,
			RetrieveFlags retrieveFlags,
			ulong ulDocumentId,
			DOMNode nodeToReuse)
		{
			RCODE rc;
			ulong pNode = 0;

			if (nodeToReuse != null)
			{
				pNode = nodeToReuse.getNode();
			}

			if ((rc = xflaim_Db_getDocument(m_pDb, uiCollection,
				retrieveFlags, ulDocumentId, ref pNode)) != 0)
			{
				throw new XFlaimException(rc);
			}

			if (nodeToReuse != null)
			{
				nodeToReuse.setNodePtr(pNode, this);
				return (nodeToReuse);
			}

			return (new DOMNode(pNode, this));
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getDocument(
			ulong pDb,
			uint uiCollection,
			RetrieveFlags retrieveFlags,
			ulong ulDocumentId,
			ref ulong pNode);

		//-----------------------------------------------------------------------------
		//	documentDone
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Indicate that modifications to a document are "done".  This allows
		/// XFLAIM to process the document as needed.
		/// </summary>
		/// <param name="uiCollection">
		/// The document's collection ID.
		/// </param>
		/// <param name="ulDocumentId">
		/// The document ID.
		/// </param>
		public void documentDone(
			uint			uiCollection,
			ulong			ulDocumentId)
		{
			RCODE rc;

			if ((rc = xflaim_Db_documentDone(m_pDb, uiCollection, ulDocumentId)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_documentDone(
			ulong pDb,
			uint uiCollection,
			ulong ulDocumentId);

		//-----------------------------------------------------------------------------
		//	documentDone
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Indicate that modifications to a document are "done".  This allows
		/// XFLAIM to process the document as needed.
		/// </summary>
		/// <param name="domNode">
		/// Root node of the document that the application has finished 
		/// modifying
		/// </param>
		public void documentDone(
			DOMNode		domNode)
		{
			RCODE rc;

			if ((rc = xflaim_Db_documentDone(m_pDb, domNode.getNode())) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_documentDone(
			ulong pDb,
			ulong  pDOMNode);

		//-----------------------------------------------------------------------------
		// createElementDef
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Creates a new element definition in the dictionary. 
		/// </summary>
		/// <param name="sNamespaceURI">
		/// The namespace URI that this definition should be
		/// created in.  If null, the default namespace will be used.
		/// </param>
		/// <param name="sElementName">
		/// The name of the element.
		/// </param>
		/// <param name="dataType">
		/// The data type for instances of this element.
		/// </param> 
		/// <param name="uiRequestedId">
		/// if non-zero, then xflaim will try to use this
		/// number as the name ID of the new definition.
		/// </param>
		/// <returns>
		/// Returns the name ID of the new definition.
		/// </returns>
		public uint createElementDef(
			string		sNamespaceURI,
			string		sElementName,
			FlmDataType	dataType,
			uint			uiRequestedId)
			
		{
			RCODE			rc;
			uint			uiNewId;

			if( (rc = xflaim_Db_createElementDef(m_pDb, sNamespaceURI,
				sElementName, dataType, uiRequestedId, out uiNewId)) != 0)
			{
				throw new XFlaimException(rc);
			}
			
			return( uiNewId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_createElementDef(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sNamespaceURI,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sElementName,
			FlmDataType dataType,
			uint uiRequestedId,
			out uint uiNewId);

		//-----------------------------------------------------------------------------
		// createUniqueElmDef
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Create a "unique" element definition - i.e., an element definition whose
		/// child elements must all be unique.
		/// </summary>
		/// <param name="sNamespaceURI">
		/// The namespace URI that this definition should be
		/// created in.  If null, the default namespace will be used.
		/// </param>
		/// <param name="sElementName">
		/// The name of the element.
		/// </param>
		/// <param name="uiRequestedId">
		/// if non-zero, then xflaim will try to use this
		/// number as the name ID of the new definition.
		/// </param>
		/// <returns>
		/// Returns the name ID of the new definition.
		/// </returns>
		public uint createUniqueElmDef(
			string		sNamespaceURI,
			string		sElementName,
			uint			uiRequestedId)
			
		{
			RCODE rc;
			uint uiNewId;

			if ((rc = xflaim_Db_createUniqueElmDef(m_pDb, sNamespaceURI,
				sElementName, uiRequestedId, out uiNewId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNewId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_createUniqueElmDef(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sNamespaceURI,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sElementName,
			uint uiRequestedId,
			out uint uiNewId);

		//-----------------------------------------------------------------------------
		// getElementNameId
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the name id for a particular element name.
		/// </summary>
		/// <param name="sNamespaceURI">
		/// The namespace URI for the element.
		/// </param>
		/// <param name="sElementName">
		/// The name of the element.
		/// </param>
		/// <returns>
		/// Returns the name ID of the element.
		/// </returns>
		public uint getElementNameId(
			string		sNamespaceURI,
			string		sElementName)
		{
			RCODE rc;
			uint uiNameId;

			if ((rc = xflaim_Db_getElementNameId(m_pDb, sNamespaceURI,
				sElementName, out uiNameId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNameId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getElementNameId(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sNamespaceURI,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sElementName,
			out uint uiNameId);

		//-----------------------------------------------------------------------------
		// createAttributeDef
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Creates a new attribute definition in the dictionary. 
		/// </summary>
		/// <param name="sNamespaceURI">
		/// The namespace URI that this definition should be
		/// created in.  If null, the default namespace will be used.
		/// </param>
		/// <param name="sAttributeName">
		/// The name of the attribute.
		/// </param>
		/// <param name="dataType">
		/// The data type for instances of this attribute.
		/// </param> 
		/// <param name="uiRequestedId">
		/// if non-zero, then XFLAIM will try to use this
		/// number as the name ID of the new definition.
		/// </param>
		/// <returns>
		/// Returns the name ID of the new definition.
		/// </returns>
		public uint createAttributeDef(
			string		sNamespaceURI,
			string		sAttributeName,
			FlmDataType	dataType,
			uint			uiRequestedId)
			
		{
			RCODE			rc;
			uint			uiNewId;

			if( (rc = xflaim_Db_createAttributeDef(m_pDb, sNamespaceURI,
				sAttributeName, dataType, uiRequestedId, out uiNewId)) != 0)
			{
				throw new XFlaimException(rc);
			}
			
			return( uiNewId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_createAttributeDef(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sNamespaceURI,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sAttributeName,
			FlmDataType dataType,
			uint uiRequestedId,
			out uint uiNewId);

		//-----------------------------------------------------------------------------
		// getAttributeNameId
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the name id for a particular attribute.
		/// </summary>
		/// <param name="sNamespaceURI">
		/// The namespace URI of the attribute.
		/// </param>
		/// <param name="sAttributeName">
		/// The name of the attribute.
		/// </param>
		/// <returns>
		/// Returns the name ID of the attribute.
		/// </returns>
		public uint getAttributeNameId(
			string sNamespaceURI,
			string sAttributeName)
		{
			RCODE rc;
			uint uiNameId;

			if ((rc = xflaim_Db_getAttributeNameId(m_pDb, sNamespaceURI,
				sAttributeName, out uiNameId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNameId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getAttributeNameId(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sNamespaceURI,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sAttributeName,
			out uint uiNameId);

		//-----------------------------------------------------------------------------
		// createPrefixDef
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Creates a new prefix definition in the dictionary. 
		/// </summary>
		/// <param name="sPrefixName">
		/// The name of the attribute.
		/// </param>
		/// <param name="uiRequestedId">
		/// if non-zero, then XFLAIM will try to use this
		/// number as the name ID of the new definition.
		/// </param>
		/// <returns>
		/// Returns the name ID of the new definition.
		/// </returns>
		public uint createPrefixDef(
			string sPrefixName,
			uint uiRequestedId)
		{
			RCODE rc;
			uint uiNewId;

			if ((rc = xflaim_Db_createPrefixDef(m_pDb, sPrefixName,
				uiRequestedId, out uiNewId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNewId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_createPrefixDef(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sPrefixName,
			uint uiRequestedId,
			out uint uiNewId);

		//-----------------------------------------------------------------------------
		// getPrefixId
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the name id for a particular prefix.
		/// </summary>
		/// <param name="sPrefixName">
		/// The name of the prefix.
		/// </param>
		/// <returns>
		/// Returns the name ID of the prefix.
		/// </returns>
		public uint getPrefixId(
			string sPrefixName)
		{
			RCODE rc;
			uint uiNameId;

			if ((rc = xflaim_Db_getPrefixId(m_pDb, sPrefixName,
				out uiNameId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNameId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getPrefixId(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sPrefixName,
			out uint uiNameId);

		//-----------------------------------------------------------------------------
		// createEncDef
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Creates a new prefix definition in the dictionary. 
		/// </summary>
		/// <param name="sEncType">
		/// Encryption type.
		/// </param>
		/// <param name="sEncName">
		/// Encryption definition name.
		/// </param>
		/// <param name="uiKeySize">
		/// Size of the encryption key.
		/// </param>
		/// <param name="uiRequestedId">
		/// if non-zero, then XFLAIM will try to use this
		/// number as the name ID of the new definition.
		/// </param>
		/// <returns>
		/// Returns the name ID of the new definition.
		/// </returns>
		public uint createEncDef(
			string sEncType,
			string sEncName,
			uint uiKeySize,
			uint uiRequestedId)
		{
			RCODE rc;
			uint uiNewId;

			if ((rc = xflaim_Db_createEncDef(m_pDb, sEncType, sEncName,
				uiKeySize, uiRequestedId, out uiNewId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNewId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_createEncDef(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sEncType,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sEncName,
			uint uiKeySize,
			uint uiRequestedId,
			out uint uiNewId);

		//-----------------------------------------------------------------------------
		// getEncDefId
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the ID for a particular encryption definition.
		/// </summary>
		/// <param name="sEncName">
		/// The name of the encryption definition.
		/// </param>
		/// <returns>
		/// Returns the name ID of the encryption definition.
		/// </returns>
		public uint getEncDefId(
			string sEncName)
		{
			RCODE rc;
			uint uiNameId;

			if ((rc = xflaim_Db_getEncDefId(m_pDb, sEncName,
				out uiNameId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNameId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getEncDefId(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sEncName,
			out uint uiNameId);

		//-----------------------------------------------------------------------------
		// createCollectionDef
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Creates a collection definition in the dictionary. 
		/// </summary>
		/// <param name="sCollectionName">
		/// The name of the collection.
		/// </param>
		/// <param name="uiEncryptionId">
		/// ID of the encryption definition that should be used
		/// to encrypt this collection.  Zero means the collection will not be encrypted.
		/// </param>
		/// <param name="uiRequestedId">
		/// if non-zero, then XFLAIM will try to use this
		/// number as the name ID of the new definition.
		/// </param>
		/// <returns>
		/// Returns the name ID of the new definition.
		/// </returns>
		public uint createCollectionDef(
			string sCollectionName,
			uint uiEncryptionId,
			uint uiRequestedId)
		{
			RCODE rc;
			uint uiNewId;

			if ((rc = xflaim_Db_createCollectionDef(m_pDb, sCollectionName,
				uiEncryptionId, uiRequestedId, out uiNewId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNewId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_createCollectionDef(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sCollectionName,
			uint uiEncryptionId,
			uint uiRequestedId,
			out uint uiNewId);

		//-----------------------------------------------------------------------------
		// getCollectionNumber
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the ID for a particular collection.
		/// </summary>
		/// <param name="sCollectionName">
		/// The name of the collection.
		/// </param>
		/// <returns>
		/// Returns the ID of the collection definition.
		/// </returns>
		public uint getCollectionNumber(
			string sCollectionName)
		{
			RCODE rc;
			uint uiNameId;

			if ((rc = xflaim_Db_getCollectionNumber(m_pDb, sCollectionName,
				out uiNameId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNameId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getCollectionNumber(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sCollectionName,
			out uint uiNameId);

		//-----------------------------------------------------------------------------
		// getIndexNumber
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the ID for a particular index.
		/// </summary>
		/// <param name="sIndexName">
		/// The name of the index.
		/// </param>
		/// <returns>
		/// Returns the ID of the index definition.
		/// </returns>
		public uint getIndexNumber(
			string sIndexName)
		{
			RCODE rc;
			uint uiNameId;

			if ((rc = xflaim_Db_getIndexNumber(m_pDb, sIndexName,
				out uiNameId)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiNameId);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getIndexNumber(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sIndexName,
			out uint uiNameId);
	
		//-----------------------------------------------------------------------------
		// getDictionaryDef
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Retrieve a dictionary definition document.
		/// </summary>
		/// <param name="dictType">
		/// The type of dictionary definition being retrieved.
		/// </param>
		/// <param name="uiDictNumber">
		/// The number the dictionary definition being retrieved.
		/// </param>
		/// <param name="nodeToReuse">
		/// An existing DOM node object can optionally be passed in.  It will
		/// be reused rather than allocating a new object.
		/// </param>
		/// <returns>
		/// Returns the root <see cref="DOMNode"/> of the document.
		/// </returns>
		public DOMNode getDictionaryDef(
			ReservedElmTag dictType,
			uint uiDictNumber,
			DOMNode nodeToReuse)
		{
			RCODE rc;
			ulong pNode = 0;

			if (nodeToReuse != null)
			{
				pNode = nodeToReuse.getNode();
			}

			if ((rc = xflaim_Db_getDictionaryDef(m_pDb, dictType,
				uiDictNumber, ref pNode)) != 0)
			{
				throw new XFlaimException(rc);
			}

			if (nodeToReuse != null)
			{
				nodeToReuse.setNodePtr(pNode, this);
				return (nodeToReuse);
			}

			return (new DOMNode(pNode, this));
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getDictionaryDef(
			ulong pDb,
			ReservedElmTag dictType,
			ulong uiDictNumber,
			ref ulong pNode);

//-----------------------------------------------------------------------------
// getDictionaryName
//-----------------------------------------------------------------------------

	/// <summary>
	/// Get a dictionary definition's name.
	/// </summary>
	/// <param name="dictType">
	/// The type of dictionary definition whose name is to be returned.
	/// </param>
	/// <param name="uiDictNumber">
	/// The number of the dictionary definition.
	/// </param>
	/// <returns>
	/// Name of the dictionary item.
	/// </returns>
 	public string getDictionaryName(
		ReservedElmTag dictType,
		uint uiDictNumber)
	{
		RCODE		rc;
		IntPtr	pszDictName;
		string	sDictName;

		if ((rc = xflaim_Db_getDictionaryName( m_pDb, dictType, 
			uiDictNumber, out pszDictName)) != 0)
		{
			throw new XFlaimException( rc);
		}

		sDictName = Marshal.PtrToStringAnsi( pszDictName);
		m_dbSystem.freeUnmanagedMem( pszDictName);
		return( sDictName);
	}

	[DllImport("xflaim")]
	private static extern RCODE xflaim_Db_getDictionaryName(
		ulong pDb,
		ReservedElmTag dictType,
		uint uiDictNumber,
		out IntPtr psTempDir);

//-----------------------------------------------------------------------------
// getElementNamespace
//-----------------------------------------------------------------------------

	/// <summary>
	/// Get an element definition's namespace.
	/// </summary>
	/// <param name="uiDictNumber">
	/// The number of the element definition.
	/// </param>
	/// <returns>
	/// Returns the namespace for the element definition.
	/// </returns>
	public string getElementNamespace(
		uint		uiDictNumber)
	{
		RCODE rc;
		IntPtr pszElmNamespace;
		string sElmNamespace;

		if ((rc = xflaim_Db_getElementNamespace(m_pDb,
			uiDictNumber, out pszElmNamespace)) != 0)
		{
			throw new XFlaimException(rc);
		}

		sElmNamespace = Marshal.PtrToStringAnsi(pszElmNamespace);
		m_dbSystem.freeUnmanagedMem(pszElmNamespace);
		return (sElmNamespace);
	}

	[DllImport("xflaim")]
	private static extern RCODE xflaim_Db_getElementNamespace(
		ulong pDb,
		uint uiDictNumber,
		out IntPtr pszElmNamespace);

//-----------------------------------------------------------------------------
// getAttributeNamespace
//-----------------------------------------------------------------------------

	/// <summary>
	/// Get an attribute definition's namespace.
	/// </summary>
	/// <param name="uiDictNumber">
	/// The number of the attribute definition.
	/// </param>
	/// <returns>
	/// Returns the namespace for the attribute definition.
	/// </returns>
	public string getAttributeNamespace(
		uint	uiDictNumber)
	{
		RCODE rc;
		IntPtr pszAttrNamespace;
		string sAttrNamespace;

		if ((rc = xflaim_Db_getAttributeNamespace(m_pDb,
			uiDictNumber, out pszAttrNamespace)) != 0)
		{
			throw new XFlaimException(rc);
		}

		sAttrNamespace = Marshal.PtrToStringAnsi(pszAttrNamespace);
		m_dbSystem.freeUnmanagedMem(pszAttrNamespace);
		return (sAttrNamespace);
	}

	[DllImport("xflaim")]
	private static extern RCODE xflaim_Db_getAttributeNamespace(
		ulong pDb,
		uint uiDictNumber,
		out IntPtr pszAttrNamespace);

//-----------------------------------------------------------------------------
// getNode
//-----------------------------------------------------------------------------

	/// <summary>
	/// Retrieves the specified node from the specified collection. 
	/// </summary>
	/// <param name="uiCollection">
	/// The collection where the node is stored.
	/// </param>
	/// <param name="ulNodeId">
	/// The ID number of the node to be retrieved.
	/// </param>
	/// <param name="nodeToReuse">
	/// </param>
	/// <returns></returns>
	public DOMNode getNode(
		uint			uiCollection,
		ulong			ulNodeId,
		DOMNode		nodeToReuse)
	{
		RCODE rc;
		ulong pNode = 0;

		if (nodeToReuse != null)
		{
			pNode = nodeToReuse.getNode();
		}

		if ((rc = xflaim_Db_getNode(m_pDb, uiCollection, ulNodeId,
			ref pNode)) != 0)
		{
			throw new XFlaimException(rc);
		}

		if (nodeToReuse != null)
		{
			nodeToReuse.setNodePtr(pNode, this);
			return (nodeToReuse);
		}

		return (new DOMNode(pNode, this));
	}

	[DllImport("xflaim")]
	private static extern RCODE xflaim_Db_getNode(
		ulong pDb,
		uint uiCollection,
		ulong ulNodeId,
		ref ulong pNode);

//-----------------------------------------------------------------------------
// getAttribute
//-----------------------------------------------------------------------------

	/// <summary>
	/// Retrieves the specified attribute node from the specified collection.
	/// </summary>
	/// <param name="uiCollection">
	/// The collection where the attribute is stored.
	/// </param>
	/// <param name="ulElementNodeId">
	/// The ID number of the element node that contains the attribute 
	/// to be retrieved.
	/// </param>
	/// <param name="uiAttrNameId">
	/// The attribute id of the attribute to be retrieved.
	/// </param>
	/// <param name="nodeToReuse">
	/// An existing DOM node object can optionally be passed in.  It will
	/// be reused rather than allocating a new object.
	/// </param>
	/// <returns>
	/// Returns the attribute node <see cref="DOMNode"/>.
	/// </returns>
	public DOMNode getAttribute(
		uint			uiCollection,
		ulong			ulElementNodeId,
		uint			uiAttrNameId,
		DOMNode		nodeToReuse)
	{
		RCODE rc;
		ulong pNode = 0;

		if (nodeToReuse != null)
		{
			pNode = nodeToReuse.getNode();
		}

		if ((rc = xflaim_Db_getAttribute(m_pDb, uiCollection, 
			ulElementNodeId, uiAttrNameId, ref pNode)) != 0)
		{
			throw new XFlaimException(rc);
		}

		if (nodeToReuse != null)
		{
			nodeToReuse.setNodePtr(pNode, this);
			return (nodeToReuse);
		}

		return (new DOMNode(pNode, this));
	}

	[DllImport("xflaim")]
	private static extern RCODE xflaim_Db_getAttribute(
		ulong pDb,
		uint uiCollection,
		ulong ulElementNodeId,
		uint uiAttrNameId,
		ref ulong pNode);

//-----------------------------------------------------------------------------
// getDataType
//-----------------------------------------------------------------------------

	/// <summary>
	/// Returns the data type that was specified for a particular dictionary
	/// definition.  NOTE: This really only applies to element and attribute
	/// definitions.
	/// </summary>
	/// <param name="dictType">
	/// The type of dictionary definition whose data type is to be returned.
	/// </param>
	/// <param name="uiDictNumber">
	/// The number of the dictionary definition.
	/// </param>
	/// <returns>
	/// Data type of the dictionary object.
	/// </returns>
	public FlmDataType getDataType(
		ReservedElmTag dictType,
		uint uiDictNumber)
	{
		RCODE rc;
		FlmDataType dataType;

		if ((rc = xflaim_Db_getDataType( m_pDb,
			dictType, uiDictNumber, out dataType)) != 0)
		{
			throw new XFlaimException(rc);
		}

		return (dataType);
	}

	[DllImport("xflaim")]
	private static extern RCODE xflaim_Db_getDataType(
		ulong pDb,
		ReservedElmTag dictType,
		uint uiDictNumer,
		out FlmDataType dataType);

//-----------------------------------------------------------------------------
// backupBegin
//-----------------------------------------------------------------------------

		/// <summary>
		/// Sets up a backup operation.
		/// </summary>
		/// <param name="bFullBackup">
		/// Specifies whether the backup is to be a full backup (true) or an 
		/// incremental backup (false).
		/// </param>
		/// <param name="bLockDb">
		/// Specifies whether the database should be locked during the back (a "warm" backup)
		/// or unlocked (a "hot" backup).
		/// </param>
		/// <param name="uiMaxLockWait">
		/// This parameter is only used if the bLockDb parameter is true.  
		/// It specifies the maximum number of seconds to wait to obtain a lock.
		/// </param>
		/// <returns>
		/// If successful, this method returns a <see cref="Backup"/> object which can then be used
		/// to perform the backup operation.  The database will be locked if bLockDb was specified.
		/// Otherwise, a read transaction will have been started to perform the backup.
		/// </returns>
		public Backup backupBegin(
			bool			bFullBackup,
			bool			bLockDb,
			uint			uiMaxLockWait)
		{
			RCODE			rc;
			ulong			pBackup;

			if( (rc = xflaim_Db_backupBegin( m_pDb, (bFullBackup ? 1 : 0),
				(bLockDb ? 1 : 0), uiMaxLockWait, out pBackup)) != 0)
			{
				throw new XFlaimException( rc);
			}

			return( new Backup( pBackup, this));
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_backupBegin(
			ulong			pDb,
			int			bFullBackup,
			int			bLockDb,
			uint			uiMaxLockWait,
			out ulong	ulBackupRef);

		//-----------------------------------------------------------------------------
		// importDocument
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Imports an XML document into the XFlaim database.  The import requires
		/// an update transaction.
		/// </summary>
		/// <param name="istream">
		/// Input stream containing the document(s) to be imported
		/// </param>
		/// <param name="uiCollection">
		/// Destination collection for imported document(s).
		/// </param>
		/// <returns>
		/// Returns import statistics <see cref="XFLM_IMPORT_STATS"/>.
		/// </returns>
		public XFLM_IMPORT_STATS importDocument(
			IStream istream,
			uint uiCollection)
		{
			RCODE rc;
			XFLM_IMPORT_STATS importStats;

			if ((rc = xflaim_Db_importDocument(m_pDb, 
				istream, uiCollection, out importStats)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (importStats);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_importDocument(
			ulong pDb,
			IStream istream,
			uint uiCollection,
			out XFLM_IMPORT_STATS importStatus);

		//-----------------------------------------------------------------------------
		// importDocument
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Imports an XML fragment into the XFlaim database.  The import requires
		/// an update transaction.
		/// </summary>
		/// <param name="istream">
		/// Input stream containing the nodes to be imported
		/// </param>
		/// <param name="nodeToLinkTo">
		/// Existing node that imported nodes will link to
		/// </param>
		/// <param name="insertLocation">
		/// Relationship of imported nodes to "nodeToLinkTo"
		/// </param>
		/// <returns>
		/// Returns import statistics <see cref="XFLM_IMPORT_STATS"/>.
		/// </returns>
		public XFLM_IMPORT_STATS importDocument(
			IStream istream,
			DOMNode nodeToLinkTo,
			eNodeInsertLoc insertLocation)
		{
			RCODE rc;
			XFLM_IMPORT_STATS importStats;

			if ((rc = xflaim_Db_importDocument(m_pDb, 
				istream, nodeToLinkTo, insertLocation,
				out importStats)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (importStats);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_importDocument(
			ulong pDb,
			IStream istream,
			DOMNode nodeToLinkTo,
			eNodeInsertLoc insertLocation,
			out XFLM_IMPORT_STATS importStatus);

		//-----------------------------------------------------------------------------
		// changeItemState
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Change a dictionary definition's state.  This routine is used to determine if
		/// the dictionary item can be deleted.  It may also be used to force the
		/// definition to be deleted - once the database has determined that the
		/// definition is not in use anywhere.  This should only be used for
		/// element definitions and attribute definitions definitions.
		/// </summary>
		/// <param name="dictType">
		/// Type of dictionary definition whose state is being changed.
		/// </param>
		/// <param name="uiDictNumber">
		/// Number of element or attribute definition whose state
		/// is to be changed
		/// </param>
		/// <param name="sState">
		/// State the definition is to be changed to.  Must be
		/// "checking", "purge", or "active".
		/// </param>
		public void changeItemState(
			ReservedElmTag dictType,
			uint uiDictNumber,
			string sState)
		{
			RCODE rc;

			if ((rc = xflaim_Db_changeItemState(m_pDb,
				dictType, uiDictNumber, sState)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_changeItemState(
			ulong pDb,
			ReservedElmTag dictType,
			uint uiDictNumber,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sState);

		//-----------------------------------------------------------------------------
		// getRflFileName
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Get the name of a roll-forward log file.
		/// </summary>
		/// <param name="uiFileNum">
		/// Roll-forward log file number whose name is to be returned.
		/// </param>
		/// <param name="bBaseOnly">
		/// If true, only the base name of the file will be returned.
		/// Otherwise, the entire path will be returned.
		/// </param>
		/// <returns>
		/// Name of the file.
		/// </returns>
		public string getRflFileName(
			uint				uiFileNum,
			bool				bBaseOnly)
		{
			RCODE rc;
			IntPtr pszFileName;
			string sFileName;

			if ((rc = xflaim_Db_getRflFileName(m_pDb, uiFileNum,
				bBaseOnly, out pszFileName)) != 0)
			{
				throw new XFlaimException(rc);
			}

			sFileName = Marshal.PtrToStringAnsi(pszFileName);
			m_dbSystem.freeUnmanagedMem(pszFileName);
			return (sFileName);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getRflFileName(
			ulong pDb,
			uint uiFileNum,
			bool bBaseOnly,
			out IntPtr sName);

		//-----------------------------------------------------------------------------
		// setNextNodeId
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Set the next node ID for a collection.  This will be the node ID for
		/// the next node that is created in the collection.  NOTE: The node ID must
		/// be greater than or equal to the current next node ID that is already
		/// set for the collection.  Otherwise, it is ignored.
		/// </summary>
		/// <param name="uiCollection">
		/// Collection whose next node ID is to be set
		/// </param>
		/// <param name="ulNextNodeId">
		/// Next node ID value
		/// </param>
		public void setNextNodeId(
			uint				uiCollection,
			ulong				ulNextNodeId)
		{
			RCODE rc;

			if ((rc = xflaim_Db_setNextNodeId(m_pDb, uiCollection, 
				ulNextNodeId)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_setNextNodeId(
			ulong pDb,
			uint uiCollection,
			ulong ulNextNodeId);

		//-----------------------------------------------------------------------------
		// setNextDictNum
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Set the next dictionary number that is to be assigned for a particular
		/// type if dictionary definition.  The specified "next dictionary number"	
		/// must be greater than the current "next dictionary number".  Otherwise,
		/// no action is taken.
		/// </summary>
		/// <param name="dictType">
		/// Type of dictionary definition whose "next dictionary number" is to 
		/// be changed.
		/// </param>
		/// <param name="uiDictNumber">
		/// Next dictionary number.
		/// </param>
		public void setNextDictNum(
			ReservedElmTag dictType,
			uint uiDictNumber)
		{
			RCODE rc;

			if ((rc = xflaim_Db_setNextDictNum(m_pDb, dictType,
				uiDictNumber)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_setNextDictNum(
			ulong pDb,
			ReservedElmTag dictType,
			ulong uiDictNumer);

		//-----------------------------------------------------------------------------
		// setRflKeepFilesFlag
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Specify whether the roll-forward log should keep or not keep RFL files.
		/// </summary>
		/// <param name="bKeep">
		/// Flag specifying whether to keep or not keep RFL files.
		/// </param>
		public void setRflKeepFilesFlag(
			bool		bKeep)
		{
			RCODE rc;

			if ((rc = xflaim_Db_setRflKeepFilesFlag(m_pDb, bKeep)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_setRflKeepFilesFlag(
			ulong pDb,
			bool bKeep);

		//-----------------------------------------------------------------------------
		// getRflKeepFlag
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Determine whether or not the roll-forward log files are being kept.
		/// </summary>
		/// <returns>
		/// Returns true if RFL files are being kept, false otherwise.
		/// </returns>
		public bool getRflKeepFlag()
		{
			RCODE rc;
			bool bKeep;

			if ((rc = xflaim_Db_getRflKeepFlag(m_pDb, out bKeep)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (bKeep);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getRflKeepFlag(
			ulong pDb,
			out bool bKeep);

		//-----------------------------------------------------------------------------
		// setRflDir
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Set the RFL directory.
		/// </summary>
		/// <param name="sRflDir">
		/// Name of RFL directory.
		/// </param>
		public void setRflDir(
			string	sRflDir)
		{
			RCODE rc;

			if ((rc = xflaim_Db_setRflDir(m_pDb, sRflDir)) != 0)
			{
				throw new XFlaimException(rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_setRflDir(
			ulong pDb,
			[MarshalAs(UnmanagedType.LPWStr)]
			string sRflDir);

		//-----------------------------------------------------------------------------
		// getRflDir
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Get the current RFL directory.
		/// </summary>
		/// <returns>
		/// Returns the current RFL directory name.
		/// </returns>
		public string getRflDir()
		{
			RCODE rc;
			IntPtr pszRflDir;
			string sRflDir;

			if ((rc = xflaim_Db_getRflDir(m_pDb, out pszRflDir)) != 0)
			{
				throw new XFlaimException(rc);
			}

			sRflDir = Marshal.PtrToStringAnsi(pszRflDir);
			m_dbSystem.freeUnmanagedMem(pszRflDir);
			return (sRflDir);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getRflDir(
			ulong pDb,
			out IntPtr pszRflDir);

		//-----------------------------------------------------------------------------
		//	getRflFileNum
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Get the current RFL file number.
		/// </summary>
		/// <returns>
		/// Returns the current RFL file number.
		/// </returns>
		public uint getRflFileNum()
		{
			RCODE rc;
			uint uiRflFileNum;

			if ((rc = xflaim_Db_getRflFileNum(m_pDb, out uiRflFileNum)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiRflFileNum);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getRflFileNum(
			ulong pDb,
			out uint uiRflFileNum);

		//-----------------------------------------------------------------------------
		// getHighestNotUsedRflFileNum
		//-----------------------------------------------------------------------------

		/// <summary>
		/// Get the highest RFL file number that is no longer in use by XFLAIM.
		/// This RFL file can be removed from the system if needed.
		/// </summary>
		/// <returns>
		/// Returns the highest RFL file number that is no longer in use.
		/// </returns>
		public uint getHighestNotUsedRflFileNum()
		{
			RCODE rc;
			uint uiRflFileNum;

			if ((rc = xflaim_Db_getHighestNotUsedRflFileNum(m_pDb, 
				out uiRflFileNum)) != 0)
			{
				throw new XFlaimException(rc);
			}

			return (uiRflFileNum);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Db_getHighestNotUsedRflFileNum(
			ulong pDb,
			out uint uiRflFileNum);

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Set size limits for RFL files.
	 * @param iMinRflSize Minimum RFL file size.  Database will roll to the
	 * next RFL file when the current RFL file reaches this size.  If possible
	 * it will complete the current transaction before rolling to the next file.
	 * @param iMaxRflSize Maximum RFL file size.  Database will not allow an
	 * RFL file to exceed this size.  Even if it is in the middle of a
	 * transaction, it will roll to the next RFL file before this size is allowed
	 * to be exceeded.  Thus, the database first looks for an opportunity to
	 * roll to the next file when the RFL file exceeds iMinRflSize.  If it can
	 * fit the current transaction in without exceeded iMaxRflSize, it will do
	 * so and then roll to the next file.  Otherwise, it will roll to the next
	 * file before iMaxRflSize is exceeded.
	 * @throws XFlaimException
	 */
		public void setRflFileSizeLimits(
			uint		uiMinRflSize,
			uint		uiMaxRflSize)
		{
		}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the minimum RFL file size.  This is the minimum size an RFL file
	 * must reach before rolling to the next RFL file.
	 * @return Returns minimum RFL file size.
	 * @throws XFlaimException
	 */
	public int getMinRflFileSize() throws XFlaimException
	{
		return( _getMinRflFileSize( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the maximum RFL file size.  This is the maximum size an RFL file
	 * is allowed to grow to.  When the current RFL file exceeds the minimum
	 * RFL file size, the database will attempt to fit the rest of the
	 * transaction in the current file.  If the transaction completes before
	 * the current RFL file grows larger than the maximum RFL file size,
	 * the database will roll to the next RFL file.  However, if the current transaction
	 * would cause the RFL file to grow larger than the maximum RFL file size,
	 * the database will roll to the next file before the transaction completes,
	 * and the transaction will be split across multiple RFL files.
	 * @return Returns maximum RFL file size.
	 * @throws XFlaimException
	 */
	public int getMaxRflFileSize() throws XFlaimException
	{
		return( _getMaxRflFileSize( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Force the database to roll to the next RFL file.
	 * @throws XFlaimException
	 */
	public void rflRollToNextFile() throws XFlaimException
	{
		_rflRollToNextFile( m_this);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Specify whether the roll-forward log should keep or not keep aborted
	 * transactions.
	 * @param bKeep Flag specifying whether to keep or not keep aborted
	 * transactions.
	 * @throws XFlaimException
	 */
	public void setKeepAbortedTransInRflFlag(
		boolean	bKeep) throws XFlaimException
	{
		_setKeepAbortedTransInRflFlag( m_this, bKeep);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Determine whether or not the roll-forward log is keeping aborted
	 * transactions.
	 * @return Returns true if aborted transactions are being kept, false otherwise.
	 * @throws XFlaimException
	 */
	public boolean getKeepAbortedTransInRflFlag() throws XFlaimException
	{
		return( _getKeepAbortedTransInRflFlag( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Specify whether the roll-forward log should automatically turn off the
	 * keeping of RFL files if the RFL volume fills up.
	 * @param bAutoTurnOff Flag specifying whether to automatically turn off the
	 * keeping of RFL files if the RFL volume fills up.
	 * @throws XFlaimException
	 */
	public void setAutoTurnOffKeepRflFlag(
		boolean	bAutoTurnOff) throws XFlaimException
	{
		_setAutoTurnOffKeepRflFlag( m_this, bAutoTurnOff);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Determine whether or not keeping of RFL files will automatically be
	 * turned off if the RFL volume fills up.
	 * @return Returns true if the keeping of RFL files will automatically be
	 * turned off when the RFL volume fills up, false otherwise.
	 * @throws XFlaimException
	 */
	public boolean getAutoTurnOffKeepRflFlag() throws XFlaimException
	{
		return( _getAutoTurnOffKeepRflFlag( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Set the file extend size for the database.  This size specifies how much
	 * to extend a database file when it needs to be extended.
	 * @param iFileExtendSize  File extend size.
	 * @throws XFlaimException
	 */
	public void setFileExtendSize(
		int	iFileExtendSize) throws XFlaimException
	{
		_setFileExtendSize( m_this, iFileExtendSize);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the file extend size for the database.
	 * @return Returns file extend size.
	 * @throws XFlaimException
	 */
	public int getFileExtendSize() throws XFlaimException
	{
		return( _getFileExtendSize( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the database version for the database.  This is the version of the
	 * database, not the code.
	 * @return Returns database version.
	 * @throws XFlaimException
	 */
	public int getDbVersion() throws XFlaimException
	{
		return( _getDbVersion( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the database block size.
	 * @return Returns database block size.
	 * @throws XFlaimException
	 */
	public int getBlockSize() throws XFlaimException
	{
		return( _getBlockSize( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the database default language.
	 * @return Returns database default language.
	 * @throws XFlaimException
	 */
	public int getDefaultLanguage() throws XFlaimException
	{
		return( _getDefaultLanguage( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the database's current transaction ID.  If no transaction is
	 * currently running, but this Db object has an exclusive lock on the database,
	 * the transaction ID of the last committed transaction will be returned.
	 * If no transaction is running, and this Db object does not have an
	 * exclusive lock on the database, zero is returned.
	 * @return Returns transaction ID.
	 * @throws XFlaimException
	 */
	public long getTransID() throws XFlaimException
	{
		return( _getTransID( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the name of the database's control file (e.g.&nbsp;mystuff.db).
	 * @return Returns control file name.
	 * @throws XFlaimException
	 */
	public String getDbControlFileName() throws XFlaimException
	{
		return( _getDbControlFileName( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the transaction ID of the last backup that was taken on the database.
	 * @return Returns last backup transaction ID.
	 * @throws XFlaimException
	 */
	public long getLastBackupTransID() throws XFlaimException
	{
		return( _getLastBackupTransID( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the number of blocks that have changed since the last backup was
	 * taken.
	 * @return Returns number of blocks that have changed.
	 * @throws XFlaimException
	 */
	public int getBlocksChangedSinceBackup() throws XFlaimException
	{
		return( _getBlocksChangedSinceBackup( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the next incremental backup sequence number for the database.
	 * @return Returns next incremental backup sequence number.
	 * @throws XFlaimException
	 */
	public int getNextIncBackupSequenceNum() throws XFlaimException
	{
		return( _getNextIncBackupSequenceNum( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the amount of disk space currently being used by data files.
	 * @return Returns disc space used by data files.
	 * @throws XFlaimException
	 */
	public long getDiskSpaceDataSize()throws XFlaimException
	{
		return( _getDiskSpaceDataSize( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the amount of disk space currently being used by rollback files.
	 * @return Returns disc space used by rollback files.
	 * @throws XFlaimException
	 */
	public long getDiskSpaceRollbackSize() throws XFlaimException
	{
		return( _getDiskSpaceRollbackSize( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the amount of disk space currently being used by RFL files.
	 * @return Returns disc space used by RFL files.
	 * @throws XFlaimException
	 */
	public long getDiskSpaceRflSize() throws XFlaimException
	{
		return( _getDiskSpaceRflSize( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the amount of disk space currently being used by all types of
	 * database files.  This includes the total of data files plus rollback
	 * files plus RFL files.
	 * @return Returns total disc space used by database files of all types.
	 * @throws XFlaimException
	 */
	public long getDiskSpaceTotalSize() throws XFlaimException
	{
		return( _getDiskSpaceTotalSize( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get error code that caused the database to force itself to close.  This should
	 * be one of the values in {@link xflaim.RCODE RCODE}.
	 * @return Returns error code that caused the database to force itself to close.
	 * @throws XFlaimException
	 */
	public int getMustCloseRC() throws XFlaimException
	{
		return( _getMustCloseRC( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get error code that caused the current transaction to require an abort.
	 * This may be one of the values in {@link xflaim.RCODE RCODE}, but not
	 * necessarily.
	 * @return Returns error code that caused the current transaction to require
	 * itself to abort.
	 * @throws XFlaimException
	 */
	public int getAbortRC() throws XFlaimException
	{
		return( _getAbortRC( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Force the current transaction to abort.  This method should be called
	 * when the code should not be the code that aborts the transation, but
	 * wants to require that the transaction be aborted by whatever module has
	 * the authority to abort or commit the transaction.  An error code may be
	 * set to indicate what error condition is causing the transaction to be
	 * aborted.
	 * @param iRc Error code that indicates why the transaction is aborting.
	 * @throws XFlaimException
	 */
	public void setMustAbortTrans(
		int	iRc) throws XFlaimException
	{
		_setMustAbortTrans( m_this, iRc);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Enable encryption for this database.
	 * @throws XFlaimException
	 */
	public void enableEncryption() throws XFlaimException
	{
		_enableEncryption( m_this);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Wrap the database key in a password.  This method is called when it is
	 * desirable to move the database to a different machine.  Normally, the
	 * database key is wrapped in the local NICI storage key - which means that
	 * the database can only be opened and accessed on that machine. -- Once
	 * the database key is wrapped in a password, the password must be
	 * supplied to the dbOpen method to open the database.
	 * @param sPassword Password the database key should be wrapped in.
	 * @throws XFlaimException
	 */
	public void wrapKey(
		String	sPassword) throws XFlaimException
	{
		_wrapKey( m_this, sPassword);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Generate a new database key.  All encryption definition keys will be
	 * re-wrapped in the new database key.
	 * @throws XFlaimException
	 */
	public void rollOverDbKey() throws XFlaimException
	{
		_rollOverDbKey( m_this);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the database serial number.
	 * @return Byte array containing the database serial number.  This number
	 * is generated and stored in the database when the database is created.
	 * @throws XFlaimException
	 */
	public byte[] getSerialNumber() throws XFlaimException
	{
		return( _getSerialNumber( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get information about the checkpoint thread's current state.
	 * @return Checkpoint thread state information is returned in a
	 * {@link xflaim.CheckpointInfo CheckpointInfo} object.
	 * @throws XFlaimException
	 */
	public CheckpointInfo getCheckpointInfo() throws XFlaimException
	{
		return( _getCheckpointInfo( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Export XML to a text file.
	 * @param startNode The node in the XML document to export.  All of its
	 * sub-tree will be exported.
	 * @param sFileName File the XML is to be exported to.  File will be
	 * overwritten.
	 * @param iFormat Formatting to use when exporting.  Should be one of
	 * {@link xflaim.ExportFormatType ExportFormatType}.
	 * @throws XFlaimException
	 */
	public void exportXML(
		DOMNode	startNode,
		String	sFileName,
		int		iFormat) throws XFlaimException
	{
		_exportXML( m_this, startNode.getThis(), sFileName, iFormat);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Export XML to a string.
	 * @param startNode The node in the XML document to export.  All of its
	 * sub-tree will be exported.
	 * @param iFormat Formatting to use when exporting.  Should be one of
	 * {@link xflaim.ExportFormatType ExportFormatType}.
	 * @throws XFlaimException
	 */
	public String exportXML(
		DOMNode	startNode,
		int		iFormat) throws XFlaimException
	{
		return( _exportXML( m_this, startNode.getThis(), iFormat));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Get the list of threads that are holding the database lock as well as
	 * the threads that are waiting to obtain the database lock.
	 * @return Returns an array of {@link xflaim.LockUser LockUser} objects.  The
	 * zeroeth element in the array is the current holder of the database lock.
	 * All other elements of the array are threads that are waiting to obtain
	 * the lock.
	 * @throws XFlaimException
	 */
	public LockUser[] getLockWaiters() throws XFlaimException
	{
		return( _getLockWaiters( m_this));
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Set a callback object that will report the progress of an index or
	 * collection deletion operation.  This object's methods are called only if
	 * the index or collection is deleted in the foreground.  The delete operation
	 * must be performed in the same thread where this method is called.
	 * @param deleteStatusObj An object that implements the {@link xflaim.DeleteStatus
	 * DeleteStatus} interface.
	 * @throws XFlaimException
	 */
	public void setDeleteStatusObject(
		DeleteStatus	deleteStatusObj) throws XFlaimException
	{
		_setDeleteStatusObject( m_this, deleteStatusObj);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Set a callback object that will report each document that is being indexed when
	 * an index definition object is added.  This object's methods are called only if
	 * the index is added in the foreground.  The index definition must be added
	 * in the same thread that sets this object.
	 * @param ixClientObj An object that implements the {@link xflaim.IxClient
	 * IxClient} interface.
	 * @throws XFlaimException
	 */
	public void setIndexingClientObject(
		IxClient			ixClientObj) throws XFlaimException
	{
		_setIndexingClientObject( m_this, ixClientObj);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Set a callback object that will report indexing progress when
	 * an index definition object is added.  This object's methods are called only if
	 * the index is added in the foreground.  The index definition must be added
	 * in the same thread that sets this object.
	 * @param ixStatusObj An object that implements the {@link xflaim.IxStatus
	 * IxStatus} interface.
	 * @throws XFlaimException
	 */
	public void setIndexingStatusObject(
		IxStatus			ixStatusObj) throws XFlaimException
	{
		_setIndexingStatusObject( m_this, ixStatusObj);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Set a callback object that will be called after a transaction commit
	 * has safely saved all transaction data to disk, but before the database
	 * is unlocked.  This allows an application to do anything it may need to do
	 * after a commit but before the database is unlocked.  The thread that
	 * performs the commit must be the thread that sets this object.
	 * @param commitClientObj An object that implements the {@link xflaim.CommitClient
	 * CommitClient} interface.
	 * @throws XFlaimException
	 */
	public void setCommitClientObject(
		CommitClient	commitClientObj) throws XFlaimException
	{
		_setCommitClientObject( m_this, commitClientObj);
	}
#endif

		//-----------------------------------------------------------------------------
		//
		//-----------------------------------------------------------------------------

#if TODO
	/**
	 * Upgrade the database to the most current database version.
	 * @throws XFlaimException
	 */
	public void upgrade() throws XFlaimException
	{
		_upgrade( m_this);
	}
#endif

	}
}
