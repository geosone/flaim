//-------------------------------------------------------------------------
// Desc:	Query structures and classes.
// Tabs:	3
//
//		Copyright (c) 1994-2006 Novell, Inc. All Rights Reserved.
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
// $Id: fquery.h 12329 2006-01-20 17:49:30 -0700 (Fri, 20 Jan 2006) ahodgkinson $
//-------------------------------------------------------------------------

#ifndef FQUERY_H
#define FQUERY_H

#include "fpackon.h"
// IMPORTANT NOTE: No other include files should follow this one except
// for fpackoff.h

class FSIndexCursor;
class FSDataCursor;
class FDynSearchSet;

#define FLM_ALL_BOOL					(FLM_TRUE | FLM_FALSE | FLM_UNK)

#define FLM_MAX_POS_KEYS			1000
#define FLM_MIN_POS_KEYS			250
#define FLM_ADDR_GROW_SIZE			100
#define FLM_KEYS_GROW_SIZE			100

#define MAX_SIGNED_VAL				2147483647L

// FQNODE status flags

#define FLM_RESOLVE_UNK					0x01
#define FLM_NOTTED						0x02
#define FLM_FOR_EVERY					0x04
#define FLM_RESULT_MASK					0xF00

#define FLM_SET_RESULT(uiStatus,uiResult) \
	((uiStatus) = ((uiStatus) & (~(FLM_RESULT_MASK))) | ((uiResult) << 8))

#define FLM_GET_RESULT(uiStatus) \
	(FLMUINT)(((uiStatus) & (FLM_RESULT_MASK)) >> 8)

/****************************************************************************
Macro:	
Desc:	 	Macros used for determining the nature and precedence of OP codes.
****************************************************************************/


#define GET_QNODE_TYPE( x)   ((x)->eOpType)

#define IS_OP( e)					(( (e) >= FIRST_OP) && ( (e) <= LAST_OP))

#define IS_COMM_ASSOC( e)		(((e) == FLM_AND_OP) || \
										((e) == FLM_OR_OP) || \
										((e) == FLM_MULT_OP) || \
										((e) == FLM_PLUS_OP) || \
										((e) == FLM_BITAND_OP) || \
										((e) == FLM_BITOR_OP) || \
										((e) == FLM_BITXOR_OP))

#define IS_LOG_OP( e)			(( (e) >= FIRST_LOG_OP) && \
										( (e) <= LAST_LOG_OP))

#define IS_COMPARE_OP( e)		((e) >= FIRST_COMPARE_OP && \
										 (e) <= LAST_COMPARE_OP)

#define IS_ARITH_OP( e)			((e) >= FIRST_ARITH_OP && \
										 (e) <= LAST_ARITH_OP)

#define IS_VAL( e)				((e) >= FIRST_VALUE && \
										 (e) <= LAST_VALUE)

#define IS_FIELD( e)				((e) == FLM_FLD_PATH || \
										 (e) == FLM_CB_FLD)

#define IS_FLD_CB(eOp,pQNode)	\
	(((eOp) == FLM_FLD_PATH) && \
	 ((pQNode)->pQAtom->val.QueryFld.fnGetField) && \
	 (!((pQNode)->pQAtom->val.QueryFld.bValidateOnly)))

#define IS_BUF_TYPE( e)			((e) == FLM_TEXT_VAL || \
										 (e) == FLM_BINARY_VAL)

#define IS_UNSIGNED( e)			((e) == FLM_UINT32_VAL)

#define IS_SIGNED( e)			((e) == FLM_INT32_VAL)

#define IS_REAL( e)				((e) == FLM_REAL_VAL)

#define DO_REVERSE( e)			(((e) == FLM_GT_OP) ? FLM_LT_OP : \
										((e) == FLM_LT_OP) ? FLM_GT_OP : \
										((e) == FLM_GE_OP) ? FLM_LE_OP : \
										((e) == FLM_LE_OP) ? FLM_GE_OP : (e))

#define REVERSIBLE( e)			((((e) == FLM_CONTAINS_OP) || \
										((e) == FLM_MATCH_BEGIN_OP)) \
										? FALSE : TRUE)


