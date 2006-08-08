//------------------------------------------------------------------------------
// Desc:	Db Class
//
// Tabs:	3
//
//		Copyright (c) 2003-2006 Novell, Inc. All Rights Reserved.
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
// $Id: Db.java 3110 2006-01-19 13:09:08 -0700 (Thu, 19 Jan 2006) dsanders $
//------------------------------------------------------------------------------
package xflaim;

/**
 * The Db class provides a number of methods that allow java applications to
 * access the XFlaim native environment, specifically, the IF_Db interface.
 */
public class Db 
{
	static
	{ 
		System.loadLibrary( "xflaim");
	}
	
	Db( 
		long			ref, 
		DbSystem 	dbSystem) throws XFlaimException
	{
		super();
		
		if( ref == 0)
		{
			throw new XFlaimException( -1, "No legal reference");
		}
		
		m_this = ref;

		if( dbSystem == null)
		{
			throw new XFlaimException( -1, "No legal dbSystem reference");
		}
		
		m_dbSystem = dbSystem;
	}

	/**
	 * Finalize method used to release native resources on garbage collection.
	 */	
	public void finalize()
	{
		close();
	}
	
	/**
	 *  Closes the database.
	 */	
	public void close()
	{
		// Release the native pDb!
		
		if( m_this != 0)
		{
			_release( m_this);
			m_this = 0;
		}
		
		// Remove our reference to the dbSystem so it can be released.
		
		m_dbSystem = null;
	}

	/**
	 * Starts a transaction.
	 * 
	 * @param eTransactionType The type of transaction to start (read or
	 * write).  Should be one of the members of {@link
	 * xflaim.TransactionType TransactionType}.
	 * @param iMaxLockWait Maximum lock wait time.  Specifies the amount of
	 * time to wait for lock requests occuring during the transaction to be
	 * granted.  Valid values are 0 through 255 seconds.  Zero is used to
	 * specify no-wait locks.
	 * @param iFlags Should be a logical OR'd combination of the members of
	 * the memers of {@link xflaim.TransactionFlags
	 * TransactionFlags}.
	 * @throws XFlaimException
	 */
	public void transBegin(
		int			eTransactionType,
		int			iMaxLockWait,
		int			iFlags) throws XFlaimException
	{
		_transBegin( m_this, eTransactionType, iMaxLockWait, iFlags);
	}
	
	public void transBegin(
		Db 			jdb) throws XFlaimException
	{
		_transBegin( m_this, jdb.m_this);
	}

	/**
	 * Commits an existing transaction.  If no transaction is running, or the
	 * transaction commit fails, an XFlaimException exception will be thrown.
	 * @throws XFlaimException
	 */
	public void transCommit() throws XFlaimException
	{
		_transCommit( m_this);
	}
	
	public int getTransType() throws XFlaimException
	{
		return( _getTransType( m_this));
	}
		
	public void doCheckpoint(
		int	iTimeout) throws XFlaimException
	{
		_doCheckpoint( m_this, iTimeout);
	}

	private native void _dbLock(
		long	lThis,
		int	iLockType,
		int	iPriority,
		int	iTimeout) throws XFlaimException;
		
	public void dbLock(
		int	iLockType,
		int	iPriority,
		int	iTimeout) throws XFlaimException
	{
		_dbLock( m_this, iLockType, iPriority, iTimeout);
	}
	
	private native void _dbUnlock(
		long	lThis) throws XFlaimException;

	public void dbUnlock() throws XFlaimException
	{
		_dbUnlock( m_this);
	}
		
	/**
	 * Aborts an existing transaction.  If no transaction is running, or the
	 * transaction commit fails, an XFlaimException exception will be thrown.
	 * 
	 * @throws XFlaimException
	 */
	public void transAbort() throws XFlaimException
	{
		_transAbort( m_this);
	}
	
	private native int _getLockType(
		long		lThis) throws XFlaimException;

	public int getLockType() throws XFlaimException
	{
		return( _getLockType( m_this));
	}

	private native boolean _getLockImplicit(
		long		lThis) throws XFlaimException;

	public boolean getLockImplicit() throws XFlaimException
	{
		return( _getLockImplicit( m_this));
	}

	private native int _getLockThreadId(
		long		lThis,
		int		iPriority) throws XFlaimException;
		
	public int getLockThreadId(
		int	iPriority) throws XFlaimException
	{
		return( _getLockThreadId( m_this, iPriority));
	}
	
	private native int _getLockNumExclQueued(
		long		lThis,
		int		iPriority) throws XFlaimException;
		
	public int getLockNumExclQueued(
		int	iPriority) throws XFlaimException
	{
		return( _getLockNumExclQueued( m_this, iPriority));
	}
	
	private native int _getLockNumSharedQueued(
		long		lThis,
		int		iPriority) throws XFlaimException;
		
	public int getLockNumSharedQueued(
		int	iPriority) throws XFlaimException
	{
		return( _getLockNumSharedQueued( m_this, iPriority));
	}
	
	private native int _getLockPriorityCount(
		long		lThis,
		int		iPriority) throws XFlaimException;
		
	public int getLockPriorityCount(
		int	iPriority) throws XFlaimException
	{
		return( _getLockPriorityCount( m_this, iPriority));
	}
	
	private native void _indexSuspend(
		long	lThis,
		int	iIndex) throws XFlaimException;

	public void indexSuspend(
		int	iIndex) throws XFlaimException
	{
		_indexSuspend( m_this, iIndex);
	}
	
