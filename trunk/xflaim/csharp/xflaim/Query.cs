//------------------------------------------------------------------------------
// Desc:	Db Check Status
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

	/// <summary>
	/// Flags for comparing strings.
	/// IMPORTANT NOTE: This needs to be kept in sync with the definitions in ftk.h
	/// </summary>
	[Flags]
	public enum CompareFlags : uint
	{
		/// <summary>Do case sensitive comparison.</summary>
		FLM_COMP_CASE_INSENSITIVE			= 0x0001,
		/// <summary>Compare multiple whitespace characters as a single space.</summary>
		FLM_COMP_COMPRESS_WHITESPACE		= 0x0002,
		/// <summary>Ignore all whitespace during comparison.</summary>
		FLM_COMP_NO_WHITESPACE				= 0x0004,
		/// <summary>Ignore all underscore characters during comparison.</summary>
		FLM_COMP_NO_UNDERSCORES				= 0x0008,
		/// <summary>Ignore all dash characters during comparison.</summary>
		FLM_COMP_NO_DASHES					= 0x0010,
		/// <summary>Treat newlines and tabs as spaces during comparison.</summary>
		FLM_COMP_WHITESPACE_AS_SPACE		= 0x0020,
		/// <summary>Ignore leading space characters during comparison.</summary>
		FLM_COMP_IGNORE_LEADING_SPACE		= 0x0040,
		/// <summary>Ignore trailing space characters during comparison.</summary>
		FLM_COMP_IGNORE_TRAILING_SPACE	= 0x0080,
		/// <summary>Compare wild cards</summary>
		FLM_COMP_WILD							= 0x0100
	}

	/// <summary>
	/// Axis types for XPATH components
	/// </summary>
	public enum eXPathAxisTypes : uint
	{
		/// <summary>Root axix</summary>
		ROOT_AXIS = 0,
		/// <summary>Child axis</summary>
		CHILD_AXIS,
		/// <summary>Parent axis</summary>
		PARENT_AXIS,
		/// <summary>Ancestor axis</summary>
		ANCESTOR_AXIS,
		/// <summary>Descendant axis</summary>
		DESCENDANT_AXIS,
		/// <summary>Following sibling axis</summary>
		FOLLOWING_SIBLING_AXIS,
		/// <summary>Preceding sibling axis</summary>
		PRECEDING_SIBLING_AXIS,
		/// <summary>Following axis</summary>
		FOLLOWING_AXIS,
		/// <summary>Preceding axis</summary>
		PRECEDING_AXIS,
		/// <summary>Attribute axis</summary>
		ATTRIBUTE_AXIS,
		/// <summary>Namespace axis</summary>
		NAMESPACE_AXIS,
		/// <summary>Self axis</summary>
		SELF_AXIS,
		/// <summary>Descendant or self axis</summary>
		DESCENDANT_OR_SELF_AXIS,
		/// <summary>Ancestor or self axis</summary>
		ANCESTOR_OR_SELF_AXIS,
		/// <summary>Meta axis - this is an extension for XFLAIM</summary>
		META_AXIS
	}

	/// <summary>
	/// Query operators.
	/// IMPORTANT NOTE: These must be kept in sync with the corresponding
	/// definitions in xflaim.h.  NOTE: Only the ones that are valid
	/// to pass into the <see cref="Query.addOperator"/> method need to be
	/// defined here.
	/// </summary>
	public enum eQueryOperators
	{
		/// <summary>Logical AND operator (&amp;&amp;)</summary>
		XFLM_AND_OP							= 1,
		/// <summary>Logical OR operator (||)</summary>
		XFLM_OR_OP							= 2,
		/// <summary>Logical NOT operator (!)</summary>
		XFLM_NOT_OP							= 3,
		/// <summary>Equality comparison operator (==)</summary>
		XFLM_EQ_OP							= 4,
		/// <summary>Not equal comparison operator (!=)</summary>
		XFLM_NE_OP							= 5,
		/// <summary>Approximately equal comparison operator (~=)</summary>
		XFLM_APPROX_EQ_OP					= 6,
		/// <summary>Less than comparison operator (&lt;)</summary>
		XFLM_LT_OP							= 7,
		/// <summary>Less than or equal comparison operator (&lt;=)</summary>
		XFLM_LE_OP							= 8,
		/// <summary>Greater than comparison operator (&gt;)</summary>
		XFLM_GT_OP							= 9,
		/// <summary>Greater than or equal comparison operator (&gt;=)</summary>
		XFLM_GE_OP							= 10,
		/// <summary>Bitwise AND arithmetic operator (&amp;)</summary>
		XFLM_BITAND_OP						= 11,
		/// <summary>Bitwise OR arithmetic operator (|)</summary>
		XFLM_BITOR_OP						= 12,
		/// <summary>Bitwise XOR arithmetic operator (^)</summary>
		XFLM_BITXOR_OP						= 13,
		/// <summary>Multiply arithmetic operator (*)</summary>
		XFLM_MULT_OP						= 14,
		/// <summary>Divide arithmetic operator (/)</summary>
		XFLM_DIV_OP							= 15,
		/// <summary>Mod arithmetic operator (%)</summary>
		XFLM_MOD_OP							= 16,
		/// <summary>Addition arithmetic operator (+)</summary>
		XFLM_PLUS_OP						= 17,
		/// <summary>Subtraction arithmetic operator (-)</summary>
		XFLM_MINUS_OP						= 18,
		/// <summary>Unary minus arithmetic operator (-)</summary>
		XFLM_NEG_OP							= 19,
		/// <summary>Left parenthesis operator</summary>
		XFLM_LPAREN_OP						= 20,
		/// <summary>Right parenthesis operator</summary>
		XFLM_RPAREN_OP						= 21,
		/// <summary>Comman operator (,)</summary>
		XFLM_COMMA_OP						= 22,
		/// <summary>Left bracket operator ([)</summary>
		XFLM_LBRACKET_OP					= 23,
		/// <summary>Right bracket operator (])</summary>
		XFLM_RBRACKET_OP					= 24
	}

	/// <summary>
	/// Types of optimizations for queries.
	/// </summary>
	public enum eQOptTypes : uint
	{
		/// <summary>No optimization</summary>
		XFLM_QOPT_NONE = 0,
		/// <summary>Using an index</summary>
		XFLM_QOPT_USING_INDEX,
		/// <summary>Full collection scan - no index</summary>
		XFLM_QOPT_FULL_COLLECTION_SCAN,
		/// <summary>Reading a single node id</summary>
		XFLM_QOPT_SINGLE_NODE_ID,
		/// <summary>No Reading a range of node ids</summary>
		XFLM_QOPT_NODE_ID_RANGE
  }

	/// <summary>
	/// Optimization information for queries.
	/// IMPORTANT NOTE: This structure must be kept in sync with the
	/// corresponding structure in C# code.
	/// </summary>
	[StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Ansi)]
	public class CS_XFLM_OPT_INFO
	{
		/// <summary>Type of optimization done</summary>
		public eQOptTypes	eOptType;
		/// <summary>Cost that was calculated for this particular optimization path.</summary>
		public uint			uiCost;
		/// <summary>
		/// Node id we are searching for - only valid if eOptType is
		/// XFLM_QOPT_SINGLE_NODE_ID or XFLM_QOPT_NODE_ID_RANGE
		/// </summary>
		public ulong		ulNodeId;
		/// <summary>
		/// End node id we are searching for - only valid if eOptType is
		/// XFLM_QOPT_NODE_ID_RANGE
		/// </summary>
		public ulong		ulEndNodeId;
		/// <summary>Index name - only valid if eOptType is XFLM_QOPT_USING_INDEX</summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
		public string		sIxName;
		/// <summary>Index number - only valid if eOptType is XFLM_QOPT_USING_INDEX</summary>
		public uint			uiIxNum;
		/// <summary>Must verify node path</summary>
		public int			bMustVerifyPath;
		/// <summary>
		/// Nodes must be retrieved to evaluate query - cannot do from the
		/// data in the index.  This is only valid if eOptType is XFLM_QOPT_USING_INDEX.
		/// </summary>
		public int			bDoNodeMatch;
		/// <summary>
		/// Can we compare on index keys?  This is only valid if eOptType is XFLM_QOPT_USING_INDEX.
		/// </summary>
		public int			bCanCompareOnKey;
		/// <summary>Total index keys read.</summary>
		public ulong		ulKeysRead;
		/// <summary>Total keys that pointed to the same document as another key.</summary>
		public ulong		ulKeyHadDupDoc;
		/// <summary>Total index keys that passed the query criteria.</summary>
		public ulong		ulKeysPassed;
		/// <summary>Total DOM nodes read.</summary>
		public ulong		ulNodesRead;
		/// <summary>Total DOM nodes tested.</summary>
		public ulong		ulNodesTested;
		/// <summary>Total DOM nodes that passed the query criteria.</summary>
		public ulong		ulNodesPassed;
		/// <summary>Total documents read.</summary>
		public ulong		ulDocsRead;
		/// <summary>Total documents that were duplicates of previously evaluated documents.</summary>
		public ulong		ulDupDocsEliminated;
		/// <summary>Total nodes that failed the validation callback.</summary>
		public ulong		ulNodesFailedValidation;
		/// <summary>Total documents that failed the validation callback.</summary>
		public ulong		ulDocsFailedValidation;
		/// <summary>Total documents that passed the query criteria.</summary>
		public ulong		ulDocsPassed;
	}

	/// <remarks>
	/// The Query class provides a number of methods that allow C#
	/// applications to query an XFLAIM database.
	/// </remarks>
	public class Query
	{
		private ulong 		m_pQuery;			// Pointer to IF_Query object in unmanaged space
		private Db			m_db;

		/// <summary>
		/// Query constructor.
		/// </summary>
		/// <param name="db">
		/// Database this query is to be associated with.
		/// </param>
		/// <param name="uiCollection">
		/// Collection this object is to be associated with.
		/// </param>
		public Query(
			Db		db,
			uint	uiCollection)
		{
			RCODE	rc;

			if ((rc = xflaim_Query_createQuery( uiCollection, out m_pQuery)) != 0)
			{
				throw new XFlaimException( rc);
			}

			if (db == null)
			{
				throw new XFlaimException( "Invalid Db object");
			}
			m_db = db;
		}
	
		/// <summary>
		/// Destructor.
		/// </summary>
		~Query()
		{
			close();
		}

		/// <summary>
		/// Return the pointer to the IF_Query object.
		/// </summary>
		/// <returns>Returns a pointer to the IF_Query object.</returns>
		internal ulong getQuery()
		{
			return( m_pQuery);
		}

		/// <summary>
		/// Close this query object
		/// </summary>
		public void close()
		{
			// Release the native pQuery!
		
			if (m_pQuery != 0)
			{
				xflaim_Query_Release( m_pQuery);
				m_pQuery = 0;
			}
		
			// Remove our reference to the Db object so it can be released.
		
			m_db = null;
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_createQuery(
			uint			uiCollection,
			out ulong	pQuery);

		[DllImport("xflaim")]
		private static extern void xflaim_Query_Release(
			ulong	pQuery);

//-----------------------------------------------------------------------------
// setLanguage
//-----------------------------------------------------------------------------

		/// <summary>
		/// Set the language for the query criteria.  This affects how string
		/// comparisons are done.  Collation is done according to the language
		/// specified.
		/// </summary>
		/// <param name="eLanguage">
		/// Language to be used for string comparisons.
		/// </param>
		public void setLanguage(
			Languages	eLanguage)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_setLanguage( m_pQuery, eLanguage)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_setLanguage(
			ulong			pQuery,
			Languages	eLanguage);

//-----------------------------------------------------------------------------
// setupQueryExpr
//-----------------------------------------------------------------------------

		/// <summary>
		/// Setup the query criteria from the passed in string.
		/// </summary>
		/// <param name="sQueryExpr">
		/// String containing the query criteria.
		/// </param>
		public void setupQueryExpr(
			string	sQueryExpr)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_setupQueryExpr( m_pQuery, m_db.getDb(), sQueryExpr)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_setupQueryExpr(
			ulong			pQuery,
			ulong			pDb,
			[MarshalAs(UnmanagedType.LPWStr), In]
			string		sQueryExpr);

//-----------------------------------------------------------------------------
// copyCriteria
//-----------------------------------------------------------------------------

		/// <summary>
		/// Copy the query criteria from one Query object into this Query object.
		/// </summary>
		/// <param name="queryToCopy">
		/// Query object whose criteria is to be copied.
		/// </param>
		public void copyCriteria(
			Query	queryToCopy)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_copyCriteria( m_pQuery, queryToCopy.getQuery())) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_copyCriteria(
			ulong			pQuery,
			ulong			pQueryToCopy);

//-----------------------------------------------------------------------------
// addXPathComponent
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add an XPATH component to a query.
		/// </summary>
		/// <param name="eXPathAxis">
		/// Type of axis for the XPATH component being added.
		/// </param>
		/// <param name="eNodeType">
		/// Type of node for the XPATH component.
		/// </param>
		/// <param name="uiNameId">
		/// Name ID for the node in the XPATH component.
		/// </param>
		public void addXPathComponent(
			eXPathAxisTypes	eXPathAxis,
			eDomNodeType		eNodeType,
			uint					uiNameId)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addXPathComponent( m_pQuery, eXPathAxis, eNodeType, uiNameId)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addXPathComponent(
			ulong					pQuery,
			eXPathAxisTypes	eXPathAxis,
			eDomNodeType		eNodeType,
			uint					uiNameId);

//-----------------------------------------------------------------------------
// addOperator
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add an operator to a query's criteria.
		/// </summary>
		/// <param name="eOperator">
		/// Operator to be added.
		/// </param>
		/// <param name="eCompareFlags">
		/// Flags for doing string comparisons.  Should be logical ORs of the
		/// enums in <see cref="CompareFlags"/>.  These flags are only used
		/// when comparing string operands.
		/// </param>
		public void addOperator(
			eQueryOperators	eOperator,
			CompareFlags		eCompareFlags)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addOperator( m_pQuery, eOperator, eCompareFlags)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addOperator(
			ulong					pQuery,
			eQueryOperators	eOperator,
			CompareFlags		eCompareFlags);

//-----------------------------------------------------------------------------
// addStringValue
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add a string value to the query's criteria.
		/// </summary>
		/// <param name="sValue">
		/// Value to be added to the criteria.
		/// </param>
		public void addStringValue(
			string	sValue)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addStringValue( m_pQuery, sValue)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addStringValue(
			ulong		pQuery,
			[MarshalAs(UnmanagedType.LPWStr), In]
			string	sValue);

//-----------------------------------------------------------------------------
// addBinaryValue
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add a binary value to the query's criteria.
		/// </summary>
		/// <param name="ucValue">
		/// Value to be added to the criteria.
		/// </param>
		public void addBinaryValue(
			byte []	ucValue)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addBinaryValue( m_pQuery, ucValue, ucValue.Length)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addBinaryValue(
			ulong		pQuery,
			[MarshalAs(UnmanagedType.LPArray), In] 
			byte []	pucValue,
			int		iValueLen);

//-----------------------------------------------------------------------------
// addULongValue
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add an unsigned long value to the query's criteria.
		/// </summary>
		/// <param name="ulValue">
		/// Value to be added to the criteria.
		/// </param>
		public void addULongValue(
			ulong	ulValue)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addULongValue( m_pQuery, ulValue)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addULongValue(
			ulong		pQuery,
			ulong		ulValue);

//-----------------------------------------------------------------------------
// addLongValue
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add an signed long value to the query's criteria.
		/// </summary>
		/// <param name="lValue">
		/// Value to be added to the criteria.
		/// </param>
		public void addLongValue(
			long	lValue)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addLongValue( m_pQuery, lValue)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addLongValue(
			ulong		pQuery,
			long		lValue);

//-----------------------------------------------------------------------------
// addUIntValue
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add an unsigned integer value to the query's criteria.
		/// </summary>
		/// <param name="uiValue">
		/// Value to be added to the criteria.
		/// </param>
		public void addUIntValue(
			uint	uiValue)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addUIntValue( m_pQuery, uiValue)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addUIntValue(
			ulong		pQuery,
			uint		uiValue);

//-----------------------------------------------------------------------------
// addIntValue
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add a signed integer value to the query's criteria.
		/// </summary>
		/// <param name="iValue">
		/// Value to be added to the criteria.
		/// </param>
		public void addIntValue(
			int	iValue)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addIntValue( m_pQuery, iValue)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addIntValue(
			ulong		pQuery,
			int		iValue);

//-----------------------------------------------------------------------------
// addBoolean
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add a boolean (true/false) predicate to the query's criteria.
		/// </summary>
		/// <param name="bValue">
		/// Value to be added to the criteria.
		/// </param>
		public void addBoolean(
			 bool	bValue)
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addBoolean( m_pQuery, (int)(bValue ? 1 : 0))) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addBoolean(
			ulong		pQuery,
			int		bValue);