// NOTE: IS_DRN_FUNC does not include FLM_CURSOR_CURRENT_DRN because it is
// used in places where that particular API is never being executed.

#define IS_DRN_FUNC( x)	(( (x) == FLM_CURSOR_FIRST_DRN) || \
										 ( (x) == FLM_CURSOR_NEXT_DRN) || \
										 ( (x) == FLM_CURSOR_LAST_DRN) || \
										 ( (x) == FLM_CURSOR_PREV_DRN))
										 

#define	FQ_COMPARE(left,right)	( \
			 ((left) == (right)) ? 0 : \
			(((left) < (right)) ?  -1 : 1 ))


/****************************************************************************
Desc:
****************************************************************************/
typedef struct F_QUERY_FLD
{
	FLMUINT *				puiFldPath;		// In child-to-parent order.
	CURSOR_GET_FIELD_CB	fnGetField;
	FLMBOOL					bValidateOnly;
	void *					pvUserData;
	FLMUINT					uiUserDataLen;
} F_QUERY_FLD;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct FQATOM
{
	FQATOM *		pNext;					// Atoms are chained in some areas.
	FlmRecord *	pFieldRec;
	QTYPES		eType;					// From enum qTypes in FLAIM.H.  
												// Describes this atom.  Value from 0
												// to FLM_Q_MAX_TYPES.
	FLMUINT		uiFlags;
	FLMUINT		uiBufLen;				// Length if the type is text or binary
	
	union
	{
		FLMUINT					uiBool;
		FLMUINT					uiVal;
		FLMINT					iVal;
		F_TIME					Time;
		F_DATE					Date;
		F_TMSTAMP				TimeStamp;
		F_QUERY_FLD				QueryFld;
		FlmUserPredicate *	pPredicate;
		FLMBYTE *				pucBuf;
	} val;									// Holds or points to the atom value.
} FQATOM;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct FQNODE
{
	QTYPES		eOpType;					// Type of QueryNode - see enum qTypes
	FLMUINT		uiNestLvl;				// Nesting level of query node.
	FLMUINT		uiStatus;				// Status of node - defs in FLAIM.h
	FQNODE *		pParent;					// Parent of this query node
	FQNODE *		pPrevSib;				// Previous sibling of this query node
	FQNODE *		pNextSib;				// Next sibling of this query node
	FQNODE *		pChild;					// Child of this query node
	FQATOM *		pQAtom;					// Atomic value(s) of this query node
} FQNODE;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct QPREDICATE
{
	FQNODE *			pPredNode;			// Root node of this predicate - unless
												// it is an exists operator or not exists,
												// it will point at the operator node.
												// Otherwise, it will point at the field
												// node.
	FLMUINT *		puiFldPath;			// Field path in child-to-parent order.
												// Points to field path in FQATOM.
	FLMBOOL			bFldSingleValued;	// Field is single valued.
	QTYPES			eOperator;			// Operator of the predicate - comes from
												// pPredNode.
	FLMBOOL			bNotted;				// Has operator been notted?
	FQATOM *			pVal;					// Points to FQATOM that has the value for
												// this predicate.  Will be NULL for unary
												// operators such as exists, not exists, etc.
	FLMBOOL			bEvaluatedNullRec;// Have we tested this against a NULL record
												// yet?
	FLMBOOL			bReturnsTrueOnNullRec;
												// Does this predicate return NULL when
												// evaluated on an empty record?
	QPREDICATE *	pNext;				// Next predicate in the list.
} QPREDICATE;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct QFIELD_PREDICATE
{
	QPREDICATE *			pPredicate;	// Pointer to predicate
	IFD *						pIfd;			// IFD for this predicate's field.
	FLMUINT					uiRank;		// Ranking of this predicate with respect
												// to this IFD.
	QFIELD_PREDICATE *	pNext;		// Next predicate involving same field
} QFIELD_PREDICATE;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct QINDEX
{
	FLMUINT					uiIndexNum;
	FLMUINT					uiNumFields;
	FLMBOOL					bDoRecMatch;
	FLMBOOL					bPredicatesRequireMatch;	// Either record match
																	// or key match is required.
																	// It doesn't matter which.
	FLMBOOL					bMultiplePredsOnIfd;			// Some IFD has multiple
																	// predicates.
	IXD *						pIxd;
	QFIELD_PREDICATE **	ppFieldPredicateList;	// One for each IFD
	FLMUINT					uiNumPredicatesCovered;
	FLMUINT					uiRank;
	QINDEX *					pNext;
	QINDEX *					pPrev;
} QINDEX;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct QTINFO
{
	FQNODE *					pTopNode;
	FQNODE *					pCurOpNode;
	FQNODE *					pCurAtomNode;
	FQNODE *					pSaveQuery;
	FLMUINT					uiNestLvl;		// Number of unclosed left parens
	FLMUINT					uiExpecting;	// Next thing that should be pushed
		#define FLM_Q_PAREN       1
		#define FLM_Q_OPERATOR    2 
		#define FLM_Q_OPERAND     4

	FLMUINT					uiFlags;			// Text flags (case sensitivity,
													// wildcards), Granularity flags
													// (sentence, paragraph, etc),
#define MAX_USER_PREDICATES	4
	FlmUserPredicate *	Predicates [MAX_USER_PREDICATES];
	FlmUserPredicate **	ppPredicates;
	FLMUINT					uiMaxPredicates;
	FLMUINT					uiNumPredicates;
} QTINFO;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct SET_DEL
{
	FLMBYTE *	pKeyBuf;
	FLMUINT		uiKeyBufLen;
	FLMUINT		uiDrn;
	FLMUINT		uiCurrBufSize;
	FLMUINT		uiAttr;
} SET_DEL;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct FQKEY
{
	SET_DEL     FromKey;
	SET_DEL     UntilKey;
	FQKEY *     pNext;
	FQKEY *     pPrev;
} FQKEY;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct POS_KEY
{
	FLMBYTE *	pucKey;
	FLMUINT		uiKeyLen;
	FLMUINT		uiDrn;		// May be domain if not leaf level key.
} POS_KEY;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct SUBQUERY
{
	SUBQUERY *				pNext;
	SUBQUERY *				pPrev;
	FQNODE *					pTree;
	OPT_INFO					OptInfo;
	FSIndexCursor *		pFSIndexCursor;	// Used of OptInfo.eOptType is
														// QOPT_USING_INDEX
	FlmUserPredicate *	pPredicate;			// Used if OptInfo.eOptType is
														// QOPT_USING_PREDICATE.
	FSDataCursor *			pFSDataCursor;		// Used if OptInfo.eOptType is
														// QOPT_PARTIAL_CONTAINER_SCAN or
														// QOPT_FULL_CONTAINER_SCAN.
	FLMBOOL					bRecReturned;		// Used if OptInfo.eOptType is
														// QOPT_SINGLE_RECORD_READ.
	FLMBOOL					bSaveRecReturned;	// Used when saving position for
														// single record reads.
	FLMUINT					uiLowDrn;
	FLMUINT					uiHighDrn;
	FLMUINT					uiNotEqualDrn;
	FLMBOOL					bHaveDrnFlds;			// Part of query involved DRN==x
	FLMUINT					uiLanguage;
	POOL						OptPool;
	FCURSOR_SUBQUERY_STATUS
								SQStatus;
	FlmRecord *				pRec;
	FLMBOOL					bRecIsAKey;
	FLMUINT					uiDrn;
	FLMBOOL					bFirstReference;
	FLMUINT					uiCurrKeyMatch;
} SUBQUERY;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct CURSOR
{
	FDB *						pDb;
	FLMUINT					uiContainer;
	FLMUINT					uiRecType;
	FLMUINT					uiIndexNum;

	// Query tree and subqueries.

	FQNODE *					pTree;
	SUBQUERY *				pSubQueryList;
	SUBQUERY *				pCurrSubQuery;
	SUBQUERY *				pSaveSubQuery;
	FLMBOOL					bInvTrans;
	FLMUINT					uiTransSeq;

	// Positioning keys.

	POS_KEY *				pPosKeyArray;
	FLMUINT					uiNumPosKeys;
	FLMBOOL					bLeafLevel;
	FLMUINT					uiLastPrcntPos;
	FLMUINT					uiLastPrcntOffs;
	FLMBOOL					bUsePrcntPos;

	FDynSearchSet *		pDRNSet;
	FLMBOOL					bEliminateDups;
	QTINFO					QTInfo;
	RCODE						rc;
	POOL						SQPool;
	POOL						QueryPool;
	void *					pOptMark;
	FLMBOOL					bOptimized;
	FLMBOOL					bEmpty;
	FLMBOOL					bOkToReturnKeys;	// OK To return index keys instead of
														// full records.
	REC_VALIDATOR_HOOK	fnRecValidator;	// Record Validator Hook
	void *					RecValData;			// Record Validator User Data
	STATUS_HOOK				fnStatus;
	void *					StatusData;
	FLMUINT					uiLastCBTime;
	FLMUINT					uiLastRecID;
	RCODE       			ReadRc;				// Will only be SUCCESS, FRC_EOF_HIT,
														// or FRC_BOF_HIT
	FLMUINT					uiTimeLimit;
	CS_CONTEXT *			pCSContext;
	FLMUINT					uiCursorId;
} CURSOR;