	private native void _indexResume(
		long	lThis,
		int	iIndex) throws XFlaimException;

	public void indexResume(
		int	iIndex) throws XFlaimException
	{
		_indexResume( m_this, iIndex);
	}
	
	private native int _indexGetNext(
		long	lThis,
		int	iCurrIndex) throws XFlaimException;

	public int indexGetNext(
		int	iCurrIndex) throws XFlaimException
	{
		return( _indexGetNext( m_this, iCurrIndex));
	}

	private native IndexStatus _indexStatus(
		long	lThis,
		int	iIndex) throws XFlaimException;
		
	public IndexStatus indexStatus(
		int	iIndex) throws XFlaimException
	{
		return( _indexStatus( m_this, iIndex));
	}

	private native int _reduceSize(
		long	lThis,
		int	iCount) throws XFlaimException;

	public int reduceSize(
		int	iCount) throws XFlaimException
	{
		return( _reduceSize( m_this, iCount));
	}
	
	/**
	 * Uses the jSearchKey to retrieve the next key from the specified
	 * index.
	 * 
	 * @param iIndex The index that is being searched
	 * @param jSearchKey The DataVector search key
	 * @param iFlags The search flags that direct how the next key will
	 * be determined.
	 * @param jFoundKey This parameter is used during subsequent calls 
	 * to keyRetrieve.  The returned DataVector is passed in as this 
	 * parameter so that it may be reused, thus preventing the unnecessary 
	 * accumulation of IF_DataVector objects in the C++ environment. 
	 */
	public void keyRetrieve(
		int				iIndex,
		DataVector		jSearchKey,
		int				iFlags,
		DataVector		jFoundKey) throws XFlaimException
	{
		long			lKey = jSearchKey.m_this;
		long			lFoundKey = (jFoundKey == null ? 0 : jFoundKey.m_this);
		
		_keyRetrieve( m_this, iIndex, lKey, iFlags, lFoundKey);
	}

	/**
	 * Creates a new document node. 
	 * @param iCollection The collection to store the new document in.
	 * @return Returns the DOMNode representing the new document.
	 * @throws XFlaimException
	 */
	 
	 public DOMNode createDocument(
	 	int			iCollection) throws XFlaimException
	{
		long 		lNewDocRef;

		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
	 		lNewDocRef =  _createDocument( m_this, iCollection);
		}
		