//-----------------------------------------------------------------------------
// addUnknown
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add an "unknown" predicate to the query's criteria.
		/// </summary>
		public void addUnknown()
		{
			RCODE		rc = 0;

			if ((rc = xflaim_Query_addUnknown( m_pQuery)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addUnknown(
			ulong		pQuery);

//-----------------------------------------------------------------------------
// getFirst
//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the first <see cref="DOMNode"/> that satisfies the query criteria.
		/// This may be a document root node, or any node within the document.  What
		/// is returned depends on how the XPATH expression was constructed.
		/// </summary>
		/// <param name="nodeToReuse">
		/// An existing <see cref="DOMNode"/> object can optionally be passed in, and
		/// it will be reused instead of a new object being allocated.
		/// </param>
		/// <param name="uiTimeLimit">
		/// Time limit (in milliseconds) for operation to complete.
		/// A value of zero indicates that the operation should not time out.
		/// </param>
		/// <returns>
		/// Returns a <see cref="DOMNode"/> object.
		/// </returns>
		public DOMNode getFirst(
			DOMNode	nodeToReuse,
			uint		uiTimeLimit)
		{
			RCODE		rc = 0;
			DOMNode	newNode;
			ulong		pNode;
			ulong		pOldNode = (nodeToReuse == null)
										? 0
										: nodeToReuse.getNode();

			if ((rc = xflaim_Query_getFirst( m_pQuery, m_db.getDb(),
											pOldNode, uiTimeLimit, out pNode)) != 0)
			{
				throw new XFlaimException( rc);
			}
			if (nodeToReuse == null)
			{
				newNode = new DOMNode( pNode, m_db);
			}
			else
			{
				newNode = nodeToReuse;
				newNode.setNodePtr( pNode, m_db);
			}
		
			return( newNode);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_getFirst(
			ulong			pQuery,
			ulong			pDb,
			ulong			pOldNode,
			uint			uiTimeLimit,
			out ulong	ppNode);

//-----------------------------------------------------------------------------
// getLast
//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the last <see cref="DOMNode"/> that satisfies the query criteria.
		/// This may be a document root node, or any node within the document.  What
		/// is returned depends on how the XPATH expression was constructed.
		/// </summary>
		/// <param name="nodeToReuse">
		/// An existing <see cref="DOMNode"/> object can optionally be passed in, and
		/// it will be reused instead of a new object being allocated.
		/// </param>
		/// <param name="uiTimeLimit">
		/// Time limit (in milliseconds) for operation to complete.
		/// A value of zero indicates that the operation should not time out.
		/// </param>
		/// <returns>
		/// Returns a <see cref="DOMNode"/> object.
		/// </returns>
		public DOMNode getLast(
			DOMNode	nodeToReuse,
			uint		uiTimeLimit)
		{
			RCODE		rc = 0;
			DOMNode	newNode;
			ulong		pNode;
			ulong		pOldNode = (nodeToReuse == null)
										? 0
										: nodeToReuse.getNode();

			if ((rc = xflaim_Query_getLast( m_pQuery, m_db.getDb(),
				pOldNode, uiTimeLimit, out pNode)) != 0)
			{
				throw new XFlaimException( rc);
			}
			if (nodeToReuse == null)
			{
				newNode = new DOMNode( pNode, m_db);
			}
			else
			{
				newNode = nodeToReuse;
				newNode.setNodePtr( pNode, m_db);
			}
		
			return( newNode);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_getLast(
			ulong			pQuery,
			ulong			pDb,
			ulong			pOldNode,
			uint			uiTimeLimit,
			out ulong	ppNode);

//-----------------------------------------------------------------------------
// getNext
//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the next <see cref="DOMNode"/> that satisfies the query criteria.
		/// This may be a document root node, or any node within the document.  What
		/// is returned depends on how the XPATH expression was constructed.
		/// </summary>
		/// <param name="nodeToReuse">
		/// An existing <see cref="DOMNode"/> object can optionally be passed in, and
		/// it will be reused instead of a new object being allocated.
		/// </param>
		/// <param name="uiTimeLimit">
		/// Time limit (in milliseconds) for operation to complete.
		/// A value of zero indicates that the operation should not time out.
		/// </param>
		/// <returns>
		/// Returns a <see cref="DOMNode"/> object.
		/// </returns>
		public DOMNode getNext(
			DOMNode	nodeToReuse,
			uint		uiTimeLimit)
		{
			RCODE		rc = 0;
			DOMNode	newNode;
			ulong		pNode;
			ulong		pOldNode = (nodeToReuse == null)
				? 0
				: nodeToReuse.getNode();

			if ((rc = xflaim_Query_getNext( m_pQuery, m_db.getDb(),
				pOldNode, uiTimeLimit, out pNode)) != 0)
			{
				throw new XFlaimException( rc);
			}
			if (nodeToReuse == null)
			{
				newNode = new DOMNode( pNode, m_db);
			}
			else
			{
				newNode = nodeToReuse;
				newNode.setNodePtr( pNode, m_db);
			}
		
			return( newNode);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_getNext(
			ulong			pQuery,
			ulong			pDb,
			ulong			pOldNode,
			uint			uiTimeLimit,
			out ulong	ppNode);

//-----------------------------------------------------------------------------
// getPrev
//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the previous <see cref="DOMNode"/> that satisfies the query criteria.
		/// This may be a document root node, or any node within the document.  What
		/// is returned depends on how the XPATH expression was constructed.
		/// </summary>
		/// <param name="nodeToReuse">
		/// An existing <see cref="DOMNode"/> object can optionally be passed in, and
		/// it will be reused instead of a new object being allocated.
		/// </param>
		/// <param name="uiTimeLimit">
		/// Time limit (in milliseconds) for operation to complete.
		/// A value of zero indicates that the operation should not time out.
		/// </param>
		/// <returns>
		/// Returns a <see cref="DOMNode"/> object.
		/// </returns>
		public DOMNode getPrev(
			DOMNode	nodeToReuse,
			uint		uiTimeLimit)
		{
			RCODE		rc = 0;
			DOMNode	newNode;
			ulong		pNode;
			ulong		pOldNode = (nodeToReuse == null)
				? 0
				: nodeToReuse.getNode();

			if ((rc = xflaim_Query_getPrev( m_pQuery, m_db.getDb(),
				pOldNode, uiTimeLimit, out pNode)) != 0)
			{
				throw new XFlaimException( rc);
			}
			if (nodeToReuse == null)
			{
				newNode = new DOMNode( pNode, m_db);
			}
			else
			{
				newNode = nodeToReuse;
				newNode.setNodePtr( pNode, m_db);
			}
		
			return( newNode);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_getPrev(
			ulong			pQuery,
			ulong			pDb,
			ulong			pOldNode,
			uint			uiTimeLimit,
			out ulong	ppNode);

//-----------------------------------------------------------------------------
// getCurrent
//-----------------------------------------------------------------------------

		/// <summary>
		/// Gets the current <see cref="DOMNode"/> that the query object returned
		/// in a prior call to <see cref="getFirst"/>, <see cref="getLast"/>,
		/// <see cref="getNext"/>, or <see cref="getPrev"/>.
		/// This may be a document root node, or any node within the document.  What
		/// is returned depends on how the XPATH expression was constructed.
		/// </summary>
		/// <param name="nodeToReuse">
		/// An existing <see cref="DOMNode"/> object can optionally be passed in, and
		/// it will be reused instead of a new object being allocated.
		/// </param>
		/// <returns>
		/// Returns a <see cref="DOMNode"/> object.
		/// </returns>
		public DOMNode getCurrent(
			DOMNode	nodeToReuse)
		{
			RCODE		rc = 0;
			DOMNode	newNode;
			ulong		pNode;
			ulong		pOldNode = (nodeToReuse == null)
				? 0
				: nodeToReuse.getNode();

			if ((rc = xflaim_Query_getCurrent( m_pQuery, m_db.getDb(),
				pOldNode, out pNode)) != 0)
			{
				throw new XFlaimException( rc);
			}
			if (nodeToReuse == null)
			{
				newNode = new DOMNode( pNode, m_db);
			}
			else
			{
				newNode = nodeToReuse;
				newNode.setNodePtr( pNode, m_db);
			}
		
			return( newNode);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_getCurrent(
			ulong			pQuery,
			ulong			pDb,
			ulong			pOldNode,
			out ulong	ppNode);

//-----------------------------------------------------------------------------
// resetQuery
//-----------------------------------------------------------------------------

		/// <summary>
		/// Resets the query criteria and results set for the query.
		/// </summary>
		public void resetQuery()
		{
			xflaim_Query_resetQuery( m_pQuery);
		}

		[DllImport("xflaim")]
		private static extern void xflaim_Query_resetQuery(
			ulong			pQuery);

//-----------------------------------------------------------------------------
// getStatsAndOptInfo
//-----------------------------------------------------------------------------

		/// <summary>
		/// Returns statistics and optimization information for the query.
		/// </summary>
		/// <returns>Returns an array of <see cref="CS_XFLM_OPT_INFO"/> objects.</returns>
		public CS_XFLM_OPT_INFO [] getStatsAndOptInfo()
		{
			RCODE						rc;
			IntPtr					pOptInfoArray;
			uint						uiNumOptInfos;
			CS_XFLM_OPT_INFO []	optInfo;

			if ((rc = xflaim_Query_getStatsAndOptInfo( m_pQuery, out pOptInfoArray,
				out uiNumOptInfos)) != 0)
			{
				throw new XFlaimException( rc);
			}
			optInfo = new CS_XFLM_OPT_INFO [uiNumOptInfos];

			for (uint uiLoop = 0; uiLoop < uiNumOptInfos; uiLoop++)
			{
				xflaim_Query_getOptInfo( pOptInfoArray, uiLoop, optInfo [uiLoop]);
			}
			m_db.getDbSystem().freeUnmanagedMem( pOptInfoArray);
			return( optInfo);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_getStatsAndOptInfo(
			ulong			pQuery,
			out IntPtr	pOptInfoArray,
			out uint		puiNumOptInfos);

		[DllImport("xflaim")]
		private static extern void xflaim_Query_getOptInfo(
			IntPtr				pOptInfoArray,
			uint					uiInfoToGet,
			CS_XFLM_OPT_INFO	optInfo);

//-----------------------------------------------------------------------------
// setDupHandling
//-----------------------------------------------------------------------------

		/// <summary>
		/// Set duplicate handling for the query.
		/// </summary>
		/// <param name="bRemoveDups">
		/// Specifies whether duplicates should be removed from the result set.
		/// </param>
		public  void setDupHandling(
			bool	bRemoveDups)
		{
			xflaim_Query_setDupHandling( m_pQuery, (int)(bRemoveDups ? 1 : 0));
		}

		[DllImport("xflaim")]
		private static extern void xflaim_Query_setDupHandling(
			ulong		pQuery,
			int		bRemoveDups);

//-----------------------------------------------------------------------------
// setIndex
//-----------------------------------------------------------------------------

		/// <summary>
		/// Set an index for the query.
		/// </summary>
		/// <param name="uiIndex">
		/// Index that the query should use.
		/// </param>
		public  void setIndex(
			uint	uiIndex)
		{
			RCODE	rc;
			if ((rc = xflaim_Query_setIndex( m_pQuery, uiIndex)) != 0)
			{
				throw new XFlaimException( rc);
			}
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_setIndex(
			ulong		pQuery,
			uint		uiIndex);

//-----------------------------------------------------------------------------
// getIndex
//-----------------------------------------------------------------------------

		/// <summary>
		/// Get the index that is being used for the query.
		/// </summary>
		/// <param name="uiIndex">
		/// Returns the index the query is using.  If more than one index is being
		/// used, this will be on the first of those indexes.
		/// </param>
		/// <param name="bHaveMultiple">
		/// Returns whether multiple indexes are being used.
		/// </param>
		public  void getIndex(
			out uint	uiIndex,
			out bool	bHaveMultiple)
		{
			RCODE	rc;
			int	bHaveMult;

			if ((rc = xflaim_Query_getIndex( m_pQuery, m_db.getDb(), out uiIndex, out bHaveMult)) != 0)
			{
				throw new XFlaimException( rc);
			}
			bHaveMultiple = bHaveMult != 0 ? true : false;
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_getIndex(
			ulong			pQuery,
			ulong			pDb,
			out uint		uiIndex,
			out int		bHaveMultiple);

//-----------------------------------------------------------------------------
// addSortKey
//-----------------------------------------------------------------------------

		/// <summary>
		/// Add a sort key to the query.
		/// </summary>
		/// <param name="ulSortKeyContext">
		/// Context that the current sort key is to be added relative to - either
		/// as a child or a sibling.  If this is the first sort key, a zero should
		/// be passed in here.  Otherwise, the value returned from a previous call
		/// to addSortKey should be passed in.
		/// </param>
		/// <param name="bChildToContext">
		/// Indicates whether this sort key should be added as a child or a sibling
		/// to the sort key context that was passed in the pSortKeyContext parameter.
		/// NOTE: If ulSortKeyContext is zero, then the bChildToContext parameter is ignored.
		/// </param>
		/// <param name="bElement">
		/// Indicates whether the current key component is an element or an attribute.
		/// </param>
		/// <param name="uiNameId">
		/// Name ID of the current key component.
		/// </param>
		/// <param name="compareFlags">
		/// Flags for doing string comparisons when sorting for this sort key component.
		/// Should be logical ORs of the members of <see cref="CompareFlags"/>.  This
		/// parameter is only used if the component is a string element or attribute.
		/// </param>
		/// <param name="uiLimit">
		/// Limit on the size of the key component.  If the component is a string element
		/// or attribute, it is the number of characters.  If the component is a binary
		/// element or attribute, it is the number of bytes.
		/// </param>
		/// <param name="uiKeyComponent">
		/// Specifies which key component this sort key component is.  A value of zero
		/// indicates that it is not a key component, but simply a context component for
		/// other key components.
		/// </param>
		/// <param name="bSortDescending">
		/// Indicates that this key component should be sorted in descending order.
		/// </param>
		/// <param name="bSortMissingHigh">
		/// Indicates that when the value for this key component is missing, it should
		/// be sorted high instead of low.
		/// </param>
		/// <returns>
		/// Returns a value that can be passed back into subsequent calls to addSortKey
		/// when this component needs to be used as a context for subsequent components.
		/// </returns>
		public ulong addSortKey(
			ulong				ulSortKeyContext,
			bool				bChildToContext,
			bool				bElement,
			uint				uiNameId,
			CompareFlags	compareFlags,
			uint				uiLimit,
			uint				uiKeyComponent,
			bool				bSortDescending,
			bool				bSortMissingHigh)
		{
			RCODE	rc;
			ulong	ulContext;

			if ((rc = xflaim_Query_addSortKey( m_pQuery, ulSortKeyContext,
				(int)(bChildToContext ? 1 : 0),
				(int)(bElement ? 1 : 0),
				uiNameId, compareFlags, uiLimit, uiKeyComponent,
				(int)(bSortDescending ? 1 : 0),
				(int)(bSortMissingHigh ? 1 : 0), out ulContext)) != 0)
			{
				throw new XFlaimException( rc);
			}
			return( ulContext);
		}

		[DllImport("xflaim")]
		private static extern RCODE xflaim_Query_addSortKey(
			ulong				pQuery,
			ulong				ulSortKeyContext,
			int				bChildToContext,
			int				bElement,
			uint				uiNameId,
			CompareFlags	compareFlags,
			uint				uiLimit,
			uint				uiKeyComponent,
			int				bSortDescending,
			int				bSortMissingHigh,
			out ulong		pulContext);

	}
}