typedef RCODE FQ_OPERATION(
	FQATOM *	lhs,							// Left hand side
	FQATOM *	rhs,							// Right hand side
	FQATOM *	pResult);					// Newly allocated result side

RCODE flmCurDbInit(
	CURSOR *		pCursor);

void flmSQFree(
	SUBQUERY *	pSubQuery,
	FLMBOOL		bFreeEverything);

void flmCurFinishTrans(			
	CURSOR *		pCursor);
	
RCODE flmCurSavePosition(
	CURSOR *	pCursor);

RCODE flmCurRestorePosition(
	CURSOR *	pCursor);

void flmCurFree(
	CURSOR *		pCursor,
	FLMBOOL		bFinishTrans);

RCODE flmCurSearch(
	eFlmFuncs		eFlmFuncId,
	CURSOR *			pCursor,
	FLMBOOL			bFirstRead,
	FLMBOOL			bReadForward,
	FLMUINT *		puiCount,
	FLMUINT *		puiSkipCount,
	FlmRecord **	ppUserRecord,
	FLMUINT *		puiDrn);

RCODE flmInitCurCS(
	CURSOR *			pCursor);

RCODE flmCurGetAtomVal(
	FlmRecord *		pRecord,
	void *			pField,
	POOL *			pPool,
	QTYPES			eFldType,
	FQATOM *			pResult);