		return (new DOMNode( lNewDocRef, this));
	}
	
	/**
	 * Creates a new root element node. This is the root node of a document
	 * in the XFlaim database.
	 * @param iCollection
	 * @param iTag
	 * @return
	 * @throws XFlaimException
	 */
	public DOMNode createRootElement(
		int		iCollection,
		int		iTag) throws XFlaimException
	{
		long 		lNewDocRef;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			lNewDocRef =  _createRootElement( m_this, iCollection, iTag);
		}
		
		return (new DOMNode( lNewDocRef, this));
	}
	
	/**
	 * Method to retrieve the first document in a specified collection.
	 * @param iCollection - The collection from which to retrieve the 
	 * first document
	 * @param jDOMNode - If this parameter is non-null, it will be assumed
	 * that it is no longer needed and will be rendered unusable upon
	 * returning from this method.
	 * @return - Returns a DOMNode which is the root node of the requested
	 * document.
	 * @throws XFlaimException
	 */
	public DOMNode getFirstDocument(
		int			iCollection,
		DOMNode		ReusedNode) throws XFlaimException
	 {
		DOMNode		jNode = null;
		long			lNewNodeRef = 0;
		long			lReusedNodeRef = 0;
		
		if (ReusedNode != null)
		{
			lReusedNodeRef = ReusedNode.getRef();
		}
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			lNewNodeRef = _getFirstDocument( m_this, iCollection, lReusedNodeRef);
		}
	
		// If we got a reference to a native DOMNode back, let's 
		// create a new DOMNode.
		
		if (lNewNodeRef != 0)
		{
			if (ReusedNode != null)
			{
				ReusedNode.setRef( lNewNodeRef, this);
				jNode = ReusedNode;
			}
			else
			{
				jNode = new DOMNode( lNewNodeRef, this);
			}
		}
			
		return( jNode);
	}
 
	/**
	 * Method to retrieve the last document in a specified collection.
	 * @param iCollection - The collection from which to retrieve the 
	 * last document
	 * @param jDOMNode - If this parameter is non-null, it will be assumed
	 * that it is no longer needed and will be rendered unusable upon
	 * returning from this method.
	 * @return - Returns a DOMNode which is the root node of the requested
	 * document.
	 * @throws XFlaimException
	 */
	public DOMNode getLastDocument(
		int			iCollection,
		DOMNode		ReusedNode) throws XFlaimException
	 {
		DOMNode		jNode = null;
		long			lNewNodeRef = 0;
		long			lReusedNodeRef = 0;
		
		if (ReusedNode != null)
		{
			lReusedNodeRef = ReusedNode.getRef();
		}
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			lNewNodeRef = _getLastDocument( m_this, iCollection, lReusedNodeRef);
		}
	
		// If we got a reference to a native DOMNode back, let's 
		// create a new DOMNode.
		
		if (lNewNodeRef != 0)
		{
			if (ReusedNode != null)
			{
				ReusedNode.setRef( lNewNodeRef, this);
				jNode = ReusedNode;
			}
			else
			{
				jNode = new DOMNode( lNewNodeRef, this);
			}
		}
			
		return( jNode);
	}
 
	public DOMNode getDocument(
		int			iCollection,
		int			iFlags,
		long			lDocumentId,
		DOMNode		ReusedNode) throws XFlaimException
	 {
		DOMNode		jNode = null;
		long			lNewNodeRef = 0;
		long			lReusedNodeRef = 0;
		
		if (ReusedNode != null)
		{
			lReusedNodeRef = ReusedNode.getRef();
		}
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			lNewNodeRef = _getDocument( m_this, iCollection, iFlags, lDocumentId, lReusedNodeRef);
		}
	
		// If we got a reference to a native DOMNode back, let's 
		// create a new DOMNode.
		
		if (lNewNodeRef != 0)
		{
			if (ReusedNode != null)
			{
				ReusedNode.setRef( lNewNodeRef, this);
				jNode = ReusedNode;
			}
			else
			{
				jNode = new DOMNode( lNewNodeRef, this);
			}
		}
			
		return( jNode);
	}
	
	private native void _documentDone(
		long			lThis,
		int			iCollection,
		long			lDocumentId) throws XFlaimException;

	public void documentDone(
		int			iCollection,
		long			lDocumentId) throws XFlaimException
	{
		_documentDone( m_this, iCollection, lDocumentId);
	}
	
	private native void _documentDone(
		long		lThis,
		long		lNode) throws XFlaimException;

	public void _documentDone(
		DOMNode		jDOMNode) throws XFlaimException
	{
		_documentDone( m_this, jDOMNode.getThis());
	}
	
	/**
	 * Creates a new element definition in the dictionary.
	 * @param sNamespaceURI The namespace URI that this definition should be
	 * created in.  If null, the default namespace will be used.
	 * @param sElementName The name of the definition.
	 * @param iDataType The type of node this definition will represent.
	 * Should be one of the constants listed in
	 * {@link xflaim.FlmDataType FlmDataType}.
	 * @param iRequestedId If non-zero, then xflaim will try to use this
	 * number as the name ID of the new definition.
	 * @return Returns the name ID of the new definition.
	 * @throws XFlaimException
	 */
	public int createElementDef(
		String		sNamespaceURI,
		String		sElementName,
		int			iDataType,
		int			iRequestedId) throws XFlaimException
		
	{
		int	iNewNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNewNameId = _createElementDef( m_this, sNamespaceURI,
											sElementName, iDataType,
											iRequestedId);
		}
		
		return( iNewNameId);
	}
	
	/**
	 * Gets the name id for a particular element name.
	 * @param sNamespaceURI The namespace URI for the element.
	 * @param sElementName The name of the element.
	 * @return Returns the name ID of the element.
	 * @throws XFlaimException
	 */
	public int getElementNameId(
		String		sNamespaceURI,
		String		sElementName) throws XFlaimException
	{
		int	iNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNameId = _getElementNameId( m_this, sNamespaceURI,
											sElementName);
		}
		
		return( iNameId);
	}
	
	public int createUniqueElmDef(
		String		sNamespaceURI,
		String		sElementName,
		int			iRequestedId) throws XFlaimException
		
	{
		int	iNewNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNewNameId = _createUniqueElmDef( m_this, sNamespaceURI,
											sElementName, iRequestedId);
		}
		
		return( iNewNameId);
	}
	
	/**
	 * Creates a new attribute definition in the dictionary.
	 * @param sNamespaceURI The namespace URI that this definition should be
	 * created in.  If null, the default namespace will be used.
	 * @param sAttributeName The name of the attribute.
	 * @param iDataType The type of node this definition will represent.
	 * Should be one of the constants listed in
	 * {@link xflaim.FlmDataType FlmDataType}.
	 * @param iRequestedId If non-zero, then xflaim will try to use this
	 * number as the name ID of the new definition.
	 * @return Returns the name ID of the new definition.
	 * @throws XFlaimException
	 */
	public int createAttributeDef(
		String		sNamespaceURI,
		String		sAttributeName,
		int			iDataType,
		int			iRequestedId) throws XFlaimException
		
	{
		int	iNewNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNewNameId = _createAttributeDef( m_this, sNamespaceURI,
											sAttributeName, iDataType,
											iRequestedId);
		}
		
		return( iNewNameId);
	}

	/**
	 * Gets the name id for a particular attribute name.
	 * @param sNamespaceURI The namespace URI for the attribute.
	 * @param sAttributeName The name of the attribute.
	 * @return Returns the name ID of the attribute.
	 * @throws XFlaimException
	 */
	public int getAttributeNameId(
		String		sNamespaceURI,
		String		sAttributeName) throws XFlaimException
	{
		int	iNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNameId = _getAttributeNameId( m_this, sNamespaceURI,
											sAttributeName);
		}
		
		return( iNameId);
	}
	
	/**
	 * Creates a new prefix definition in the dictionary.
	 * @param sPrefixName The name of the prefix.
	 * @param iRequestedId If non-zero, then xflaim will try to use this
	 * number as the name ID of the new definition.
	 * @return Returns the name ID of the new definition.
	 * @throws XFlaimException
	 */
	public int createPrefixDef(
		String		sPrefixName,
		int			iRequestedId) throws XFlaimException
		
	{
		int	iNewNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNewNameId = _createPrefixDef( m_this, sPrefixName, iRequestedId);
		}
		
		return( iNewNameId);
	}

	/**
	 * Gets the name id for a particular prefix name.
	 * @param sPrefixName The name of the prefix.
	 * @return Returns the name ID of the prefix.
	 * @throws XFlaimException
	 */
	public int getPrefixId(
		String		sPrefixName) throws XFlaimException
	{
		int	iNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNameId = _getPrefixId( m_this, sPrefixName);
		}
		
		return( iNameId);
	}
	
	/**
	 * Creates a new encryption definition in the dictionary.
	 * @param sEncType Type of encryption key.
	 * @param sEncName The name of the encryption definition.
	 * @param iKeySize Size of the encryption key.
	 * @param iRequestedId If non-zero, then xflaim will try to use this
	 * number as the name ID of the new definition.
	 * @return Returns the name ID of the new definition.
	 * @throws XFlaimException
	 */
	public int createEncDef(
		String		sEncType,
		String		sEncName,
		int			iKeySize,
		int			iRequestedId) throws XFlaimException
		
	{
		int	iNewNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNewNameId = _createEncDef( m_this, sEncType, sEncName, iKeySize,
									iRequestedId);
		}
		
		return( iNewNameId);
	}

	/**
	 * Gets the name id for a particular encryption definition name.
	 * @param sEncName The name of the encryption definition.
	 * @return Returns the name ID of the encryption definition.
	 * @throws XFlaimException
	 */
	public int getEncDefId(
		String		sEncName) throws XFlaimException
	{
		int	iNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNameId = _getEncDefId( m_this, sEncName);
		}
		
		return( iNameId);
	}
	
	/**
	 * Creates a new collection in the dictionary.
	 * @param sEncName The name of the collection.
	 * @param iEncNumber ID of the encryption definition that should be used
	 * to encrypt this collection.  Zero means the collection will not be encrypted.
	 * @param iRequestedId If non-zero, then xflaim will try to use this
	 * number as the name ID of the new definition.
	 * @return Returns the name ID of the new definition.
	 * @throws XFlaimException
	 */
	public int createCollectionDef(
		String		sCollectionName,
		int			iEncNumber,
		int			iRequestedId) throws XFlaimException
		
	{
		int	iNewNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNewNameId = _createCollectionDef( m_this, sCollectionName, iEncNumber,
									iRequestedId);
		}
		
		return( iNewNameId);
	}

	/**
	 * Gets the collection number for a particular collection name.
	 * @param sCollectionName The name of the collection.
	 * @return Returns the number of the collection.
	 * @throws XFlaimException
	 */
	public int getCollectionNumber(
		String		sCollectionName) throws XFlaimException
	{
		int	iNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNameId = _getCollectionNumber( m_this, sCollectionName);
		}
		
		return( iNameId);
	}
	
	/**
	 * Gets the index number for a particular index name.
	 * @param sIndexName The name of the index.
	 * @return Returns the number of the index.
	 * @throws XFlaimException
	 */
	public int getIndexNumber(
		String		sIndexName) throws XFlaimException
	{
		int	iNameId;
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			iNameId = _getIndexNumber( m_this, sIndexName);
		}
		
		return( iNameId);
	}
	
	public DOMNode getDictionaryDef(
		int			iDictType,
		int			iDictNumber,
		DOMNode		ReusedNode) throws XFlaimException
	 {
		DOMNode		jNode = null;
		long			lNewNodeRef = 0;
		long			lReusedNodeRef = 0;
		
		if (ReusedNode != null)
		{
			lReusedNodeRef = ReusedNode.getRef();
		}
		
		// See the comments in the DOMNode::finalize() function for an
		// explanation of this call synchronized call
		
		synchronized( this)
		{
			lNewNodeRef = _getDictionaryDef( m_this, iDictType, iDictNumber,
										lReusedNodeRef);
		}
	
		// If we got a reference to a native DOMNode back, let's 
		// create a new DOMNode.
		
		if (lNewNodeRef != 0)
		{
			if (ReusedNode != null)
			{
				ReusedNode.setRef( lNewNodeRef, this);
				jNode = ReusedNode;
			}
			else
			{
				jNode = new DOMNode( lNewNodeRef, this);
			}
		}
			
		return( jNode);
	}
	
 	public String getDictionaryName(
 		int	iDictType,
		int	iDictNumber) throws XFlaimException
	{
		return( _getDictionaryName( m_this, iDictType, iDictNumber));
	}
 
	public String getElementNamespace(
		int	iDictNumber) throws XFlaimException
	{
		return( _getElementNamespace( m_this, iDictNumber));
	}
		
	public String getAttributeNamespace(
		int	iDictNumber) throws XFlaimException
	{
		return( _getAttributeNamespace( m_this, iDictNumber));
	}
		
	/**
	 * Retrieves the specified node from the specified collection
	 * @param iCollection The collection where the node is stored.
	 * @param lNodeId The ID number of the node to be retrieved
	 * @param ReusedNode Optional.  An existing instance of DOMNode who's
	 * contents will be replaced with that of the new node.  If null, a
	 * new instance will be allocated.
	 * @return Returns a DOMNode representing the retrieved node.
	 * @throws XFlaimException
	 */
	public DOMNode getNode(
		int			iCollection,
		long			lNodeId,
		DOMNode		ReusedNode) throws XFlaimException
		
	{
		long			lReusedNodeRef = 0;
		DOMNode		NewNode = null;
		long			lNewNodeRef = 0;
		
		if (ReusedNode != null)
		{
			lReusedNodeRef = ReusedNode.getRef();
		}
		
		// See the comments in DOMNode::finalize() for an explanation
		// of this synchronized call
		
		synchronized( this)
		{
			lNewNodeRef = _getNode( m_this, iCollection, lNodeId, lReusedNodeRef);
		}
		
		if (lNewNodeRef != 0)
		{
			if (ReusedNode == null)
			{
				NewNode = new DOMNode(lNewNodeRef, this);
			}
			else
			{
				NewNode=ReusedNode;
				NewNode.setRef( lNewNodeRef, this);
			}
		}
		
		return( NewNode);		
	}

	/**
	 * Retrieves the specified attribute node from the specified collection
	 * @param iCollection The collection where the attribute is stored.
	 * @param lElementNodeId The ID number of the element node that contains
	 * the attribute to be retrieved.
	 * @param iAttrNameId The attribute id of the attribute to be retrieved.
	 * @param ReusedNode Optional.  An existing instance of DOMNode who's
	 * contents will be replaced with that of the new node.  If null, a
	 * new instance will be allocated.
	 * @return Returns a DOMNode representing the retrieved node.
	 * @throws XFlaimException
	 */
	public DOMNode getAttribute(
		int			iCollection,
		long			lElementNodeId,
		int			iAttrNameId,
		DOMNode		ReusedNode) throws XFlaimException
	{
		long			lReusedNodeRef = 0;
		long			lNewNodeRef = 0;
		DOMNode		NewNode = null;
		
		if (ReusedNode != null)
		{
			lReusedNodeRef = ReusedNode.getRef();
		}
		
		// See the comments in DOMNode::finalize() for an explanation
		// of this synchronized call
		
		synchronized( this)
		{
			lNewNodeRef = _getAttribute( m_this, iCollection, lElementNodeId,
										iAttrNameId, lReusedNodeRef);
		}
		
		if (lNewNodeRef != 0)
		{
			if (ReusedNode == null)
			{
				NewNode = new DOMNode(lNewNodeRef, this);
			}
			else
			{
				NewNode=ReusedNode;
				NewNode.setRef( lNewNodeRef, this);
			}
		}
		
		return( NewNode);		
	}

	public int getDataType(
		int	iDictType,
		int	iNameId) throws XFlaimException
	{
		return( _getDataType( m_this, iDictType, iNameId));
	}

	/**
	 * Sets up XFlaim to perform a backup operation
	 * @param eBackupType The type of backup to perform.  Must be one of the
	 * members of {@link xflaim.FlmBackupType
	 * FlmBackupType}.
	 * @param eTransType The type of transaction in which the backup operation
	 * will take place.   Must be one of the members of
	 * {@link xflaim.TransactionType TransactionType}. 
	 * @param iMaxLockWait  Maximum lock wait time.  Specifies the amount of
	 * time to wait for lock requests occuring during the backup operation to
	 * be granted.  Valid values are 0 through 255 seconds.  Zero is used to
	 * specify no-wait locks.
	 * @param ReusedBackup Optional.  An existing instance of Backup that
	 * will be reset with the new settings.  If null, a new instance will
	 * be allocated.
	 * @return Returns an instance of Backup configured to perform the
	 * requested backup operation
	 * @throws XFlaimException
	 */
	public Backup backupBegin(
		int			eBackupType,
		int			eTransType,
		int			iMaxLockWait,
		Backup		ReusedBackup) throws XFlaimException
	{
		long 			lReusedRef = 0;
		long 			lNewRef = 0;
		Backup 		NewBackup;
		
		if (ReusedBackup != null)
		{
			lReusedRef = ReusedBackup.getRef();
		}

		// See to comments in the finalize function for an explanation of this
		// synchronized call
		
		synchronized( this)
		{
			lNewRef = _backupBegin( m_this, eBackupType, eTransType,
									iMaxLockWait, lReusedRef);
		}
		
		if (ReusedBackup == null)
		{
			NewBackup = new Backup(lNewRef, this);
		}
		else
		{
			NewBackup = ReusedBackup;
			NewBackup.setRef( lNewRef, this);
		}
		
		return( NewBackup);
	}

	/**
	 * Imports an XML document into the XFlaim database.  The import requires
	 * an update transaction (TransactionType.UPDATE_TRANS). If the document
	 * cannot be imported, an XFlaimEXception exception will be thrown.
	 * @param jIStream
	 * @param iCollection
	 * @throws XFlaimException
	 */
	public ImportStats Import(
		PosIStream		jIStream,
		int				iCollection) throws XFlaimException
	{
		return( _import( m_this, jIStream.getThis(), iCollection, 0,
						InsertLoc.XFLM_LAST_CHILD));
	}
	
	/**
	 * Imports an XML document into the XFlaim database.  The import requires
	 * an update transaction (TransactionType.UPDATE_TRANS). If the document
	 * cannot be imported, an XFlaimEXception exception will be thrown.
	 * @param jIStream
	 * @param iCollection
	 * @param NodeToLinkTo
	 * @param iInsertLoc.  Should be one of the members of {@link
	 * xflaim.InsertLoc InsertLoc}.
	 * @throws XFlaimException
	 */
	public ImportStats Import(
		PosIStream		jIStream,
		int				iCollection,
		DOMNode			NodeToLinkTo,
		int				iInsertLoc) throws XFlaimException
	{
		if (NodeToLinkTo == null)
		{
			return( _import( m_this, jIStream.getThis(), iCollection, 0, iInsertLoc));
		}
		else
		{
			return( _import( m_this, jIStream.getThis(), iCollection,
							NodeToLinkTo.getThis(), iInsertLoc));
		}
	}
	
	public void changeItemState(
		int				iDictType,
		int				iDictNum,
		String			sState) throws XFlaimException
	{
		_changeItemState( m_this, iDictType, iDictNum, sState);
	}

	public String getRflFileName(
		int				iFileNum,
		boolean			bBaseOnly) throws XFlaimException
	{
		return( _getRflFileName( m_this, iFileNum, bBaseOnly));
	}
		
	public void setNextNodeId(
		int				iCollection,
		long				lNextNodeId) throws XFlaimException
	{
		_setNextNodeId( m_this, iCollection, lNextNodeId);
	}

	public void setNextDictNum(
		int				iDictType,
		int				iDictNumber) throws XFlaimException
	{
		_setNextDictNum( m_this, iDictType, iDictNumber);
	}
	
	public void setRflKeepFilesFlag(
		boolean			bKeep) throws XFlaimException
	{
		_setRflKeepFilesFlag( m_this, bKeep);
	}
		
	public boolean getRflKeepFlag() throws XFlaimException
	{
		return( _getRflKeepFlag( m_this));
	}
	
	public void setRflDir(
		String			sRflDir) throws XFlaimException
	{
		_setRflDir( m_this, sRflDir);
	}
		
	public String getRflDir() throws XFlaimException
	{
		return( _getRflDir( m_this));
	}
	
	public int getRflFileNum() throws XFlaimException
	{
		return( _getRflFileNum( m_this));
	}

	public int getHighestNotUsedRflFileNum() throws XFlaimException
	{
		return( _getHighestNotUsedRflFileNum( m_this));
	}

	public void setRflFileSizeLimits(
		int				iMinRflSize,
		int				iMaxRflSize) throws XFlaimException
	{
		_setRflFileSizeLimits( m_this, iMinRflSize, iMaxRflSize);
	}

	public int getMinRflFileSize() throws XFlaimException
	{
		return( _getMinRflFileSize( m_this));
	}
	
	public int getMaxRflFileSize() throws XFlaimException
	{
		return( _getMaxRflFileSize( m_this));
	}

	public void rflRollToNextFile() throws XFlaimException
	{
		_rflRollToNextFile( m_this);
	}

	public void setKeepAbortedTransInRflFlag(
		boolean			bKeep) throws XFlaimException
	{
		_setKeepAbortedTransInRflFlag( m_this, bKeep);
	}

	public boolean getKeepAbortedTransInRflFlag() throws XFlaimException
	{
		return( _getKeepAbortedTransInRflFlag( m_this));
	}

	public void setAutoTurnOffKeepRflFlag(
		boolean			bAutoTurnOff) throws XFlaimException
	{
		_setAutoTurnOffKeepRflFlag( m_this, bAutoTurnOff);
	}

	public boolean getAutoTurnOffKeepRflFlag() throws XFlaimException
	{
		return( _getAutoTurnOffKeepRflFlag( m_this));
	}

	public void setFileExtendSize(
		int				iFileExtendSize) throws XFlaimException
	{
		_setFileExtendSize( m_this, iFileExtendSize);
	}

	public int getFileExtendSize() throws XFlaimException
	{
		return( _getFileExtendSize( m_this));
	}
	
	public int getDbVersion() throws XFlaimException
	{
		return( _getDbVersion( m_this));
	}

	public int getBlockSize() throws XFlaimException
	{
		return( _getBlockSize( m_this));
	}

	public int getDefaultLanguage() throws XFlaimException
	{
		return( _getDefaultLanguage( m_this));
	}
	
	public long getTransID() throws XFlaimException
	{
		return( _getTransID( m_this));
	}

	public String getDbControlFileName() throws XFlaimException
	{
		return( _getDbControlFileName( m_this));
	}
	
	public long getLastBackupTransID() throws XFlaimException
	{
		return( _getLastBackupTransID( m_this));
	}

	public int getBlocksChangedSinceBackup() throws XFlaimException
	{
		return( _getBlocksChangedSinceBackup( m_this));
	}

	public int getNextIncBackupSequenceNum() throws XFlaimException
	{
		return( _getNextIncBackupSequenceNum( m_this));
	}
	
	public long getDiskSpaceDataSize()throws XFlaimException
	{
		return( _getDiskSpaceDataSize( m_this));
	}

	public long getDiskSpaceRollbackSize() throws XFlaimException
	{
		return( _getDiskSpaceRollbackSize( m_this));
	}
		
	public long getDiskSpaceRflSize() throws XFlaimException
	{
		return( _getDiskSpaceRflSize( m_this));
	}
	
	public long getDiskSpaceTotalSize() throws XFlaimException
	{
		return( _getDiskSpaceTotalSize( m_this));
	}
	
	public int getMustCloseRC() throws XFlaimException
	{
		return( _getMustCloseRC( m_this));
	}

	public int getAbortRC() throws XFlaimException
	{
		return( _getAbortRC( m_this));
	}

	public void setMustAbortTrans(
		int				iRc) throws XFlaimException
	{
		_setMustAbortTrans( m_this, iRc);
	}

	public void enableEncryption() throws XFlaimException
	{
		_enableEncryption( m_this);
	}

	public void wrapKey(
		String			sPassword) throws XFlaimException
	{
		_wrapKey( m_this, sPassword);
	}
		
	public void rollOverDbKey() throws XFlaimException
	{
		_rollOverDbKey( m_this);
	}

	public byte[] getSerialNumber() throws XFlaimException
	{
		return( _getSerialNumber( m_this));
	}

	public CheckpointInfo getCheckpointInfo() throws XFlaimException
	{
		return( _getCheckpointInfo( m_this));
	}
		
	/**
	 * Export XML to a text file.
	 * 
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
			
	/**
	 * Export XML to a string.
	 * 
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
			
	public LockUser[] getLockWaiters() throws XFlaimException
	{
		return( _getLockWaiters( m_this));
	}
			
	public void setDeleteStatusObject(
		DeleteStatus	deleteStatusObj) throws XFlaimException
	{
		_setDeleteStatusObject( m_this, deleteStatusObj);
	}
	
	public void setIndexingClientObject(
		IxClient			ixClientObj) throws XFlaimException
	{
		_setIndexingClientObject( m_this, ixClientObj);
	}
		
	public void setIndexingStatusObject(
		IxStatus			ixStatusObj) throws XFlaimException
	{
		_setIndexingStatusObject( m_this, ixStatusObj);
	}
	
	public void setCommitClientObject(
		CommitClient	commitClientObj) throws XFlaimException
	{
		_setCommitClientObject( m_this, commitClientObj);
	}

	public void upgrade() throws XFlaimException
	{
		_upgrade( m_this);
	}
		
	
	private native void _release(
		long				lThis);

	private native void _transBegin(
		long				lThis,
		int				iTransactionType,
		int 				iMaxlockWait,
		int 				iFlags) throws XFlaimException;

	private native void _transBegin(
		long			lThis,
		long			lSrcDb) throws XFlaimException;
		
	private native void _transCommit(
		long	lThis) throws XFlaimException;
	
	private native void _transAbort(
		long	lThis) throws XFlaimException;

	private native int _getTransType(
		long	lThis) throws XFlaimException;
	
	private native void _doCheckpoint(
		long	lThis,
		int	iTimeout) throws XFlaimException;
		
	private native ImportStats _import(
		long				lThis,
		long				lIStream,
		int				iCollection,
		long				lNodeToLinkTo,
		int				iInsertLoc) throws XFlaimException;

 	private native long _getFirstDocument(
 		long				lThis,
 		int				iCollection,
 		long				lOldNodeRef) throws XFlaimException;
 
 	private native long _getLastDocument(
 		long				lThis,
 		int				iCollection,
 		long				lOldNodeRef) throws XFlaimException;
 
 	private native long _getDocument(
 		long				lThis,
 		int				iCollection,
		int				iFlags,
		long				lDocumentId,
 		long				lOldNodeRef) throws XFlaimException;
 
 	private native long _getNode(
 		long				lThis,
 		int				iCollection,
 		long				lNodeId,
 		long				lOldNodeRef) throws XFlaimException;

	private native long _getAttribute(
		long			lThis,
		int			iCollection,
		long			lElementNodeId,
		int			iAttrNameId,
		long			lOldNodeRef) throws XFlaimException;
		
	private native long _createDocument(
		long				lThis,
		int				iCollection) throws XFlaimException;

	private native long _createRootElement(
		long				lThis,
		int				iCollection,
		int				iTag) throws XFlaimException;
		
	private native int _createElementDef(
		long				lThis,
		String			sNamespaceURI,
		String			sElementName,
		int				iDataType,
		int				iRequestedId) throws XFlaimException;
		
	private native int _getElementNameId(
		long				lThis,
		String			sNamespaceURI,
		String			sElementName) throws XFlaimException;
		
	private native int _createUniqueElmDef(
		long				lThis,
		String			sNamespaceURI,
		String			sElementName,
		int				iRequestedId) throws XFlaimException;
		
	private native int _createAttributeDef(
		long				lThis,
		String			sNamespaceURI,
		String			sAttributeName,
		int				iDataType,
		int				iRequestedId) throws XFlaimException;
		
	private native int _getAttributeNameId(
		long				lThis,
		String			sNamespaceURI,
		String			sAttributeName) throws XFlaimException;
		
	private native int _createPrefixDef(
		long				lThis,
		String			sPrefixName,
		int				iRequestedId) throws XFlaimException;
		
	private native int _getPrefixId(
		long				lThis,
		String			sPrefixName) throws XFlaimException;
		
	private native int _createEncDef(
		long				lThis,
		String			sEncType,
		String			sEncName,
		int				iKeySize,
		int				iRequestedId) throws XFlaimException;
		
	private native int _getEncDefId(
		long				lThis,
		String			sEncName) throws XFlaimException;
		
	private native int _createCollectionDef(
		long				lThis,
		String			sCollectionName,
		int				iEncNumber,
		int				iRequestedId) throws XFlaimException;
		
	private native int _getCollectionNumber(
		long				lThis,
		String			sCollectionName) throws XFlaimException;
		
	private native int _getIndexNumber(
		long				lThis,
		String			sIndexName) throws XFlaimException;
		
 	private native long _getDictionaryDef(
 		long				lThis,
 		int				iDictType,
		int				iDictNumber,
 		long				lOldNodeRef) throws XFlaimException;
 
	private native String _getDictionaryName(
		long	lThis,
		int	iDictType,
		int	iDictNumber) throws XFlaimException;
		
	private native String _getElementNamespace(
		long	lThis,
		int	iDictNumber) throws XFlaimException;
		
	private native String _getAttributeNamespace(
		long	lThis,
		int	iDictNumber) throws XFlaimException;
		
	private native int _getDataType(
		long	lThis,
		int	iDictType,
		int	iNameId) throws XFlaimException;

	private native long _backupBegin(
		long				lThis,
		int				eBackupType,
		int				eTransType,
		int				iMaxLockWait,
		long				lReusedRef) throws XFlaimException;

	private native void _keyRetrieve(
		long				lThis,
		int				iIndex,
		long				lKey,
		int				iFlags,
		long				lFoundKey) throws XFlaimException;
 
	private native void _changeItemState(
		long				lThis,
		int				iDictType,
		int				iDictNum,
		String			sState) throws XFlaimException;

	private native String _getRflFileName(
		long				lThis,
		int				iFileNum,
		boolean			bBaseOnly) throws XFlaimException;
		
	private native void _setNextNodeId(
		long				lThis,
		int				iCollection,
		long				lNextNodeId) throws XFlaimException;

	private native void _setNextDictNum(
		long				lThis,
		int				iDictType,
		int				iDictNumber) throws XFlaimException;

	private native void _setRflKeepFilesFlag(
		long				lThis,
		boolean			bKeep) throws XFlaimException;
		
	private native boolean _getRflKeepFlag(
		long				lThis) throws XFlaimException;
		
	private native void _setRflDir(
		long				lThis,
		String			sRflDir) throws XFlaimException;
		
	private native String _getRflDir(
		long				lThis) throws XFlaimException;
	
	private native int _getRflFileNum(
		long				lThis) throws XFlaimException;

	private native int _getHighestNotUsedRflFileNum(
		long				lThis) throws XFlaimException;

	private native void _setRflFileSizeLimits(
		long				lThis,
		int				iMinRflSize,
		int				iMaxRflSize) throws XFlaimException;

	private native int _getMinRflFileSize(
		long				lThis) throws XFlaimException;
	
	private native int _getMaxRflFileSize(
		long				lThis) throws XFlaimException;

	private native void _rflRollToNextFile(
		long				lThis) throws XFlaimException;

	private native void _setKeepAbortedTransInRflFlag(
		long				lThis,
		boolean			bKeep) throws XFlaimException;

	private native boolean _getKeepAbortedTransInRflFlag(
		long				lThis) throws XFlaimException;

	private native void _setAutoTurnOffKeepRflFlag(
		long				lThis,
		boolean			bAutoTurnOff) throws XFlaimException;

	private native boolean _getAutoTurnOffKeepRflFlag(
		long				lThis) throws XFlaimException;

	private native void _setFileExtendSize(
		long				lThis,
		int				iFileExtendSize) throws XFlaimException;

	private native int _getFileExtendSize(
		long				lThis) throws XFlaimException;

	private native int _getDbVersion(
		long				lThis) throws XFlaimException;

	private native int _getBlockSize(
		long				lThis) throws XFlaimException;

	private native int _getDefaultLanguage(
		long				lThis) throws XFlaimException;

	private native long _getTransID(
		long				lThis) throws XFlaimException;

	private native String _getDbControlFileName(
		long				lThis) throws XFlaimException;

	private native long _getLastBackupTransID(
		long				lThis) throws XFlaimException;

	private native int _getBlocksChangedSinceBackup(
		long				lThis) throws XFlaimException;

	private native int _getNextIncBackupSequenceNum(
		long				lThis) throws XFlaimException;

	private native long _getDiskSpaceDataSize(
		long				lThis) throws XFlaimException;

	private native long _getDiskSpaceRollbackSize(
		long				lThis) throws XFlaimException;
		
	private native long _getDiskSpaceRflSize(
		long				lThis) throws XFlaimException;
	
	private native long _getDiskSpaceTotalSize(
		long				lThis) throws XFlaimException;

	private native int _getMustCloseRC(
		long				lThis) throws XFlaimException;

	private native int _getAbortRC(
		long				lThis) throws XFlaimException;

	private native void _setMustAbortTrans(
		long				lThis,
		int				iRc) throws XFlaimException;
		
	private native void _enableEncryption(
		long				lThis) throws XFlaimException;

	private native void _wrapKey(
		long				lThis,
		String			sPassword) throws XFlaimException;
		
	private native void _rollOverDbKey(
		long				lThis) throws XFlaimException;
			
	private native byte[] _getSerialNumber(
		long				lThis) throws XFlaimException;
		
	private native CheckpointInfo _getCheckpointInfo(
		long				lThis) throws XFlaimException;
		
	private native void _exportXML(
		long		lThis,
		long		lStartNode,
		String	sFileName,
		int		iFormat) throws XFlaimException;
			
	private native String _exportXML(
		long		lThis,
		long		lStartNode,
		int		iFormat) throws XFlaimException;
			
	private native LockUser[] _getLockWaiters(
		long		lThis) throws XFlaimException;
		
	private native void _setDeleteStatusObject(
		long				lThis,
		DeleteStatus	deleteStatusObj) throws XFlaimException;
			
	private native void _setIndexingClientObject(
		long				lThis,
		IxClient			ixClientObj) throws XFlaimException;
		
	private native void _setIndexingStatusObject(
		long				lThis,
		IxStatus			ixStatusObj) throws XFlaimException;
		
	private native void _setCommitClientObject(
		long				lThis,
		CommitClient	commitClientObj) throws XFlaimException;
		
	private native void _upgrade(
		long				lThis) throws XFlaimException;
		
	long 					m_this;
	private DbSystem 	m_dbSystem;
}