RCODE flmCurGetAtomFromRec(
	FDB *				pDb,
	POOL *			pPool,
	FQATOM *			pTreeAtom,
	FlmRecord *		pRecord,
	QTYPES			eFldType,
	FLMBOOL			bGetAtomVals,
	FQATOM *			pResult,
	FLMBOOL			bHaveKey);

RCODE flmCurEvalCriteria(
	CURSOR *			pCursor,
	SUBQUERY *		pSubQuery,
	FlmRecord *		pRecord,
	FLMBOOL			bHaveKey,
	FLMUINT *		puiResult);

void flmCompareOperands(
	FLMUINT			uiLang,
	FQATOM *			pLhs,
	FQATOM *			pRhs,
	QTYPES			eOp,
	FLMBOOL			bResolveUnknown,
	FLMBOOL			bForEvery,
	FLMBOOL			bNotted,
	FLMBOOL			bHaveKey,
	FLMUINT *		puiTrueFalse);

RCODE flmCurEvalCompareOp(
	FDB *				pDb,
	SUBQUERY *		pSubQuery,
	FlmRecord *		pRecord,
	FQNODE *			pQNode,
	QTYPES			eOp,
	FLMBOOL			bHaveKey,
	FQATOM *			pResult);

RCODE flmCurDoNeg(
	FQATOM *			pResult);

FLMUINT flmCurDoMatchOp(
	FQATOM *			lhs,
	FQATOM *			rhs,
	FLMUINT			uiLang,
	FLMBOOL			bLeadingWildCard,
	FLMBOOL			bTrailingWildCard);

FLMINT flmCurDoRelationalOp(
	FQATOM *			lhs,
	FQATOM *			rhs,
	FLMUINT			uiLang);	

FLMUINT flmCurDoContainsOp(
	FQATOM *			lhs,
	FQATOM *			rhs,
	FLMUINT			uiLang);

RCODE flmCurDoNeg(
	FQATOM *			pResult);

FLMINT flmTextCompare(
	FLMBYTE *		pLeftBuf,
	FLMUINT			uiLeftLen,
	FLMBYTE *		pRightBuf,
	FLMUINT			uiRightLen,
	FLMUINT			uiFlags,
	FLMUINT			uiLang);

FLMUINT flmTextMatch(
	FLMBYTE *		pLeftBuf,
	FLMUINT			uiLeftLen,
	FLMBYTE *		pRightBuf,
	FLMUINT			uiRightLen,
	FLMUINT			uiFlags,
	FLMBOOL			bLeadingWildCard,
	FLMBOOL			bTrailingWildCard,
	FLMUINT			uiLang);

RCODE flmCurMakeKeyFromRec(
	FDB *				pDb,
	IXD *				pIxd,
	POOL *			pPool,
	FlmRecord *		pRec,
	FLMBYTE **		ppucKeyBuffer,
	FLMUINT *		puiKeyLen);

RCODE flmCurSetPosFromDRN(
	CURSOR *			pCursor,
	FLMUINT			uiDRN);

RCODE flmCurCopyQNode(
	FQNODE *			pSrcNode,
	QTINFO *			pDestQTInfo,
	FQNODE *  * 	ppDestNode,
	POOL *			pPool);

RCODE flmCurPrep(
	CURSOR *			pCursor);

void flmCurFreePosKeys(
	CURSOR *			pCursor);

RCODE flmCurSetupPosKeyArray(
	CURSOR *			pCursor);

RCODE flmCurGetPercentPos(
	CURSOR *			pCursor,
	FLMUINT *		puiPrcntPos);
	
RCODE flmCurSetPercentPos(
	CURSOR *			pCursor,
	FLMUINT			uiPrcntPos);
	
RCODE flmSQSetupFullContainerScan(
	SUBQUERY *	pSubQuery);

RCODE flmCurOptimize(
	CURSOR *			pCursor,
	FLMBOOL			bStratified);

RCODE flmCurPartitionTree(
	CURSOR *			pCursor);

RCODE flmCurAddRefPredicate(
	QTINFO *					pQTInfo,
	FlmUserPredicate *	pPredicate);

void flmCurLinkFirstChild(
	FQNODE *			pParent,
	FQNODE *			pChild);

void flmCurLinkLastChild(
	FQNODE *			pParent,
	FQNODE *			pChild);

RCODE flmPutValInAtom(
	void *			pAtom,
	QTYPES			eValType,
	void *			pvVal,
	FLMUINT			uiValLen,
	FLMUINT			uiFlags);

RCODE flmCurMakeQNode(
	POOL *			pPool,
	QTYPES			eType,
	void *			pVal,
	FLMUINT			uiStrLen,
	FLMUINT			uiFlags,
	FQNODE *  *		ppQNode);

RCODE flmCurGraftNode(
	POOL *			pPool,
	FQNODE *			pQNode,
	QTYPES			eGraftOp,
	FQNODE *  *		ppQTree);

void flmLogQuery(
	F_LogMessage *	pLogMsg,
	FLMUINT			uiIndent,
	CURSOR *			pCursor);

FINLINE void flmCurFinishTransactions(
	CURSOR *		pCursor,
	FLMBOOL		bSetToNull)
{
	flmCurFinishTrans( pCursor);
	if (bSetToNull)
	{
		pCursor->pDb = NULL;
	}
}

void flmCurFreeSQList(
	CURSOR *	pCursor,
	FLMBOOL	bFreeEverything);

FLMUINT flmGetPathLen(
	FLMUINT *		pFldPath);

#include "fpackoff.h"

#endif