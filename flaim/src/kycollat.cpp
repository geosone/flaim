//-------------------------------------------------------------------------
// Desc:	FLAIM collation routines and tables
// Tabs:	3
//
//		Copyright (c) 1990-2006 Novell, Inc. All Rights Reserved.
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
// $Id: kycollat.cpp 12313 2006-01-19 15:14:44 -0700 (Thu, 19 Jan 2006) dsanders $
//-------------------------------------------------------------------------

#include "flaimsys.h"

// Collating sequence defines

#define COLLS								32					// first collating number (space/end of line)
#define COLS1								(COLLS+9)		// quotes
#define COLS2								(COLS1+5)		// parens
#define COLS3								(COLS2+6)		// money
#define COLS4								(COLS3+6)		// math ops
#define COLS5								(COLS4+8)		// math others
#define COLS6								(COLS5+14)		// others: %#&@\_|~
#define COLS7								(COLS6+13)		// greek
#define COLS8								(COLS7+25)		// numbers
#define COLS9								(COLS8+10)		// alphabet
#define COLS10								(COLS9+60)		// cyrillic
#define COLS10h							(COLS9+42)		// hebrew - writes over european and cyrilic
#define COLS10a							(COLS10h+28)	// arabic - inclusive from 198(C6)- 252(FC)
#define COLS11								253				// End of list - arabic goes to the end
#define COLS0_ARABIC						COLS11			// Set if arabic accent marking
#define COLS0_HEBREW						COLS11			// Set if hebrew accent marking
#define COLSOEM							254				// OEM character in upper range - non-collatable
#define COLS0_UNICODE					254				// Use this for UNICODE									
#define COLS0								255				// graphics/misc - chars without a collate value

// State table information for double character sorting

#define STATE1 							1
#define STATE2 							2
#define STATE3 							3
#define STATE4 							4
#define STATE5 							5
#define STATE6 							6
#define STATE7 							7
#define STATE8 							8
#define STATE9 							9
#define STATE10 							10
#define STATE11 							11
#define AFTERC 							12
#define AFTERH 							13
#define AFTERL 							14
#define INSTAE 							15
#define INSTOE 							16
#define INSTSG 							17
#define INSTIJ 							18
#define WITHAA 							19

#define START_COL							12
#define START_ALL 						(START_COL + 1)	// all US and European
#define START_DK 							(START_COL + 2)	// Danish
#define START_IS 							(START_COL + 3)	// Icelandic
#define START_NO 							(START_COL + 4)	// Norwegian
#define START_SU 							(START_COL + 5)	// Finnish
#define START_SV 							(START_COL + 5)	// Swedish
#define START_YK 							(START_COL + 6)	// Ukrain
#define START_TK 							(START_COL + 7)	// Turkish
#define START_CZ 							(START_COL + 8)	// Czech
#define START_SL 							(START_COL + 8)	// Slovak

#define FIXUP_AREA_SIZE					24

#define ASCTBLLEN 						95	
#define MNTBLLEN 							219
#define SYMTBLLEN 						9
#define GRKTBLLEN							219
#define CYRLTBLLEN						200
#define HEBTBL1LEN						27
#define HEBTBL2LEN						35
#define AR1TBLLEN   						158
#define AR2TBLLEN							179

#define COMPARE_COLLATION				1
#define COMPARE_COL_AND_SUBCOL		2
#define COMPARE_VALUE					3

#define NULL_SUB_COL_CHECK				NULL
#define NULL_CASE_CHECK					NULL
#define NULL_WILD_CARD_CHECK			NULL

#define MAX_SUBCOL_BUF					500		// (((MAX_KEY_SIZ / 4) * 3 + fluff
#define MAX_LOWUP_BUF	  				150 		// ((MAX_KEY_SIZ - (MAX_KEY_SIZ / 8)) / 8) * 2

#define SET_CASE_BIT						0x01
#define SET_KATAKANA_BIT				0x01
#define SET_WIDTH_BIT					0x02
#define COLS_ASIAN_MARK_VAL			0x40
#define COLS_ASIAN_MARKS				0x140

FSTATIC RCODE KYCmpKeyElmBld(
	FDB *				pDb,
	IXD *				pIxd,
	FLMUINT			uiContainerNum,
	IFD *				pIfd,
	FLMUINT			uiAction,
	FLMUINT			uiDrn,
	FLMBOOL *		pbHadUniqueKeys,
	FLMUINT			uiCdlEntry,
	FLMUINT			uiCompoundPos,
	FLMBYTE *		pKeyBuf,
	FLMUINT			uiKeyLen,
	FLMBYTE *		pLowUpBuf,
	FLMUINT			uiLuLen,
	FlmRecord *		pRecord,
	FLD_CONTEXT *	pFldContext);

FSTATIC RCODE KYFormatText(
	const FLMBYTE *	psVal,
	FLMUINT				uiSrcLen,
	FLMBOOL				bMinSpaces,
	FLMBOOL				bNoUnderscore,
	FLMBOOL				bNoSpace,
	FLMBOOL				bNoDash,
	FLMBOOL				bEscChar,
	FLMBOOL				bInputTruncated,
	FLMBYTE *			psDestBuf,
	FLMUINT *			puiDestLen);

FSTATIC RCODE AsiaFlmTextToColStr(
	const FLMBYTE *	Str,
	FLMUINT 				uiStrLen,
	FLMBYTE *			ColStr,
	FLMUINT *			puiColStrLenRV,
	FLMUINT 				uiUppercaseFlag,
	FLMUINT *			puiCollationLen,
	FLMUINT *			puiCaseLenRV,
	FLMUINT				uiCharLimit,
	FLMBOOL				bFirstSubstring,
	FLMBOOL *			pbDataTruncated);

FSTATIC FLMUINT16	flmTextGetSubCol(
	FLMUINT16			ui16WPValue,
	FLMUINT16			ui16ColValue,
	FLMUINT				uiLangId);

FSTATIC FLMINT	flmTextCompareSingleChar(
	FLMBYTE **			ppLeftText,	
	FLMUINT *			puiLeftLen,	
	FLMUINT *			puiLeftWpChar2,
	FLMBYTE **			ppRightText,
	FLMUINT *			puiRightLen,
	FLMUINT *			puiRightWpChar2,
	FLMINT *				piSubColCompare,
	FLMINT *				piCaseCompare,	
	FLMBOOL *			pbHitWildCard,
	FLMINT				iCompareType,	
	FLMUINT16 *			pui16ColVal,
	FLMUINT				uiFlags,
	FLMUINT				uiLangId);

FSTATIC FLMUINT FWWSGetColStr(
	FLMBYTE *			fColStr,
	FLMUINT * 			fcStrLenRV,
	FLMBYTE *			wordStr,
	FLMUINT				fWPLang,
	FLMBOOL *			pbDataTruncated,
	FLMBOOL *			pbFirstSubstring);

FSTATIC FLMUINT FWWSCmbSubColBuf(
	FLMBYTE *			wordStr,
	FLMUINT *			wdStrLenRV,
	FLMBYTE *			subColBuf,
	FLMBOOL				hebrewArabicFlag);

FSTATIC FLMUINT FWWSToMixed(
	FLMBYTE *			wordStr,
	FLMUINT				wdStrLen,
	FLMBYTE *			lowUpBitStr,
	FLMUINT				fWPLang);

FSTATIC FLMUINT AsiaParseCase(
	FLMBYTE *			WordStr,
	FLMUINT *			uiWordStrLenRV,
	FLMBYTE *			pCaseBits);

FSTATIC RCODE FTextToColStr(
	const FLMBYTE *	pucStr,
	FLMUINT 				uiStrLen,
	FLMBYTE *			pucCollatedStr,
	FLMUINT *			puiCollatedStrLen,
	FLMUINT  			uiUppercaseFlag,
	FLMUINT *			puiCollationLen,
	FLMUINT *			puiCaseLen,
	FLMUINT				uiLanguage,
	FLMUINT				uiCharLimit,
	FLMBOOL				bFirstSubstring,
	FLMBOOL *			pbOriginalCharsLost,
	FLMBOOL *			pbDataTruncated);
	
FSTATIC FLMUINT16 flmAsiaGetCollation(
	FLMUINT16			ui16WpChar,
	FLMUINT16			ui16NextWpChar,
	FLMUINT16			ui16PrevColValue,
	FLMUINT16 *			pui16ColValue,
	FLMUINT16 *			pui16SubColVal,
	FLMBYTE *			pucCaseBits,
	FLMUINT16			uiUppercaseFlag);
	
FSTATIC FLMUINT16 flmCheckDoubleCollation(
	FLMUINT16 *			pui16WpChar,
	FLMBOOL *			pbTwoIntoOne,
	const FLMBYTE **	ppucInputStr,
	FLMUINT				uiLanguage);

FSTATIC FLMUINT AsiaParseSubCol(
	FLMBYTE *			WordStr,
	FLMUINT *			puiWordStrLen,
	FLMBYTE *			SubColBuf);
	
FSTATIC FLMUINT FColStrToText(
	FLMBYTE *			fColStr,
	FLMUINT *			fcStrLenRV,
	FLMBYTE *			textStr,
	FLMUINT	   		fWPLang,
	FLMBYTE *			postBuf,
	FLMUINT *			postBytesRV,
	FLMBOOL *			pbDataTruncated,
	FLMBOOL *			pbFirstSubstring);
	
FSTATIC FLMUINT16 ZenToHankaku(
	FLMUINT16			ui16WpChar,
	FLMUINT16 *			DakutenOrHandakutenRV);
	
FSTATIC FLMUINT16 HanToZenkaku(
	FLMUINT16			ui16WpChar,
	FLMUINT16			ui16NextWpChar,
	FLMUINT16 *			pui16Zenkaku);
	
/****************************************************************************
Desc:
****************************************************************************/
typedef struct BYTE_WORD_TBL
{
	FLMBYTE		ByteValue;
	FLMUINT16	WordValue;
} BYTE_WORD_TBL;

/****************************************************************************
Desc:
****************************************************************************/
typedef struct TBL_B_TO_BP 
{
	FLMBYTE			key;
	FLMBYTE *		charPtr;
} TBL_B_TO_BP;

/****************************************************************************
Desc:
****************************************************************************/
#define BYTES_IN_BITS( bits) \
	((bits + 7) >> 3)

/****************************************************************************
Desc:
****************************************************************************/
#define TEST1BIT( buf, bPos) \
	((((buf)[ (bPos) >> 3]) >> (7 - ((bPos) & 7))) & 1)
	
/****************************************************************************
Desc:
****************************************************************************/
#define GET1BIT( buf, bPos) \
	((((buf)[ (bPos) >> 3]) >> (7 - ((bPos) & 7))) & 1)

/****************************************************************************
Desc:
****************************************************************************/
#define GETnBITS( n, bf, bit) 					\
	(((unsigned int)( 								\
		((unsigned char)bf[ (bit) >> 3] << 8)	\
		| 													\
		(unsigned char)bf[ ((bit) >> 3) + 1]	\
	  ) >> (16 - (n) - ((bit) & 7))				\
	 ) & ((1 << (n)) - 1)				  			\
	)

/****************************************************************************
Desc:
****************************************************************************/
#define SET_BIT( buf, bPos) \
	((buf)[(bPos) >> 3] |=  (FLMBYTE)((1 << (7 - ((bPos) & 7)))))

/****************************************************************************
Desc:
****************************************************************************/
#define RESET_BIT( buf, bPos) \
	((buf)[(bPos) >> 3] &= (FLMBYTE)(~(1 << (7 - ((bPos) & 7)))))

/****************************************************************************
Desc:
****************************************************************************/
#define	SETnBITS( n, bf, bit, v) 			\
{	(bf)[ (bit) >> 3] |= 		  				\
		(FLMBYTE)(((v) << (8 - (n))) 		  	\
		>> 											\
		((bit) & 7)); 				  				\
	(bf)[ ((bit) >> 3) + 1] = 					\
		(FLMBYTE)((v) 								\
		<< 											\
		(16 - (n) - ((bit) & 7)));				\
}
	
/****************************************************************************
Desc:
****************************************************************************/
FLMUINT16 flmValuea[] = 
{
	STATE1,
	STATE3,
	STATE2,
	STATE2,
	STATE8,
	STATE8,
	STATE1,
	STATE3,
	STATE9,
	STATE10,									// No longer in use
	STATE10,									// No longer in use
	STATE4,
	STATE6,
	STATE6,
	STATE5,
	INSTAE,
	INSTOE,
	AFTERC,
	AFTERH,
	AFTERL,
	STATE7,
	STATE6,
	INSTSG,									// ss for German
	INSTIJ,
	STATE11,									// aa - no longer in use
	WITHAA,									// aa - no longer in use
	START_CZ,								// Czech
	START_DK,								// Danish
	START_NO,								// Norwegian
	START_SL,								// Slovak
	START_TK,								// Turkish
	START_SU,								// Finnish
	START_IS,								// Icelandic
	START_SV,								// Swedish
	START_YK,								// Ukrainian
	
	// Single character fixups
	
	COLS9,		COLS9,		COLS9,		COLS9,		// US and others
	COLS9+1,		COLS9+1,		COLS9+21,	COLS9+21,
	COLS9+30,	COLS9+30,	COLS9+21,	COLS9+21,
	COLS10+43,	COLS10+43,	COLS9+12,	COLS9+12,
	COLS9+3,		COLS9+3,		COLS9+25,	COLS9+25,
	COLS9+27,	COLS9+27,	COLS9+35,	COLS9+35,
		
	COLS9+45,	COLS9+45,	COLS9+55,	COLS9+55,	// DANISH
	COLS9+42,	COLS9+42,	COLS9+53,	COLS9+53,
	COLS9+30,	COLS9+30,	COLS9+49,	COLS9+49,
	COLS10+43,	COLS10+43,	COLS9+12,	COLS9+12,
	COLS9+3,		COLS9+3,		COLS9+25,	COLS9+25,
	COLS9+27,	COLS9+27,	COLS9+35,	COLS9+35,

	COLS9,		COLS9,		COLS9,		COLS9,		// Icelandic
	COLS9+46,	COLS9+46,	COLS9+50,	COLS9+50,
	COLS9+30,	COLS9+30,	COLS9+54,	COLS9+54,
	COLS10+43,	COLS10+43,	COLS9+12,	COLS9+12,
	COLS9+3,		COLS9+3,		COLS9+25,	COLS9+25,
	COLS9+27,	COLS9+27,	COLS9+35,	COLS9+35,

	COLS9,		COLS9,		COLS9+51,	COLS9+51,	// Norwegian
	COLS9+43,	COLS9+43,	COLS9+21,	COLS9+21,
	COLS9+30,	COLS9+30,	COLS9+47,	COLS9+47,
	COLS10+43,	COLS10+43,	COLS9+12,	COLS9+12,
	COLS9+3,		COLS9+3,		COLS9+25,	COLS9+25,
	COLS9+27,	COLS9+27,	COLS9+35,	COLS9+35,

	COLS9+48,	COLS9+48,	COLS9+44,	COLS9+44,	// Finnish/Swedish
	COLS9+1,		COLS9+1,		COLS9+52,	COLS9+52,
	COLS9+30,	COLS9+30,	COLS9+21,	COLS9+21,
	COLS10+43,	COLS10+43,	COLS9+12,	COLS9+12,
	COLS9+3,		COLS9+3,		COLS9+25,	COLS9+25,
	COLS9+27,	COLS9+27,	COLS9+35,	COLS9+35,

	COLS9,		COLS9,		COLS9,		COLS9,		// Ukrain
	COLS9+1,		COLS9+1,		COLS9+21,	COLS9+21,
	COLS9+30,	COLS9+30,	COLS9+21,	COLS9+21,
	COLS10+48,	COLS10+48,	COLS9+12,	COLS9+12,
	COLS9+3,		COLS9+3,		COLS9+25,	COLS9+25,
	COLS9+27,	COLS9+27,	COLS9+35,	COLS9+35,

	COLS9,		COLS9,		COLS9,		COLS9,		// Turkish
	COLS9+1,		COLS9+1,		COLS9+21,	COLS9+21,
	COLS9+30,	COLS9+30,	COLS9+21,	COLS9+21,
	COLS9+43,	COLS9+43,	COLS9+11,	COLS9+11,
	COLS9+3,		COLS9+3,		COLS9+25,	COLS9+25,
	COLS9+27,	COLS9+27,	COLS9+35,	COLS9+35,

	COLS9,		COLS9,		COLS9,		COLS9,		// Czech / Slovak
	COLS9+1,		COLS9+1,		COLS9+21,	COLS9+21,
	COLS9+30,	COLS9+30,	COLS9+21,	COLS9+21,
	COLS10+43,	COLS10+43,	COLS9+12,	COLS9+12,
	COLS9+5,		COLS9+5,		COLS9+26,	COLS9+26,
	COLS9+28,	COLS9+28,	COLS9+36,	COLS9+36
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE flmAsc60Tbl[ ASCTBLLEN + 2] =
{
	0x20,										// Initial character offset
	ASCTBLLEN,								// Length of this table
	COLLS,									// <Spc>
	COLLS+5,									// !
	COLS1,									// "
	COLS6+1,									// #
	COLS3,									// $
	COLS6,									// %
	COLS6+2,									//	 &
	COLS1+1,									// '
	COLS2,									// (
	COLS2+1,									// )
	COLS4+2,									// *
	COLS4,									// +
	COLLS+2,									// ,
	COLS4+1,									// -
	COLLS+1,									// .
	COLS4+3,									// /
	COLS8,									// 0
	COLS8+1,									// 1
	COLS8+2,									// 2
	COLS8+3,									// 3
	COLS8+4,									// 4
	COLS8+5,									// 5
	COLS8+6,									// 6
	COLS8+7,									// 7
	COLS8+8,									// 8
	COLS8+9,									// 9
	COLLS+3,									// :
	COLLS+4,									// ;
	COLS5,									// <
	COLS5+2,									// =
	COLS5+4,									// >
	COLLS+7,									// ?
	COLS6+3,									// @
	COLS9,									// A
	COLS9+2,									// B
	COLS9+3,									// C
	COLS9+6,									// D
	COLS9+7,									// E
	COLS9+8,									// F
	COLS9+9,									// G
	COLS9+10,								// H
	COLS9+12,								// I
	COLS9+14,								// J
	COLS9+15,								// K
	COLS9+16,								// L
	COLS9+18,								// M
	COLS9+19,								// N
	COLS9+21,								// O
	COLS9+23,								// P
	COLS9+24,								// Q
	COLS9+25,								// R
	COLS9+27,								// S
	COLS9+29,								// T
	COLS9+30,								// U
	COLS9+31,								// V
	COLS9+32,								// W
	COLS9+33,								// X
	COLS9+34,								// Y
	COLS9+35,								// Z
	COLS9+40,								// [
	COLS6+4,									// backslash
	COLS9+41,								// ]
	COLS4+4,									// ^
	COLS6+5,									// _
	COLS1+2,									// `
	COLS9,									// a
	COLS9+2,									// b
	COLS9+3,									// c
	COLS9+6,									// d
	COLS9+7,									// e
	COLS9+8,									// f
	COLS9+9,									// g
	COLS9+10,								// h
	COLS9+12,								// i
	COLS9+14,								// j
	COLS9+15,								// k
	COLS9+16,								// l
	COLS9+18,								// m
	COLS9+19,								// n
	COLS9+21,								// o
	COLS9+23,								// p
	COLS9+24,								// q
	COLS9+25,								// r
	COLS9+27,								// s
	COLS9+29,								// t
	COLS9+30,								// u
	COLS9+31,								// v
	COLS9+32,								// w
	COLS9+33,								// x
	COLS9+34,								// y
	COLS9+35,								// z
	COLS2+4,									// {
	COLS6+6,									// |
	COLS2+5,									// }
	COLS6+7									// ~
};

/****************************************************************************
Desc:	Multinational table
****************************************************************************/
FLMBYTE flmMn60Tbl[ MNTBLLEN + 2] =
{
	23,										// Initial character offset
	MNTBLLEN,								// Length of this table
	COLS9+27,								// German Double s
	COLS9+15,								// Icelandic k
	COLS9+14,								// Dotless j

	// IBM Charset

	COLS9,									// A Acute
	COLS9,									// a Acute
	COLS9,									// A Circumflex
	COLS9,									// a Circumflex
	COLS9,									// A Diaeresis or Umlaut
	COLS9,									// a Diaeresis or Umlaut
	COLS9,									// A Grave
	COLS9,									// a Grave
	COLS9,									// A Ring
	COLS9,									// a Ring
	COLS9+1,									// AE digraph
	COLS9+1,									// ae digraph
	COLS9+3,									// C Cedilla
	COLS9+3,									// c Cedilla
	COLS9+7,									// E Acute
	COLS9+7,									// e Acute
	COLS9+7,									// E Circumflex
	COLS9+7,									// e Circumflex
	COLS9+7,									// E Diaeresis or Umlaut
	COLS9+7,									// e Diaeresis or Umlaut
	COLS9+7,									// E Grave
	COLS9+7,									// e Grave
	COLS9+12,								// I Acute
	COLS9+12,								// i Acute
	COLS9+12,								// I Circumflex
	COLS9+12,								// i Circumflex
	COLS9+12,								// I Diaeresis or Umlaut
	COLS9+12,								// i Diaeresis or Umlaut
	COLS9+12,								// I Grave
	COLS9+12,								// i Grave
	COLS9+20,								// N Tilde
	COLS9+20,								// n Tilde
	COLS9+21,								// O Acute
	COLS9+21,								// o Acute
	COLS9+21,								// O Circumflex
	COLS9+21,								// o Circumflex
	COLS9+21,								// O Diaeresis or Umlaut
	COLS9+21,								// o Diaeresis or Umlaut
	COLS9+21,								// O Grave
	COLS9+21,								// o Grave
	COLS9+30,								// U Acute
	COLS9+30,								// u Acute
	COLS9+30,								// U Circumflex
	COLS9+30,								// u Circumflex
	COLS9+30,								// U Diaeresis or Umlaut
	COLS9+30,								// u Diaeresis or Umlaut
	COLS9+30,								// U Grave
	COLS9+30,								// u Grave
	COLS9+34,								// Y Diaeresis or Umlaut
	COLS9+34,								// y Diaeresis or Umlaut

	// IBM foreign

	COLS9,									// A Tilde
	COLS9,									// a Tilde
	COLS9+6,									// D Cross Bar
	COLS9+6,									// d Cross Bar
	COLS9+21,								// O Slash
	COLS9+21,								// o Slash
	COLS9+21,								// O Tilde
	COLS9+21,								// o Tilde
	COLS9+34,								// Y Acute
	COLS9+34,								// y Acute
	COLS9+6,									// Uppercase Eth
	COLS9+6,									// Lowercase Eth
	COLS9+37,								// Uppercase Thorn
	COLS9+37,								// Lowercase Thorn

	// Teletex chars

	COLS9,									// A Breve
	COLS9,									// a Breve
	COLS9,									// A Macron
	COLS9,									// a Macron
	COLS9,									// A Ogonek
	COLS9,									// a Ogonek
	COLS9+3,									// C Acute
	COLS9+3,									// c Acute
	COLS9+3,									// C Caron or Hachek
	COLS9+3,									// c Caron or Hachek
	COLS9+3,									// C Circumflex
	COLS9+3,									// c Circumflex
	COLS9+3,									// C Dot Above
	COLS9+3,									// c Dot Above
	COLS9+6,									// D Caron or Hachek (Apostrophe Beside)
	COLS9+6,									// d Caron or Hachek (Apostrophe Beside)
	COLS9+7,									// E Caron or Hachek
	COLS9+7,									// e Caron or Hachek
	COLS9+7,									// E Dot Above
	COLS9+7,									// e Dot Above
	COLS9+7,									// E Macron
	COLS9+7,									// e Macron
	COLS9+7,									// E Ogonek
	COLS9+7,									// e Ogonek
	COLS9+9,									// G Acute
	COLS9+9,									// g Acute
	COLS9+9,									// G Breve
	COLS9+9,									// g Breve
	COLS9+9,									// G Caron or Hachek
	COLS9+9,									// g Caron or Hachek
	COLS9+9,									// G Cedilla (Apostrophe Under)
	COLS9+9,									// g Cedilla (Apostrophe Over)
	COLS9+9,									// G Circumflex
	COLS9+9,									// g Circumflex
	COLS9+9,									// G Dot Above
	COLS9+9,									// g Dot Above
	COLS9+10,								// H Circumflex
	COLS9+10,								// h Circumflex
	COLS9+10,								// H Cross Bar
	COLS9+10,								// h Cross Bar
	COLS9+12,								// I Dot Above (Sharp Accent)
	COLS9+12,								// i Dot Above (Sharp Accent)
	COLS9+12,								// I Macron
	COLS9+12,								// i Macron
	COLS9+12,								// I Ogonek
	COLS9+12,								// i Ogonek
	COLS9+12,								// I Tilde
	COLS9+12,								// i Tilde
	COLS9+13,								// IJ Digraph
	COLS9+13,								// ij Digraph
	COLS9+14,								// J Circumflex
	COLS9+14,								// j Circumflex
	COLS9+15,								// K Cedilla (Apostrophe Under)
	COLS9+15,								// k Cedilla (Apostrophe Under)
	COLS9+16,								// L Acute
	COLS9+16,								// l Acute
	COLS9+16,								// L Caron or Hachek (Apostrophe Beside)
	COLS9+16,								// l Caron or Hachek (Apostrophe Beside)
	COLS9+16,								// L Cedilla (Apostrophe Under)
	COLS9+16,								// l Cedilla (Apostrophe Under)
	COLS9+16,								// L Center Dot
	COLS9+16,								// l Center Dot
	COLS9+16,								// L Stroke
	COLS9+16,								// l Stroke
	COLS9+19,								// N Acute
	COLS9+19,								// n Acute
	COLS9+19,								// N Apostrophe
	COLS9+19,								// n Apostrophe
	COLS9+19,								// N Caron or Hachek
	COLS9+19,								// n Caron or Hachek
	COLS9+19,								// N Cedilla (Apostrophe Under)
	COLS9+19,								// n Cedilla (Apostrophe Under)
	COLS9+21,								// O Double Acute
	COLS9+21,								// o Double Acute
	COLS9+21,								// O Macron
	COLS9+21,								// o Macron
	COLS9+22,								// OE digraph
	COLS9+22,								// oe digraph
	COLS9+25,								// R Acute
	COLS9+25,								// r Acute
	COLS9+25,								// R Caron or Hachek
	COLS9+25,								// r Caron or Hachek
	COLS9+25,								// R Cedilla (Apostrophe Under)
	COLS9+25,								// r Cedilla (Apostrophe Under)
	COLS9+27,								// S Acute
	COLS9+27,								// s Acute
	COLS9+27,								// S Caron or Hachek
	COLS9+27,								// s Caron or Hachek
	COLS9+27,								// S Cedilla
	COLS9+27,								// s Cedilla
	COLS9+27,								// S Circumflex
	COLS9+27,								// s Circumflex
	COLS9+29,								// T Caron or Hachek (Apostrophe Beside)
	COLS9+29,								// t Caron or Hachek (Apostrophe Beside)
	COLS9+29,								// T Cedilla (Apostrophe Under)
	COLS9+29,								// t Cedilla (Apostrophe Under)
	COLS9+29,								// T Cross Bar
	COLS9+29,								// t Cross Bar
	COLS9+30,								// U Breve
	COLS9+30,								// u Breve
	COLS9+30,								// U Double Acute
	COLS9+30,								// u Double Acute
	COLS9+30,								// U Macron
	COLS9+30,								// u Macron
	COLS9+30,								// U Ogonek
	COLS9+30,								// u Ogonek
	COLS9+30,								// U Ring
	COLS9+30,								// u Ring
	COLS9+30,								// U Tilde
	COLS9+30,								// u Tilde
	COLS9+32,								// W Circumflex
	COLS9+32,								// w Circumflex
	COLS9+34,								// Y Circumflex
	COLS9+34,								// y Circumflex
	COLS9+35,								// Z Acute
	COLS9+35,								// z Acute
	COLS9+35,								// Z Caron or Hachek
	COLS9+35,								// z Caron or Hachek
	COLS9+35,								// Z Dot Above
	COLS9+35,								// z Dot Above
	COLS9+19,								// Uppercase Eng
	COLS9+19,								// Lowercase Eng

	// other

	COLS9+6,									// D Macron
	COLS9+6,									// d Macron
	COLS9+16,								// L Macron
	COLS9+16,								// l Macron
	COLS9+19,								// N Macron
	COLS9+19,								// n Macron
	COLS9+25,								// R Grave
	COLS9+25,								// r Grave
	COLS9+27,								// S Macron
	COLS9+27,								// s Macron
	COLS9+29,								// T Macron
	COLS9+29,								// t Macron
	COLS9+34,								// Y Breve
	COLS9+34,								// y Breve
	COLS9+34,								// Y Grave
	COLS9+34,								// y Grave
	COLS9+6,									// D Apostrophe Beside
	COLS9+6,									// d Apostrophe Beside
	COLS9+21,								// O Apostrophe Beside
	COLS9+21,								// o Apostrophe Beside
	COLS9+30,								// U Apostrophe Beside
	COLS9+30,								// u Apostrophe Beside
	COLS9+7,									// E breve
	COLS9+7,									// e breve
	COLS9+12,								// I breve
	COLS9+12,								// i breve
	COLS9+12,								// dotless I
	COLS9+12,								// dotless i
	COLS9+21,								// O breve
	COLS9+21									// o breve
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE flmSym60Tbl[ SYMTBLLEN + 2] = 
{
	11,										// Initial character offset
	SYMTBLLEN,								// Length of this table
	COLS3+2,									// pound
	COLS3+3,									// yen
	COLS3+4,									// pacetes
	COLS3+5,									// floren
	COLS0,	
	COLS0,
	COLS0,
	COLS0,
	COLS3+1,									// cent
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE flmGrk60Tbl[ GRKTBLLEN + 2] = 
{
	0,											// Starting offset
	GRKTBLLEN,								// Length
	COLS7,									// Uppercase Alpha
	COLS7,									// Lowercase Alpha
	COLS7+1,									// Uppercase Beta
	COLS7+1,									// Lowercase Beta
	COLS7+1,									// Uppercase Beta Medial
	COLS7+1,									// Lowercase Beta Medial
	COLS7+2,									// Uppercase Gamma
	COLS7+2,									// Lowercase Gamma
	COLS7+3,									// Uppercase Delta
	COLS7+3,									// Lowercase Delta
	COLS7+4,									// Uppercase Epsilon
	COLS7+4,									// Lowercase Epsilon
	COLS7+5,									// Uppercase Zeta
	COLS7+5,									// Lowercase Zeta
	COLS7+6,									// Uppercase Eta
	COLS7+6,									// Lowercase Eta
	COLS7+7,									// Uppercase Theta
	COLS7+7,									// Lowercase Theta
	COLS7+8,									// Uppercase Iota
	COLS7+8,									// Lowercase Iota
	COLS7+9,									// Uppercase Kappa
	COLS7+9,									// Lowercase Kappa
	COLS7+10,								// Uppercase Lambda
	COLS7+10,								// Lowercase Lambda
	COLS7+11,								// Uppercase Mu
	COLS7+11,								// Lowercase Mu
	COLS7+12,								// Uppercase Nu
	COLS7+12,								// Lowercase Nu
	COLS7+13,								// Uppercase Xi
	COLS7+13,								// Lowercase Xi
	COLS7+14,								// Uppercase Omicron
	COLS7+14,								// Lowercase Omicron
	COLS7+15,								// Uppercase Pi
	COLS7+15,								// Lowercase Pi
	COLS7+16,								// Uppercase Rho
	COLS7+16,								// Lowercase Rho
	COLS7+17,								// Uppercase Sigma
	COLS7+17,								// Lowercase Sigma
	COLS7+17,								// Uppercase Sigma Terminal
	COLS7+17,								// Lowercase Sigma Terminal
	COLS7+18,								// Uppercase Tau
	COLS7+18,								// Lowercase Tau
	COLS7+19,								// Uppercase Upsilon
	COLS7+19,								// Lowercase Upsilon
	COLS7+20,								// Uppercase Phi
	COLS7+20,								// Lowercase Phi
	COLS7+21,								// Uppercase Chi
	COLS7+21,								// Lowercase Chi
	COLS7+22,								// Uppercase Psi
	COLS7+22,								// Lowercase Psi
	COLS7+23,								// Uppercase Omega
	COLS7+23,								// Lowercase Omega

	// Other Modern Greek Characters [8,52]

	COLS7,									// Uppercase ALPHA Tonos high prime
	COLS7,									// Lowercase Alpha Tonos - acute
	COLS7+4,									// Uppercase EPSILON Tonos - high prime
	COLS7+4,									// Lowercase Epslion Tonos - acute
	COLS7+6,									// Uppercase ETA Tonos - high prime
	COLS7+6,									// Lowercase Eta Tonos - acute
	COLS7+8,									// Uppercase IOTA Tonos - high prime
	COLS7+8,									// Lowercase iota Tonos - acute
	COLS7+8,									// Uppercase IOTA Diaeresis
	COLS7+8,									// Lowercase iota diaeresis
	COLS7+14,								// Uppercase OMICRON Tonos - high prime
	COLS7+14,								// Lowercase Omicron Tonos - acute
	COLS7+19,								// Uppercase UPSILON Tonos - high prime
	COLS7+19,								// Lowercase Upsilon Tonos - acute
	COLS7+19,								// Uppercase UPSILON Diaeresis
	COLS7+19,								// Lowercase Upsilon diaeresis
	COLS7+23,								// Uppercase OMEGA Tonos - high prime
	COLS7+23,								// Lowercase Omega Tonso - acute

	// Variants [8,70]

	COLS7+4,									// epsilon (variant)
	COLS7+7,									// theta (variant)
	COLS7+9,									// kappa (variant)
	COLS7+15,								// pi (variant)
	COLS7+16,								// rho (variant)
	COLS7+17,								// sigma (variant)
	COLS7+19,								// upsilon (variant)
	COLS7+20,								// phi (variant)
	COLS7+23,								// omega (variant)
	
	// Greek Diacritic marks [8,79]	

	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,
	COLS0,									// 8,108 end of diacritic marks

	// Ancient Greek [8,109]
	
	COLS7,									// alpha grave
	COLS7,									// alpha circumflex
	COLS7,									// alpha w/iota
	COLS7,									// alpha acute w/iota
	COLS7,									// alpha grave w/iota
	COLS7,									// alpha circumflex w/Iota
	COLS7,									// alpha smooth
	COLS7,									// alpha smooth acute
	COLS7,									// alpha smooth grave
	COLS7,									// alpha smooth circumflex
	COLS7,									// alpha smooth w/Iota
	COLS7,									// alpha smooth acute w/Iota
	COLS7,									// alpha smooth grave w/Iota
	COLS7,									// alpha smooth circumflex w/Iota
	COLS7,									// alpha rough
	COLS7,									// alpha rough acute
	COLS7,									// alpha rough grave
	COLS7,									// alpha rough circumflex
	COLS7,									// alpha rough w/Iota
	COLS7,									// alpha rough acute w/Iota
	COLS7,									// alpha rough grave w/Iota
	COLS7,									// alpha rough circumflex w/Iota
	COLS7+4,									// epsilon grave
	COLS7+4,									// epsilon smooth
	COLS7+4,									// epsilon smooth acute
	COLS7+4,									// epsilon smooth grave
	COLS7+4,									// epsilon rough
	COLS7+4,									// epsilon rough acute
	COLS7+4,									// epsilon rough grave
	COLS7+6,									// eta grave
	COLS7+6,									// eta circumflex
	COLS7+6,									// eta w/iota
	COLS7+6,									// eta acute w/iota
	COLS7+6,									// eta grave w/Iota
	COLS7+6,									// eta circumflex w/Iota
	COLS7+6,									// eta smooth
	COLS7+6,									// eta smooth acute
	COLS7+6,									// eta smooth grave
	COLS7+6,									// eta smooth circumflex
	COLS7+6,									// eta smooth w/Iota
	COLS7+6,									// eta smooth acute w/Iota
	COLS7+6,									// eta smooth grave w/Iota
	COLS7+6,									// eta smooth circumflex w/Iota
	COLS7+6,									// eta rough
	COLS7+6,									// eta rough acute
	COLS7+6,									// eta rough grave
	COLS7+6,									// eta rough circumflex
	COLS7+6,									// eta rough w/Iota
	COLS7+6,									// eta rough acute w/Iota
	COLS7+6,									// eta rough grave w/Iota
	COLS7+6,									// eta rough circumflex w/Iota
	COLS7+8,									// iota grave
	COLS7+8,									// iota circumflex
	COLS7+8,									// iota acute diaeresis
	COLS7+8,									// iota grave diaeresis
	COLS7+8,									// iota smooth
	COLS7+8,									// iota smooth acute
	COLS7+8,									// iota smooth grave
	COLS7+8,									// iota smooth circumflex
	COLS7+8,									// iota rough
	COLS7+8,									// iota rough acute
	COLS7+8,									// iota rough grave
	COLS7+8,									// iota rough circumflex
	COLS7+14,								// omicron grave
	COLS7+14,								// omicron smooth
	COLS7+14,								// omicron smooth acute
	COLS7+14,								// omicron smooth grave
	COLS7+14,								// omicron rough
	COLS7+14,								// omicron rough acute
	COLS7+14,								// omicron rough grave
	COLS7+16,								// rho smooth
	COLS7+16,								// rho rough
	COLS7+19,								// upsilon grave
	COLS7+19,								// upsilon circumflex
	COLS7+19,								// upsilon acute diaeresis
	COLS7+19,								// upsilon grave diaeresis
	COLS7+19,								// upsilon smooth
	COLS7+19,								// upsilon smooth acute
	COLS7+19,								// upsilon smooth grave
	COLS7+19,								// upsilon smooth circumflex
	COLS7+19,								// upsilon rough
	COLS7+19,								// upsilon rough acute
	COLS7+19,								// upsilon rough grave
	COLS7+19,								// upsilon rough circumflex
	COLS7+23,								// omega grave
	COLS7+23,								// omega circumflex
	COLS7+23,								// omega w/Iota
	COLS7+23,								// omega acute w/Iota
	COLS7+23,								// omega grave w/Iota
	COLS7+23,								// omega circumflex w/Iota
	COLS7+23,								// omega smooth
	COLS7+23,								// omega smooth acute
	COLS7+23,								// omega smooth grave
	COLS7+23,								// omega smooth circumflex
	COLS7+23,								// omega smooth w/Iota
	COLS7+23,								// omega smooth acute w/Iota
	COLS7+23,								// omega smooth grave w/Iota
	COLS7+23,								// omega smooth circumflex w/Iota
	COLS7+23,								// omega rough
	COLS7+23,								// omega rough acute
	COLS7+23,								// omega rough grave
	COLS7+23,								// omega rough circumflex
	COLS7+23,								// omega rough w/Iota
	COLS7+23,								// omega rough acute w/Iota
	COLS7+23,								// omega rough grave w/Iota
	COLS7+23,								// omega rough circumflex w/Iota
	COLS7+24,								// Uppercase Stigma--the number 6
	COLS7+24,								// Uppercase Digamma--Obsolete letter used as 6
	COLS7+24,								// Uppercase Koppa--Obsolete letter used as 90
	COLS7+24									// Uppercase Sampi--Obsolete letter used as 900
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE flmCyrl60Tbl[ CYRLTBLLEN + 2] =
{
	0,											// Starting offset
	CYRLTBLLEN,								// Length of table
	COLS10,									// Russian uppercase A
	COLS10,									// Russian lowercase A
	COLS10+1,								// Russian uppercase BE
	COLS10+1,								// Russian lowercase BE
	COLS10+2,								// Russian uppercase VE
	COLS10+2,								// Russian lowercase VE
	COLS10+3,								// Russian uppercase GHE
	COLS10+3,								// Russian lowercase GHE
	COLS10+5,								// Russian uppercase DE
	COLS10+5,								// Russian lowercase DE
	COLS10+8,								// Russian uppercase E
	COLS10+8,								// Russian lowercase E
	COLS10+9,								// Russian lowercase YO
	COLS10+9,								// Russian lowercase YO
	COLS10+11,								// Russian uppercase ZHE
	COLS10+11,								// Russian lowercase ZHE
	COLS10+12,								// Russian uppercase ZE
	COLS10+12,								// Russian lowercase ZE
	COLS10+14,								// Russian uppercase I
	COLS10+14,								// Russian lowercase I
	COLS10+17,								// Russian uppercase SHORT I
	COLS10+17,								// Russian lowercase SHORT I
	COLS10+19,								// Russian uppercase KA
	COLS10+19,								// Russian lowercase KA
	COLS10+20,								// Russian uppercase EL
	COLS10+20,								// Russian lowercase EL
	COLS10+22,								// Russian uppercase EM
	COLS10+22,								// Russian lowercase EM
	COLS10+23,								// Russian uppercase EN
	COLS10+23,								// Russian lowercase EN
	COLS10+25,								// Russian uppercase O
	COLS10+25,								// Russian lowercase O
	COLS10+26,								// Russian uppercase PE
	COLS10+26,								// Russian lowercase PE
	COLS10+27,								// Russian uppercase ER
	COLS10+27,								// Russian lowercase ER
	COLS10+28,								// Russian uppercase ES
	COLS10+28,								// Russian lowercase ES
	COLS10+29,								// Russian uppercase TE
	COLS10+29,								// Russian lowercase TE
	COLS10+32,								// Russian uppercase U
	COLS10+32,								// Russian lowercase U
	COLS10+34,								// Russian uppercase EF
	COLS10+34,								// Russian lowercase EF
	COLS10+35,								// Russian uppercase HA
	COLS10+35,								// Russian lowercase HA
	COLS10+36,								// Russian uppercase TSE
	COLS10+36,								// Russian lowercase TSE
	COLS10+37,								// Russian uppercase CHE
	COLS10+37,								// Russian lowercase CHE
	COLS10+39,								// Russian uppercase SHA
	COLS10+39,								// Russian lowercase SHA
	COLS10+40,								// Russian uppercase SHCHA
	COLS10+40,								// Russian lowercase SHCHA
	COLS10+41,								// Russian lowercase ER (also hard sign)
	COLS10+41,								// Russian lowercase ER (also hard sign)
	COLS10+42,								// Russian lowercase ERY
	COLS10+42,								// Russian lowercase ERY
	COLS10+43,								// Russian lowercase SOFT SIGN
	COLS10+43,								// Russian lowercase SOFT SIGN
	COLS10+45,								// Russian uppercase REVERSE E
	COLS10+45,								// Russian lowercase REVERSE E
	COLS10+46,								// Russian uppercase YU
	COLS10+46,								// Russian lowercase yu
	COLS10+47,								// Russian uppercase YA
	COLS10+47,								// Russian lowercase ya
	COLS0,									// Russian uppercase EH
	COLS0,									// Russian lowercase eh
	COLS10+7,								// Macedonian uppercase SOFT DJ
	COLS10+7,								// Macedonian lowercase soft dj
	COLS10+4,								// Ukrainian uppercase HARD G
	COLS10+4,								// Ukrainian lowercase hard g
	COLS0,									// GE bar
	COLS0,									// ge bar
	COLS10+6,								// Serbian uppercase SOFT DJ
	COLS10+6,								// Serbian lowercase SOFT DJ
	COLS0,									// IE (variant)
	COLS0,									// ie (variant)
	COLS10+10,								// Ukrainian uppercase YE
	COLS10+10,								// Ukrainian lowercase YE
	COLS0,									// ZHE with right descender
	COLS0,									// zhe with right descender
	COLS10+13,								// Macedonian uppercase ZELO
	COLS10+13,								// Macedonian lowercase ZELO
	COLS0,									// Old Slovanic uppercase Z
	COLS0,									// Old Slovanic uppercase z
	COLS0,									// II with macron
	COLS0,									// ii with mscron
	COLS10+15,								// Ukrainian uppercase I
	COLS10+15,								// Ukrainian lowercase I
	COLS10+16,								// Ukrainian uppercase I with Two Dots
	COLS10+16,								// Ukrainian lowercase I with Two Dots
	COLS0,									// Old Slovanic uppercase I ligature
	COLS0,									// Old Slovanic lowercase I ligature
	COLS10+18,								// Serbian--Macedonian uppercase JE
	COLS10+18,								// Serbian--Macedonian lowercase JE
	COLS10+31,								// Macedonian uppercase SOFT K
	COLS10+31,								// Macedonian lowercase SOFT K
	COLS0,									// KA with right descender
	COLS0,									// ka with right descender
	COLS0,									// KA ogonek
	COLS0,									// ka ogonek
	COLS0,									// KA vertical bar
	COLS0,									// ka vertical bar
	COLS10+21,								// Serbian--Macedonian uppercase SOFT L
	COLS10+21,								// Serbian--Macedonian lowercase SOFT L
	COLS0,									// EN with right descender
	COLS0,									// en with right descender
	COLS10+24,								// Serbian--Macedonian uppercase SOFT N
	COLS10+24,								// Serbian--Macedonian lowercase SOFT N
	COLS0,									// ROUND OMEGA
	COLS0,									// round omega
	COLS0,									// OMEGA
	COLS0,									// omega
	COLS10+30,								// Serbian uppercase SOFT T
	COLS10+30,								// Serbian lowercase SOFT T
	COLS10+33,								// Byelorussian uppercase SHORT U
	COLS10+33,								// Byelorussian lowercase SHORT U
	COLS0,									// U with macron
	COLS0,									// u with macron
	COLS0,									// STRAIGHT U
	COLS0,									// straight u
	COLS0,									// STRAIGHT U bar
	COLS0,									// straight u bar
	COLS0,									// OU ligature
	COLS0,									// ou ligature
	COLS0,									// KHA with right descender
	COLS0,									// kha with right descender
	COLS0,									// KHA ogonek
	COLS0,									// kha ogonek
	COLS0,									// H
	COLS0,									// h
	COLS0,									// OMEGA titlo
	COLS0,									// omega titlo
	COLS10+38,								// Serbian uppercase HARD DJ
	COLS10+38,								// Serbian lowercase HARD DJ
	COLS0,									// CHE with right descender
	COLS0,									// che with right descender
	COLS0,									// CHE vertical bar
	COLS0,									// che vertical bar
	COLS0,									// Old Slavonic SHCHA (variant)
	COLS0,									// old SLAVONIC shcha (variant)
	COLS10+44,								// Old Russian uppercase YAT
	COLS10+44,								// Old Russian lowercase YAT

	// END OF UNIQUE COLLATED BYTES 
	// CHARACTERS BELOW MUST HAVE HAVE THEIR OWN
	// SUB-COLLATION VALUE TO COMPARE CORRECTLY.
	
	COLS0,									// Old Bulgarian uppercase YUS
	COLS0,									// Old Bulgarian lowercase YUS
	COLS0,									// Old Slovanic uppercase YUS MALYI
	COLS0,									// Old Slovanic uppercase YUS MALYI
	COLS0,									// KSI
	COLS0,									// ksi
	COLS0,									// PSI
	COLS0,									// psi
	COLS0,									// Old Russian uppercase FITA
	COLS0,									// Old Russian lowercase FITA
	COLS0,									// Old Russian uppercase IZHITSA
	COLS0,									// Old Russian lowercase IZHITSA
	COLS0,									// Russian uppercase A acute
	COLS0,									// Russian lowercase A acute
	COLS10+8,								// Russian uppercase E acute
	COLS10+8,								// Russian lowercase E acute
	COLS0,									// E acute
	COLS0,									// e acute
	COLS10+14,								// II acute
	COLS10+14,								// ii acute
	COLS0,									// I acute
	COLS0,									// i acute
	COLS0,									// YI acute
	COLS0,									// yi acute
	COLS10+25,								// O acute
	COLS10+25,								// o acute
	COLS10+32,								// U acute
	COLS10+32,								// u acute
	COLS10+42,								// YERI acute
	COLS10+42,								// YERI acute
	COLS10+45,								// REVERSED E acute
	COLS10+45,								// reversed e acute
	COLS10+46,								// YU acute
	COLS10+46,								// yu acute
	COLS10+47,								// YA acute
	COLS10+47,								// ya acute
	COLS10,									// A grave
	COLS10,									// a grave
	COLS10+8,								// E grave
	COLS10+8,								// e grave
	COLS10+9,								// YO grave
	COLS10+9,								// yo grave
	COLS10+14,								// I grave
	COLS10+14,								// i grave
	COLS10+25,								// O grave
	COLS10+25,								// o grave
	COLS10+32,								// U grave
	COLS10+32,								// u grave
	COLS10+42,								// YERI grave
	COLS10+42,								// yeri grave
	COLS10+45,								// REVERSED E grave
	COLS10+45,								// reversed e grave
	COLS10+46,								// IU (YU) grave
	COLS10+46,								// iu (yu) grave
	COLS10+47,								// ia (YA) grave
	COLS10+47,								// ia (ya) grave ******* [10,199]
};

/****************************************************************************
Desc:	The Hebrew characters are collated over the Russian characters.
		Therefore sorting both Hebrew and Russian is impossible to do.
****************************************************************************/
FLMBYTE flmHeb60TblA[ HEBTBL1LEN + 2] = 
{
	0,											// Starting offset
	HEBTBL1LEN,								// Length of table
	COLS10h+0,								// Alef
	COLS10h+1,								// Bet
	COLS10h+2,								// Gimel
	COLS10h+3,								// Dalet
	COLS10h+4,								// He
	COLS10h+5,								// Vav
	COLS10h+6,								// Zayin
	COLS10h+7,								// Het
	COLS10h+8,								// Tet
	COLS10h+9,								// Yod
	COLS10h+10,								// Kaf (final) [9,10]
	COLS10h+11,								// Kaf
	COLS10h+12,								// Lamed
	COLS10h+13,								// Mem (final)
	COLS10h+14,								// Mem
	COLS10h+15,								// Nun (final)
	COLS10h+16,								// Nun
	COLS10h+17,								// Samekh
	COLS10h+18,								// Ayin
	COLS10h+19,								// Pe (final)
	COLS10h+20,								// Pe [9,20]
	COLS10h+21,								// Tsadi (final)
	COLS10h+22,								// Tsadi 
	COLS10h+23,								// Qof
	COLS10h+24,								// Resh
	COLS10h+25,								// Shin
	COLS10h+26								// Tav [9,26]
};

/****************************************************************************
Desc:	This is the ANCIENT HEBREW SCRIPT piece.  The actual value will be
		stored in the subcollation.  This way we don't play 
		diacritic/subcollation games.
****************************************************************************/
FLMBYTE flmHeb60TblB[ HEBTBL2LEN + 2] =
{
	84,				
	HEBTBL2LEN,		
	COLS10h+0,								// Alef Dagesh [9,84]
	COLS10h+1,								// Bet Dagesh
	COLS10h+1,								// Vez - looks like a bet
	COLS10h+2,								// Gimel Dagesh
	COLS10h+3,								// Dalet Dagesh
	COLS10h+4,								// He Dagesh
	COLS10h+5,								// Vav Dagesh [9,90]
	COLS10h+5,								// Vav Holem
	COLS10h+6,								// Zayin Dagesh
	COLS10h+7,								// Het Dagesh
	COLS10h+8,								// Tet Dagesh
	COLS10h+9,								// Yod Dagesh 
	COLS10h+9,								// Yod Hiriq [9,96] - not on my list
	COLS10h+11,								// Kaf Dagesh
	COLS10h+10,								// Kaf Dagesh (final)
	COLS10h+10,								// Kaf Sheva (final) 
	COLS10h+10,								// Kaf Tsere (final) [9,100]
	COLS10h+10,								// Kaf Segol (final) 
	COLS10h+10,								// Kaf Patah (final) 
	COLS10h+10,								// Kaf Qamats (final)
	COLS10h+10,								// Kaf Dagesh Qamats (final)
	COLS10h+12,								// Lamed Dagesh
	COLS10h+14,								// Mem Dagesh
	COLS10h+16,								// Nun Dagesh
	COLS10h+15,								// Nun Qamats (final)
	COLS10h+17,								// Samekh Dagesh 
	COLS10h+20,								// Pe Dagesh [9,110]
	COLS10h+20,								// Fe - just guessing this is like Pe - was +21	
	COLS10h+22,								// Tsadi Dagesh
	COLS10h+23,								// Qof Dagesh
	COLS10h+25,								// Sin (with sin dot)
	COLS10h+25,								// Sin Dagesh (with sin dot)
	COLS10h+25,								// Shin
	COLS10h+25,								// Shin Dagesh
	COLS10h+26								// Tav Dagesh [9,118]
};

/****************************************************************************
Desc:	The Arabic characters are collated OVER the Russian characters
		Therefore sorting both Arabic and Russian in the same database
		is not supported.

		Arabic starts with a bunch of accents/diacritic marks that are
		Actually placed OVER a preceeding character.  These accents are
		ignored while sorting the first pass - when collation == COLS0.

		There are 4 possible states for all/most arabic characters:
			�� - occurs as the only character in a word 
			�� - appears at the first of the word
			�� - appears at the middle of a word
			�� - appears at the end of the word

		Usually only the simple version of the letter is stored.
		Therefore we should not have to worry about sub-collation
		of these characters.

		The arabic characters with diacritics differ however.  The alef has
		sub-collation values to sort correctly.  There is not any more room
		to add more collation values.  Some chars in CS14 are combined when
		urdu, pashto and sindhi characters overlap.
****************************************************************************/
FLMBYTE flmAr160Tbl[ AR1TBLLEN + 2] =
{
	38,										// Starting offset
	AR1TBLLEN,								// Length of table
	COLLS+2,									// , comma
	COLLS+3,									// : colon
	COLLS+7,									// ? question mark
	COLS4+2,									// * asterick
	COLS6,									// % percent
	COLS9+41,								// >> alphabetic - end of list)
	COLS9+40,								// << alphabetic - end of list)
	COLS2,									// ( 
	COLS2+1,									// ) 
	COLS8+1,									// �� One 
	COLS8+2,									// �� Two 
	COLS8+3,									// �� Three
	COLS8+4,									// �� Four
	COLS8+5,									// �� Five
	COLS8+6,									// �� Six 
	COLS8+7,									// �� Seven
	COLS8+8,									// �� Eight
	COLS8+9,									// �� Nine
	COLS8+0,									// �� Zero
	COLS8+2,									// �� Two (Handwritten)
	COLS10a+1,								// �� alif
	COLS10a+1,								// �� alif
	COLS10a+2,								// �� ba
	COLS10a+2,								// �� ba
	COLS10a+2,								// �� ba
	COLS10a+2,								// �� ba
	COLS10a+6,								// �� ta
	COLS10a+6,								// �� ta
	COLS10a+6,								// �� ta
	COLS10a+6,								// �� ta
	COLS10a+8,								// �� tha
	COLS10a+8,								// �� tha
	COLS10a+8,								// �� tha
	COLS10a+8,								// �� tha
	COLS10a+12,								// �� jiim
	COLS10a+12,								// �� jiim
	COLS10a+12,								// �� jiim
	COLS10a+12,								// �� jiim
	COLS10a+16,								// �� Ha
	COLS10a+16,								// �� Ha
	COLS10a+16,								// �� Ha
	COLS10a+16,								// �� Ha
	COLS10a+17,								// �� kha
	COLS10a+17,								// �� kha
	COLS10a+17,								// �� kha
	COLS10a+17,								// �� kha
	COLS10a+20,								// �� dal
	COLS10a+20,								// �� dal
	COLS10a+22,								// �� dhal
	COLS10a+22,								// �� dhal
	COLS10a+27,								// �� ra
	COLS10a+27,								// �� ra
	COLS10a+29,								// �� ziin
	COLS10a+29,								// �� ziin
	COLS10a+31,								// �� siin
	COLS10a+31,								// �� siin
	COLS10a+31,								// �� siin
	COLS10a+31,								// �� siin
	COLS10a+32,								// �� shiin
	COLS10a+32,								// �� shiin
	COLS10a+32,								// �� shiin
	COLS10a+32,								// �� shiin
	COLS10a+34,								// �� Sad
	COLS10a+34,								// �� Sad
	COLS10a+34,								// �� Sad
	COLS10a+34,								// �� Sad
	COLS10a+35,								// �� Dad
	COLS10a+35,								// �� Dad
	COLS10a+35,								// �� Dad
	COLS10a+35,								// �� Dad
	COLS10a+36,								// �� Ta
	COLS10a+36,								// �� Ta
	COLS10a+36,								// �� Ta
	COLS10a+36,								// �� Ta
	COLS10a+37,								// �� Za
	COLS10a+37,								// �� Za
	COLS10a+37,								// �� Za
	COLS10a+37,								// �� Za
	COLS10a+38,								// �� 'ain
	COLS10a+38,								// �� 'ain
	COLS10a+38,								// �� 'ain
	COLS10a+38,								// �� 'ain
	COLS10a+39,								// �� ghain
	COLS10a+39,								// �� ghain
	COLS10a+39,								// �� ghain
	COLS10a+39,								// �� ghain
	COLS10a+40,								// �� fa  
	COLS10a+40,								// �� fa
	COLS10a+40,								// �� fa
	COLS10a+40,								// �� fa
	COLS10a+42,								// �� Qaf
	COLS10a+42,								// �� Qaf
	COLS10a+42,								// �� Qaf
	COLS10a+42,								// �� Qaf
	COLS10a+43,								// �� kaf
	COLS10a+43,								// �� kaf
	COLS10a+43,								// �� kaf
	COLS10a+43,								// �� kaf
	COLS10a+46,								// �� lam
	COLS10a+46,								// �� lam
	COLS10a+46,								// �� lam
	COLS10a+46,								// �� lam
	COLS10a+47,								// �� miim
	COLS10a+47,								// �� miim
	COLS10a+47,								// �� miim
	COLS10a+47,								// �� miim
	COLS10a+48,								// �� nuun
	COLS10a+48,								// �� nuun
	COLS10a+48,								// �� nuun
	COLS10a+48,								// �� nuun
	COLS10a+49,								// �� ha
	COLS10a+49,								// �� ha
	COLS10a+49,								// �� ha
	COLS10a+49,								// �� ha
	COLS10a+6, 								// �� ta marbuuTah
	COLS10a+6, 								// �� ta marbuuTah
	COLS10a+50,								// �� waw
	COLS10a+50,								// �� waw
	COLS10a+53,								// �� ya
	COLS10a+53,								// �� ya
	COLS10a+53,								// �� ya
	COLS10a+53,								// �� ya
	COLS10a+52,								// �� alif maqSuurah
	COLS10a+52,								// �� ya   maqSuurah?
	COLS10a+52,								// �� ya   maqSuurah?
	COLS10a+52,								// �� alif maqSuurah
	COLS10a+0,								// �� hamzah accent - never appears alone

	// Store the sub-collation as the actual
	// character value from this point on
	
	COLS10a+1,								// �� alif hamzah
	COLS10a+1,								// �� alif hamzah
	COLS10a+1,								// �� hamzah-under-alif
	COLS10a+1,								// �� hamzah-under-alif
	COLS10a+1,								// �� waw hamzah
	COLS10a+1,								// �� waw hamzah
	COLS10a+1,								// �� ya hamzah
	COLS10a+1,								// �� ya hamzah
	COLS10a+1,								// �� ya hamzah 
	COLS10a+1,								// �� ya hamzah
	COLS10a+1,								// �� alif fatHataan
	COLS10a+1,								// �� alif fatHataan
	COLS10a+1,								// �� alif maddah
	COLS10a+1,								// �� alif maddah
	COLS10a+1,								// �� alif waSlah
	COLS10a+1,								// �� alif waSlah (final)
	
	// LIGATURES
	//		Should NEVER be stored so will not worry
	//		about breaking up into pieces for collation.
	// NOTE:
	// 	Let's store the "Lam" collation value (+42)
	//		below and in the sub-collation store the
	// 	actual character.  This will sort real close.
	// 	The best implementation is to 
	// 	break up ligatures into its base pieces.
	
	COLS10a+46,								// �� lamalif
	COLS10a+46,								// �� lamalif
	COLS10a+46,								// �� lamalif hamzah
	COLS10a+46,								// �� lamalif hamzah
	COLS10a+46,								// �� hamzah-under-lamalif
	COLS10a+46,								// �� hamzah-under-lamalif
	COLS10a+46,								// �� lamalif fatHataan
	COLS10a+46,								// �� lamalif fatHataan
	COLS10a+46,								// �� lamalif maddah
	COLS10a+46,								// �� lamalif maddah
	COLS10a+46,								// �� lamalif waSlah
	COLS10a+46,								// �� lamalif waSlah
	COLS10a+46,								// �� Allah - khaDalAlif
	COLS0_ARABIC,							// �� taTwiil
	COLS0_ARABIC							// �� taTwiil
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE flmAlefSubColTbl[] = 
{
	1,											// �� alif hamzah
	1,											// �� alif hamzah
	3,											// �� hamzah-under-alif
	3,											// �� hamzah-under-alif
	2,											// �� waw hamzah
	2,											// �� waw hamzah
	4,											// �� ya hamzah
	4,											// �� ya hamzah
	4,											// �� ya hamzah
	4,											// �� ya hamzah
	5,											// �� alif fatHataan
	5,											// �� alif fatHataan
	0,											// �� alif maddah
	0,											// �� alif maddah
	6,											// �� alif waSlah
	6											// �� alif waSlah (final)
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE flmAr260Tbl[ AR2TBLLEN + 2] =
{
	41,										// Starting offset
	AR2TBLLEN,								// Length of table
	COLS8+4,									// Farsi and Urdu Four
	COLS8+4,									// Urdu Four
	COLS8+5,									// Farsi and Urdu Five
	COLS8+6,									// Farsi Six
	COLS8+6,									// Farsi and Urdu Six
	COLS8+7,									// Urdu Seven
	COLS8+8,									// Urdu Eight
	COLS10a+3,								// Sindhi bb - baa /w 2 dots below (67b)
	COLS10a+3,
	COLS10a+3,
	COLS10a+3,
	COLS10a+4,								// Sindhi bh - baa /w 4 dots below (680)
	COLS10a+4,
	COLS10a+4,
	COLS10a+4,
	COLS10a+5,								// Malay, Kurdish, Pashto, Farsi, Sindhi, and Urdu p
	COLS10a+5,								// =peh - taa /w 3 dots below (67e)
	COLS10a+5,
	COLS10a+5,
	COLS10a+7,								// Urdu T - taa /w small tah
	COLS10a+7,
	COLS10a+7,
	COLS10a+7,
	COLS10a+7,								// Pashto T - taa /w ring (forced to combine)
	COLS10a+7,
	COLS10a+7,
	COLS10a+7,
	COLS10a+9,								// Sindhi th - taa /w 4 dots above (67f)
	COLS10a+9,
	COLS10a+9,
	COLS10a+9,
	COLS10a+10,								// Sindhi Tr - taa /w 3 dots above (67d)
	COLS10a+10,
	COLS10a+10,
	COLS10a+10,
	COLS10a+11,								// Sindhi Th - taa /w 2 dots above (67a)
	COLS10a+11,
	COLS10a+11,
	COLS10a+11,
	COLS10a+13,								// Sindhi jj - haa /w 2 middle dots verticle (684)
	COLS10a+13,		
	COLS10a+13,		
	COLS10a+13,		
	COLS10a+14,								// Sindhi ny - haa /w 2 middle dots (683)
	COLS10a+14,		
	COLS10a+14,		
	COLS10a+14,		
	COLS10a+15,								// Malay, Kurdish, Pashto, Farsi, Sindhi, and Urdu ch
	COLS10a+15,								// =tcheh (686)
	COLS10a+15,		
	COLS10a+15,		
	COLS10a+15,								// Sindhi chh - haa /w middle 4 dots (687)
	COLS10a+15,								// forced to combine
	COLS10a+15,		
	COLS10a+15,		
	COLS10a+18,								// Pashto ts - haa /w 3 dots above (685)
	COLS10a+18,		
	COLS10a+18,		
	COLS10a+18,		
	COLS10a+19,								// Pashto dz - hamzah on haa (681)
	COLS10a+19,		
	COLS10a+19,		
	COLS10a+19,		
	COLS10a+21,								// Urdu D - dal /w small tah (688)
	COLS10a+21,
	COLS10a+21,								// Pashto D - dal /w ring (689) forced to combine
	COLS10a+21,		
	COLS10a+23,								// Sindhi dh - dal /w 2 dots above (68c)
	COLS10a+23,		
	COLS10a+24,								// Sindhi D - dal /w 3 dots above (68e)
	COLS10a+24,		
	COLS10a+25,								// Sindhi Dr - dal /w dot below (68a)
	COLS10a+25,		
	COLS10a+26,								// Sindhi Dh - dal /w 2 dots below (68d)
	COLS10a+26,		
	COLS10a+28,								// Pashto r - ra /w ring (693)
	COLS10a+28,		
	COLS10a+28,								// Urdu R - ra /w small tah (691) forced to combine
	COLS10a+28,
	COLS10a+28,								// Sindhi r - ra /w 4 dots above (699) forced to combine
	COLS10a+28,		
	COLS10a+27,								// Kurdish rolled r - ra /w 'v' below (695)
	COLS10a+27,		
	COLS10a+27,		
	COLS10a+27,		
	COLS10a+30,								// Kurdish, Pashto, Farsi, Sindhi, and Urdu Z
	COLS10a+30,								// = jeh - ra /w 3 dots above (698)
	COLS10a+30,								// Pashto zz - ra /w dot below & dot above (696)
	COLS10a+30,								// forced to combine
	COLS10a+30,								// Pashto g - not in unicode! - forced to combine
	COLS10a+30,		
	COLS10a+33,								// Pashto x - seen dot below & above (69a)
	COLS10a+33,		
	COLS10a+33,		
	COLS10a+33,		
	COLS10a+39,								// Malay ng - old maly ain /w 3 dots above (6a0)
	COLS10a+39,								// forced to combine
	COLS10a+39,		
	COLS10a+39,		
	COLS10a+41,								// Malay p, Kurdish v - Farsi ? - fa /w 3 dots above
	COLS10a+41,								// = veh - means foreign words (6a4)
	COLS10a+41,		
	COLS10a+41,		
	COLS10a+41,								// Sindhi ph - fa /w 4 dots above (6a6) forced to combine
	COLS10a+41,		
	COLS10a+41,		
	COLS10a+41,		
	COLS10a+43,								// Misc k - open caf (6a9)
	COLS10a+43,
	COLS10a+43,
	COLS10a+43,
	COLS10a+43,								// misc k - no unicode - forced to combine
	COLS10a+43,
	COLS10a+43,
	COLS10a+43,
	COLS10a+43,								// Sindhi k - swash caf (various) (6aa) -forced to combine
	COLS10a+43,		
	COLS10a+43,		
	COLS10a+43,		
	COLS10a+44,								// Persian/Urdu g - gaf (6af)
	COLS10a+44,
	COLS10a+44,
	COLS10a+44,
	COLS10a+44,								// Persian/Urdu g - no unicode
	COLS10a+44,
	COLS10a+44,
	COLS10a+44,
	COLS10a+44,								// malay g - gaf /w ring (6b0)
	COLS10a+44,		
	COLS10a+44,		
	COLS10a+44,		
	COLS10a+44,								// Sindhi ng  - gaf /w 2 dots above (6ba)
	COLS10a+44,								// forced to combine ng only
	COLS10a+44,		
	COLS10a+44,		
	COLS10a+45,								// Sindhi gg - gaf /w 2 dots vertical below (6b3)
	COLS10a+45,		
	COLS10a+45,		
	COLS10a+45,		
	COLS10a+46,								// Kurdish velar l - lam /w small v (6b5)
	COLS10a+46,		
	COLS10a+46,		
	COLS10a+46,		
	COLS10a+46,								// Kurdish Lamalif with diacritic - no unicode
	COLS10a+46,
	COLS10a+48,								// Urdu n - dotless noon (6ba)
	COLS10a+48,
	COLS10a+48,
	COLS10a+48,
	COLS10a+48,								// Pashto N - noon /w ring (6bc) - forced to combine
	COLS10a+48,		
	COLS10a+48,		
	COLS10a+48,		
	COLS10a+48,								// Sindhi N - dotless noon/w small tah (6bb)
	COLS10a+48,								// forced to combine
	COLS10a+48,		
	COLS10a+48,		
	COLS10a+50,								// Kurdish o - waw /w small v (6c6)
	COLS10a+50,		
	COLS10a+50,								// Kurdish o - waw /w bar above (6c5)
	COLS10a+50,		
	COLS10a+50,								// Kurdish o - waw /w 2 dots above (6ca)
	COLS10a+50,		
	COLS10a+51,								// Urdu h - no unicode
	COLS10a+51,
	COLS10a+51,
	COLS10a+51,
	COLS10a+52,								// Kurdish � - ya /w small v (6ce)
	COLS10a+52,		
	COLS10a+52,		
	COLS10a+52,		
	COLS10a+54,								// Urdu y - ya barree (6d2)
	COLS10a+54,		
	COLS10a+54,								// Malay ny - ya /w 3 dots below (6d1) forced to combine
	COLS10a+54,		
	COLS10a+54,		
	COLS10a+54,		
	COLS10a+51,								// Farsi hamzah - hamzah on ha (6c0) forced to combine
	COLS10a+51
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE flmAr2BitTbl[] = 
{	
	0xF0,										// 64..71
	0x00,										// 72..79
	0x00,										// 80..87
	0x0F,										// 88..95 - 92..95
	0x00,										// 96..103
	0x00,										// 104..111
	0x03,										// 112..119
	0xFC,										// 120..127
	0xFF,										// 128..135
	0xF0,										// 136..143 - 136..139
	0xFF,										// 144..151 - 144..147, 148..159
	0xFF,										// 152..159
	0x0F,										// 160..167 - 164..175
	0xFF,										// 168..175
	0x0F,										// 176..183 - 180..185
	0xFF,										// 184..191 - 186..197
	0xFF,										// 192..199 - 198..203
	0xFF,										// 200..207 - 204..207
	0xF3,										// 208..215 - 208..211 , 214..217
	0xF0										// 216..219 - 218..219
};

/****************************************************************************
Desc:	This table describes and gives addresses for collating
		character sets.  Each line corresponds to a character set.
****************************************************************************/
TBL_B_TO_BP flmCol60Tbl[] = 
{
	{CHSASCI, flmAsc60Tbl},				// ascii - " " - "~"
	{CHSMUL1, flmMn60Tbl},				// multinational
	{CHSSYM1, flmSym60Tbl},				// symbols
	{CHSGREK, flmGrk60Tbl},				// Greek
	{CHSCYR,  flmCyrl60Tbl},			// Cyrillic - Russian
	{0xFF, 	 0}							// table terminator
};

/****************************************************************************
Desc:
****************************************************************************/
TBL_B_TO_BP flmHebArabicCol60Tbl[] = 
{
	{CHSASCI, 	flmAsc60Tbl},			// ascii - " " - "~"
	{CHSMUL1, 	flmMn60Tbl},			// multinational
	{CHSSYM1, 	flmSym60Tbl},			// symbols
	{CHSGREK, 	flmGrk60Tbl},			// Greek
	{CHSHEB,		flmHeb60TblA},			// Hebrew
	{CHSHEB,		flmHeb60TblB},			// Hebrew
	{CHSARB1,	flmAr160Tbl},			// Arabic Set 1
	{CHSARB2,	flmAr260Tbl},			// Arabic Set 2
	{0xff, 		0}							// table terminator
};

/****************************************************************************
Desc:	The diacritical to collated table translates the first 26 characters 
		of character set #1 into a 5 bit value for "correct" sorting 
		sequence for that diacritical (DCV) - diacritic collated value.
	
		The attempt here is to convert the collated character value
		along with the DCV to form the original character.

		The diacriticals are in an order to fit the most languages.
		Czech, Swedish, and Finnish will have to manual reposition the
		ring above (assign it a value greater then the umlaut)

		This table is index by the diacritical value.
****************************************************************************/
FLMBYTE flmDia60Tbl[] =
{
	2,											// grave		offset = 0
	16,										//	centerd	offset = 1
	7,											//	tilde		offset = 2
	4,											//	circum	offset = 3
	12,										//	crossb	offset = 4
	10,										//	slash		offset = 5
	1,											//	acute		offset = 6
	6,											//	umlaut	offset = 7
												// In SU, SV and CZ will = 9
	17,										//	macron	offset = 8
	18,										//	aposab	offset = 9
	19,										//	aposbes	offset = 10
	20,										//	aposba	offset = 11
	21,										//	aposbc	offset = 12
	22,										//	abosbl	offset = 13
	8,											// ring		offset = 14
	13,										//	dota		offset = 15
	23,										//	dacute	offset = 16
	11,										//	cedilla	offset = 17
	14,										//	ogonek	offset = 18
	5,											//	caron		offset = 19
	15,										//	stroke	offset = 20
	24,										//	bara 		offset = 21
	3,											//	breve		offset = 22
	0,											// dbls		offset = 23 sorts as 'ss'
	25,										//	dotlesi	offset = 24
	26											// dotlesj	offset = 25
};

/****************************************************************************
Desc:  	Map special chars in CharSet (x24) to collation values
****************************************************************************/
BYTE_WORD_TBL flmCh24ColTbl[] =
{
	{1,	COLLS + 2},						// comma
	{2,	COLLS + 1},						// maru
	{5,	COLS_ASIAN_MARKS + 2},		// chuuten
	{10,	COLS_ASIAN_MARKS},			// dakuten
	{11,	COLS_ASIAN_MARKS + 1},		// handakuten
	{43,	COLS2 + 2},						// angled brackets
	{44,	COLS2 + 3},					
	{49,	COLS2 + 2},						// pointy brackets
	{50,	COLS2 + 3},	
	{51,	COLS2 + 2},						// double pointy brackets
	{52,	COLS2 + 3},	
	{53,	COLS1},							// Japanese quotes
	{54,	COLS1},
	{55,	COLS1},							// hollow Japanese quotes
	{56,	COLS1},
	{57,	COLS2 + 2},						// filled rounded brackets
	{58,	COLS2 + 3}	
};

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT16 colToWPChr[ COLS11 - COLLS] =
{
	0x20,										// colls			-	<Spc>
	0x2e,										// colls+1		-	.
	0x2c,										// colls+2		-	,
	0x3a,										// colls+3		-	:
	0x3b,										// colls+4		-	;
	0x21,										// colls+5		-	!
	0,											// colls+6		-	NO VALUE
	0x3f,										// colls+7		-	?
	0,											// colls+8		-	NO VALUE
	0x22,										// cols1			-	"
	0x27,										// cols1+1		-	'
	0x60,										// cols1+2		-	`
	0,											// cols1+3		-	NO VALUE
	0,											// cols1+4		-	NO VALUE
	0x28,										// cols2			-	(
	0x29,										// cols2+1		-	)
	0x5b,										// cols2+2		-	japanese angle brackets
	0x5d,										// cols2+3		-	japanese angle brackets
	0x7b,										// cols2+4		-	{
	0x7d,										// cols2+5		-	}
	0x24,										// cols3			-	$
	0x413,									// cols3+1		-	cent
	0x40b,									// cols3+2		-	pound
	0x40c,									// cols3+3		-	yen
	0x40d,									// cols3+4		-	pacetes
	0x40e,									// cols3+5		-	floren
	0x2b,										// cols4			-	+
	0x2d,										// cols4+1		-	-
	0x2a,										// cols4+2		-	*
	0x2f,										// cols4+3		-	/
	0x5e,										// cols4+4		-	^
	0,											// cols4+5		-	NO VALUE
	0,											// cols4+6		-	NO VALUE
	0,											// cols4+7		-	NO VALUE
	0x3c,										// cols5			-	<
	0,											// cols5+1		-	NO VALUE
	0x3d,										// cols5+2		-	=
	0,											// cols5+3		-	NO VALUE
	0x3e,										// cols5+4		-	>
	0,											// cols5+5		-	NO VALUE
	0,											// cols5+6		-	NO VALUE
	0,											// cols5+7		-	NO VALUE
	0,											// cols5+8		-	NO VALUE
	0,											// cols5+9		-	NO VALUE
	0,											// cols5+10		-	NO VALUE
	0,											// cols5+11		-	NO VALUE
	0,											// cols5+12		-	NO VALUE
	0,											// cols5+13		-	NO VALUE
	0x25,										// cols6			-	%
	0x23,										// cols6+1		-	#
	0x26,										// cols6+2		-	&
	0x40,										// cols6+3		-	@
	0x5c,										// cols6+4		-	backslash
	0x5f,										// cols6+5		-	_
	0x7c,										// cols6+6		-	|
	0x7e,										// cols6+7		-	~
	0,											// cols6+8		- NO VALUE
	0,											// cols6+9		- NO VALUE
	0,											// cols6+10		- NO VALUE
	0,											// cols6+11		- NO VALUE
	0,											// cols6+12		- NO VALUE
	0x800,									// cols7			-	Uppercase Alpha
	0x802,									// cols7+1		-	Uppercase Beta
	0x806,									// cols7+2		-	Uppercase Gamma
	0x808,									// cols7+3		-	Uppercase Delta
	0x80a,									// cols7+4		-	Uppercase Epsilon
	0x80c,									// cols7+5		-	Uppercase Zeta
	0x80e,									// cols7+6		-	Uppercase Eta
	0x810,									// cols7+7		-	Uppercase Theta
	0x812,									// cols7+8		-	Uppercase Iota
	0x814,									// cols7+9		-	Uppercase Kappa
	0x816,									// cols7+10		-	Uppercase Lambda
	0x818,									// cols7+11		-	Uppercase Mu
	0x81a,									// cols7+12		-	Uppercase Nu
	0x81c,									// cols7+13		-	Uppercase Xi
	0x81e,									// cols7+14		-	Uppercase Omicron
	0x820,									// cols7+15		-	Uppercase Pi
	0x822,									// cols7+16		-	Uppercase Rho
	0x824,									// cols7+17		-	Uppercase Sigma
	0x828,									// cols7+18		-	Uppercase Tau
	0x82a,									// cols7+19		-	Uppercase Upsilon
	0x82c,									// cols7+20		-	Uppercase Phi
	0x82e,									// cols7+21		-	Uppercase Chi
	0x830,									// cols7+22		-	Uppercase Psi
	0x832,									// cols7+23		-	Uppercase Omega
	0,											// cols7+24 	- NO VALUE
	0x30,										// cols8			-	0
	0x31,										// cols8+1		-	1
	0x32,										// cols8+2		-	2
	0x33,										// cols8+3		-	3
	0x34,										// cols8+4		-	4
	0x35,										// cols8+5		-	5
	0x36,										// cols8+6		-	6
	0x37,										// cols8+7		-	7
	0x38,										// cols8+8		-	8
	0x39,										// cols8+9		-	9
	0x41,										// cols9			-	A
	0x124,									// cols9+1		-	AE digraph
	0x42,										// cols9+2		-	B
	0x43,										// cols9+3		-	C
	0xffff,									// cols9+4		-	CH in spanish
	0x162,									// cols9+5		-	Holder for C caron in Czech
	0x44,										// cols9+6		-	D
	0x45,										// cols9+7		-	E
	0x46,										// cols9+8		-	F
	0x47,										// cols9+9		-	G
	0x48,										// cols9+10		-	H
	0xffff,									// cols9+11		-	CH in czech or dotless i in turkish
	0x49,										// cols9+12		-	I
	0x18a,									// cols9+13		-	IJ Digraph
	0x4a,										// cols9+14		-	J
	0x4b,										// cols9+15		-	K
	0x4c,										// cols9+16		-	L
	0xffff,									// cols9+17		-	LL in spanish
	0x4d,										// cols9+18		-	M
	0x4e,										// cols9+19		-	N
	0x138,									// cols9+20		-	N Tilde
	0x4f,										// cols9+21		-	O
	0x1a6,									// cols9+22		-	OE digraph
	0x50,										// cols9+23		-	P
	0x51,										// cols9+24		-	Q
	0x52,										// cols9+25		-	R
	0x1aa,									// cols9+26		-	Holder for R caron in Czech
	0x53,										// cols9+27		-	S
	0x1b0,									// cols9+28		-	Holder for S caron in Czech
	0x54,										// cols9+29		-	T
	0x55,										// cols9+30		-	U
	0x56,										// cols9+31		-	V
	0x57,										// cols9+32		-	W
	0x58,										// cols9+33		-	X
	0x59,										// cols9+34		-	Y
	0x5a,										// cols9+35		-	Z
	0x1ce,									// cols9+36		-	Holder for Z caron in Czech
	0x158,									// cols9+37		-	Uppercase Thorn
	0,											// cols9+38		-	???
	0,											// cols9+39		-	???
	0x5b,										// cols9+40		-	[ (note: alphabetic - end of list)
	0x5d,										// cols9+41		-	] (note: alphabetic - end of list)
	0x124,									// cols9+42		- AE diagraph - DK
	0x124,									// cols9+43 	- AE diagraph - NO
	0x122,									// cols9+44 	- A ring      - SW
	0x11E,									// cols9+45 	- A diaeresis - DK
	0x124,									// cols9+46		- AE diagraph - IC
	0x150,									// cols9+47 	- O slash     - NO
	0x11e,									// cols9+48		- A diaeresis - SW
	0x150,									// cols9+49		- O slash     - DK
	0x13E,									// cols9+50		- O Diaeresis - IC
	0x122,									// cols9+51		- A ring      - NO
	0x13E,									// cols9+52		- O Diaeresis - SW
	0x13E,									// cols9+53		- O Diaeresis - DK
	0x150,									// cols9+54 	- O slash     - IC
	0x122,									// cols9+55		- A ring      - DK
	0x124,									// cols9+56		- AE diagraph future
	0x13E,									// cols9+57 	- O Diaeresis future
	0x150,									// cols9+58 	- O slash     future
	0,											// cols9+59 	- NOT USED    future
	0xA00,									// cols10		-	Russian A
	0xA02,									// cols10+1		-	Russian BE
	0xA04,									// cols10+2		-	Russian VE
	0xA06,									// cols10+3		-	Russian GHE 
	0xA46,									// cols10+4		-	Ukrainian HARD G
	0xA08,									// cols10+5		-	Russian DE
	0xA4a,									// cols10+6		-	Serbian SOFT DJ
	0xA44,									// cols10+7		-	Macedonian SOFT DJ
	0xA0a,									// cols10+8		-	Russian E
	0xA0c,									// cols10+9		-  Russian YO
	0xA4e,									// cols10+10	-	Ukrainian YE
	0xA0e,									// cols10+11	-	Russian ZHE
	0xA10,									// cols10+12	-	Russian ZE
	0xA52,									// cols10+13	-	Macedonian ZELO
	0xA12,									// cols10+14	-	Russian I
	0xA58,									// cols10+15	-	Ukrainian I
	0xA5a,									// cols10+16	-	Ukrainian I with Two dots
	0xA14,									// cols10+17	-	Russian SHORT I
	0xA5e,									// cols10+18	-	Serbian--Macedonian JE
	0xA16,									// cols10+19	-	Russian KA
	0xA18,									// cols10+20	-	Russian EL
	0xA68,									// cols10+21	-	Serbian--Macedonian SOFT L
	0xA1a,									// cols10+22	-	Russian EM
	0xA1c,									// cols10+23	-	Russian EN
	0xA6c,									// cols10+24	-	Serbian--Macedonian SOFT N
	0xA1e,									// cols10+25	-	Russian O
	0xA20,									// cols10+26	-	Russian PE
	0xA22,									// cols10+27	-	Russian ER
	0xA24,									// cols10+28	-	Russian ES
	0xA26,									// cols10+29	-	Russian TE
	0xA72,									// cols10+30	-	Serbian SOFT T
	0xA60,									// cols10+31	-	Macedonian SOFT K
	0xA28,									// cols10+32	-	Russian U
	0xA74,									// cols10+33	-	Byelorussian SHORT U
	0xA2a,									// cols10+34	-	Russian EF
	0xA2c,									// cols10+35	-	Russian HA
	0xA2e,									// cols10+36	-	Russian TSE
	0xA30,									// cols10+37	-	Russian CHE
	0xA86,									// cols10+38	-	Serbian HARD DJ
	0xA32,									// cols10+39	-	Russian SHA
	0xA34,									// cols10+40	-	Russian SHCHA
	0xA36,									// cols10+41	-	Russian ER
	0xA38,									// cols10+42	-	Russian ERY
	0xA3a,									// cols10+43	-	Russian SOFT SIGN
	0xA8e,									// cols10+44	-	Old Russian YAT
	0xA3c,									// cols10+45	-	Russian uppercase	REVERSE E
	0xA3e,									// cols10+46	-	Russian YU
	0xA40,									// cols10+47	-	Russian YA
	0xA3a,									// cols10+48	-	Russian SOFT SIGN - UKRAIN ONLY
 	0											// cols10+49	- 	future
};

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT16 HebArabColToWPChr[ ] = 
{
	0x0D00 +164,							// hamzah
	0x0D00 + 58,							// [13,177] alef maddah
	0x0D00 + 60,							// baa
	0x0E00 + 48,							// Sindhi bb
	0x0E00 + 52,							// Sindhi bh
	0x0E00 + 56,							// Misc p = peh
	0x0D00 +152,							// taa marbuuTah
	0x0E00 + 60,							// Urdu T
	0x0D00 + 68,							// thaa
	0x0E00 + 68,							// Sindhi th
	0x0E00 + 72,							// Sindhi tr
	0x0E00 + 76,							// Sindhi Th
	0x0D00 + 72,							// jiim - jeem
	0x0E00 + 80,							// Sindhi jj
	0x0E00 + 84,							// Sindhi ny
	0x0E00 + 88,							// Misc ch
	0x0D00 + 76,							// Haa
	0x0D00 + 80,							// khaa
	0x0E00 + 96,							// Pashto ts
	0x0E00 +100,							// Pashto dz
	0x0D00 + 84,							// dal
	0x0E00 +104,							// Urdu D
	0x0D00 + 86,							// thal
	0x0E00 +108,							// Sindhi dh
	0x0E00 +110,							// Sindhi D 
	0x0E00 +112,							// Sindhi Dr
	0x0E00 +114,							// Sindhi Dh
	0x0D00 + 88,							// ra
	0x0E00 +116,							// Pashto r
	0x0D00 + 90,							// zain
	0x0E00 +126,							// Mizc Z
	0x0D00 + 92,							// seen
	0x0D00 + 96,							// sheen
	0x0E00 +132,							// Pashto x
	0x0D00 +100,							// Sad
	0x0D00 +104,							// Dad
	0x0D00 +108,							// Tah
	0x0D00 +112,							// Za (dhah)
	0x0D00 +116,							// 'ain
	0x0D00 +120,							// ghain
	0x0D00 +124,							// fa
	0x0E00 +140,							// Malay p, kurdish v = veh
	0x0D00 +128,							// Qaf
	0x0D00 +132,							// kaf (caf)
	0x0E00 +160,							// Persian/Urdu gaf
	0x0E00 +176,							// Singhi gg
	0x0D00 +136,							// lam - all ligature variants
	0x0D00 +140,							// meem
	0x0D00 +144,							// noon
	0x0D00 +148,							// ha - arabic language only
	0x0D00 +154,							// waw
	0x0D00 +148,							// ha - non-arabic language
	0x0D00 +160,							// alef maqsurah
	0x0D00 +156,							// ya
	0x0E00 +212								// Urdu ya barree
};

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT16	ArabSubColToWPChr[] = 
{
	0x0D00 +177,							// Alef maddah
	0x0D00 +165,							// Alef Hamzah
	0x0D00 +169,							// Waw hamzah
	0x0D00 +167,							// Hamzah under alef
	0x0D00 +171,							// ya hamzah
	0x0D00 +175,							// alef fathattan
	0x0D00 +179,							// alef waslah
	0x0D00 + 58,							// alef
	0x0D00 + 64								// taa - after taa marbuuTah
};

/****************************************************************************
Desc:	Turns a collated diacritic value into the original diacritic value
****************************************************************************/
FLMBYTE ml1_COLtoD[ 27] = 
{
	23,										// dbls sort value = 0  sorts as 'ss'
	6,											//	acute	sort value = 1
	0,											// grave	sort value = 2
	22,										//	breve	sort value = 3
	3,											//	circum sort value = 4
	19,										//	caron	sort value = 5
	7,											//	umlaut sort value = 6
	2,											//	tilde	sort value = 7
	14,										// ring sort value = 8
	7,											//	umlaut in SU, SV and CZ after ring = 9
	5,											//	slash	sort value = 10
	17,	 									//	cedilla sort value = 11
	4,											//	crossb sort value = 12
	15,	 									//	dota sort value = 13
	18,	 									//	ogonek sort value = 14
	20,	 									//	stroke sort value = 15
	1, 	 									//	centerd sort value = 16
	8,											//	macron sort value = 17
	9,											//	aposab sort value = 18
	10,	 									//	aposbes sort value = 19
	11,	 									//	aposba sort value = 20
	12,	 									//	aposbc sort value = 21
	13,	 									//	abosbl sort value = 22
	16,	 									//	dacute sort value = 23
	21,	 									//	bara sort value = 24
	24,	 									//	dotlesi sort value = 25
	25											// dotlesj sort value = 26
};

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT16 flmIndexi[] = 
{
	0,
	11,
	14,
	15,
	17,
	18,
	19,
	21,
	22,
	23,
	24,
	25,
	26,
	35,
	59
};

/****************************************************************************
Desc:
****************************************************************************/
FLMUINT16 flmIndexj[] =
{
	FLM_CA_LANG,							// Catalan
	FLM_CF_LANG,							// Canadian French
	FLM_CZ_LANG,							// Czech
	FLM_SL_LANG,							// Slovak
	FLM_DE_LANG,							// German
	FLM_SD_LANG,							// Swiss German
	FLM_ES_LANG,							// Spanish (Spain)
	FLM_FR_LANG,							// French
	FLM_NL_LANG,							// Netherlands
	0xFFFF,									// DK_LANG,	Danish
	0xFFFF,									// NO_LANG,	Norwegian
	0x0063,									// c
	0x006c,									// l
	0x0197,									// l with center dot
	0x0063,									// c
	0x0125,									// ae digraph
	0x01a7,									// oe digraph
	0x0068,									// h
	0x0068,									// h
	0x006c,									// l
	0x0101,									// center dot alone
	0x006c,									// l
	0x0117,									// �	(for German)
	0x018b,									// ij digraph
	0x0000,									// was 'a' - will no longer map 'aa' to a-ring
	0x0000,									// was 'a'
	FLM_CZ_LANG,
	FLM_DK_LANG,
	FLM_NO_LANG,
	FLM_SL_LANG,
	FLM_TK_LANG,
	FLM_SU_LANG,
	FLM_IS_LANG,
	FLM_SV_LANG,
	FLM_YK_LANG,
	0x011e,									// A diaeresis
	0x011f,									// a diaeresis
	0x0122,									// A ring
	0x0123,									// a ring
	0x0124,									// AE diagraph
	0x0125,									// ae diagraph
	0x013e,									// O diaeresis
	0x013f,									// o diaeresis
	0x0146,									// U diaeresis
	0x0147,									// u diaeresis
	0x0150,									// O slash
	0x0151,									// o slash
	0x0A3a,									// CYRILLIC SOFT SIGN
	0x0A3b,									// CYRILLIC soft sign
	0x01ee,									// dotless i - turkish
	0x01ef,									// dotless I - turkish
	0x0162,									// C Hacek/caron
	0x0163,									// c Hacek/caron
	0x01aa,									// R Hacek/caron
	0x01ab,									// r Hacek/caron
	0x01b0,									// S Hacek/caron
	0x01b1,									// s Hacek/caron
	0x01ce,									// Z Hacek/caron
	0x01cf,									// z Hacek/caron
}; 

/****************************************************************************
Desc:			Kana subcollation values
  	 				BIT 0: set if large char
					BIT 1: set if voiced
					BIT 2: set if half voiced
Notes: 		To save space should be nibbles
IMPORTANT:	The '1' entries that do not have a matching '0' entry have been
				changed to zero to save space in the subcollation area.
****************************************************************************/
FLMBYTE flmKanaSubColTbl[] = 
{
	0,1,0,1,0,1,0,1,0,1,					// a    A   i   I   u   U   e   E   o   O
	1,3,0,3,0,3,1,3,0,3,					// KA  GA  KI  GI  KU  GU  KE  GE  KO  GO
	0,3,0,3,0,3,0,3,0,3,					// SA  ZA SHI  JI  SU  ZU  SE  ZE  SO  ZO
	0,3,0,3,0,1,3,0,3,0,3,				// TA  DA CHI  JI tsu TSU  ZU  TE DE TO DO
	0,0,0,0,0,								// NA NI NU NE NO
	0,3,5,0,3,5,0,3,5,					// HA BA PA HI BI PI FU BU PU
	0,3,5,0,3,5,							// HE BE PE HO BO PO
	0,0,0,0,0,								// MA MI MU ME MO
	0,1,0,1,0,1,							// ya YA yu YU yo YO
	0,0,0,0,0,								// RA RI RU RE RO
	0,1,0,0,0,								// wa WA WI WE WO		
	0,3,0,0									// N VU ka ke
};

/****************************************************************************
Desc:	Map KataKana (CharSet x26) to collation values.
		Kana collating values are two byte values where the high byte is 0x01.
****************************************************************************/
FLMBYTE 	KanaColTbl[] = 
{
	 0, 0, 1, 1, 2, 2, 3, 3, 4, 4,	// a    A   i   I   u   U   e   E   o   O
 	 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,	// KA  GA  KI  GI  KU  GU  KE  GE  KO  GO
	10,10,11,11,12,12,13,13,14,14,	// SA  ZA SHI  JI  SU  ZU  SE  ZE  SO  ZO
	15,15,16,16,17,17,17,18,18,19,19,// TA DA CHI JI tsu TSU  ZU  TE DE TO DO
	20,21,22,23,24,						// NA NI NU NE NO
	25,25,25,26,26,26,27,27,27,		// HA BA PA HI BI PI FU BU PU
	28,28,28,29,29,29,					// HE BE PE HO BO PO
	30,31,32,33,34,						// MA MI MU ME MO
	35,35,36,36,37,37,					// ya YA yu YU yo YO
	38,39,40,41,42,						// RA RI RU RE RO
	43,43,44,45,46,						// wa WA WI WE WO
	47, 2, 5, 8								// N VU ka ke
};

/****************************************************************************
Desc: Map KataKana collated value to vowel value for use for the
		previous char.
****************************************************************************/
FLMBYTE KanaColToVowel[] = 
{
	0,1,2,3,4,								// a   i   u  e  o 
	0,1,2,3,4,								// ka  ki  ku ke ko
	0,1,2,3,4,								// sa shi  su se so
	0,1,2,3,4,								// ta chi tsu te to
	0,1,2,3,4,								// na  ni  nu ne no
	0,1,2,3,4,								// ha  hi  hu he ho
	0,1,2,3,4,								// ma  mi  mu me mo
	0,2,4,									// ya  yu  yo
	0,1,2,3,4,								// ra  ri  ru re ro
	0,1,3,4,									// wa  wi  we wo
};

/****************************************************************************
Desc:
****************************************************************************/
static FLMBYTE ColToKanaTbl[ 48] =
{
	0,											// a=0, A=1
	2,											// i=2, I=3
	4,											// u=4, U=5, VU=83
	6,											// e=6, E=7
	8,											// o=8, O=9
	84,										// KA=10, GA=11, ka=84
	12,										// KI=12, GI=13
	14,										// KU=14, GU=15
	85,										// KE=16, GE=17, ke=85
	18,										// KO=18, GO=19
	20,										// SA=20, ZA=21
	22,										// SHI=22, JI=23
	24,										// SU=24, ZU=25
	26,										// SE=26, ZE=27
	28,										// SO=28, ZO=29
	30,										// TA=30, DA=31
	32,										// CHI=32, JI=33
	34,										// tsu=34, TSU=35, ZU=36
	37,										// TE=37, DE=38
	39,										// TO=39, DO=40
	41,										// NA
	42,										// NI
	43,										// NU
	44,										// NE
	45,										// NO
	46,										// HA, BA, PA
	49,										// HI, BI, PI
	52,										// FU, BU, PU
	55,										// HE, BE, PE
	58,										// HO, BO, PO
	61,										// MA
	62,										// MI
	63,										// MU
	64,										// ME
	65,										// MO
	66,										// ya, YA
	68,										// yu, YU
	70,										// yo, YO
	72,										// RA
	73,										// RI
	74,										// RU
	75,										// RE
	76,										// RO
	77,										// wa, WA
	79,										// WI
	80,										// WE
	81,										// WO
	82											// N
};

/****************************************************************************
Desc: Maps from charset 11 to CS24 (punctuation) (starting from 11,0)
****************************************************************************/
FLMBYTE From0AToZen[] = 
{
	0, 	9,		40,	0x53, 			// sp ! " #
	0x4f, 0x52, 0x54,	38, 				// $ % & '
	0x29,	0x2a,	0x55,	0x3b, 			// ( ) * +
	3,		0x1d,	4,		0x1e	 			// , - . /
};
	
/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE From0BToZen[] = 
{
	6,		7,		0x42,	0x40,				// : ; < =
	0x43,	8,		0x56						// > ? @
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE From0CToZen[] =
{
	0x2d,	0x1f,	0x2e, 					// [ \ ]
	0x0f,	0x11,	0x0d						// ^ _ `
};
	
/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE From0DToZen[] =
{
	0x2f,	0x22,	0x30,	0x20 				// { | } ~
};

/****************************************************************************
Desc:
****************************************************************************/
FLMBYTE From8ToZen[] = 
{
	0x5e, 0x7e, 0x5f, 0x7f, 0x5f, 0xFF, 0x60, 0x80,
	0x61, 0x81, 0x62, 0x82, 0x63, 0x83, 0x64, 0x84,
	0x65, 0x85, 0x66, 0x86, 0x67, 0x87, 0x68, 0x88,
	0x69, 0x89, 0x6a, 0x8a, 0x6b, 0x8b, 0x6c, 0x8c,
	0x6d, 0x8d, 0x6e, 0x8e, 0x6f, 0x8f, 0x6f, 0xFF,
	0x70, 0x90, 0x71, 0x91, 0x72, 0x92, 0x73, 0x93,
	0x74, 0x94, 0x75, 0x95
};

/****************************************************************************
Desc:
****************************************************************************/
static FLMBYTE From11AToZen[] =
{
	2,											// japanese period
	0x35,										// left bracket
	0x36,										// right bracket
	0x01,										// comma
	0x05										// chuuten
};

/****************************************************************************
Desc:
****************************************************************************/
static FLMBYTE	From11BToZen[] = 
{
	0x51,										// wo
	0,2,4,6,8,0x42,0x44,0x46,0x22,	// small a i u e o ya yu yo tsu
	0xFF, 1, 3, 5, 7, 9,					// dash (x241b) a i u e o
	0x0a, 0x0c, 0x0e, 0x10, 0x12,		// ka ki ku ke ko
	0x14, 0x16, 0x18, 0x1a, 0x1c,		// sa shi su se so
	0x1e, 0x20, 0x23, 0x25, 0x27,		// ta chi tsu te to
	0x29, 0x2a, 0x2b, 0x2c, 0x2d,		// na ni nu ne no
	0x2e, 0x31, 0x34, 0x37, 0x3a,		// ha hi fu he ho
	0x3d, 0x3e, 0x3f, 0x40, 0x41,		// ma mi mu me mo
	0x43, 0x45, 0x47,						// ya yu yo
	0x48, 0x49, 0x4a, 0x4b, 0x4c,		// ra ri ru re ro
	0x4e, 0x52								// WA N
};

/****************************************************************************
Desc:	Convert Zenkaku (double wide) to Hankaku (single wide)
		Character set 0x24 maps to single wide chars in other char sets.
		This enables collation values to be found on some symbols.
		This is also used to convert symbols from hankaku to Zen24.
****************************************************************************/
BYTE_WORD_TBL Zen24ToHankaku[] =
{
	{	0  ,0x0020 },						// space
	{	1  ,0x0b03 },						// japanese comma
	{	2  ,0x0b00 },						// circle period
	{	3  ,  44	 },						// comma
	{	4  ,  46	 },						// period
	{	5  ,0x0b04 },						// center dot
	{	6  ,  58	 },						// colon
	{	7  ,  59	 },						// semicolon
	{	8  ,  63	 },						// question mark
	{	9  ,  33	 },						// exclamation mark
	{	10 ,0x0b3d },						// dakuten
	{	11 ,0x0b3e },						// handakuten
	{	12 ,0x0106 },						// accent mark
	{	13 ,  96	 },						// accent mark
	{	14 ,0x0107 },						// umlat
	{	15 ,  94	 },						// caret
	{	16 ,0x0108 },						// macron
	{	17 ,  95	 },						// underscore
	{	27 ,0x0b0f },						// extend vowel
	{	28 ,0x0422 },						// mdash
	{	29 ,  45	 },						// hyphen
	{	30 ,  47  },     					// slash
	{	31 ,0x0607 },						// backslash
	{	32 , 126	 },						// tilde
	{	33 ,0x0611 },						// doubleline
	{	34 ,0x0609 },						// line
	{	37 ,0x041d },						// left apostrophe
	{	38 ,0x041c },						// right apostrophe
	{	39 ,0x0420 },						// left quote
	{	40 ,0x041f },						// right quote
	{	41 ,  40	 },						// left paren
	{	42 ,  41	 },						// right paren
	{	45 ,  91	 },						// left bracket
	{	46 ,  93	 },						// right bracket
	{	47 , 123	 },						// left curly bracket
	{	48 , 125	 },						// right curly bracket
	{	53 ,0x0b01 },						// left j quote
	{	54 ,0x0b02 },						// right j quote
	{	59 ,  43	 },						// plus
	{	60 ,0x0600 },						// minus
	{	61 ,0x0601 },						// plus/minus
	{	62 ,0x0627 },						// times
	{	63 ,0x0608 },						// divide
	{	64 ,  61	 },						// equal
	{	65 ,0x0663 },						// unequal
	{	66 ,  60	 },						// less
	{	67 ,  62	 },						// greater
	{	68 ,0x0602 },						// less/equal
	{	69 ,0x0603 },						// greater/equal
	{	70 ,0x0613 },						// infinity
	{	71 ,0x0666 },						// traingle dots
	{	72 ,0x0504 },						// man
	{	73 ,0x0505 },						// woman
	{	75 ,0x062d },						// prime
	{	76 ,0x062e },						// double prime
	{	78 ,0x040c },						// yen
	{	79 ,  36	 },						// dollar
	{	80 ,0x0413 },						// cent
	{	81 ,0x040b },						// pound
	{	82 ,  37	 },						// percent
	{	83 ,  35	 },						// #
	{	84 ,  38	 },						// &
	{	85 ,  42	 },						// *
	{	86 ,  64	 },						// @
	{	87 ,0x0406 },						// squiggle
	{	89 ,0x06b8 },						// filled star
	{	90 ,0x0425 },						// hollow circle
	{	91 ,0x042c },						// filled circle
	{	93 ,0x065f },						// hollow diamond
	{	94 ,0x0660 },						// filled diamond
	{	95 ,0x0426 },						// hollow box
	{	96 ,0x042e },						// filled box
	{	97 ,0x0688 },						// hollow triangle
	{	99 ,0x0689 },						// hollow upside down triangle
	{	103,0x0615 },						// right arrow
	{	104,0x0616 },						// left arrow
	{	105,0x0617 },						// up arrow
	{	106,0x0622 },						// down arrow
	{	119,0x060f },
	{	121,0x0645 },
	{	122,0x0646 },
	{	123,0x0643 },
	{	124,0x0644 },
	{	125,0x0642 },						// union
	{	126,0x0610 },						// intersection
	{	135,0x0655 },
	{	136,0x0656 },
	{	138,0x0638 },						// right arrow
	{	139,0x063c },						// left/right arrow
	{	140,0x067a },
	{	141,0x0679 },
	{	153,0x064f },						// angle
	{	154,0x0659 },
	{	155,0x065a },
	{	156,0x062c },
	{	157,0x062b },
	{	158,0x060e },
	{	159,0x06b0 },
	{	160,0x064d },
	{	161,0x064e },
	{	162,0x050e },						// square root
	{	164,0x0604 },
	{	175,0x0623 },						// angstrom
	{	176,0x044b },						// percent
	{	177,0x051b },						// sharp
	{	178,0x051c },						// flat
	{	179,0x0509 },						// musical note
	{	180,0x0427 },						// dagger
	{	181,0x0428 },						// double dagger
	{	182,0x0405 },						// paragraph
	{	187,0x068f }						// big hollow circle
};

/****************************************************************************
Desc: Maps CS26 to CharSet 11
****************************************************************************/
FLMBYTE 	MapCS26ToCharSet11[ 86 ] = 
{
	0x06,										// 0     a
	0x10,										//	1     A 
	0x07,										//	2     i
	0x11,										//	3     I 
	0x08,										//	4     u
	0x12,										//	5     U 
	0x09,										//	6     e
	0x13,										//	7     E 
	0x0a,										//	8     o
	0x14,										//	9     O
	0x15,										//	0x0a  KA
	0x95,										//       GA - 21 followed by 0x3D dakuten
	0x16,										// 0x0c  KI
	0x96,										//       GI
	0x17,										//	0x0e  KU
	0x97,										//       GU
	0x18,										// 0x10  KE
	0x98,										//       GE
	0x19,										// 0x12  KO
	0x99,										//       GO
	0x1a,										//	0x14  SA
	0x9a,										//       ZA
	0x1b,										//	0x16  SHI
	0x9b,										//       JI
	0x1c,										//	0x18  SU
	0x9c,										//       ZU
	0x1d,										//	0x1a  SE
	0x9d,										//       ZE
	0x1e,										//	0x1c  SO
	0x9e,										//       ZO
	0x1f,										//	0x1e  TA
	0x9f,										//       DA
	0x20,										//	0x20  CHI
	0xa0,										//       JI
	0x0e,										//	0x22  small tsu
	0x21,										//	0x23  TSU
	0xa1,										//       ZU
	0x22,										//	0x25  TE
	0xa2,										//       DE
	0x23,										//	0x27  TO
	0xa3,										//       DO
	0x24,										//	0x29  NA
	0x25,										//	0x2a  NI
	0x26,										// 0x2b  NU
	0x27,										//	0x2c  NE
	0x28,										//	0x2d  NO
	0x29,										//	0x2e  HA
	0xa9,										// 0x2f  BA
	0xe9,										// 0x30  PA
	0x2a,										//	0x31  HI
	0xaa,										// 0x32  BI
	0xea,										// 0x33  PI
	0x2b,										//	0x34  FU
	0xab,										// 0x35  BU
	0xeb,										// 0x36  PU
	0x2c,										//	0x37  HE
	0xac,										// 0x38  BE
	0xec,										// 0x39  PE
	0x2d,										//	0x3a  HO
	0xad,										// 0x3b  BO
	0xed,										// 0x3c  PO
	0x2e,										//	0x3d  MA
	0x2f,										//	0x3e  MI
	0x30,										//	0x3f  MU
	0x31,										//	0x40  ME
	0x32,										//	0x41  MO
	0x0b,										//	0x42  small ya
	0x33,										//	0x43  YA
	0x0c,										//	0x44  small yu
	0x34,										//	0x45  YU
	0x0d,										//	0x46  small yo
	0x35,										//	0x47  YO
	0x36,										//	0x48  RA
	0x37,										//	0x49  RI
	0x38,										//	0x4a  RU
	0x39,										//	0x4b  RE
	0x3a,										//	0x4c  RO
	0xff,										// 0x4d  small wa
	0x3b,										//	0x4e  WA
	0xff,										// 0x4f  WI
	0xff,										// 0x50  WE
	0x05,										//	0x51	WO
	0x3c,										//	0x52	N
	0xff,										// 0x53  VU
	0xff, 									// 0x54  ka
	0xff 										// 0x55  ke
};

/****************************************************************************
Desc:
****************************************************************************/
FINLINE FLMBOOL charIsUpper(
	FLMUINT16	ui16Char)
{
	return( (FLMBOOL)((ui16Char < 0x7F)
							? (FLMBOOL)((ui16Char >= ASCII_LOWER_A &&
											 ui16Char <= ASCII_LOWER_Z)
											 ? (FLMBOOL)FALSE
											 : (FLMBOOL)TRUE)
							: flmIsUpper( ui16Char)));
}

/****************************************************************************
Desc:	getNextCharState can be thought of as a 2 dimentional array with
		i and j as the row and column indicators respectively.  If a value
		exists at the intersection of i and j, it is returned.  Sparse array 
		techniques are used to minimize memory usage.
****************************************************************************/
FINLINE FLMUINT16 getNextCharState(
	FLMUINT		i,
	FLMUINT		j)
{
	FLMUINT		k;
	FLMUINT		x;

	for( k = flmIndexi[ x = (i > START_COL) ? (START_ALL) : i ];
		  k <= (FLMUINT) (flmIndexi[ x + 1] - 1);
		  k++ )
	{
		if(  j == flmIndexj[ k])
		{
			return( flmValuea[ (i > START_COL) 
				?	(k + (FIXUP_AREA_SIZE * (i - START_ALL))) 
				: k]);
		}
	}

	return(0);
}

/**************************************************************************
Desc:	Find the collating value of a WP character
Ret:	Collating value (COLS0 is high value - undefined WP char)
***************************************************************************/
FLMUINT16 flmGetCollation(
	FLMUINT16	ui16WpChar,
	FLMUINT		uiLanguage)
{
	FLMUINT16		ui16State;
	FLMBYTE			ucCharVal;
	FLMBYTE			ucCharSet;
	FLMBOOL			bHebrewArabicFlag = FALSE;
	TBL_B_TO_BP *	pColTbl = flmCol60Tbl;

	// State ONLY for non-US

	if (uiLanguage != FLM_US_LANG)
	{
		if (uiLanguage == FLM_AR_LANG ||		// Arabic
			 uiLanguage == FLM_FA_LANG ||		// Farsi - persian
			 uiLanguage == FLM_HE_LANG ||		// Hebrew
			 uiLanguage == FLM_UR_LANG) 		// Urdu
		{
			pColTbl = flmHebArabicCol60Tbl;
			bHebrewArabicFlag = TRUE;
		}	
		else
		{

			// Check if uiLanguage candidate for alternate double collating

			ui16State = getNextCharState( START_COL, uiLanguage);
			if (0 != (ui16State = getNextCharState( (ui16State
							?	ui16State		// look at special case languages
							:	START_ALL),		// look at US and European
							(FLMUINT) ui16WpChar)))
			{
				return( ui16State);
			}
		}
	}

	ucCharVal = (FLMBYTE)ui16WpChar;
	ucCharSet = (FLMBYTE)(ui16WpChar >> 8);
	
	// This is an optimized version of f_b_bp_citrp() inline for performance

	do
	{
		if (pColTbl->key == ucCharSet)
		{
			FLMBYTE *	pucColVals;

			pucColVals = pColTbl->charPtr;

			// Above lower range of table?

			if (ucCharVal >= *pucColVals)
			{

				// Make value zero based to index

				ucCharVal -= *pucColVals++;

				// Below maximum number of table entries?

				if (ucCharVal < *pucColVals++)
				{

					// Return collated value.

					return( pucColVals[ ucCharVal]);
				}
			}
		}

		// Go to next table entry

		pColTbl++;
	} while (pColTbl->key != 0xFF);

	if (bHebrewArabicFlag)
	{
		if (ucCharSet == CHSHEB ||
			 ucCharSet == CHSARB1 ||
			 ucCharSet == CHSARB2)
		{

			// Same as COLS0_HEBREW

			return( COLS0_ARABIC);
		}
	}

	// Defaults for characters that don't have a collation value.

	return( COLS0);
}

/****************************************************************************
Desc:	Add an field into the CDL (Compound Data List) for this ISK.
****************************************************************************/
RCODE KYCmpKeyAdd2Lst(
	FDB *			pDb,
	IXD *			pIxd,				// Index definition
	IFD *			pIfd,				// Index field definition
	void *		pvField,			// Field whose value is part of the key
	void *		pRootContext)	// Points to root context of field path
{
	RCODE				rc = FERR_OK;
	CDL *				pCdl;
	KREF_CNTRL * 	pKrefCntrl;
	CDL **			ppCdlTbl;
	FLMUINT			uiCdlEntry;
	FLMUINT			uiIxEntry;

 	pKrefCntrl = &pDb->KrefCntrl;
	ppCdlTbl = pKrefCntrl->ppCdlTbl;
	
	flmAssert( ppCdlTbl != NULL);

	// Figure out which CDL and index entry to use

	uiIxEntry = (FLMUINT) (pIxd - pDb->pDict->pIxdTbl);
	uiCdlEntry = (FLMUINT) (pIfd - pDb->pDict->pIfdTbl);

	if( RC_BAD( rc = pDb->TempPool.poolAlloc( sizeof( CDL), (void **)&pCdl)))
	{
		goto Exit;
	}
	
	flmAssert( pKrefCntrl->pIxHasCmpKeys != NULL);

	pKrefCntrl->pIxHasCmpKeys [uiIxEntry] = TRUE;
	pCdl->pField = pvField;
	pCdl->pRootContext = pRootContext;

	// Insert at first of CDL list

	pCdl->pNext = ppCdlTbl [uiCdlEntry];
	ppCdlTbl [uiCdlEntry] = pCdl;
	pKrefCntrl->bHaveCompoundKey = TRUE;

Exit:

	return( rc);
}

/****************************************************************************
Desc:		Called when an entire record has been processed by the key 
			building functions. Builds and add all compound keys to the table.
****************************************************************************/
RCODE KYBuildCmpKeys(
	FDB *				pDb,
	FLMUINT			uiAction,
	FLMUINT			uiContainerNum,
	FLMUINT			uiDrn,
	FLMBOOL *		pbHadUniqueKeys,
	FlmRecord *		pRecord)
{
	RCODE				rc = FERR_OK;
	KREF_CNTRL *	pKrefCntrl = &pDb->KrefCntrl;
	CDL **			ppCdlTbl = pKrefCntrl->ppCdlTbl;
	FLMBYTE *		pKeyBuf = pKrefCntrl->pKrefKeyBuf;
	FLMBYTE *		pIxHasCmpKeys = pKrefCntrl->pIxHasCmpKeys;
	IXD *				pIxd;
	IFD *				pIfd;
	IFD *				pFirstIfd;
	FLMUINT			uiFirstCdlEntry;
	FLMUINT			uiCdlEntry;
	FLMBOOL			bBuildCmpKeys;
	FLMUINT			uiIxEntry;
	FLMUINT			uiTotalIndexes;
  	FLMUINT			uiIfdCnt;
	FLMUINT			uiKeyLen;
	FLMBYTE    		LowUpBuf [MAX_LOWUP_BUF];
	FLD_CONTEXT		fldContext;
	FDICT *			pDict = pDb->pDict;

	LowUpBuf[0] = '\0';

	if( pKrefCntrl->bHaveCompoundKey == FALSE)
	{
		goto Exit;
	}
	
	flmAssert( pKeyBuf != NULL && pIxHasCmpKeys != NULL);
	pKrefCntrl->bHaveCompoundKey = FALSE;

	// Loop through all of the indexes looking for a CDL entry.
	// VISIT: We need to find the indexes faster than looping!

	uiTotalIndexes = pDict->uiIxdCnt;
  	for (uiIxEntry = 0; uiIxEntry < uiTotalIndexes; uiIxEntry++)
	{
		// See if the index has compound keys to build.

		if( !pIxHasCmpKeys [uiIxEntry])
		{
			continue;
		}
		pIxd = pDict->pIxdTbl + uiIxEntry;
		pIxHasCmpKeys [uiIxEntry] = FALSE;
		bBuildCmpKeys = TRUE;

		// Make sure that all required fields are present.

		pFirstIfd = pIfd = pIxd->pFirstIfd;
		uiCdlEntry = uiFirstCdlEntry = (FLMUINT) (pFirstIfd - pDict->pIfdTbl);
		
		for (uiIfdCnt = 0;
			  uiIfdCnt < pIxd->uiNumFlds;
			  pIfd++, uiCdlEntry++, uiIfdCnt++)
		{
			FLMUINT		uiCompoundPos;
			FLMBOOL 		bHitFound;
			
			// Loop on each compound field piece looking for REQUIRED field
			// without any data - then we don't have to build a key.
			
			bHitFound = (pIfd->uiFlags & IFD_REQUIRED_PIECE) ? FALSE : TRUE;
			uiCompoundPos = pIfd->uiCompoundPos;
			
			for(;;)
			{
				if( !bHitFound)
				{
					if( ppCdlTbl [uiCdlEntry])
					{
						bHitFound = TRUE;
					}	
				}
				
				if( (pIfd->uiFlags & IFD_LAST) || 
					 ((pIfd+1)->uiCompoundPos != uiCompoundPos))
				{
					break;
				}
				
				pIfd++;
				uiCdlEntry++;
				uiIfdCnt++;
			}
			
			if( !bHitFound)
			{
				bBuildCmpKeys = FALSE;
				break;
			}
		}
		
		//  Build the individual compound keys.

		if( bBuildCmpKeys)
      {
			uiKeyLen = 0;
			f_memset( &fldContext, 0, sizeof(FLD_CONTEXT));

			if( RC_BAD(rc = KYCmpKeyElmBld( pDb, pIxd, uiContainerNum,
										pFirstIfd,
										uiAction, uiDrn, pbHadUniqueKeys,
										uiFirstCdlEntry, 0, pKeyBuf,
										uiKeyLen, LowUpBuf, 0,
										pRecord, &fldContext)))
			{
				goto Exit;
			}
		}

		// Reset the CDL pointers to NULL

		f_memset( (void *) (&ppCdlTbl [ uiFirstCdlEntry ]),
					 0, sizeof(CDL *) * pIxd->uiNumFlds);
	}

Exit:

	return( rc);
}

/****************************************************************************
Desc:		Build all compound keys for a record.
****************************************************************************/
RCODE KYCmpKeyElmBld(
	FDB *				pDb,
	IXD *				pIxd,					// Index definition.
	FLMUINT			uiContainerNum,
	IFD *				pIfd,					// Index field definition.
	FLMUINT			uiAction,
	FLMUINT			uiDrn,
	FLMBOOL *		pbHadUniqueKeys,
	FLMUINT			uiCdlEntry,			// CDL entry for the IFD. 
  	FLMUINT			uiCompoundPos,		// Compound Piece number - zero based
	FLMBYTE *		pKeyBuf,				// Key buffer to build the key in
	FLMUINT			uiKeyLen,			// Total length left in the key buffer
	FLMBYTE *		pLowUpBuf,			// For POST compound keys place bits here.
	FLMUINT    		uiLuLen,				// Length used in pLowUpBuf.
	FlmRecord *		pRecord,				// Record being indexed.
	FLD_CONTEXT *	pFldContext)		// State to verify all fields are siblings.
{
	RCODE					rc = FERR_OK;
	CDL **				pCdlTbl = pDb->KrefCntrl.ppCdlTbl;
	CDL *					pCdl = pCdlTbl [uiCdlEntry];
	FLMBYTE *			pTmpBuf = NULL;
	void *				pvMark = NULL;
	IFD *					pNextIfdPiece;
	void *				pvField;
	void *				pSaveParentAnchor;
	FLMUINT				uiNextCdlEntry;
	FLMBOOL				bBuiltKeyPiece;
	FLMUINT				uiElmLen;
  	FLMUINT     		uiPostFlag;
	FLMUINT    			uiPostLen;
	FLMUINT		  		uiTempLuLen;
	FLMUINT				uiPieceLuLen;
	FLMUINT				uiNextPiecePos;
	FLMUINT				uiLanguage;
	FLMUINT				uiMaxKeySize = (pIxd->uiContainerNum)
											? MAX_KEY_SIZ
											: MAX_KEY_SIZ - getIxContainerPartLen( pIxd);
	FLMBOOL				bFldIsEncrypted = FALSE;

	if ((uiLanguage = pIxd->uiLanguage) == 0xFFFF)
	{
		uiLanguage = pDb->pFile->FileHdr.uiDefaultLanguage;
	}

	// Test for compound key being tons of levels
	
	flmAssert( uiCompoundPos < MAX_COMPOUND_PIECES);

	// Set if this piece is part of post

	uiPostFlag = IFD_IS_POST_TEXT( pIfd);

	// Add the DELIMITER, except on the first key element

	if( uiCompoundPos != 0)
	{
		IFD *			pPrevIfd = pIfd - 1;
		
		if( (uiLanguage >= FLM_FIRST_DBCS_LANG) &&
			 (uiLanguage <= FLM_LAST_DBCS_LANG) &&
			 (IFD_GET_FIELD_TYPE( pPrevIfd) == FLM_TEXT_TYPE) &&
			 (!(pPrevIfd->uiFlags & IFD_CONTEXT)))
		{
			pKeyBuf [uiKeyLen++] = 0;
		}
		pKeyBuf [uiKeyLen++] = COMPOUND_MARKER;
	}

	// Determine the next IFD compound piece.

	for( pNextIfdPiece = (IFD *)NULL,
			uiNextCdlEntry = uiCdlEntry + 1,
			uiNextPiecePos = 0; 
		 ((pIfd+uiNextPiecePos)->uiFlags & IFD_LAST) == 0; )
	{
		if( (pIfd+uiNextPiecePos)->uiCompoundPos !=
			 (pIfd+uiNextPiecePos+1)->uiCompoundPos)
		{
			pNextIfdPiece = pIfd + uiNextPiecePos + 1;
			uiNextCdlEntry = uiCdlEntry + uiNextPiecePos + 1;
			break;
		}

		if( !pCdl)
		{
			pIfd++;
			pCdl = pCdlTbl [ ++uiCdlEntry];
			uiNextCdlEntry = uiCdlEntry + 1;
		}
		else
		{
			uiNextPiecePos++;
		}
	}

	pSaveParentAnchor = pFldContext->pParentAnchor;
	bBuiltKeyPiece = FALSE;	

	// Loop on each CDL, but do at least once

	while( pCdl || !bBuiltKeyPiece)
	{
		// Restore context values for each iteration
		
		pFldContext->pParentAnchor = pSaveParentAnchor;

		// If there is a field to process, verify that its path is
		// relative to the previous non-null compound pieces
		
		if( pCdl)
		{
			pvField = pCdl->pField;
			
			// Validate the current and previous root contexts

			if( KYValidatePathRelation( pRecord, pCdl->pRootContext, pvField,
										pFldContext, uiCompoundPos) == FERR_FAILURE)
			{
				// This field didn't pass the test, get the next field.
				
				goto Next_CDL_Field;
			}
		}
		else
		{
			pvField = NULL;
		}
		
		bBuiltKeyPiece = TRUE;
		uiPostLen = uiElmLen = 0;
		uiTempLuLen = uiLuLen;

		if( pCdl && (pIfd->uiFlags & (IFD_EACHWORD | IFD_SUBSTRING)) && 
			(pRecord->getDataType( pvField) == FLM_TEXT_TYPE) && 
			pRecord->getDataLength( pvField) &&	
			((!pRecord->isEncryptedField( pvField)	|| 
				(pRecord->isEncryptedField( pvField) &&  
					pDb->pFile->bInLimitedMode))))
		{
			const FLMBYTE *	pText = pRecord->getDataPtr( pvField);
			FLMUINT				uiTextLen = pRecord->getDataLength( pvField);
			FLMUINT	 			uiWordLen;
			FLMBOOL				bReturn;
			FLMBOOL				bFirstSubstring = (pIfd->uiFlags & IFD_SUBSTRING)
																? TRUE 
																: FALSE;

			if( !pTmpBuf)
			{
				pvMark = pDb->TempPool.poolMark();
				
				if( RC_BAD( rc = pDb->TempPool.poolAlloc( MAX_KEY_SIZ + 8, 
					(void **)&pTmpBuf)))
				{
					goto Exit;
				}
			}

			// Loop on each WORD in the value

			for(;;)
			{
				bReturn = (pIfd->uiFlags & IFD_EACHWORD)
								? (FLMBOOL) KYEachWordParse( &pText, &uiTextLen,
												pIfd->uiLimit,
												pTmpBuf, &uiWordLen)
								: (FLMBOOL) KYSubstringParse( &pText, &uiTextLen,
												pIfd->uiFlags, pIfd->uiLimit,
												pTmpBuf, &uiWordLen);
				if( !bReturn)
				{
					break;
				}

				uiTempLuLen = uiLuLen;

				// Compute number of bytes left

				uiElmLen = uiMaxKeySize - uiKeyLen - uiTempLuLen;
				
				if( RC_BAD( rc = KYCollateValue( &pKeyBuf [uiKeyLen], &uiElmLen,
							pTmpBuf, uiWordLen, pIfd->uiFlags, pIfd->uiLimit,
							NULL, &uiPieceLuLen, uiLanguage, TRUE,
							bFirstSubstring, FALSE, NULL)))
				{
					goto Exit;
				}

				bFirstSubstring = FALSE;
				
				if( uiPostFlag)
				{
					uiElmLen -= uiPieceLuLen;
					f_memcpy( &pLowUpBuf [uiTempLuLen], 
										&pKeyBuf[ uiKeyLen + uiElmLen ], uiPieceLuLen);
					uiTempLuLen += uiPieceLuLen;
				}

				if( !pNextIfdPiece)
				{

					// All ISKs have been added so now output the key

					if( uiTempLuLen )
					{
						uiPostLen = KYCombPostParts( pKeyBuf,
															(FLMUINT)(uiKeyLen + uiElmLen),
															pLowUpBuf, uiTempLuLen,
															uiLanguage,
															(FLMUINT)(pIfd->uiFlags) );
					}

					if( RC_BAD( rc = KYAddToKrefTbl( pDb, pIxd, uiContainerNum,
											pIfd, uiAction, uiDrn, pbHadUniqueKeys,
											pKeyBuf,
											(FLMUINT)(uiKeyLen + uiElmLen + uiPostLen),
											TRUE, FALSE, FALSE)))
					{
						goto Cleanup1;
					}
				}
				else if( RC_BAD( rc))
				{
					goto Cleanup1;
				}
				else
				{

					// RECURSIVE CALL to the Next ISK provided no overflow

					if( RC_BAD( rc = KYCmpKeyElmBld( pDb, pIxd, uiContainerNum,
													pNextIfdPiece,
													uiAction, uiDrn, pbHadUniqueKeys,
													uiNextCdlEntry, 
													uiCompoundPos + 1, pKeyBuf,
													(FLMUINT)(uiKeyLen + uiElmLen), pLowUpBuf,
													uiTempLuLen, pRecord, pFldContext)))
					{
						goto Cleanup1;
					}
				}
			
				if( (pIfd->uiFlags & IFD_SUBSTRING) && 
					(uiTextLen == 1 && 
						!(uiLanguage >= FLM_FIRST_DBCS_LANG && 
							uiLanguage <= FLM_LAST_DBCS_LANG)))
				{
					break;
				}
			}

Cleanup1:

			if (RC_BAD( rc))
			{
				goto Exit;
			}
		}
		else
		{
			if( pvField)
			{
				if( pIfd->uiFlags & IFD_CONTEXT)
				{
					pKeyBuf [uiKeyLen] = KY_CONTEXT_PREFIX;
					f_UINT16ToBigEndian( (FLMUINT16)pRecord->getFieldID( pvField),
									&pKeyBuf [uiKeyLen + 1]);
		   		uiKeyLen += KY_CONTEXT_LEN;
				}
				else if( pRecord->getDataLength( pvField))
				{
					const FLMBYTE *	pExportValue = pRecord->getDataPtr( pvField);
					FLMUINT				uiDataLength = pRecord->getDataLength( pvField);
					
					if (pRecord->isEncryptedField( pvField) &&
						 pDb->pFile->bInLimitedMode)
					{
						pExportValue = pRecord->getEncryptionDataPtr( pvField);
						uiDataLength = pRecord->getEncryptedDataLength( pvField);
						bFldIsEncrypted = TRUE;
					}

       			// Compute number of bytes left

					uiElmLen = uiMaxKeySize - uiKeyLen - uiLuLen;
					
					if( RC_BAD( rc = KYCollateValue( &pKeyBuf [uiKeyLen], &uiElmLen,
									pExportValue,
									uiDataLength, pIfd->uiFlags,
									pIfd->uiLimit, NULL, &uiPieceLuLen, 
									uiLanguage, TRUE, FALSE, FALSE, NULL, NULL,
									bFldIsEncrypted)))
					{
						goto Exit;
					}

					if( uiPostFlag )
					{
						uiElmLen -= uiPieceLuLen;
						f_memcpy( &pLowUpBuf [uiTempLuLen], 
											&pKeyBuf [uiKeyLen + uiElmLen], uiPieceLuLen);
						uiTempLuLen += uiPieceLuLen;
					}
				}
			}

			if( !pNextIfdPiece)
			{

				// All IFDs have been added so now output the key

				if( uiTempLuLen)
				{
					uiPostLen = KYCombPostParts( pKeyBuf,
														(FLMUINT)(uiKeyLen + uiElmLen),
														pLowUpBuf, uiTempLuLen,
														uiLanguage, (FLMUINT)(pIfd->uiFlags));
				}

				if( RC_BAD( rc = KYAddToKrefTbl( pDb, pIxd, uiContainerNum,
											pIfd, uiAction, uiDrn, pbHadUniqueKeys,
											pKeyBuf,
											(FLMUINT)(uiKeyLen + uiElmLen + uiPostLen),
											TRUE, FALSE, bFldIsEncrypted)))
				{
					goto Exit;
				}
			}
			else if( RC_BAD( rc))
			{
				goto Exit;
			}
			else
			{
				if( RC_BAD( rc = KYCmpKeyElmBld( pDb, pIxd, uiContainerNum,
												pNextIfdPiece,
												uiAction, uiDrn, pbHadUniqueKeys,
												uiNextCdlEntry, 
												uiCompoundPos + 1, pKeyBuf,
												(FLMUINT)(uiKeyLen + uiElmLen), pLowUpBuf,
												uiTempLuLen, pRecord, pFldContext)))
				{
					goto Exit;
				}
			}
		}
		
Next_CDL_Field:
		
		if( pCdl)
		{
			pCdl = pCdl->pNext;
		}

		// If the CDL list is empty, goto the next IFD if same uiCompoundPos.

		while ((!pCdl) && ((pIfd->uiFlags & IFD_LAST) == 0) && 
				(pIfd->uiCompoundPos == (pIfd+1)->uiCompoundPos))
		{
			pIfd++;
			pCdl = pCdlTbl [++uiCdlEntry];
		}
		
		// If all fields failed the validate field path test and this piece of
		// the compound key is required, then goto exit NOW which will not
		// build any key with the previous built key pieces.

		if( !pCdl && !bBuiltKeyPiece && ((pIfd->uiFlags & IFD_OPTIONAL) == 0))
		{
			goto Exit;
		}
	}

Exit:

	if( pvMark)
	{
		pDb->TempPool.poolReset( pvMark);
	}

	return( rc);
}


/****************************************************************************
Desc:		Validate that the current field is related to the other fields
			in the compound key index.  The context (left-most) fields of the
			field paths must all be siblings of each other in order to 
			be related.
****************************************************************************/
RCODE KYValidatePathRelation(
	FlmRecord *		pRecord,
	void *			pCurContext,
	void *			pCurFld,
	FLD_CONTEXT *	pFldContext,
	FLMUINT			uiCompoundPos)
{
	RCODE			rc = FERR_OK;
	void *		pCurParent;
	FLMUINT		uiPrevCompoundPos;
	FLMBOOL		bMatchedContext;

	// If too many compound levels, just exit and don't check.
	
	if( uiCompoundPos >= MAX_COMPOUND_PIECES)
	{
		goto Exit;
	}

	pCurParent = pRecord->parent( pCurContext);

	// First time in is the easy case - just set the parent anchor.
	// A value of NULL is OK.

	if( uiCompoundPos == 0)
	{
		pFldContext->pParentAnchor = pCurParent;
		goto Exit;
	}

	bMatchedContext = FALSE;
	uiPrevCompoundPos = uiCompoundPos;
	
	while( uiPrevCompoundPos--)
	{
		if( pFldContext->rootContexts[ uiPrevCompoundPos] == pCurContext)
		{
			// Check this field against the current field values.

			rc = KYVerifyMatchingPaths( pRecord, pCurContext, pCurFld, 
						pFldContext->leafFlds[ uiPrevCompoundPos]);

			// Return failure on any failure.  Otherwise continue.
			
			if( rc == FERR_FAILURE)
			{
				goto Exit;
			}
			
			bMatchedContext = TRUE;
		}
	}
	
	if( bMatchedContext)
	{
		// If we had some base relation match, there is no need to 
		// verify that the parents are the same.
		
		goto Exit;
	}

	// Verify that the parent anchor equals the parent of pCurContext.

	if( pFldContext->pParentAnchor != pCurParent)
	{
		rc = RC_SET( FERR_FAILURE);
		goto Exit;
	}

Exit:

	// Set the state variables for this compound position.
	
	if( RC_OK(rc))
	{
		pFldContext->rootContexts[ uiCompoundPos ] = pCurContext;
		pFldContext->leafFlds[ uiCompoundPos] = pCurFld;
	}
	
	return( rc);
}

/****************************************************************************
Desc:		Verify that two paths with a common context match paths.
			If the tag of pCurContext has a previous match in the compound
			key, the field should also match (more of a relational validation).
			
			This means that for keys (A.B.C.D  AND A.B.C.E) the 'A.B.C' fields 
			should be the same field.  ALL previous field pieces must be 
			checked for this.  This could be (but isn't being) done by 
			finding the best match" and only comparing the current with the 
			best match.

			Hard Example:
				Do these fields match - A.B.D.E.F and A.C.D.E.G?
				We don't want to keep the field path of the two fields around 
				because this is more state than we need right now.  These match
				only if the 'A's are the same field.
		
					A						A
						B						C
							D						D
								E						E
									F						G
****************************************************************************/
RCODE KYVerifyMatchingPaths(
	FlmRecord *		pRecord,
	void *			pCurContext,			// Same value as pMatchFld's context.
	void *			pCurFld,					// Current field
	void *			pMatchFld)				// Some field from a previous piece.
{
	RCODE				rc = FERR_OK;
	FLMUINT			uiCurLevel;
	FLMUINT			uiMatchLevel;
	FLMBOOL			bMismatchFound = FALSE;

	// If a field equals a context then don't bother to check.

	if( (pCurContext == pCurFld) || (pCurContext == pMatchFld))
	{
		goto Exit;
	}

	// Go up the parent line until levels match.
	
	uiCurLevel = pRecord->getLevel( pCurFld);
	uiMatchLevel = pRecord->getLevel( pMatchFld);
	flmAssert( pRecord->getLevel( pCurContext) < uiCurLevel);

	while( uiCurLevel != uiMatchLevel)
	{
		if( uiCurLevel > uiMatchLevel)
		{
			pCurFld = pRecord->parent( pCurFld);
			uiCurLevel--;
		}
		else
		{
			pMatchFld = pRecord->parent( pMatchFld);
			uiMatchLevel--;
		}
	}
	
	// Go up until you hit the matching context.

	while( pCurFld != pCurContext)
	{
		if( pRecord->getFieldID( pCurFld) == pRecord->getFieldID( pMatchFld))
		{
			// If the fields are NOT the same we MAY have a mismatch.
			
			if( pCurFld != pMatchFld)
			{
				bMismatchFound = TRUE;
			}
		}
		else
		{
			// Tags are different - start over checking
			
			bMismatchFound = FALSE;
		}
		
		// Go to the next parent.
		
		pCurFld = pRecord->parent( pCurFld);
		pMatchFld = pRecord->parent( pMatchFld);
	}
	
	if( bMismatchFound)
	{
		rc = RC_SET( FERR_FAILURE);
		goto Exit;
	}

Exit:

	return( rc);
}


/****************************************************************************
Desc:		Combine the bits from all POST text keys.
****************************************************************************/
FLMUINT KYCombPostParts(
	FLMBYTE *	pKeyBuf,
	FLMUINT	  	uiKeyLen,
	FLMBYTE *	pLowUpBuf,
	FLMUINT	  	uiLuLen,
	FLMUINT		uiLanguage,
	FLMUINT		uiIfdAttr)
{
	FLMUINT		wReturnLen;

	if( !uiLuLen)
	{
		return( 0);
	}

	wReturnLen = (FLMUINT)(uiLuLen + 2);
	if( (uiLanguage >= FLM_FIRST_DBCS_LANG) && 
		 (uiLanguage <= FLM_LAST_DBCS_LANG) &&
		 ((uiIfdAttr & 0x0F) == FLM_TEXT_TYPE) &&
		 (!(uiIfdAttr & IFD_CONTEXT )))
	{
		pKeyBuf [uiKeyLen++] = 0;
		wReturnLen++;
	}
	
	pKeyBuf [uiKeyLen++] = END_COMPOUND_MARKER;
	f_memcpy( &pKeyBuf [uiKeyLen], pLowUpBuf, uiLuLen);
	pKeyBuf [uiKeyLen + uiLuLen] = (FLMBYTE) uiLuLen;
	
	return( wReturnLen );
}

/****************************************************************************
Desc:	Create an index key given a keyTree and index definition. This routine
		works on a normal data tree - used in FlmKeyBuild. where
     	a data record is traversed with field paths being checked.
****************************************************************************/
RCODE KYTreeToKey(
	FDB *				pDb,
	IXD *				pIxd,
	FlmRecord *		pRecord,
	FLMUINT			uiContainerNum,
	FLMBYTE * 		pKeyBuf,
	FLMUINT *		puiKeyLenRV,
	FLMUINT			uiFlags)
{
	RCODE					rc = FERR_OK;
	IFD *					pIfd;
	void *				pvMatchField;
	FLMBYTE *			pToKey = pKeyBuf;
	const FLMBYTE *	pExportPtr;
	FLMUINT				uiToKeyLen;
	FLMUINT				uiTotalLen;
	FLMINT				nth;
	FLMINT				iMissingFlds;
	FLMUINT     		uiIskPostFlag;
	FLMUINT    			uiLuLen;
	FLMUINT				uiPieceLuLen;
	FLMUINT				uiLanguage;
	FLMUINT				uiIsPost = 0;
	FLMBOOL				bIsAsianCompound;
	FLMBOOL				bIsCompound;
	FLMBYTE    			LowUpBuf [MAX_LOWUP_BUF];
	FLMUINT				uiMaxKeySize = (pIxd->uiContainerNum)
										? MAX_KEY_SIZ
										: MAX_KEY_SIZ - getIxContainerPartLen( pIxd);
	
	if ((uiLanguage = pIxd->uiLanguage) == 0xFFFF)
	{
		uiLanguage = pDb->pFile->FileHdr.uiDefaultLanguage;
	}
	
	uiLuLen = 0;
	iMissingFlds = 0;
	uiTotalLen = 0;

	pIfd = pIxd->pFirstIfd;
	bIsCompound = (pIfd->uiFlags & IFD_COMPOUND) ? TRUE : FALSE;
	
	for (;;pIfd++)
	{
		uiIsPost |= (FLMUINT) (uiIskPostFlag = (FLMUINT)IFD_IS_POST_TEXT( pIfd));

  		bIsAsianCompound =((uiLanguage >= FLM_FIRST_DBCS_LANG) && 
								 (uiLanguage <= FLM_LAST_DBCS_LANG) && 
								 (IFD_GET_FIELD_TYPE( pIfd) == FLM_TEXT_TYPE) &&
								 (!(pIfd->uiFlags & IFD_CONTEXT)));
		nth = 1;
		uiToKeyLen = 0;

		// Find matching node in the tree - if not found skip and continue

FIND_NXT:

		if( (pvMatchField = pRecord->find( pRecord->root(), 
				pIfd->uiFldNum, nth)) != NULL)
		{

			// Match was found, now if flagged, validate its parent path

			if( uiFlags & KY_PATH_CHK_FLAG)
			{
				FLMUINT *	puiFieldPath;
				void *		pTempField = pvMatchField;
				FLMUINT		uiCurrentFld;

				puiFieldPath = pIfd->pFieldPathCToP;
				
				for( uiCurrentFld = 1; puiFieldPath [uiCurrentFld]; uiCurrentFld++)
				{
					if( ((pTempField = pRecord->parent( pTempField)) == NULL) ||
						 (pRecord->getFieldID( pTempField) != puiFieldPath [uiCurrentFld]))
					{
						nth++;
						goto FIND_NXT;
					}
				}
			}
			
      	// Convert the node's key value to the index type.
			// Compute maximum bytes remaining.

			uiToKeyLen = uiMaxKeySize - uiTotalLen;

			// Take the tag and make it the key

			if( pIfd->uiFlags & IFD_CONTEXT)
			{

        		// Output the tag number.

				*pToKey = KY_CONTEXT_PREFIX;
				f_UINT16ToBigEndian( (FLMUINT16) pRecord->getFieldID( 
					pvMatchField), &pToKey [1]);
				uiToKeyLen = KY_CONTEXT_LEN;
			}
			else
			{
				pExportPtr = pRecord->getDataPtr( pvMatchField);

				if( RC_BAD( rc = KYCollateValue( pToKey, &uiToKeyLen, pExportPtr,
							pRecord->getDataLength( pvMatchField),
							pIfd->uiFlags, pIfd->uiLimit,
							NULL, &uiPieceLuLen, uiLanguage, bIsCompound, 
							(FLMBOOL) ((pIfd->uiFlags & IFD_SUBSTRING)
									? (pRecord->isLeftTruncated( pvMatchField) 
										? FALSE : TRUE)
									: FALSE),
							pRecord->isRightTruncated( pvMatchField), NULL)))
				{
					goto Exit;
				}

				if( pRecord->isRightTruncated( pvMatchField))
				{
					// If the string is EXACTLY the length of the truncation
					// length then it should, but doesn't, set the truncation flag.  
					// The code didn't match the design intent.
					
					f_memmove( &pToKey[ uiToKeyLen - uiPieceLuLen + 1],	
								  &pToKey[ uiToKeyLen - uiPieceLuLen], uiPieceLuLen);
					pToKey[ uiToKeyLen - uiPieceLuLen] = COLL_TRUNCATED;
					uiToKeyLen++;
				}

				if( uiIskPostFlag)
				{
					uiToKeyLen -= uiPieceLuLen;
					f_memcpy( &LowUpBuf [uiLuLen], 
									&pToKey [uiToKeyLen], uiPieceLuLen );
					uiLuLen += uiPieceLuLen;
				}
			}
		}

		//	Check here if key found else the fields are missing.

		if( uiToKeyLen)
		{
			iMissingFlds = 0;
			pToKey    += uiToKeyLen;
			uiTotalLen += uiToKeyLen;

			// Go to the last IFD with the same compound position.

			while( ((pIfd->uiFlags & IFD_LAST) == 0) &&
					 (pIfd->uiCompoundPos == (pIfd+1)->uiCompoundPos))
			{
				pIfd++;
			}
		}
		else
		{
			// Continue if there are still fields with same compound position.
			
			if( ((pIfd->uiFlags & IFD_LAST) == 0) &&
				 (pIfd->uiCompoundPos == (pIfd+1)->uiCompoundPos))
			{
				continue;
			}
		
			iMissingFlds++;
			
			if( bIsAsianCompound)
			{
				iMissingFlds++;
			}
   	}

		// Check if done. 
		
		if( pIfd->uiFlags & IFD_LAST)
		{
   	 	break;
		}

		if( bIsCompound)
		{
			if( bIsAsianCompound)
			{
				*pToKey++ = 0;
				uiTotalLen++;
			}	
			
			*pToKey++ = COMPOUND_MARKER;
			uiTotalLen++;
		}
		else if( uiToKeyLen)
		{
			break;
		}
	}
	
	// Back up iMissingFlds-1 because last
	// field does not have compound marker.
	// Add 4 bytes of foxes for high values.

	if( iMissingFlds && (uiFlags & KY_HIGH_FLAG) && bIsCompound)
	{
		// Ignore the last one or two iMissingFlds values because a compound
		// marker was not added to the end of the key.

		if( bIsAsianCompound)
		{
			iMissingFlds--;
		}
		
		uiTotalLen -= --iMissingFlds;
		pToKey -= iMissingFlds;

		// Fill with high values to the end of the buffer.
		// It is easy for double byte ASIAN collation values to all be 0xFF.

		if( uiTotalLen < uiMaxKeySize)
		{
			f_memset( pToKey, 0xFF, uiMaxKeySize - uiTotalLen );
			pToKey += (uiMaxKeySize - uiTotalLen);
			uiTotalLen += (uiMaxKeySize - uiTotalLen);
		}
	}
	else if( uiIsPost)
	{
		uiTotalLen += KYCombPostParts( pKeyBuf, uiTotalLen, LowUpBuf, uiLuLen,
											  uiLanguage, (FLMUINT)(pIfd->uiFlags));
	}

	// Add container number to the key if the index is on all containers.

	if (!pIxd->uiContainerNum)
	{
		appendContainerToKey( pIxd, uiContainerNum, pKeyBuf, &uiTotalLen);
	}
	
	*puiKeyLenRV = uiTotalLen;
	
Exit:

	return( rc);
}

/****************************************************************************
Desc:	Build a collated key value piece.
****************************************************************************/
RCODE KYCollateValue(
	FLMBYTE *			pDest,
	FLMUINT *			puiDestLenRV,
	const FLMBYTE *	pSrc,
	FLMUINT				uiSrcLen,
	FLMUINT				uiFlags,
	FLMUINT				uiLimit,
	FLMUINT *			puiCollationLen,
	FLMUINT *			puiLuLenRV,
	FLMUINT				uiLanguage,
	FLMBOOL				bCompoundPiece,
	FLMBOOL				bFirstSubstring,
	FLMBOOL				bInputTruncated,
	FLMBOOL *			pbDataTruncated,
	FLMBOOL *			pbOriginalCharsLost,
	FLMBOOL				bFldIsEncrypted)
{
	RCODE			rc = FERR_OK;
	FLMUINT		uiDestLen;
	FLMUINT		uiDataType = uiFlags & 0x0F;

	// Treat an encrypted field as binary for collation purposes.
	
	if (bFldIsEncrypted)
	{
		uiDataType = FLM_BINARY_TYPE;
	}
	
	if( puiLuLenRV)
	{
		*puiLuLenRV = 0;
	}

	if( (uiDestLen = *puiDestLenRV) == 0)
	{
		return( RC_SET( FERR_KEY_OVERFLOW));
	}

	if( uiDataType == FLM_TEXT_TYPE)
	{
		FLMUINT	uiCharLimit;
		FLMBYTE	byTmpBuf[ MAX_KEY_SIZ + 8];

		if( uiFlags & (IFD_MIN_SPACES | IFD_NO_UNDERSCORE | 
							IFD_NO_SPACE | IFD_NO_DASH | IFD_ESC_CHAR)) 
		{
			if( RC_BAD( rc = KYFormatText( 
					pSrc, uiSrcLen,
					(uiFlags & IFD_MIN_SPACES) ? TRUE : FALSE,
					(uiFlags & IFD_NO_UNDERSCORE) ? TRUE : FALSE,
					(uiFlags & IFD_NO_SPACE) ? TRUE : FALSE,
					(uiFlags & IFD_NO_DASH) ? TRUE : FALSE,
					(uiFlags & IFD_ESC_CHAR) ? TRUE : FALSE,
					bInputTruncated, byTmpBuf, &uiSrcLen)))
			{
				goto Exit;
			}
			
			pSrc = (FLMBYTE *) byTmpBuf;
		}

		uiCharLimit = uiLimit ? uiLimit : IFD_DEFAULT_LIMIT;
		
		if( (uiLanguage >= FLM_FIRST_DBCS_LANG ) && (uiLanguage <= FLM_LAST_DBCS_LANG))
		{
			rc = AsiaFlmTextToColStr( pSrc, uiSrcLen, pDest, &uiDestLen,
								(uiFlags & IFD_UPPER), puiCollationLen, puiLuLenRV, 
								uiCharLimit, bFirstSubstring, pbDataTruncated);
		}
		else
		{
			rc = FTextToColStr( pSrc, uiSrcLen, pDest, &uiDestLen,
								(uiFlags & IFD_UPPER), puiCollationLen, puiLuLenRV, 
								uiLanguage, uiCharLimit, bFirstSubstring,
								pbOriginalCharsLost, pbDataTruncated);
		}
	}
	
	// uiDestLen could be set to zero if text and no value.

	if( !uiSrcLen || !uiDestLen)
	{
		if( !bCompoundPiece)
		{
			// Zero length key. Any value under 0x1F would work.
			
			if( (uiLanguage >= FLM_FIRST_DBCS_LANG ) && 
				 (uiLanguage <= FLM_LAST_DBCS_LANG))
			{
				pDest [0] = 0;
				pDest [1] = NULL_KEY_MARKER;
				uiDestLen = 2;
			}
			else
			{
				pDest [0] = NULL_KEY_MARKER;
				uiDestLen = 1;
			}
		}
		else
		{
			uiDestLen = 0;
		}
		
		goto Exit;
	}

 	switch (uiDataType)
	{
		case FLM_TEXT_TYPE:
		{
			break;
		}

		case FLM_NUMBER_TYPE:
		{
			FLMBYTE *			pOutput = pDest + 1;
			const FLMBYTE *	pTempSrc = pSrc;
			FLMUINT				uiBytesOutput = 1;
			FLMUINT				uiMaxOutLen = uiDestLen;
			FLMINT				iHiInNibble = 1;
			FLMINT				iHiOutNibble = 1;
			FLMUINT				uiSigSign = SIG_POS;
			FLMUINT				uiMagnitude = COLLATED_NUM_EXP_BIAS - 1;
			FLMBYTE				byValue;

			for (rc = FERR_OK;;)
			{
				switch( byValue = (iHiInNibble++ & 1)
							?	(FLMBYTE)(*pTempSrc >> 4)
							:	(FLMBYTE)(*pTempSrc++ & 0x0F))
				{
					case 0x0B:								// Negative Sign code
					{
						uiSigSign = 0;
						continue;
					}
					
					case 0x0A:  							// Ignore for now - not implemented
					case 0x0C:
					case 0x0D:
					case 0x0E:
					{
						continue;
					}
					
					case 0x0F:								// Terminator
					{
						*pDest = (FLMBYTE)(uiSigSign |
										((uiSigSign ? uiMagnitude : ~uiMagnitude) & 0x7F));
						goto NumDone;
					}
					
					default:
					{
						uiMagnitude++;
						
						if( uiSigSign)
						{
							byValue += COLLATED_DIGIT_OFFSET;
						}
						else
						{
							// Invert for key collation
							
							byValue = (FLMBYTE)((COLLATED_DIGIT_OFFSET + 9) - byValue);
						}
						
						if( iHiOutNibble++ & 1)
						{
							if( uiBytesOutput++ == uiMaxOutLen)
							{
								uiBytesOutput = 0;
								rc = RC_SET( FERR_KEY_OVERFLOW);
								goto NumDone;
							}

							*pOutput = (FLMBYTE)((byValue << 4) | 0x0F);
						}
						else
						{
							*pOutput++ &= (FLMBYTE)(byValue | 0xF0); 
						}
						
						continue;
					}
				}
			}
			
NumDone:

			uiDestLen = uiBytesOutput;
			break;
		}

		case FLM_BINARY_TYPE:
		{
			FLMUINT				uiLength = uiSrcLen;
			const FLMBYTE *	tmpSrc = pSrc;
			FLMBYTE *			tmpDest = pDest;
			FLMBOOL				bTruncated = FALSE;

			if( uiLength >= uiLimit)
			{
				uiLength = uiLimit;
				bTruncated = TRUE;
			}

			if( uiDestLen < (uiLength << 1))
			{
				// Compute length so will not overflow

				uiLength = (FLMUINT)(uiDestLen >> 1);
			}
			else
			{
				uiDestLen = (FLMUINT)(uiLength << 1);
			}

			// Convert each byte to two bytes

			while( uiLength--)
			{
				*tmpDest++ = (FLMBYTE)(COLLS + ((*tmpSrc) >> 4));
				*tmpDest++ = (FLMBYTE)(COLLS + ((*tmpSrc++) & 0x0F));
			}

			if( bTruncated)
			{
				*tmpDest++ = COLL_TRUNCATED;
			}
			
			break;
		}

		case FLM_CONTEXT_TYPE:
		{
			if( uiDestLen < 5)
			{
				uiDestLen = 0;
				rc = RC_SET( FERR_KEY_OVERFLOW);
			}
			else
			{
				*pDest = 0x1F;
				f_UINT32ToBigEndian( FB2UD( pSrc), pDest + 1);
				uiDestLen = 5;
				rc = FERR_OK;
			}
			
			break;
		}

		default:
		{
			rc = RC_SET( FERR_CONV_BAD_DEST_TYPE);
			break;
		}
	}

Exit:

	*puiDestLenRV = uiDestLen;
	return( rc);
}

/****************************************************************************
Desc:		Format text removing leading and trailing spaces.  Treat 
			underscores as spaces.  As options, remove all spaces and dashes.
Ret:		FERR_OK always.  WIll truncate so text will fill MAX_KEY_SIZ.
			Allocate 8 more than MAX_KEY_SIZ for psDestBuf.
Visit:	Pass in uiLimit and pass back a truncated flag when the
			string is truncated.  This was not done because we will have
			to get the exact truncated count that is done in f_tocoll.cpp
			and that could introduce some bugs.
****************************************************************************/
RCODE KYFormatText(
	const FLMBYTE *	psVal,			// Points to value source 
	FLMUINT				uiSrcLen,		// Length of the key-NOT NULL TERMINATED
												// Booleans below are zero or NON-zero
	FLMBOOL				bMinSpaces,		// Remove leading/trailing/multiple spaces
	FLMBOOL				bNoUnderscore,	// Convert underscore to space
	FLMBOOL				bNoSpace,		// Remove all spaces
	FLMBOOL				bNoDash,			// Remove all dashes (hyphens)
	FLMBOOL				bEscChar,		// Literal '*' or '\\' char after '\\' esc char
	FLMBOOL				bInputTruncated,// TRUE if input key data is truncated.
	FLMBYTE *			psDestBuf,		// (out) Destination buffer
	FLMUINT *			puuiDestLen)	// (out) Length of key in destination buffer.
{
	RCODE				rc = FERR_OK;
	FLMBYTE *		psDestPtr = psDestBuf;
	FLMBYTE			ucValue;
	FLMBYTE			objType;
	FLMUINT			uiCurPos = 0;
	FLMUINT			uiDestPos = 0;
	FLMUINT			uiOldDestPos = 0;
	FLMUINT 			objLength;
	FLMBOOL			bLastCharWasSpace = bMinSpaces;

	for( ; uiCurPos < uiSrcLen && uiDestPos < MAX_KEY_SIZ - 1;
		 uiCurPos += objLength)
	{
		ucValue = psVal [uiCurPos];
		objLength = 1;
		uiOldDestPos = uiDestPos;
		objType = (FLMBYTE)(flmTextObjType( ucValue));

		switch( objType)
		{
			case ASCII_CHAR_CODE:  						// 0nnnnnnn
			{
				if( (ucValue == ASCII_SPACE) || 
					 ((ucValue == ASCII_UNDERSCORE) && bNoUnderscore))
				{
					if( bLastCharWasSpace || bNoSpace)
					{
						break;
					}
					
					// Sets to true if we want to minimize spaces.
					
					bLastCharWasSpace = bMinSpaces;
					ucValue = ASCII_SPACE;
				}
				else if( (ucValue == ASCII_DASH) && bNoDash)
				{
					break;
				}
				else
				{
					if( (ucValue == ASCII_BACKSLASH) && bEscChar && 
							(psVal [uiCurPos+1] == ASCII_WILDCARD || 
							 psVal [uiCurPos+1] == ASCII_BACKSLASH))
					{
						ucValue = psVal [uiCurPos+1];
						objLength++;
					}
					
					bLastCharWasSpace = FALSE;
				}
				
				psDestPtr[ uiDestPos++] = ucValue;
				break;
			}
			
			case WHITE_SPACE_CODE:						// 110nnnnn
			{
				if( bLastCharWasSpace || bNoSpace)
				{
					break;
				}
			
				// Sets to true if we want to minimize spaces.
				
				bLastCharWasSpace = bMinSpaces;
				psDestPtr[ uiDestPos++] = ASCII_SPACE;
				break;
			}
			
			case CHAR_SET_CODE:	  						// 10nnnnnn
			case UNK_EQ_1_CODE:
			case OEM_CODE:
			{
				bLastCharWasSpace = FALSE;
				psDestPtr[ uiDestPos++] = psVal [uiCurPos];
				psDestPtr[ uiDestPos++] = psVal [uiCurPos+1];
				objLength = 2;
				break;
			}
			
			case UNICODE_CODE:							// Unconvertable UNICODE code
			case EXT_CHAR_CODE:							// Full extended character
			{
				bLastCharWasSpace = FALSE;
				psDestPtr[ uiDestPos++] = psVal [uiCurPos];
				psDestPtr[ uiDestPos++] = psVal [uiCurPos+1];
				psDestPtr[ uiDestPos++] = psVal [uiCurPos+2];
				objLength = 3;
				break;
			}
			
			case UNK_GT_255_CODE:
			{
				bLastCharWasSpace = FALSE;
				objLength = 1 + sizeof( FLMUINT) + FB2UW( &psVal [uiCurPos + 1]);
				break;
			}
			
			case UNK_LE_255_CODE:
			{
				bLastCharWasSpace = FALSE;
				objLength = 2 + (FLMUINT) (psVal [uiCurPos+1]);
				break;
			}
			
			default:
			{
				psDestPtr[ uiDestPos++] = psVal [uiCurPos];
				bLastCharWasSpace = FALSE;
				break;
			}
		}
	}

	// On overflow - back out of the last character.
	
	if( uiDestPos >= MAX_KEY_SIZ - 1)
	{
		uiDestPos = uiOldDestPos;
		bLastCharWasSpace = FALSE;
	}
	
	// Handle the trailing space if present.
	// bLastCharWasSpace cannot be set to true if bNoSpace is true.
	
	if( bLastCharWasSpace && uiDestPos && !bInputTruncated)
	{
		uiDestPos--;
	}
	
	psDestPtr[ uiDestPos] = '\0';
	*puuiDestLen = (FLMUINT) uiDestPos;

	return( rc);
}

/****************************************************************************
Desc:  	Convert a text string to a collated string.
****************************************************************************/
RCODE AsiaFlmTextToColStr(
	const FLMBYTE *	Str,					// Points to the internal TEXT string
	FLMUINT 				StrLen,				// Length of the internal TEXT string
	FLMBYTE *			ColStr,				// Output collated string
	FLMUINT *			ColStrLenRV,		// Collated string length return value
													// Input value is MAX num of bytes in buffer
	FLMUINT				UppercaseFlag,		// Set if to convert to uppercase
	FLMUINT *			puiCollationLen,	// Returns the collation bytes length
	FLMUINT *			puiCaseLen,			// Returns length of case bytes
	FLMUINT				uiCharLimit,		// Max number of characters in this key piece
	FLMBOOL				bFirstSubstring,	// TRUE is this is the first substring key
	FLMBOOL *			pbDataTruncated)
{
	RCODE					rc = FERR_OK;
	const FLMBYTE *	pszStrEnd;
	FLMUINT				Length;
	FLMUINT				uiTargetColLen = *ColStrLenRV - 12;
	FLMBYTE				SubColBuf[MAX_SUBCOL_BUF + 1];
	FLMBYTE				LowUpBuf[MAX_LOWUP_BUF + MAX_LOWUP_BUF + 2];
	FLMUINT				ColLen;
	FLMUINT				SubColBitPos;
	FLMUINT				LowUpBitPos;
	FLMUINT				Flags;
	FLMUINT16			NextWpChar;
	FLMUINT16			UnicodeChar;
	FLMUINT16			ColValue;
	FLMBOOL				bDataTruncated = FALSE;

	ColLen = 0;
	SubColBitPos = 0;
	LowUpBitPos = 0;
	Flags = 0;
	UnicodeChar = 0;
	ColValue = 0;

	// Don't allow any key component to exceed 256 bytes regardless of the
	// user-specified character or byte limit. The goal is to prevent any
	// single key piece from consuming too much of the key (which is
	// limited to 640 bytes) and thus "starving" other pieces, resulting in
	// a key overflow error.

	if (uiTargetColLen > 256)
	{
		uiTargetColLen = 256;
	}

	// Make sure SubColBuf and LowUpBuf are set to 0's

	f_memset( SubColBuf, 0, sizeof( SubColBuf));
	f_memset( LowUpBuf, 0, sizeof( LowUpBuf));

	pszStrEnd = &Str[StrLen];
	NextWpChar = 0;

	while ((Str < pszStrEnd) || NextWpChar || UnicodeChar)
	{
		FLMUINT16	WpChar;								// Current WP character
		FLMUINT		ObjLength;
		FLMUINT16	SubColVal;							// Sub-collated value (diacritic)
		FLMBYTE		CaseFlags;

		// Get the next character from the TEXT String. NOTE: OEM
		// characters will be returned as character set ZERO, the character
		// will be greater than 127.

		WpChar = NextWpChar;

		for (NextWpChar = 0;
			  (!WpChar || !NextWpChar) && !UnicodeChar && (Str < pszStrEnd);
			  Str += ObjLength)
		{
			FLMBYTE		ObjType;
			FLMBYTE		CurByte;
			FLMUINT16	CurWpChar = 0;

			CurByte = *Str;
			ObjType = (FLMBYTE) (flmTextObjType( CurByte));
			ObjLength = 1;
			
			switch (ObjType)
			{
				case ASCII_CHAR_CODE:
				{
					CurWpChar = (FLMUINT16) CurByte;
					break;
				}
				
				case CHAR_SET_CODE:
				{
					ObjLength = 2;
					CurWpChar = (FLMUINT16) (((FLMUINT16) (CurByte & 
													(~CHAR_SET_MASK)) << 8) + 
													(FLMUINT16) *(Str + 1));
					break;
				}
				
				case WHITE_SPACE_CODE:
				{
					CurByte &= (~WHITE_SPACE_MASK);
					CurWpChar = ((CurByte == HARD_HYPHEN) ||
									 (CurByte == HARD_HYPHEN_EOL) ||
									 (CurByte == HARD_HYPHEN_EOP)) 
									 	? 0x2D			// Minus sign - character set zero
										: 0x20;			// Space -- character set zero
					break;
				}

				case UNK_GT_255_CODE:
				{
					ObjLength = 1 + sizeof(FLMUINT16) + FB2UW( Str + 1);
					break;
				}
				
				case UNK_LE_255_CODE:
				{
					ObjLength = 2 + (FLMUINT16) * (Str + 1);
					break;
				}
				
				case UNK_EQ_1_CODE:
				{
					ObjLength = 2;
					break;
				}
				
				case EXT_CHAR_CODE:
				{
					ObjLength = 3;
					CurWpChar = (FLMUINT16) (((FLMUINT16) *(Str + 1) << 8) +
													  (FLMUINT16) *(Str + 2));
					break;
				}
				
				case OEM_CODE:
				{
					ObjLength = 2;

					// OEM characters are always >= 128.
					// We use character set zero to process them.
					
					CurWpChar = (FLMUINT16) * (Str + 1);
					break;
				}
				
				case UNICODE_CODE:
				{
					ObjLength = 3;
					UnicodeChar = (FLMUINT16) (((FLMUINT16) *(Str + 1) << 8) +
														 (FLMUINT16) *(Str + 2));
					CurWpChar = 0;
					break;
				}
				
				default:
				{
					// Shouldn't ever get to this point
					
					continue;
				}
			}

			if (!WpChar)
			{
				WpChar = CurWpChar;
			}
			else
			{
				NextWpChar = CurWpChar;
			}
		}

		// If we didn't get a character, break out of the outer processing
		// loop.

		if (!WpChar && !UnicodeChar)
		{
			break;
		}

		if (WpChar)
		{
			if (flmAsiaGetCollation( WpChar, NextWpChar, ColValue, &ColValue,
											&SubColVal, &CaseFlags,
											(FLMUINT16) UppercaseFlag) == 2)
			{

				// Took the NextWpChar value

				NextWpChar = 0;
			}
		}
		else
		{

			// This handles all of the UNICODE characters that could not be
			// converted to WP characters - which will include most of the
			// Asian characters.

			CaseFlags = 0;
			if (UnicodeChar < 0x20)
			{
				ColValue = 0xFFFF;

				// Setting SubColVal to a high code will ensure that the code
				// that the UnicodeChar will be stored in its full 16 bits in
				// the sub-collation area.

				SubColVal = 0xFFFF;

				// NOTE: UnicodeChar SHOULD NOT be set to zero here. It will
				// be set to zero below.

			}
			else
			{
				ColValue = UnicodeChar;
				SubColVal = 0;
				UnicodeChar = 0;
			}
		}

		// Store the values in 2 bytes

		ColStr[ColLen++] = (FLMBYTE) (ColValue >> 8);
		ColStr[ColLen++] = (FLMBYTE) (ColValue & 0xFF);

		if (SubColVal)
		{
			Flags |= HAD_SUB_COLLATION;
			if (SubColVal <= 31) // 5 bit - store bits 10
			{
				SET_BIT( SubColBuf, SubColBitPos);
				SubColBitPos += 1 + 1;	// Stores a zero
				SETnBITS( 5, SubColBuf, SubColBitPos, SubColVal);

				SubColBitPos += 5;
			}
			else	// 2 bytes - store bits 110 or 11110
			{
				FLMUINT	Temp;

				SET_BIT( SubColBuf, SubColBitPos);
				SubColBitPos++;
				SET_BIT( SubColBuf, SubColBitPos);
				SubColBitPos++;

				if (!WpChar && UnicodeChar)	// Store as "11110"
				{
					SubColVal = UnicodeChar;
					UnicodeChar = 0;
					SET_BIT( SubColBuf, SubColBitPos);
					SubColBitPos++;
					SET_BIT( SubColBuf, SubColBitPos);
					SubColBitPos++;
				}

				SubColBitPos++;					// Skip past the zero

				// Go to the next byte boundary to write the WP char

				SubColBitPos = (SubColBitPos + 7) & (~7);
				Temp = BYTES_IN_BITS( SubColBitPos);

				// Need to store HIGH-Low - PC format is Low-high!

				SubColBuf[Temp] = (FLMBYTE) (SubColVal >> 8);
				SubColBuf[Temp + 1] = (FLMBYTE) (SubColVal);

				SubColBitPos += 16;
			}
		}
		else
		{
			SubColBitPos++;
		}

		// Save case information - always 2 bits worth for asian

		if (CaseFlags & 0x02)
		{
			SET_BIT( LowUpBuf, LowUpBitPos);
		}

		LowUpBitPos++;

		if (CaseFlags & 0x01)
		{
			SET_BIT( LowUpBuf, LowUpBitPos);
		}

		LowUpBitPos++;

		// Check to see if ColLen is within 1 byte of max

		if ((ColLen >= uiCharLimit) || 
			 (ColLen + BYTES_IN_BITS( SubColBitPos) + 
					     BYTES_IN_BITS( LowUpBitPos) >= uiTargetColLen))
		{

			// Still something left?

			if ((Str < pszStrEnd) || NextWpChar || UnicodeChar)
			{
				bDataTruncated = TRUE;
			}

			// Hit the max. number of characters

			break;
		}
	}

	if (puiCollationLen)
	{
		*puiCollationLen = ColLen;
	}

	// Add the first substring marker - also serves as making the string
	// non-null.

	if (bFirstSubstring)
	{
		ColStr[ColLen++] = 0;
		ColStr[ColLen++] = COLL_FIRST_SUBSTRING;
	}

	if (bDataTruncated)
	{
		ColStr[ColLen++] = 0;
		ColStr[ColLen++] = COLL_TRUNCATED;
	}

	if (!ColLen && !SubColBitPos)
	{
		if (puiCaseLen)
		{
			*puiCaseLen = 0;
		}

		goto Exit;
	}

	// Done putting the String into 3 sections - build the COLLATED KEY

	if (Flags & HAD_SUB_COLLATION)
	{
		ColStr[ColLen++] = 0;
		ColStr[ColLen++] = COLL_MARKER | SC_SUB_COL;

		// Move the Sub-collation (diacritics) into the collating String

		Length = (FLMUINT) (BYTES_IN_BITS( SubColBitPos));
		f_memcpy( &ColStr[ColLen], SubColBuf, Length);
		ColLen += Length;
	}

	// Always represent the marker as 2 bytes and case bits in asia

	ColStr[ColLen++] = 0;
	ColStr[ColLen++] = COLL_MARKER | SC_MIXED;
	Length = (FLMUINT) (BYTES_IN_BITS( LowUpBitPos));
	
	f_memcpy( &ColStr[ColLen], LowUpBuf, Length);
	
	if (puiCaseLen)
	{
		*puiCaseLen = (FLMUINT) (Length + 2);
	}

	ColLen += Length;

Exit:

	if (pbDataTruncated)
	{
		*pbDataTruncated = bDataTruncated;
	}

	*ColStrLenRV = (FLMUINT) ColLen;
	return (rc);
}

/****************************************************************************
Desc:  	Convert a text string to a collated string.
			If FERR_CONV_DEST_OVERFLOW is returned the string is truncated as
			best as it can be.  The caller must decide to return the error up
			or deal with the truncation.
Return:	RCODE = SUCCESS or FERR_CONV_DEST_OVERFLOW
VISIT:	If the string is EXACTLY the length of the truncation 
			length then it should, but doesn't, set the truncation flag.  
			The code didn't match the design intent.  Fix next major
			version.
****************************************************************************/
RCODE FTextToColStr(
	const FLMBYTE *	pucStr,					// Points to the internal TEXT string
	FLMUINT 				uiStrLen,				// Length of the internal TEXT string
	FLMBYTE *			pucCollatedStr,		// Returns collated string
	FLMUINT *			puiCollatedStrLen,	// Returns total collated string length
														// Input is maximum bytes in buffer
	FLMUINT  			uiUppercaseFlag,		// Set if to convert to uppercase
	FLMUINT *			puiCollationLen,		// Returns the collation bytes length
	FLMUINT *			puiCaseLen,				// Returns length of case bytes
	FLMUINT				uiLanguage,				// Language
	FLMUINT				uiCharLimit,			// Max number of characters in this key piece
	FLMBOOL				bFirstSubstring,		// TRUE is this is the first substring key
	FLMBOOL *			pbOriginalCharsLost,
	FLMBOOL *			pbDataTruncated)
{
	RCODE					rc = FERR_OK;
	const FLMBYTE *	pucStrEnd;				// Points to the end of the string
	FLMUINT16			ui16Base;				// Value of the base character
	FLMUINT16			ui16SubColVal;			// Sub-collated value (diacritic)
	FLMUINT 				uiObjLength = 0;
	FLMUINT 				uiLength;				// Temporary variable for length
	FLMUINT 				uiTargetColLen = *puiCollatedStrLen - 8;	// 4=ovhd,4=worse char
	FLMUINT				uiObjType;
	FLMBOOL				bDataTruncated = FALSE;

	// Need to increase the buffer sizes to not overflow.
	// Characaters without COLL values will take up 3 bytes in
	// the ucSubColBuf[] and easily overflow the buffer.
	// Hard coded the values so as to minimize changes.

	FLMBYTE		ucSubColBuf[ MAX_SUBCOL_BUF + 301];	// Holds sub-collated values(diac)
	FLMBYTE		ucCaseBits[ MAX_LOWUP_BUF + 81];		// Holds case bits
	FLMUINT16	ui16WpChr;			// Current WP character
	FLMUNICODE	unichr = 0;			// Current unconverted Unicode character
	FLMUINT16	ui16WpChr2;			// 2nd character if any; default 0 for US lang
	FLMUINT		uiColLen;			// Return value of collated length
	FLMUINT		uiSubColBitPos;	// Sub-collation bit position
	FLMUINT	 	uiCaseBitPos;		// Case bit position
	FLMUINT		uiFlags;				// Clear all bit flags
	FLMBOOL		bHebrewArabic = FALSE;	// Set if language is hebrew, arabic, farsi
	FLMBOOL		bTwoIntoOne;

	uiColLen = 0;
	uiSubColBitPos = 0;
	uiCaseBitPos = 0;
	uiFlags = 0;
	ui16WpChr2 = 0;

	// Don't allow any key component to exceed 256 bytes regardless of the
	// user-specified character or byte limit.  The goal is to prevent
	// any single key piece from consuming too much of the key (which is
	// limited to 640 bytes) and thus "starving" other pieces, resulting
	// in a key overflow error.

	if( uiTargetColLen > 256)
	{
		uiTargetColLen = 256;
	}

	// Code below sets ucSubColBuf[] and ucCaseBits[] values to zero.

	if (uiLanguage != FLM_US_LANG)
	{
		if (uiLanguage == FLM_AR_LANG ||		// Arabic
			 uiLanguage == FLM_FA_LANG ||		// Farsi - persian
			 uiLanguage == FLM_HE_LANG ||		// Hebrew
			 uiLanguage == FLM_UR_LANG)			// Urdu
		{
			bHebrewArabic = TRUE;
		}
	}
	pucStrEnd = &pucStr [uiStrLen];

	while (pucStr < pucStrEnd)
	{

		// Set the case bits and sub-collation bits to zero when
		// on the first bit of the byte.

		if (!(uiCaseBitPos & 0x07))
		{
			ucCaseBits [uiCaseBitPos >> 3] = 0;
		}
		if (!(uiSubColBitPos & 0x07))
		{
			ucSubColBuf [uiSubColBitPos >> 3] = 0;
		}

		// Get the next character from the TEXT string.

		for (ui16WpChr = ui16SubColVal = 0;	// Default sub-collation value
			  !ui16WpChr && pucStr < pucStrEnd;
			  pucStr += uiObjLength)
		{
			FLMBYTE	ucChar = *pucStr;
				
			uiObjType = flmTextObjType( ucChar);
			switch (uiObjType)
			{
				case ASCII_CHAR_CODE:  			// 0nnnnnnn
				{
					uiObjLength = 1;

					// Character set zero is assumed.

					ui16WpChr = (FLMUINT16)ucChar;
					continue;
				}
				
				case CHAR_SET_CODE:	  			// 10nnnnnn
				{
					uiObjLength = 2;

					// Character set followed by character

					ui16WpChr = (((FLMUINT16)(ucChar & (~CHAR_SET_MASK)) << 8)
							+ (FLMUINT16)*(pucStr + 1));
					continue;
				}
				
				case WHITE_SPACE_CODE:			// 110nnnnn
				{
					uiObjLength = 1;
					ucChar &= (~WHITE_SPACE_MASK);
					ui16WpChr = (ucChar == HARD_HYPHEN ||
									 ucChar == HARD_HYPHEN_EOL ||
									 ucChar == HARD_HYPHEN_EOP)
									? (FLMUINT16)0x2D	// Minus sign -- character set 0
									: (FLMUINT16)0x20;// Space -- character set zero
					continue;
				}

				case UNK_GT_255_CODE:
				{
					uiObjLength = 3 + FB2UW( pucStr + 1);
					continue;
				}
				
				case UNK_LE_255_CODE:
				{				
					uiObjLength = 2 + (FLMUINT16)*(pucStr + 1);
					continue;
				}
				
				case UNK_EQ_1_CODE:
				{
					uiObjLength = 2;
					continue;
				}
				
				case EXT_CHAR_CODE:
				{
					uiObjLength = 3;

					// Character set followed by character

					ui16WpChr = (((FLMUINT16)*(pucStr + 1) << 8)
							  + (FLMUINT16)*(pucStr + 2));
					continue;
				}
				
				case OEM_CODE:
				{

					// OEM characters are always >= 128
					// Use character set zero to process them.

					uiObjLength = 2;
					ui16WpChr = (FLMUINT16)*(pucStr + 1);
					continue;
				}
				
				case UNICODE_CODE:			// Unconvertable UNICODE code
				{
					uiObjLength = 3;

					// Unicode character followed by unicode character set

					unichr = (FLMUINT16)(((FLMUINT16)*(pucStr + 1) << 8)
								+ (FLMUINT16)*(pucStr + 2));
					ui16WpChr = UNK_UNICODE_CODE;
					continue;
				}
				
				default:
				{

					// Should not happen, but don't return an error

					flmAssert( 0);
					continue;
				}
			}
		}

		// If we didn't get a character, break out of while loop.

		if (!ui16WpChr)
		{
			break;
		}

		// flmCheckDoubleCollation modifies ui16WpChr if a digraph or a double
		// character sequence is found.  If a double character is found, pucStr
		// is incremented and ui16WpChr2 is set to 1.  If a digraph is found,
		// pucStr is not changed, but ui16WpChr contains the first character and
		// ui16WpChr2 contains the second character of the digraph.

		if (uiLanguage != FLM_US_LANG)
		{
			ui16WpChr2 = flmCheckDoubleCollation( &ui16WpChr, &bTwoIntoOne,
											&pucStr, uiLanguage);
		}

		// Save the case bit

		if (!uiUppercaseFlag)
		{

			// charIsUpper returns TRUE if upper case, 0 if lower case.

			if (!charIsUpper( ui16WpChr))
			{
				uiFlags |= HAD_LOWER_CASE;
			}
			else
			{

				// Set if upper case.

				SET_BIT( ucCaseBits, uiCaseBitPos);
			}
			
			uiCaseBitPos++;
		}

		// Handle OEM characters, non-collating characters,
		// characters with subcollating values, double collating
		// values.

		// Get the collated value from the WP character-if not collating value

		if ((pucCollatedStr[ uiColLen++] =
				(FLMBYTE)(flmGetCollation( ui16WpChr, uiLanguage))) >= COLS11)
		{
			FLMUINT	uiTemp;

			// Save OEM characters just like non-collating characters

			// If lower case, convert to upper case.

			if (!charIsUpper( ui16WpChr))
			{
				ui16WpChr &= ~1;
			}

			// No collating value given for this WP char.
			// Save original WP char (2 bytes) in subcollating
			// buffer.

			// 1110 is a new code that will store an insert over
			// the character OR a non-convertable unicode character.
			// Store with the same alignment as "store_extended_char"
			// below.

			// 11110 is code for unmappable UNICODE value.
			// A value 0xFE will be the collation value.  The sub-collation
			// value will be 0xFFFF followed by the UNICODE value.
			// Be sure to eat an extra case bit.

			// See specific Hebrew and Arabic comments in the
			//	switch statement below.

			// Set the next byte that follows in the sub collation buffer.

			ucSubColBuf [(uiSubColBitPos + 8) >> 3] = 0;

			if (bHebrewArabic && (pucCollatedStr [uiColLen-1] == COLS0_ARABIC))
			{

				// Store first bit of 1110, fall through & store remaining 3 bits

				SET_BIT( ucSubColBuf, uiSubColBitPos);
				uiSubColBitPos++;

				// Don't store collation value

				uiColLen--;
			}
			else if (unichr)
			{
				ui16WpChr = unichr;
				unichr = 0;

				// Store 11 out of 11110

				SET_BIT( ucSubColBuf, uiSubColBitPos);
				uiSubColBitPos++;
				
				SET_BIT( ucSubColBuf, uiSubColBitPos);
				uiSubColBitPos++;
				
				if (!uiUppercaseFlag)
				{
					ucCaseBits [(uiCaseBitPos + 7) >> 3] = 0;

					// Set upper case bit.

					SET_BIT( ucCaseBits, uiCaseBitPos);
					uiCaseBitPos++;
				}
			}	
			
store_extended_char:

			// Set the next byte that follows in the sub collation buffer.

			ucSubColBuf [(uiSubColBitPos + 8) >> 3] = 0;
			ucSubColBuf [(uiSubColBitPos + 16) >> 3] = 0;
			uiFlags |= HAD_SUB_COLLATION;

			// Set 110 bits in sub-collation - continued from above.
			// No need to explicitly set the zero, but must increment
			// for it.

			SET_BIT( ucSubColBuf, uiSubColBitPos);
			uiSubColBitPos++;
			
			SET_BIT( ucSubColBuf, uiSubColBitPos);
			uiSubColBitPos += 2;

			// store_aligned_word: This label is not referenced.
			// Go to the next byte boundary to write the character.

			uiSubColBitPos = (uiSubColBitPos + 7) & (~7);
			uiTemp = BYTES_IN_BITS( uiSubColBitPos);

			// Need to big-endian - so it will sort correctly.

			ucSubColBuf [uiTemp] = (FLMBYTE)(ui16WpChr >> 8);
			ucSubColBuf [uiTemp + 1] = (FLMBYTE)(ui16WpChr);
			uiSubColBitPos += 16;
			ucSubColBuf [uiSubColBitPos >> 3] = 0;
		}
		else
		{
			// Had a collation value
			// Add the lower/uppercase bit if a mixed case output.
			// If not lower ASCII set - check diacritic value for sub-collation

			if (!(ui16WpChr & 0xFF00))
			{

				// ASCII character set - set a single 0 bit - just need to
				// increment to do this.

				uiSubColBitPos++;
			}
			else
			{
				FLMBYTE	ucTmpChar = (FLMBYTE)ui16WpChr;
				FLMBYTE	ucCharSet = (FLMBYTE)(ui16WpChr >> 8);

				// Convert char to uppercase because case information
				// is stored above.  This will help
				// ensure that the "ETA" doesn't sort before "eta"

				if (!charIsUpper(ui16WpChr))
				{
					ui16WpChr &= ~1;
				}

				switch (ucCharSet)
				{
					case CHSMUL1:	// Multinational 1
					{

						// If we cannot break down a char into base and
						// diacritic we cannot combine the charaacter
						// later when converting back the key.  In that case,
						// write the entire WP char in the sub-collation area.

						if (flmCh6Brkcar( ui16WpChr, &ui16Base, &ui16SubColVal))
						{
							goto store_extended_char;
						}

						// Write the FLAIM diacritic sub-collation value.
						// Prefix is 2 bits "10".  Remember to leave
						// "111" alone for the future.
						// NOTE: The "unlaut" character must sort after the "ring"
						// character.

						ui16SubColVal = ((ui16SubColVal & 0xFF) == umlaut	&&
											  (uiLanguage == FLM_SU_LANG || 
												uiLanguage == FLM_SV_LANG || 
												uiLanguage == FLM_CZ_LANG || 
												uiLanguage == FLM_SL_LANG))
							?	(FLMUINT16)(flmDia60Tbl[ ring] + 1)
							:	(FLMUINT16)(flmDia60Tbl[ ui16SubColVal & 0xFF]);
				
store_sub_col:
						// Set the next byte that follows in the sub collation buffer.

						ucSubColBuf[ (uiSubColBitPos + 8) >> 3] = 0;
						uiFlags |= HAD_SUB_COLLATION;

						// Set the 10 bits - no need to explicitly set the zero, but
						// must increment for it.

						SET_BIT( ucSubColBuf, uiSubColBitPos);
						uiSubColBitPos += 2;

						// Set sub-collation bits.

						SETnBITS( 5, ucSubColBuf, uiSubColBitPos, ui16SubColVal);
						uiSubColBitPos += 5;
						break;
					}
						
					case CHSGREK:		// Greek
					{
						if (ucTmpChar >= 52  ||		// Keep case bit for 52-69 else ignore
          				 ui16WpChr == 0x804 ||	// [ 8,4] BETA Medial | Terminal
							 ui16WpChr == 0x826) 	// [ 8,38] SIGMA terminal
						{
							goto store_extended_char;
						}
							
						// No subcollation to worry about - set a zero bit by
						// incrementing the bit position.

						uiSubColBitPos++;
						break;
					}
					
					case CHSCYR:
					{
						if (ucTmpChar >= 144)
						{
							goto store_extended_char;
						}

						// No subcollation to worry about - set a zero bit by
						// incrementing the bit position.

						uiSubColBitPos++;

						// VISIT: Georgian covers 208-249 - no collation defined yet

						break;
					}
					
					case CHSHEB:		// Hebrew
					{
						// Three sections in Hebrew:
						//		0..26 - main characters
						//		27..83 - accents that apear over previous character
						//		84..118- dagesh (ancient) hebrew with accents

						// Because the ancient is only used for sayings & scriptures
						// we will support a collation value and in the sub-collation
						// store the actual character because sub-collation is in 
						// character order.

            		if (ucTmpChar >= 84)		// Save ancient - value 84 and above
						{
							goto store_extended_char;
						}

						// No subcollation to worry about - set a zero bit by
						// incrementing the bit position.

						uiSubColBitPos++;
						break;
					}
						
					case CHSARB1:		// Arabic 1
					{
						// Three sections in Arabic:						
						//		00..37  - accents that display OVER a previous character
						//		38..46  - symbols 
						//		47..57  - numbers
						//		58..163 - characters
						//		164     - hamzah accent
						//		165..180- common characters with accents
						//		181..193- ligatures - common character combinations
						//		194..195- extensions - throw away when sorting
					
						if (ucTmpChar <= 46)
						{
							goto store_extended_char;	// save original character
						}

						if (pucCollatedStr[ uiColLen-1] == COLS10a+1)	// Alef?
						{	
							ui16SubColVal = (ucTmpChar >= 165)
								? (FLMUINT16)(flmAlefSubColTbl[ ucTmpChar - 165 ])
								: (FLMUINT16)7;			// Alef subcol value
							goto store_sub_col;
						}
						
						if (ucTmpChar >= 181)			// Ligatures - char combination
						{
							goto store_extended_char;	// save original character
						}

						if (ucTmpChar == 64)				// taa exception
						{
							ui16SubColVal = 8;
							goto store_sub_col;
						}

						// No subcollation to worry about - set a zero bit by
						// incrementing the bit position.

						uiSubColBitPos++;
						break;
					}
					
					case CHSARB2:			// Arabic 2
					{

						// There are some characters that share the same slot
						// Check the bit table if above character 64
						
						if (ucTmpChar >= 64 &&
							 flmAr2BitTbl[(ucTmpChar - 64) >> 3] & 
								(0x80 >> (ucTmpChar & 0x07)))
						{
							goto store_extended_char;	// Will save original
						}

						// No subcollation to worry about - set a zero bit by
						// incrementing the bit position.

						uiSubColBitPos++;
						break;
					}

					default:
					{
						// Increment bit position to set a zero bit.

						uiSubColBitPos++;
						break;
					}
				}
			}

			// Now let's worry about double character sorting

			if (ui16WpChr2)
			{
				if (pbOriginalCharsLost)
				{
					*pbOriginalCharsLost = TRUE;
				}

				// Set the next byte that follows in the sub collation buffer.

				ucSubColBuf [(uiSubColBitPos + 7) >> 3] = 0;

				if (bTwoIntoOne)
				{
					// Sorts after character in ui16WpChr after call to
					// flmCheckDoubleCollation
					// Write the char 2 times so lower/upper bits are correct.
					// Could write infinite times because of collation rules.

					pucCollatedStr[ uiColLen] = ++pucCollatedStr[ uiColLen-1];
					uiColLen++;

					// If original was upper case, set one more upper case bit

					if (!uiUppercaseFlag)
					{
						ucCaseBits[ (uiCaseBitPos + 7) >> 3] = 0;
						if (!charIsUpper( (FLMUINT16) *(pucStr - 1)))
						{
							uiFlags |= HAD_LOWER_CASE;
						}
						else
						{
							SET_BIT( ucCaseBits, uiCaseBitPos);
						}
						
						uiCaseBitPos++;
					}

					// Take into account the diacritical space

					uiSubColBitPos++;
				}
				else
				{
					// We have a digraph, get second collation value

					pucCollatedStr[ uiColLen++] =
						(FLMBYTE)(flmGetCollation( ui16WpChr2, uiLanguage));

					// Normal case, assume no diacritics set

					uiSubColBitPos++;

					// If first was upper, set one more upper bit.

					if (!uiUppercaseFlag)
					{
						ucCaseBits [(uiCaseBitPos + 7) >> 3] = 0;
						if (charIsUpper( ui16WpChr))
						{
							SET_BIT( ucCaseBits, uiCaseBitPos);
						}
						uiCaseBitPos++;

						// no need to reset the uiFlags
					}
				}
			}
		}

		// Check to see if uiColLen is at some overflow limit.

		if (uiColLen >= uiCharLimit ||
			 uiColLen + BYTES_IN_BITS( uiSubColBitPos) + 
						  BYTES_IN_BITS( uiCaseBitPos) >= uiTargetColLen)
		{

			// We hit the maximum number of characters.

			if (pucStr < pucStrEnd)
			{
				bDataTruncated = TRUE;
			}
			
			break;
		}
	}

	if (puiCollationLen)
	{
		*puiCollationLen = uiColLen;
	}

	// Add the first substring marker - also serves as making the string non-null.

	if (bFirstSubstring)
	{
		pucCollatedStr [uiColLen++] = COLL_FIRST_SUBSTRING;
	}

	if (bDataTruncated)
	{
		pucCollatedStr[ uiColLen++ ] = COLL_TRUNCATED;
	}

	if (!uiColLen && !uiSubColBitPos)
	{
		if (puiCaseLen)
		{
			*puiCaseLen = 0;
		}
		goto Exit;
	}

	// Store extra zero bit in the sub-collation area for Hebrew/Arabic

	if (bHebrewArabic)
	{
		uiSubColBitPos++;
	}

	// Done putting the string into 4 sections - build the COLLATED KEY
	// Don't set uiUppercaseFlag earlier than here because SC_LOWER may be zero

	uiUppercaseFlag = (uiLanguage == FLM_GR_LANG) ? SC_LOWER : SC_UPPER;

	// The default terminating characters is (COLL_MARKER|SC_UPPER)
	// Did we write anything to the subcollation area?

	if (uiFlags & HAD_SUB_COLLATION)
	{
		// Writes out a 0x7

		pucCollatedStr [uiColLen++] = COLL_MARKER | SC_SUB_COL;

		// Move the sub-collation into the collating string

		uiLength = BYTES_IN_BITS( uiSubColBitPos);
		f_memcpy( &pucCollatedStr[uiColLen], ucSubColBuf, uiLength);
		uiColLen += uiLength;
	}

	// Move the upper/lower case stuff - force bits for Greek ONLY
	// This is such a small size that a memcpy is not worth it

	if (uiFlags & HAD_LOWER_CASE)
	{
		FLMUINT		uiNumBytes = BYTES_IN_BITS( uiCaseBitPos);
		FLMBYTE *	pucCasePtr = ucCaseBits;

		// Output the 0x5
	
		pucCollatedStr [uiColLen++] = (FLMBYTE)(COLL_MARKER | SC_MIXED);
		if (puiCaseLen)
		{
			*puiCaseLen = uiNumBytes + 1;
		}

		if (uiUppercaseFlag == SC_LOWER)
		{

			// Negate case bits for languages (like GREEK) that sort
			// upper case before lower case.

			while (uiNumBytes--)
			{
				pucCollatedStr [uiColLen++] = ~(*pucCasePtr++);
			}
		}
		else
		{
			while (uiNumBytes--)
			{
				pucCollatedStr [uiColLen++] = *pucCasePtr++;
			}
		}
	}
	else
	{

		// All characters are either upper or lower case, as determined
		// by uiUppercaseFlag.

		pucCollatedStr [uiColLen++] = (FLMBYTE)(COLL_MARKER | uiUppercaseFlag);
		if( puiCaseLen)
		{
			*puiCaseLen = 1;
		}
	}
	
Exit:

	if( pbDataTruncated)
	{
		*pbDataTruncated = bDataTruncated;
	}

	*puiCollatedStrLen = uiColLen;
	return( rc);
}

/****************************************************************************
Desc:  	Return the sub-collation value of a WPText character.
			Unconvered Unicode values always have a sub-collation
			value of 11110 + Unicode Value.
****************************************************************************/
FLMUINT16 flmTextGetSubCol(
	FLMUINT16		ui16WPValue,		// WP Character value.
	FLMUINT16		ui16ColValue,		// Collation Value (for arabic)
	FLMUINT			uiLangId)			// WP Language ID.
{
	FLMUINT16		ui16SubColVal;
	FLMBYTE			byCharVal;
	FLMBYTE			byCharSet;
	FLMUINT16		ui16Base;

	// Easy case first.

	ui16SubColVal = 0;
	if( (ui16WPValue & 0xFF00 ) == 0)
	{
		goto Exit;
	}

	// From here down default ui16SubColVal is WP value.

	ui16SubColVal = ui16WPValue;
	
	byCharVal = (FLMBYTE) ui16WPValue;
	byCharSet = (FLMBYTE) (ui16WPValue >> 8);
	
	// Convert char to uppercase because case information
	// is stored above.  This will help
	// insure that the "ETA" doesn't sort before "eta"
	// could use is lower code here for added performance.
	
	// This just happens to work with all WP character values

	if (!flmIsUpper( ui16WPValue))
	{
		ui16WPValue &= ~1;
	}

	switch( byCharSet)
	{
		case CHSMUL1:
		{
			// If you cannot break down a char into base and
			// diacritic then you cannot combine the charaacter
			// later when converting back the key.  So, write
			// the entire WP char in the sub-collation area.
			// We can ONLY SUPPORT MULTINATIONAL 1 for brkcar()

			if( flmCh6Brkcar( ui16WPValue, &ui16Base, &ui16SubColVal))
			{

				// WordPerfect character cannot be broken down.
				// If we had a collation value other than 0xFF (COLS0), don't
				// return a sub-collation value.  This will allow things like
				// upper and lower AE digraphs to compare properly.

				if (ui16ColValue != COLS0)
				{
					ui16SubColVal = 0;
				}
				goto Exit;
			}
			
			// Write the FLAIM diacritic sub-collation value.
			// Prefix is 2 bits "10".  Remember to leave
			// "111" alone for the future.
			
			ui16SubColVal = ((ui16SubColVal & 0xFF) == umlaut && 
									((uiLangId == FLM_SU_LANG) || 
									 (uiLangId == FLM_SV_LANG) || 
									 (uiLangId == FLM_CZ_LANG) || 
									 (uiLangId == FLM_SL_LANG)))
				?	(FLMUINT16)(flmDia60Tbl[ ring] + 1)
				:	(FLMUINT16)(flmDia60Tbl[ ui16SubColVal & 0xFF]);
	
			break;
		}

		case CHSGREK:
		{
			if( (byCharVal >= 52)  || (ui16WPValue == 0x804) || 
				 (ui16WPValue == 0x826))
			{
				ui16SubColVal = ui16WPValue;
			}
			break;
		}
			
		case CHSCYR:
		{
			if( byCharVal >= 144)
			{
				ui16SubColVal = ui16WPValue;
			}
			break;
		}
			
		case CHSHEB:
		{
		
			// Three sections in Hebrew:
			// 	0..26 - main characters
			//		27..83 - accents that apear over previous character
			//		84..118- dagesh (ancient) hebrew with accents
			//
			// Because the ancient is only used for sayings & scriptures
			// we will support a collation value and in the sub-collation
			// store the actual character because sub-collation is in 
			// character order.

         if( byCharVal >= 84)
			{
				ui16SubColVal = ui16WPValue;
			}
			
			break;
		}
			
		case CHSARB1:
		{
		
			// Three sections in Arabic:						
			//		00..37  - accents that display OVER a previous character
			//		38..46  - symbols 
			//		47..57  - numbers
			//		58..163 - characters
			//		164     - hamzah accent
			//		165..180- common characters with accents
			//		181..193- ligatures - common character combinations
			//		194..195- extensions - throw away when sorting
		
			if( byCharVal <= 46 )
			{
				ui16SubColVal = ui16WPValue;
			}
			else
			{
				if( ui16ColValue == COLS10a + 1)
				{	
					ui16SubColVal = (byCharVal >= 165)
						? (FLMUINT16)(flmAlefSubColTbl[ byCharVal - 165 ])
						: (FLMUINT16)7;								// Alef subcol value
				}
				else
				{
					if( byCharVal >= 181)							// Ligatures - char combination
					{
						ui16SubColVal = ui16WPValue;
					}
					else if( byCharVal == 64)						// taa exception
					{
						ui16SubColVal = 8;
					}
				}
			}
			break;
		}
			
		case	CHSARB2:
		{
			// There are some characters that share the same slot
			// Check the bit table if above character 64
			
			if ((byCharVal >= 64) &&
				 (flmAr2BitTbl[(byCharVal-64)>> 3] & (0x80 >> (byCharVal&0x07))))
			{
				ui16SubColVal = ui16WPValue;
			}
			
			break;
		}
	}

Exit:

	return( ui16SubColVal);
}

/****************************************************************************
Desc: Get the original string from an asian collation string Ret: Length
		of the word string in bytes
****************************************************************************/
FLMUINT AsiaConvertColStr(
	FLMBYTE*		CollatedStr,		// Points to the Flaim collated string
	FLMUINT*		CollatedStrLenRV, // Length of the Flaim collated string
	FLMBYTE*		WordStr,				// Output string to build - WP word string
	FLMBOOL*		pbDataTruncated,	// Set to TRUE if data was truncated
	FLMBOOL*		pbFirstSubstring) // Set to TRUE if marker exists
{
	FLMBYTE*		pWordStr = WordStr;
	FLMUINT		Length = *CollatedStrLenRV;	// May optimize as a register
	FLMUINT		CollStrPos = 0;					// Position in CollatedStr[]
	FLMBOOL		bHadExtended = FALSE;
	FLMUINT		WordStrLen;
	FLMUINT16	ColChar;								// 2 byte value for asian

	while (Length)
	{
		FLMBYTE	CharVal;
		FLMBYTE	CharSet;
		
		CharSet = CollatedStr[CollStrPos];
		CharVal = CollatedStr[CollStrPos + 1];
		ColChar = (FLMUINT16) ((CharSet << 8) + CharVal);

		if (ColChar <= MAX_COL_OPCODE)
		{
			break;
		}

		CollStrPos += 2;
		Length -= 2;
		
		if (CharSet == 0)			// Normal Latin/Greek/Cyrillic value
		{
			ColChar = colToWPChr[CharVal - COLLS];
		}
		else if (CharSet == 1)	// katakana or hiragana character
		{
			if (CharVal > sizeof(ColToKanaTbl)) // Special cases below
			{
				if (CharVal == COLS_ASIAN_MARK_VAL)
				{							// dakuten
					ColChar = 0x240a;
				}
				else if (CharVal == COLS_ASIAN_MARK_VAL + 1)
				{							// handakuten
					ColChar = 0x240b;
				}
				else if (CharVal == COLS_ASIAN_MARK_VAL + 2)
				{							// chuuten
					ColChar = 0x2405;
				}
				else
				{
					ColChar = 0xFFFF; // error
				}
			}
			else
			{
				ColChar = (FLMUINT16) (0x2600 + ColToKanaTbl[CharVal]);
			}
		}
		else if (CharSet != 0xFF || CharVal != 0xFF) // Asian characters
		{

			// Insert zeroes that will be treated as a signal for
			// uncoverted unicode characters later on. NOTE: Cannot use
			// 0xFFFF, because we need to be able to detect this case in
			// the sub-collation stuff, and we don't want to confuse it
			// with the 0xFFFF that may have been inserted in another
			// case. THIS IS A REALLY BAD HACK, BUT IT IS THE BEST WE CAN
			// DO FOR NOW!

			*pWordStr++ = 0;
			*pWordStr++ = 0;
			bHadExtended = TRUE;
		}

		// else does not have a collation value - found in sub-collation
		// part

		UW2FBA( ColChar, pWordStr);		// Put the uncollation value back
		pWordStr += 2;
	}

	UW2FBA( 0, pWordStr);					// NULL Terminate the string
	WordStrLen = (FLMUINT) (pWordStr - WordStr);

	// Parse through the sub-collation and case information.
	// Watch out for COMP CollStrPosT indexes-doesn't have case info after
	// Here are values for some of the codes:
	//		[ 0x01] - end for fields case info follows - for COMP POST indexes
	//		[ 0x02] - compound marker
	//		[ 0x05] - case bits follow
	//		[ 0x06] - case information is all uppercase
	//		[ 0x07] - beginning of sub-collation information
	//		[ 0x08] - first substring field that is made
	//		[ 0x09] - truncation marker for text and binary
	// 
	// Asian chars the case information should always be there and not
	// compressed out.  This is because the case information could change
	// the actual width of the character from 0x26xx to charset 11.

	if (Length)
	{
		ColChar = (FLMUINT16) ((CollatedStr[CollStrPos] << 8) + 
										CollatedStr[CollStrPos + 1]);

		// First substring is before truncated.

		if (ColChar == COLL_FIRST_SUBSTRING)
		{
			if (pbFirstSubstring)
			{
				*pbFirstSubstring = TRUE;	// Don't need to initialize to FALSE.
			}

			Length -= 2;
			CollStrPos += 2;
			ColChar = (FLMUINT16) ((CollatedStr[CollStrPos] << 8) + 
											CollatedStr[CollStrPos + 1]);
		}

		if (ColChar == COLL_TRUNCATED)
		{
			if (pbDataTruncated)
			{
				*pbDataTruncated = TRUE;	// Don't need to initialize to FALSE.
			}

			Length -= 2;
			CollStrPos += 2;
			ColChar = (FLMUINT16) ((CollatedStr[CollStrPos] << 8) + 
											CollatedStr[CollStrPos + 1]);
		}

		if (ColChar == (COLL_MARKER | SC_SUB_COL))
		{
			FLMUINT	TempLen;

			// Do another pass on the word string adding diacritics/voicings

			CollStrPos += 2;
			Length -= 2;
			TempLen = AsiaParseSubCol( WordStr, &WordStrLen,
											  &CollatedStr[CollStrPos]);
			CollStrPos += TempLen;
			Length -= TempLen;
		}
		else
		{
			goto check_case;
		}
	}

	// Does the case info follow? - It may not because of post indexes

	if (Length)
	{
		ColChar = (FLMUINT16) ((CollatedStr[CollStrPos] << 8) + 
										CollatedStr[CollStrPos + 1]);
check_case:

		if (ColChar == (COLL_MARKER | SC_MIXED))
		{
			CollStrPos += 2;
			CollStrPos += AsiaParseCase( WordStr, &WordStrLen,
												 &CollatedStr[CollStrPos]);

			// Set bHadExtended to FALSE, because they will have been taken
			// care of in this pass.

			bHadExtended = FALSE;
		}
	}

	// Change embedded zeroes to 0xFFFFs

	if (bHadExtended)
	{
		FLMUINT		uiCnt;
		FLMBYTE*		pucTmp;

		for (uiCnt = 0, pucTmp = WordStr;
			  uiCnt < WordStrLen;
			  uiCnt += 2, pucTmp += 2)
		{
			if (FB2UW( pucTmp) == 0)
			{
				UW2FBA( 0xFFFF, pucTmp);
			}
		}
	}

	// Following marker is 2 bytes if post otherwise will be 1 byte ;
	// Should make a pass and count the extended characters
	
	*CollatedStrLenRV = CollStrPos;		// value should be on 0x01 or 0x02 flag
	return (WordStrLen);						// Return the length of the word string
}

/****************************************************************************
Desc: 	Combine the diacritic 5 and 16 bit values to an existing word
			string. 
		
Ret:		Number of bytes parsed 

Notes:	For each bit in the sub-collation section: 
				0   - no subcollation information 
				10  - take next 5 bits - will tell about diacritics 
						or japanese vowel
				110 -	align to next byte and take word value as extended
						character
****************************************************************************/
FLMUINT AsiaParseSubCol(
	FLMBYTE *	WordStr,			// Existing word string to modify
	FLMUINT *	puiWordStrLen, // Wordstring length in bytes
	FLMBYTE *	SubColBuf)		// Diacritic values in 5 bit sets
{
	FLMUINT		SubColBitPos = 0;
	FLMUINT		NumWords = *puiWordStrLen >> 1;
	FLMUINT16	Diac;
	FLMUINT16	WpChar;

	// For each word in the word string ...

	while (NumWords--)
	{

		// Have to skip 0, because it is not accounted for in the
		// sub-collation bits. It was inserted when we encountered
		// unconverted unicode characters (Asian). Will be converted to
		// something else later on. SEE NOTE ABOVE.

		if (FB2UW( WordStr) == 0)
		{
			WordStr += 2;
			continue;
		}

		// This macro DOESN'T increment bitPos

		if (TEST1BIT( SubColBuf, SubColBitPos))
		{

			// Bits 10 - take next 5 bits Bits 110 align and take next word
			// Bits 11110 align and take unicode value
			//

			SubColBitPos++;
			
			if (!TEST1BIT( SubColBuf, SubColBitPos))
			{
				SubColBitPos++;
				Diac = (FLMUINT16) (GETnBITS( 5, SubColBuf, SubColBitPos));
				SubColBitPos += 5;

				if ((WpChar = FB2UW( WordStr)) < 0x100)
				{
					if ((WpChar >= 'A') && (WpChar <= 'Z'))
					{

						// Convert to WP diacritic and combine characters

						flmCh6Cmbcar( &WpChar, WpChar, (FLMUINT16) ml1_COLtoD[Diac]);

						// Even if cmbcar fails, WpChar is still set to a valid
						// value
					}
					else							// Symbols from charset 0x24
					{
						WpChar = (FLMUINT16) (0x2400 + 
													 flmCh24ColTbl[Diac - 1].ByteValue);
					}
				}
				else if (WpChar >= 0x2600) // Katakana
				{

					// Voicings - will allow to select original char
					//		000 - some 001 are changed to 000 to save space
					//		001 - set if large char (uppercase)
					//		010 - set if voiced
					//		100 - set if half voiced
					//
					// Should NOT match voicing or wouldn't be here!

					FLMBYTE	CharVal = (FLMBYTE) (WpChar & 0xFF);

					// Try exceptions first so don't access out of bounds

					if (CharVal == 84)
					{
						WpChar = (FLMUINT16) (0x2600 + ((Diac == 1) 
										? (FLMUINT16) 10 
										: (FLMUINT16) 11));
					}
					else if (CharVal == 85)
					{
						WpChar = (FLMUINT16) (0x2600 + ((Diac == 1) 
										? (FLMUINT16) 16 
										: (FLMUINT16) 17));
					}

					// Try the next 2 slots, if not then value is 83,84 or 85

					else if (flmKanaSubColTbl[CharVal + 1] == Diac)
					{
						WpChar++;
					}
					else if ((flmKanaSubColTbl[CharVal + 2] == Diac))
					{
						WpChar += 2;
					}

					// last exception below

					else if (CharVal == 4)
					{
						WpChar = 0x2600 + 83;
					}

					// else leave alone! - invalid storage

				}

				UW2FBA( WpChar, WordStr);	// Set if changed or not
			}
			else						// "110"
			{
				FLMUINT	Temp;

				SubColBitPos++;	// Skip second '1'

				if (TEST1BIT( SubColBuf, SubColBitPos))
				{

					// Unconvertable UNICODE character ;
					// The format will be 4 bytes, 0xFF, 0xFF, 2 byte Unicode
					
					shiftN( WordStr, (FLMUINT16) (NumWords + NumWords + 4), 2);
					
					WordStr += 2;							// Skip the 0xFFFF for now
					SubColBitPos += 2;					// Skip next "11"
					(*puiWordStrLen) += 2;
				}

				SubColBitPos++;							// Skip the zero

				// Round up to next byte

				SubColBitPos = (SubColBitPos + 7) & (~7);
				Temp = BYTES_IN_BITS( SubColBitPos);
				WordStr[1] = SubColBuf[Temp];			// Character set
				WordStr[0] = SubColBuf[Temp + 1];	// Character
				SubColBitPos += 16;
			}
		}
		else
		{
			SubColBitPos++;	// Be sure to increment this!
		}

		WordStr += 2;			// Next WP character
	}

	return (BYTES_IN_BITS( SubColBitPos));
}

/****************************************************************************
Desc:		The case bits for asia are:
				Latin/Greek/Cyrillic
					01 - case bit set if character is uppercase
					10 - double wide character in CS 0x25xx, 0x26xx and 0x27xx
				Japanese
					00 - double wide hiragana 0x255e..25b0
					01 - double wide katakana 0x2600..2655
					10 - single wide symbols from charset 11 that map to CS24??
					11 - single wide katakana from charset 11
****************************************************************************/
FLMUINT AsiaParseCase(
	FLMBYTE *		WordStr,			// Existing word string to modify
	FLMUINT *		WordStrLenRV,	// Length of the WordString in bytes
	FLMBYTE *		pCaseBits)		// Lower/upper case bit string
{
	FLMUINT		WordStrLen = *WordStrLenRV;
	FLMUINT		uiWordCnt;
	FLMUINT		uiExtraBytes = 0;
	FLMUINT16	WpChar;
	FLMBYTE		TempByte = 0;
	FLMBYTE		MaskByte;

	// For each character in the word string ...

	for (uiWordCnt = WordStrLen >> 1, 
		  MaskByte = 0;
		  uiWordCnt--;)
	{
		FLMBYTE	CharSet;
		FLMBYTE	CharVal;

		WpChar = FB2UW( WordStr);		// Get the next character

		// Must skip any 0xFFFFs or zeroes that were inserted.

		if (WpChar == 0xFFFF || WpChar == 0)
		{

			// Put back 0xFFFF in case it was a zero.

			UW2FBA( 0xFFFF, WordStr);
			WordStr += 2;
			uiExtraBytes += 2;
			continue;
		}

		if (MaskByte == 0)				// Time to get another byte
		{
			TempByte = *pCaseBits++;
			MaskByte = 0x80;
		}

		CharSet = (FLMBYTE) (WpChar >> 8);
		CharVal = (FLMBYTE) (WpChar & 0xFF);

		if (WpChar < 0x2400)				// SINGLE WIDE - NORMAL CHARACTERS
		{
			if (TempByte & MaskByte)	// convert to double wide?
			{

				// Latin/greek/cyrillic Convert to uppercase double wide char

				if (CharSet == 0)			// Latin - uppercase
				{

					// May convert to 0x250F (Latin) or CS24

					if (WpChar >= 'A' && WpChar <= 'Z')
					{
						// Convert to double wide
						
						WpChar = (FLMUINT16) (WpChar - 0x30 + 0x250F);
					}
					else
					{
						HanToZenkaku( WpChar, 0, &WpChar);
					}
				}
				else if (CharSet == 8)					// Greek
				{
					if (CharVal > 38)
					{											// Adjust for spaces in greek
						CharVal -= 2;
					}

					if (CharVal > 4)
					{
						CharVal -= 2;
					}

					WpChar = (FLMUINT16) ((CharVal >> 1) + 0x265E);
				}
				else if (CharSet == 10)					// Cyrillic
				{
					WpChar = (FLMUINT16) ((CharVal >> 1) + 0x2700);
				}
				else
				{
					HanToZenkaku( WpChar, 0, &WpChar);
				}

				CharSet = (FLMBYTE) (WpChar >> 8);	// Less code this way
				CharVal = (FLMBYTE) (WpChar & 0xFF);
			}

			MaskByte >>= 1;						// Next bit

			if ((TempByte & MaskByte) == 0)	// Change to lower case?
			{
				switch (CharSet)					// Convert WpChar to lower case
				{
					case 0:
					{
						WpChar |= 0x20;			// Bit zero only if lower case
						break;
					}
					
					case 1:
					{
						if (CharVal >= 26)
						{
							WpChar++;
						}
						
						break;
					}
					
					case 8:
					{
						if (CharVal <= 69)
						{						// All lowercase after 69
							WpChar++;
						}
						
						break;
					}
					
					case 10:
					{
						if (CharVal <= 199)
						{						// No cases after 199
							WpChar++;
						}
						
						break;
					}
					
					case 0x25:
					case 0x26:
					{
						// should be double wide latin or greek

						WpChar += 0x20;	// Add offset to convert to lowercase
						break;
					}
					
					case 0x27:				// double wide cyrillic only
					{
						WpChar += 0x30;	// Add offset to convert to lowercase
						break;
					}
				}
			}
		}
		else	// JAPANESE CHARACTERS
		{
			if (TempByte & MaskByte)							// Original chars from
																		// CharSet 11
			{
				if (CharSet == 0x26)								// Convert to ZenToHankaku
				{
					FLMUINT16	NextChar = 0;

					WpChar = ZenToHankaku( WpChar, &NextChar);

					if (NextChar)									// Move everyone down
					{
						uiWordCnt++;
						shiftN( WordStr, uiWordCnt + uiWordCnt + 2, 2);
						UW2FBA( WpChar, WordStr);
						WordStr += 2;
						WpChar = NextChar;						// Store this below

						*WordStrLenRV = *WordStrLenRV + 2;	// Adjust length
						
						// Don't change WordStrLen - returns number of bits used
					}
				}
				else if (CharSet == 0x24)
				{
					WpChar = ZenToHankaku( WpChar, (FLMUINT16*) 0);
				}

				MaskByte >>= 1;						// Eat next bit
			}
			else
			{
				MaskByte >>= 1;						// Next bit
				if ((TempByte & MaskByte) == 0)	// Convert to hiragana?
				{
					// kanji will also fall through here

					if (CharSet == 0x26)
					{
						WpChar = (FLMUINT16) (0x255E + CharVal);	// Convert to
																				// hiragana
					}
				}
			}
		}

		UW2FBA( WpChar, WordStr);
		WordStr += 2;
		MaskByte >>= 1;
	}
	
	// Should be 2 bits for each character

	uiWordCnt = WordStrLen - uiExtraBytes;
	return (BYTES_IN_BITS( uiWordCnt));
}

/****************************************************************************
Desc:		Returns the collation value of the input WP character.
			If in charset 11 will convert the character to Zenkaku (double wide).
In:		ui16WpChar - Char to collate off of - could be in CS0..14 or x24..up
			ui16NextWpChar - next WP char for CS11 voicing marks
			ui16PrevColValue - previous collating value - for repeat/vowel repeat
			pui16ColValue - returns 2 byte collation value
			pui16SubColVal - 0, 6 or 16 bit value for the latin sub collation
									or the kana size & vowel voicing
									001 - set if large (upper) character
									010 - set if voiced
									100 - set if half voiced
								
			pucCaseBits - returns 2 bits 
				Latin/Greek/Cyrillic
					01 - case bit set if character is uppercase
					10 - double wide character in CS 0x25xx, 0x26xx and 0x27xx
				Japanese
					00 - double wide hiragana 0x255e..25b0
					01 - double wide katakana 0x2600..2655
					10 - double wide symbols that map to charset 11
					11 - single wide katakana from charset 11
Ret:		0 - no valid collation value 
					high values set for pui16ColValue
					Sub-collation gets original WP character value
			1 - valid collation value
			2 - valid collation value and used the ui16NextWpChar
			
Terms:	HANKAKU - single wide characters in charsets 0..14
			ZENKAKU - double wide characters in charsets 0x24..end of kanji
			KANJI   - collation values are 0x2900 less than WPChar value
****************************************************************************/
FLMUINT16 flmAsiaGetCollation(
	FLMUINT16	ui16WpChar,			// WP char to get collation values
	FLMUINT16	ui16NextWpChar,	// Next WP char - for CS11 voicing marks
	FLMUINT16	ui16PrevColValue, // Previous collating value
	FLMUINT16 *	pui16ColValue,		// Returns collation value
	FLMUINT16 *	pui16SubColVal,	// Returns sub-collation value
	FLMBYTE *	pucCaseBits,		// Returns case bits value
	FLMUINT16	uiUppercaseFlag)	// Set if to convert to uppercase
{
	FLMUINT16	ui16ColValue;
	FLMUINT16	ui16SubColVal;
	FLMBYTE		ucCaseBits = 0;
	FLMBYTE		ucCharSet = ui16WpChar >> 8;
	FLMBYTE		ucCharVal = ui16WpChar & 0xFF;
	FLMUINT16	ui16Hankaku;
	FLMUINT		uiLoop;
	FLMUINT16	ui16ReturnValue = 1;

	ui16ColValue = ui16SubColVal = 0;

	// Kanji or above

	if (ucCharSet >= 0x2B)
	{

		// Puts 2 or above into high byte.

		ui16ColValue = ui16WpChar - 0x2900;

		// No subcollation or case bits need to be set

		goto Exit;
	}

	// Single wide character? (HANKAKU)

	if (ucCharSet < 11)
	{

		// Get the values from a non-asian character LATIN, GREEK or
		// CYRILLIC. The width bit may have been set on a jump to label from
		// below.

Latin_Greek_Cyrillic:

		// YES: Pass FLM_US_LANG because this is what we want - Prevents double
		// character sorting.

		ui16ColValue = flmGetCollation( ui16WpChar, FLM_US_LANG);

		if (uiUppercaseFlag || flmIsUpper( ui16WpChar))
		{

			// Uppercase - set case bit

			ucCaseBits |= SET_CASE_BIT;
		}

		// Character for which there is no collation value?

		if (ui16ColValue == COLS0)
		{
			ui16ReturnValue = 0;
			
			if (!flmIsUpper( ui16WpChar))
			{
				// Convert to uppercase

				ui16WpChar--;
			}

			ui16ColValue = 0xFFFF;
			ui16SubColVal = ui16WpChar;
		}
		else if (ucCharSet)						// Don't bother with ascii
		{
			if (!flmIsUpper( ui16WpChar))
			{
				// Convert to uppercase

				ui16WpChar--;
			}

			if (ucCharSet == CHSMUL1)
			{
				FLMUINT16	ui16Base;
				FLMUINT16	ui16Diacritic;

				ui16SubColVal = !flmCh6Brkcar( ui16WpChar, &ui16Base, &ui16Diacritic) 
											? flmDia60Tbl[ui16Diacritic & 0xFF] 
											: ui16WpChar;
			}
			else if (ucCharSet == CHSGREK)	// GREEK
			{
				if (ui16WpChar >= 0x834 ||		// [8,52] or above
					 ui16WpChar == 0x804 ||		// [8,4] BETA Medial | Terminal
					 ui16WpChar == 0x826)
				{	
					// [8,38] SIGMA terminal
					
					ui16SubColVal = ui16WpChar;
				}
			}
			else if (ucCharSet == CHSCYR)			// CYRILLIC
			{
				if (ui16WpChar >= 0xA90)			// [10, 144] or above
				{
					ui16SubColVal = ui16WpChar;	// Dup collation values
				}
			}

			// else don't need a sub collation value

		}

		goto Exit;
	}

	// Single wide Japanese character?

	if (ucCharSet == 11)
	{
		FLMUINT16	ui16KanaChar;

		// Convert charset 11 to Zenkaku (double wide) CS24 or CS26 hex.
		// All characters in charset 11 will convert to CS24 or CS26. When
		// combining the collation and the sub-collation values.

		if (HanToZenkaku( ui16WpChar, ui16NextWpChar, &ui16KanaChar) == 2)
		{
			// Return 2

			ui16ReturnValue++;
		}

		ucCaseBits |= SET_WIDTH_BIT;				// Set so will allow to go back
		ui16WpChar = ui16KanaChar;					// If in CS24 will fall through
															// to ZenKaku
		ucCharSet = ui16KanaChar >> 8;
		ucCharVal = ui16KanaChar & 0xFF;
	}

	if (ui16WpChar < 0x2400)
	{
		// In some other character set

		goto Latin_Greek_Cyrillic;
	}
	else if (ui16WpChar >= 0x255e && ui16WpChar <= 0x2655)
	{
		if (ui16WpChar >= 0x2600)
		{
			ucCaseBits |= SET_KATAKANA_BIT;
		}

		// HIRAGANA and KATAKANA Kana contains both hiragana and katakana.
		// The tables contain the same characters in same order

		if (ucCharSet == 0x25)
		{
			// Change value to be in character set 26

			ucCharVal -= 0x5E;
		}

		ui16ColValue = 0x0100 + KanaColTbl[ucCharVal];
		ui16SubColVal = flmKanaSubColTbl[ucCharVal];
		goto Exit;
	}

	if ((ui16Hankaku = ZenToHankaku( ui16WpChar, (FLMUINT16*) 0)) != 0)
	{
		if ((ui16Hankaku >> 8) != 11)
		{
			ui16WpChar = ui16Hankaku;
			ucCharSet = ui16WpChar >> 8;
			ucCharVal = ui16WpChar & 0xFF;
			ucCaseBits |= SET_WIDTH_BIT;
			goto Latin_Greek_Cyrillic;
		}
	}

	// 0x2400..0x24bc Japanese symbols that cannot be converted to
	// Hankaku. All 6 original symbol chars from 11 will also be here.
	// First try to find a collation value of the symbol. The sub-collation
	// value will be the position in the CS24 table + 1.

	for (uiLoop = 0;
		  uiLoop < (sizeof(flmCh24ColTbl) / sizeof(BYTE_WORD_TBL));
		  uiLoop++)
	{
		if (ucCharVal == flmCh24ColTbl[uiLoop].ByteValue)
		{
			if ((ui16ColValue = flmCh24ColTbl[uiLoop].WordValue) < 0x100)
			{
				// Don't save for chuuten, dakuten, handakuten

				ui16SubColVal = (FLMUINT16) (uiLoop + 1);
			}
			
			break;
		}
	}

	if (!ui16ColValue)
	{
		// Now see if it's a repeat or repeat-vowel character

		if ((((ucCharVal >= 0x12) && (ucCharVal <= 0x15)) ||
			 (ucCharVal == 0x17) || (ucCharVal == 0x18)) &&
			 ((ui16PrevColValue >> 8) == 1))
		{
			ui16ColValue = ui16PrevColValue;

			// Store original WP character

			ui16SubColVal = ui16WpChar;
		}
		else if ((ucCharVal == 0x1B) &&						// repeat vowel?
					(ui16PrevColValue >= 0x100) &&
					(ui16PrevColValue < COLS_ASIAN_MARKS)) // Previous kana char?
		{
			ui16ColValue = 0x0100 + KanaColToVowel[ui16PrevColValue & 0xFF];

			// Store original WP character

			ui16SubColVal = ui16WpChar;
		}
		else
		{
			ui16ReturnValue = 0;
			ui16ColValue = 0xFFFF;			// No collation value
			ui16SubColVal = ui16WpChar;	// Never have changed if gets here
		}
	}

Exit:

	*pui16ColValue = ui16ColValue;
	*pui16SubColVal = ui16SubColVal;
	*pucCaseBits = ucCaseBits;

	return (ui16ReturnValue);
}

/*****************************************************************************
Desc:		Get the Flaim collating string and convert back to a WP word string
Ret:		Length of new WP word string
*****************************************************************************/
FLMUINT FWWSGetColStr(
	FLMBYTE *		fColStr,				// Points to the Flaim collated string
	FLMUINT *		fcStrLenRV,			// Length of the Flaim collated string
	FLMBYTE *		wordStr,				// Output string to build - WP word string
	FLMUINT			fWPLang,				// FLAIM WP language number
	FLMBOOL *		pbDataTruncated,	// Set to TRUE if truncated
	FLMBOOL *		pbFirstSubstring) // Sets to TRUE if first substring
{
	FLMBYTE *	wsPtr = wordStr;			// Points to the word string data area
	FLMUINT		length = *fcStrLenRV;	// May optimize as a register
	FLMUINT		pos = 0;						// Position in fColStr[]
	FLMUINT		bitPos;						// Computed bit position
	FLMUINT		colChar;						// Not portable if a FLMBYTE value
	FLMUINT		wdStrLen;
	FLMBOOL		hebrewArabicFlag = 0;	// Set if hebrew/arabic language

	// WARNING: The code is duplicated for performance reasons. The US code
	// below is much more optimized so any changes must be done twice.

	if (fWPLang != FLM_US_LANG)				// Code for NON-US languages
	{
		if ((fWPLang == FLM_AR_LANG) ||		// Arabic
			 (fWPLang == FLM_FA_LANG) ||		// Farsi - persian
			 (fWPLang == FLM_HE_LANG) ||		// Hebrew
			 (fWPLang == FLM_UR_LANG))			// Urdu
		{	
			hebrewArabicFlag++;
		}

		while (length && (fColStr[pos] > MAX_COL_OPCODE))
		{
			length--;
			colChar = (FLMUINT) fColStr[pos++];
			
			switch (colChar)
			{
				case COLS9 + 4:							// ch in spanish
				case COLS9 + 11:							// ch in czech
				{
					// Put the WP char in the word string

					UW2FBA( (FLMUINT16) 'C', wsPtr);
					wsPtr += 2;
					colChar = (FLMUINT) 'H';
					pos++;									// move past second duplicate char
					break;
				}
				
				case COLS9 + 17:							// ll in spanish
				{
					// Put the WP char in the word string

					UW2FBA( (FLMUINT16) 'L', wsPtr);
					wsPtr += 2;
					colChar = (FLMUINT) 'L';
					pos++;									// move past duplicate character
					break;
				}
				
				case COLS0:									// Non collating character
				{
					// Actual character is in sub-collation area

					colChar = (FLMUINT) 0xFFFF;
					break;
				}
				
				default:
				{

					if (hebrewArabicFlag && (colChar >= COLS10h))
					{
						colChar = (colChar < COLS10a)
										? (FLMUINT) (0x900 + (colChar - (COLS10h)))				// Hebrew
										: (FLMUINT) (HebArabColToWPChr[colChar - (COLS10a)]); // Arabic
					}
					else
					{
						colChar = (FLMUINT) colToWPChr[colChar - COLLS];
					}
					
					break;
				}
			}

			UW2FBA( (FLMUINT16) colChar, wsPtr);
			wsPtr += 2;
		}
	}
	else
	{
		while (length && (fColStr[pos] > MAX_COL_OPCODE))
		{
			length--;

			// Move in the WP value given uppercase collated value

			colChar = (FLMUINT) fColStr[pos++];

			if (colChar == COLS0)
			{
				colChar = (FLMUINT) 0xFFFF;
			}
			else
			{
				colChar = (FLMUINT) colToWPChr[colChar - COLLS];
			}

			UW2FBA( (FLMUINT16) colChar, wsPtr);
			wsPtr += 2;
		}
	}

	// NULL Terminate the string

	UW2FBA( (FLMUINT16) 0, wsPtr);
	wdStrLen = pos + pos;

	// Parse through the sub-collation and case information.
	// Watch out for COMP CollStrPosT indexes-doesn't have case info after
	// Here are values for some of the codes:
	//		[ 0x01] - end for fields case info follows - for COMP POST indexes
	//		[ 0x02] - compound marker
	//		[ 0x05] - case bits follow
	//		[ 0x06] - case information is all uppercase
	//		[ 0x07] - beginning of sub-collation information
	//		[ 0x08] - first substring field that is made
	//		[ 0x09] - truncation marker for text and binary
	// 
	// Asian chars the case information should always be there and not
	// compressed out.  This is because the case information could change
	// the actual width of the character from 0x26xx to charset 11.
	
	if (length && fColStr[pos] == COLL_FIRST_SUBSTRING)
	{
		if (pbFirstSubstring)
		{
			*pbFirstSubstring = TRUE;		// Don't need to initialize to FALSE.
		}

		length--;
		pos++;
	}

	if (length && fColStr[pos] == COLL_TRUNCATED)
	{
		if (pbDataTruncated)
		{
			*pbDataTruncated = TRUE;		// Don't need to initialize to FALSE.
		}

		length--;
		pos++;
	}

	if (length && (fColStr[pos] == (COLL_MARKER | SC_SUB_COL)))
	{
		FLMUINT	tempLen;

		// Do another pass on the word string adding the diacritics

		bitPos = FWWSCmbSubColBuf( wordStr, &wdStrLen, &fColStr[++pos],
										  hebrewArabicFlag);

		// Move pos to next byte value

		tempLen = BYTES_IN_BITS( bitPos);
		pos += tempLen;
		length -= tempLen + 1;				// The 1 includes the 0x07 byte
	}

	// Does the case info follow?

	if (length && (fColStr[pos] > COMPOUND_MARKER))
	{

		// Take care of the lower and upper case conversion If mixed case
		// then convert using case bits

		if (fColStr[pos++] & SC_MIXED)	// Increment pos here!
		{

			// Don't pre-increment pos on line below!

			pos += FWWSToMixed( wordStr, wdStrLen, &fColStr[pos], fWPLang);
		}

		// else 0x04 or 0x06 - all characters already in uppercase

	}

	*fcStrLenRV = pos;						// pos should be on the 0x01 or 0x02 flag
	return (wdStrLen);						// Return the length of the word string
}

/**************************************************************************
Desc: 	Combine the diacritic 5 bit values to an existing word string
Todo:		May want to check flmCh6Cmbcar() for CY return value
***************************************************************************/
FLMUINT FWWSCmbSubColBuf(
	FLMBYTE*		wordStr,				// Existing word string to modify
	FLMUINT*		wdStrLenRV,			// Wordstring length in bytes
	FLMBYTE*		subColBuf,			// Diacritic values in 5 bit sets
	FLMBOOL		hebrewArabicFlag) // Set if language is Hebrew or Arabic
{
	FLMUINT		subColBitPos = 0;
	FLMUINT		numWords = *wdStrLenRV >> 1;
	FLMUINT16	diac;
	FLMUINT16	wpchar;
	FLMUINT		temp;

	// For each word in the word string ...

	while (numWords--)
	{

		// Label used for hebrew/arabic - additional subcollation can follow ;
		// This macro DOESN'T increment bitPos
		
		if (TEST1BIT( subColBuf, subColBitPos))
		{
			
			// If "11110" - unmappable unicode char - 0xFFFF is before it
			// If "1110" then INDEX extended char is inserted
			// If "110" then extended char follows that replaces collation 
			// If "10" then take next 5 bits which contain the diacritic 
			// subcollation value.

after_last_character:

			subColBitPos++;		// Eat the first 1 bit
			
			if (!TEST1BIT( subColBuf, subColBitPos))
			{
				subColBitPos++;	// Eat the 0 bit
				diac = (FLMUINT16) (GETnBITS( 5, subColBuf, subColBitPos));
				subColBitPos += 5;

				if ((wpchar = FB2UW( wordStr)) < 0x100)	// If not extended base..
				{

					// Convert to WP diacritic and combine characters

					flmCh6Cmbcar( &wpchar, wpchar, (FLMUINT16) ml1_COLtoD[diac]);

					// Even if cmbcar fails, wpchar is still set to a valid
					// value

					UW2FBA( wpchar, wordStr);
				}
				else if ((wpchar & 0xFF00) == 0x0D00)		// arabic?
				{
					wpchar = ArabSubColToWPChr[diac];
					UW2FBA( wpchar, wordStr);
				}

				// else diacritic is extra info ;
				// cmbcar should not handle extended chars for this design
			}
			else						// "110" or "1110" or "11110"
			{
				subColBitPos++;	// Eat the 2nd '1' bit
				if (TEST1BIT( subColBuf, subColBitPos))	// Test the 3rd bit
				{

					// 1110 - shift wpchars down 1 word and insert value below

					subColBitPos++;	// Eat the 3rd '1' bit
					*wdStrLenRV += 2; // Return 2 more bytes

					if (TEST1BIT( subColBuf, subColBitPos))	// Test 4th bit
					{

						// Unconvertable UNICODE character.
						//
						// The format will be 4 bytes, 0xFF, 0xFF, 2 byte Unicode
						
						shiftN( wordStr, numWords + numWords + 4, 2);
						subColBitPos++;							// Eat the 4th '1' bit
						wordStr += 2;								// Skip the 0xFFFF for now
					}
					else
					{
						// Move down 2 byte NULL and rest of the 2 byte characters.
						// The extended character does not have a 0xFF col value
						
						shiftN( wordStr, numWords + numWords + 2, 2);
						numWords++;
					}
				}

				subColBitPos++;									// Skip past the zero bit
				subColBitPos = (subColBitPos + 7) & (~7); // roundup to next byte
				temp = BYTES_IN_BITS( subColBitPos);		// compute position
				wordStr[1] = subColBuf[temp];					// Character set
				wordStr[0] = subColBuf[temp + 1];			// Character

				subColBitPos += 16;
			}
		}
		else
		{
			subColBitPos++;
		}

		wordStr += 2;						// Next WP character
	}

	if (hebrewArabicFlag)
	{
		if (TEST1BIT( subColBuf, subColBitPos))
		{

			// Hebrew/Arabic can have trailing accents that don't have a
			// matching collation value. Keep looping in this case.  Note that
			// subColBitPos isn't incremented above.

			numWords = 0;					// set so won't loop forever!
			goto after_last_character; // process trailing bit
		}

		subColBitPos++;					// Eat the last '0' bit
	}

	return (subColBitPos);
}

/**************************************************************************
Desc: 	Convert the word string to lower case chars given low/upp bit string
Out:	 	WP characters have modified to their original case
Ret:		Number of bytes used in the lower/upper buffer
Notes:	Only WP to lower case conversion is done here for each bit NOT set.
***************************************************************************/
FLMUINT FWWSToMixed(
	FLMBYTE *		wordStr,			// Existing word string to modify
	FLMUINT			wdStrLen,		// Length of the wordstring in bytes
	FLMBYTE*			lowUpBitStr,	// Lower/upper case bit string
	FLMUINT			fWPLang)
{
	FLMUINT	numWords;
	FLMUINT	tempWord;
	FLMBYTE	tempByte = 0;
	FLMBYTE	maskByte;
	FLMBYTE	xorByte;

	xorByte = (fWPLang == FLM_US_LANG)
						? (FLMBYTE) 0 
						: (fWPLang == FLM_GR_LANG)
								? (FLMBYTE) 0xFF 
								: (FLMBYTE) 0;

	// For each word in the word string ...

	for (numWords = wdStrLen >> 1, maskByte = 0; 
		  numWords--; 
		  wordStr += 2, maskByte >>= 1)
	{
		if (maskByte == 0)					// Time to get another byte
		{
			tempByte = xorByte ^ *lowUpBitStr++;
			maskByte = 0x80;
		}

		if ((tempByte & maskByte) == 0)	// If lowercase conver - else is upper
		{

			// Convert to lower case - COLL -> WP is already in upper case

			tempWord = (FLMUINT) FB2UW( wordStr);
			
			if ((tempWord >= ASCII_UPPER_A) && (tempWord <= ASCII_UPPER_Z))
			{
				tempWord |= 0x20;
			}
			else
			{
				FLMBYTE	charVal = (FLMBYTE) (tempWord & 0xFF);
				FLMBYTE	charSet = (FLMBYTE) (tempWord >> 8);

				// check if charact within region of character set

				if (((charSet == CHSMUL1) && 
						((charVal >= 26) && (charVal <= 241))) ||
					 ((charSet == CHSGREK) && (charVal <= 69)) || 
					 ((charSet == CHSCYR) && (charVal <= 199)))
				{
					tempWord |= 0x01;
				}
			}

			UW2FBA( (FLMUINT16) tempWord, wordStr);
		}
	}

	numWords = wdStrLen >> 1;
	return (BYTES_IN_BITS( numWords));
}

/********************************************************************
Desc: Build a responce tree of NODEs for the key output.
*********************************************************************/
RCODE flmIxKeyOutput(
	IXD *				pIxd,
	FLMBYTE *		pucFromKey,
	FLMUINT			uiKeyLen,
	FlmRecord **	ppKeyRV,			// Returns key
	FLMBOOL			bFullFldPaths)	// If true add full field paths
{
	RCODE				rc = FERR_OK;
	FlmRecord *		pKey = NULL;
	void *			pvField;
	FLMBYTE			ucKeyBuf[ MAX_KEY_SIZ + 12];
	FLMBYTE *		pucToKey = &ucKeyBuf[ 0];
	FLMBYTE *		pucPostBuf = NULL;
	IFD *				pIfd;
	FLMUINT			uiLongValue;
	FLMUINT			uiToKeyLen;
	FLMUINT			uiLanguage = pIxd->uiLanguage;
	FLMUINT			uiFromKeyLen;
	FLMUINT 			uiFromRemaining;
	FLMUINT			uiPostLen;
	FLMUINT			uiPostPos = 0;
	FLMUINT			uiTempFromKeyLen;
	FLMUINT			uiFldType;
	FLMUINT			uiDataType;
	FLMBOOL			bDataRightTruncated;
	FLMBOOL			bFirstSubstring;
	FLMBOOL			bSigSign;
	FLMBYTE			ucTemp;
	FLMUINT			uiContainer;
	FLMUINT			uiMaxKeySize;

	// If the index is on all containers, see if this key has
	// a container component.  If so, strip it off.

	if( (uiContainer = pIxd->uiContainerNum) == 0)
	{
		FLMUINT	uiContainerPartLen = getIxContainerPartLen( pIxd);

		if (uiKeyLen <= uiContainerPartLen)
		{
			flmAssert( 0);
			rc = RC_SET( FERR_BTREE_ERROR);
			goto Exit;
		}
		
		uiContainer = getContainerFromKey( pucFromKey, uiKeyLen);

		// Subtract off the bytes for the container part.

		uiKeyLen -= uiContainerPartLen;
		uiMaxKeySize = MAX_KEY_SIZ - uiContainerPartLen;
	}
	else
	{
		uiMaxKeySize = MAX_KEY_SIZ;
	}

	flmAssert( uiLanguage != 0xFFFF);

	if (*ppKeyRV)
	{
		if( (*ppKeyRV)->isReadOnly() || (*ppKeyRV)->isCached())
		{
			(*ppKeyRV)->Release();
			*ppKeyRV = NULL;
		}
		else
		{
			(*ppKeyRV)->clear();
		}
	}

	if( (pKey = *ppKeyRV) == NULL)
	{
		if( (pKey = f_new FlmRecord) == NULL)
		{
			rc = RC_SET( FERR_MEM);
			goto Exit;
		}

		*ppKeyRV = pKey;
	}

	pKey->setContainerID( uiContainer);

	uiFromKeyLen = uiFromRemaining = uiKeyLen;
	pIfd = pIxd->pFirstIfd;

	// If post index, get post low/up section.

	if( pIfd->uiFlags & IFD_POST )
	{

		// Last byte has low/upper length

		uiPostLen = pucFromKey[ uiFromKeyLen - 1 ];
		pucPostBuf = &pucFromKey[ uiFromKeyLen - uiPostLen - 1 ];
		uiPostPos = 0;
	}

	if (RC_BAD( rc = pKey->insertLast( 0, FLM_KEY_TAG, FLM_CONTEXT_TYPE, NULL)))
	{
		goto Exit;
	}

	// Loop for each compound piece of key

	for( ;;)								
	{
	   FLMBOOL	bIsAsianCompound;
	   FLMUINT	uiMarker;

		bDataRightTruncated = bFirstSubstring = FALSE;

  		bIsAsianCompound = (FLMBOOL)(((uiLanguage >= FLM_FIRST_DBCS_LANG) && 
												(uiLanguage <= FLM_LAST_DBCS_LANG) &&
												(IFD_GET_FIELD_TYPE( pIfd) == FLM_TEXT_TYPE) &&
												(!(pIfd->uiFlags & IFD_CONTEXT)))
													? (FLMBOOL)TRUE
													: (FLMBOOL)FALSE);
	   
		uiMarker = (FLMUINT)((bIsAsianCompound)
									? (FLMUINT)((FLMUINT)(*pucFromKey << 8) +
													*(pucFromKey+1))
									: (FLMUINT) *pucFromKey);
		uiFldType = (FLMUINT) IFD_GET_FIELD_TYPE( pIfd);
		uiDataType = IFD_GET_FIELD_TYPE( pIfd);	
		
		// Hit a compound marker or end of key marker
		// Check includes COMPOUND_MARKER & END_COMPOUND_MARKER

		if( uiMarker <= NULL_KEY_MARKER)		
		{

			// If the field is required or single field then generate an empty node.

			if( ((pIfd->uiFlags & IFD_OPTIONAL) == 0) ||
				 (uiFldType == FLM_TEXT_TYPE) ||
				 (uiFldType == FLM_BINARY_TYPE) ||
				 ((pIfd->uiFlags & IFD_LAST) && !pIfd->uiCompoundPos ))
			{
				if( RC_BAD( rc = flmBuildKeyPaths( pIfd, pIfd->uiFldNum,
											uiDataType, bFullFldPaths, pKey, &pvField)))
					goto Exit;
			}
			if( uiMarker == END_COMPOUND_MARKER)	// Used for post keys
				break;

			uiFromKeyLen = 0;		// This piece is zero - skip it - may be others
		}
		else
		{

			// If compound key or if only field used in index
			// output the key elements field number or else 'NA'

			if( pIfd->uiFlags & IFD_CONTEXT)
			{
				if( RC_BAD( rc = flmBuildKeyPaths( pIfd,
											f_bigEndianToUINT16( &pucFromKey [1]),
											uiDataType, bFullFldPaths, pKey, &pvField)))
				{
					goto Exit;
				}
				uiFromKeyLen = KY_CONTEXT_LEN;
			}

			else
			{
				if( RC_BAD( rc = flmBuildKeyPaths( pIfd, pIfd->uiFldNum,
											uiDataType, bFullFldPaths, pKey, &pvField)))
				{
					goto Exit;
				}

				// Grab only the Nth section of key if compound key
				//	Null out key if uiToKeyLen gets 0
				
				UD2FBA( 0, pucToKey);

				switch( uiDataType)
				{
					case FLM_TEXT_TYPE:

						uiTempFromKeyLen = uiFromKeyLen;
						uiToKeyLen = FColStrToText( pucFromKey, &uiTempFromKeyLen, pucToKey,
												uiLanguage, pucPostBuf, &uiPostPos, 
												&bDataRightTruncated, &bFirstSubstring);
						uiFromKeyLen = uiTempFromKeyLen;
						break;

					case FLM_NUMBER_TYPE:
					{
						FLMUINT		uiFirstColNibble;		// Current collated nibble
						FLMUINT		uiFirstNumNibble;		// Current output nibble
						FLMBYTE *	pucOutPtr;				// Output pointer
						FLMBYTE *	pucColPtr;
						FLMUINT		uiBytesProcessed;

						// Start at byte after sign/magnitude byte

						pucColPtr = pucFromKey + 1;
						uiBytesProcessed = 1;
						uiFirstColNibble = 1;

						// Determine the sign of the number

						pucOutPtr = pucToKey;
						if( (bSigSign = (*pucFromKey & SIG_POS)) == 0)
						{
							*pucOutPtr = 0xB0;
							uiFirstNumNibble = 0;
						}
						else
						{
							uiFirstNumNibble = 1;
						}

						// Parse through the collated number outputting data
						// to the buffer as we go.

						for( ;;)
						{
							// Determine what we are pointing at

							if( (ucTemp = *pucColPtr) <= COMPOUND_MARKER)
							{
								break;
							}

							if( uiFirstColNibble++ & 1)
							{
								ucTemp >>= 4;
							}
							else
							{
								ucTemp &= 0x0F;
								pucColPtr++;
								uiBytesProcessed++;
							}

							// A hex F signifies the end of a collated number with an
							// odd number of nibbles

							if( ucTemp == 0x0F)
							{
								break;
							}

							// Convert collated number nibble to BCD nibble
							// and lay it in buffer

							ucTemp -= COLLATED_DIGIT_OFFSET;

							// Is number negative?

							if( !bSigSign)
							{
								// Negative values are ~ed

								ucTemp = (FLMBYTE)(10 -(ucTemp + 1));
							}

							if( uiFirstNumNibble++ & 1)
							{
								*pucOutPtr = (FLMBYTE)(ucTemp << 4);
							}
							else
							{
								*pucOutPtr++ += ucTemp;
							}

							if( uiBytesProcessed == uiFromKeyLen)
							{
								break;
							}
						}

						// Append Terminator code to internal number

						*pucOutPtr++ |= (uiFirstNumNibble & 1) ? 0xFF : 0x0F;
						uiToKeyLen = (FLMUINT) (pucOutPtr - pucToKey);
						uiFromKeyLen = uiBytesProcessed;
						rc = FERR_OK;
						break;
					}

					case FLM_BINARY_TYPE:
					{
						FLMUINT			uiMaxLength;
						FLMBYTE *		pucSrc = pucFromKey;

						uiMaxLength = ((uiFromKeyLen >> 1) < uiMaxKeySize)
												? (FLMUINT)(uiFromKeyLen >> 1)
												: (FLMUINT)uiMaxKeySize;
						uiToKeyLen = 0;
						while( (uiToKeyLen < uiMaxLength) && ((ucTemp = *pucSrc) >= COLLS))
						{

							// Take two bytes from source to make one byte in dest

							pucToKey[ uiToKeyLen++] =
								(FLMBYTE)(((ucTemp - COLLS) << 4) + (*(pucSrc + 1) - COLLS));
							pucSrc += 2;
						}

						if( (uiToKeyLen < (uiFromKeyLen >> 1)) && (*pucSrc >= COLLS))
						{
							rc = RC_SET( FERR_CONV_DEST_OVERFLOW);
						}
						else
						{
							rc = FERR_OK;
							uiFromKeyLen = uiToKeyLen << 1;

							// FLAIM has a bug where the binary fields don't have
							// the COLL_TRUNCATED value on truncated values.
							// The good news is that we know the true length of
							// binary fields.
							if( *pucSrc == COLL_TRUNCATED)
							{
								uiFromKeyLen++;
								bDataRightTruncated = TRUE;
							}
							else if( uiToKeyLen >= pIfd->uiLimit) 
							{
								bDataRightTruncated = TRUE;
							}
						}
						break;
					}

					case FLM_CONTEXT_TYPE:
					default:
						uiFromKeyLen = 5;

						uiLongValue = f_bigEndianToUINT32( pucFromKey + 1);
						UD2FBA( (FLMUINT32)uiLongValue, pucToKey);
						uiToKeyLen = 4;
						break;
				}

				if( RC_BAD( rc))
				{
					goto Exit;
				}

				// Allocate and Copy Value into the node

				if( uiToKeyLen)
				{
					FLMBYTE *	pucValue;

					if( RC_BAD(rc = pKey->allocStorageSpace( pvField,
								uiDataType, uiToKeyLen, 0, 0, 0, &pucValue, NULL)))
					{
						goto Exit;
					}
					
					f_memcpy( pucValue, pucToKey, uiToKeyLen);
				}

				// Set first sub-string and truncated flags.

				if( (pIfd->uiFlags & IFD_SUBSTRING) && !bFirstSubstring)
				{
					pKey->setLeftTruncated( pvField, TRUE);
				}
				if( bDataRightTruncated)
				{
					pKey->setRightTruncated( pvField, TRUE);
				}
			}
		}

		// Compute variables for next section of compound key
		// Add 1 for compound marker if still is stuff in key

		if( uiFromRemaining != uiFromKeyLen)
		{
			uiFromKeyLen += (FLMUINT)(bIsAsianCompound ? (FLMUINT)2 : (FLMUINT)1);
		}
		
		pucFromKey += uiFromKeyLen;
		
		if( (uiFromKeyLen = (uiFromRemaining -= uiFromKeyLen)) == 0)
		{
			break;
		}
		
		while( ((pIfd->uiFlags & IFD_LAST) == 0) 
		   &&   (pIfd->uiCompoundPos == (pIfd+1)->uiCompoundPos))
		{
			pIfd++;
		}
		
		if( pIfd->uiFlags & IFD_LAST)
		{
			break;
		}
		
		pIfd++;
	}

	// Check if we have one field left. 

	if( (pIfd->uiFlags & IFD_LAST) == 0)
	{
		while( (pIfd->uiFlags & IFD_LAST) == 0) 
		{
			pIfd++;
		}
		if( (pIfd->uiFlags & IFD_OPTIONAL) == 0)
		{
			if( RC_BAD( rc = flmBuildKeyPaths( pIfd, pIfd->uiFldNum,
										uiDataType, bFullFldPaths, pKey, &pvField)))
			{
				goto Exit;
			}
		}
	}

Exit:

	return( rc);
}

/****************************************************************************
Desc:	This module will read all references of an index key.
		The references will be output number defined as REFS_PER_NODE
****************************************************************************/
RCODE flmBuildKeyPaths(
	IFD *			pIfd,
	FLMUINT		uiFldNum,
	FLMUINT		uiDataType,
	FLMBOOL		bFullFldPaths,
	FlmRecord *	pKey,
	void **		ppvField)
{
	RCODE			rc = FERR_OK;
	void *		pvField;
	void *		pvParentField;
	void *		pvChildField;
	FLMUINT *	pFieldPath;
	FLMUINT		uiTempDataType;
	FLMUINT		uiFieldPos;
	FLMUINT		uiTargetFieldID;

	if( !bFullFldPaths)
	{
		rc = pKey->insertLast( 1, uiFldNum, uiDataType, &pvField);
		goto Exit;
	}

	pFieldPath = pIfd->pFieldPathPToC;
	pvParentField = pKey->root();
	uiFieldPos = 0;
	
	// Loop finding field matches.

	pvField = pKey->find( pvParentField, pFieldPath[ uiFieldPos]);
	if( pvField)
	{
		pvParentField = pvField;
		uiFieldPos++;
		uiTargetFieldID = pFieldPath[ uiFieldPos];

		// Loop finding matching children from this point on.

		for( pvChildField = pKey->firstChild( pvParentField); pvChildField; )
		{
			if( pKey->getFieldID( pvChildField) == uiTargetFieldID)
			{
				// On the child field?
				
				if( pFieldPath[ uiFieldPos + 1] == 0)
				{
					pvField = pvChildField;
					
					// Set the data type in case the data length is zero.
					
					pKey->allocStorageSpace( pvField, uiDataType, 0, 0, 0, 0, NULL, NULL);
					break;
				}
				
				pvParentField = pvChildField;
				uiFieldPos++;
				uiTargetFieldID = pFieldPath[ uiFieldPos];
				pvChildField = pKey->firstChild( pvParentField);
			}
			else
			{
				pvChildField = pKey->nextSibling( pvChildField);
			}
		}
	}

	// Insert the rest of the field path down to the value field (uiFieldPos==0).

	uiTempDataType = FLM_CONTEXT_TYPE;
	for( ; pFieldPath[ uiFieldPos]; uiFieldPos++)
	{
		// Add the real data type for the last field, otherwise set as context.
		
		if( pFieldPath[ uiFieldPos + 1] == 0)
		{
			uiTempDataType = uiDataType;
		}

		if( RC_BAD( rc = pKey->insert( pvParentField, INSERT_LAST_CHILD,
								pFieldPath[ uiFieldPos], uiTempDataType, &pvField)))
		{
			goto Exit;
		}
		pvParentField = pvField;
	}

Exit:

	*ppvField = pvField;
	return( rc);
}

/****************************************************************************
Desc:  	Compare only the leading left and right characters according
			to the many flags that are passed in.  This routine operates
			to save and set state for the calling routine.
TODO:
			This routine does NOT support Asian, Hebrew, or Arabic language
			collations.  In addition, flmCheckDoubleCollation() is not called 
			for other non-US lanagues.  There is still a lot of work to do!
			This is our default US compare and it is not very good for JP.

Return:	Signed value of compare.  
			<0 if less than, 0 if equal, >0 if greater than.
			
Asian Notes:
			The asian compare takes two characters and may use one or both.
			This makes the algorithm complex so we may have to build full
			tests to see what we broke.
			
NDS Notes:
			The right side (search string) is already formatted according
			to the space/dash rules of the syntax.  
****************************************************************************/
FLMINT flmTextCompareSingleChar(
	FLMBYTE **		ppLeftText,		// [in] Points to current value.
											// [out] Points to next character if equals.
	FLMUINT *		puiLeftLen,		// [in] Bytes remaining in text string.
											// [out] Bytes remaining in text string.
	FLMUINT *		puiLeftWpChar2,// Second left character - for double characters
	FLMBYTE **		ppRightText,	// [in] Points to current value.
											// [out] Points to next character if equals.
	FLMUINT *		puiRightLen,	// [in] Bytes remaining in text string.
											// [out] Bytes remaining in text string.
	FLMUINT *		puiRightWpChar2,// Second right character - for double characters.
	FLMINT *			piSubColCompare,//[in] If NULL disregard the subcollation
											// values if collation values are equal.
											// [out] If equals is returned, value is
											// set ONLY if the signed value of comparing
											// the sub-collation values is not equal.
											// See lengthy unicode compare below.
	FLMINT *			piCaseCompare,	// [in] If NULL disregard the case bits
											// if collation values are equal.  Japanese
											// values are an exception to this rule.
											// [out] If equals is returned, value is
											// set ONLY if the signed value of comparing 
											// the case values is not equal.
	FLMBOOL *		pbHitWildCard,	// [in] If NULL then do not look for wild
											// cards in the right text string.
											// [out] If non-null, a wild card (*,?) will
											// be looked for on the RIGHT SIDE ONLY.
											// If '?' is found 0 will be returned and
											// pointers are advanced.  If '*' is found,
											// this value will be set to TRUE and the
											// right side is advanced.  If no wild 
											// card is found the value will not be set.
	FLMINT			iCompareType,	// COMPARE_COLLATION, COMPARE_COL_AND_SUBCOL, COMPARE_VALUE
	FLMUINT16 * 	pui16ColVal,	// Needed for asian collation compare.
	FLMUINT			uiFlags,			// FLM_* flags
	FLMUINT			uiLangId)		// FLAIM/WordPerfect Lanaguge id.
{
	FLMBYTE *		pLeftText = *ppLeftText;
	FLMBYTE *		pRightText = *ppRightText;
	FLMINT			iCompare = 0;
	FLMUINT			uiRightFlags = uiFlags;
	FLMUINT16		ui16LeftWPChar;
	FLMUINT16		ui16LeftUniChar;
	FLMUINT16		ui16RightWPChar;
	FLMUINT16		ui16RightUniChar;
	FLMUINT			uiLeftValueLen;
	FLMUINT			uiRightValueLen;
	FLMUINT16		ui16LeftCol;
	FLMUINT16		ui16RightCol;
	FLMUINT			uiLeftWpChar2 = *puiLeftWpChar2;
	FLMUINT			uiRightWpChar2 = *puiRightWpChar2;
	FLMBOOL			bLeftTwoIntoOne;
	FLMBOOL			bRightTwoIntoOne;

	// Get the next character from the TEXT string.  NOTE: OEM characters
	// will be returned as a UNICODE character.  A unicode character here
	// is a value that cannot be converted to the WP set (no good collation value)..

	uiLeftValueLen = flmTextGetValue( pLeftText, *puiLeftLen, &uiLeftWpChar2,
									uiFlags, &ui16LeftWPChar, &ui16LeftUniChar);
									
	uiRightValueLen = flmTextGetValue( pRightText, *puiRightLen, &uiRightWpChar2,
							uiRightFlags, &ui16RightWPChar, &ui16RightUniChar);

	// At this point, the double character, if any, should have been consumed.

	flmAssert( !uiLeftWpChar2 && !uiRightWpChar2);

	// Check for the following escape characters: "\\" "*" and "\\" "\\"

	if( ui16RightWPChar == ASCII_BACKSLASH)
	{
		if( pRightText[ uiRightValueLen ] == ASCII_BACKSLASH)
		{
			uiRightValueLen++;
		}
		else if( pRightText[ uiRightValueLen ] == ASCII_WILDCARD)
		{
			ui16RightWPChar = ASCII_WILDCARD;
			uiRightValueLen++;
		}
	}
	// Checking for wild cards in the right string? (Always a WP character)
	else if( pbHitWildCard)	
	{

		// The '*' wildcard means to match zero or many characters.
		// The sick case of "A*B" compared to "A**B" should be considered.

		if( ui16RightWPChar == ASCII_WILDCARD)
		{
			// Eat all duplicate wild cards.
			
			while( pRightText[ uiRightValueLen] == ASCII_WILDCARD)
			{
				uiRightValueLen++;
			}

			// Advance the right value.  Keep left value alone.  
			// Return equals (default).

			*pbHitWildCard = TRUE;

			// Don't advance the left value.

			uiLeftValueLen = 0;
			uiLeftWpChar2 = *puiLeftWpChar2;
			goto Exit;
		}
	}

	// First section is to compare just WP values.
	
	if( ui16LeftWPChar && ui16RightWPChar)
	{
		FLMUINT16	ui16LeftSubCol;
		FLMUINT16	ui16RightSubCol;

		if (iCompareType == COMPARE_VALUE)
		{

			// Check the obvious case of equal WP values.

			if( ui16LeftWPChar != ui16RightWPChar) 
			{
				iCompare = -1;
			}
			goto Exit;
		}

		// JP compare code.

		if (uiLangId >= FLM_FIRST_DBCS_LANG && uiLangId <= FLM_LAST_DBCS_LANG)
		{
			FLMUINT		uiNextLeftLen;
			FLMUINT		uiNextRightLen;
			FLMUINT16	ui16NextLeftWPChar;
			FLMUINT16	ui16NextRightWPChar;
			FLMUINT16	ui16ColVal = pui16ColVal ? *pui16ColVal : 0;
			FLMBYTE		ucLeftCaseValue;
			FLMBYTE		ucRightCaseValue;

			// Should have already consumed double character, if any

			flmAssert( !uiLeftWpChar2 && !uiRightWpChar2);
			uiNextLeftLen  = flmTextGetValue( pLeftText+uiLeftValueLen, 
										*puiLeftLen, &uiLeftWpChar2, uiFlags,
										&ui16NextLeftWPChar, &ui16LeftUniChar);
			uiNextRightLen = flmTextGetValue( pRightText+uiRightValueLen,
									*puiRightLen, &uiRightWpChar2, uiFlags,
									&ui16NextRightWPChar, &ui16RightUniChar);

			// nextL/R WPChar may be zero.

			if (flmAsiaGetCollation( ui16LeftWPChar, ui16NextLeftWPChar, 
					ui16ColVal, &ui16LeftCol, &ui16LeftSubCol, 
					&ucLeftCaseValue, FALSE) == 2)
			{
				uiLeftValueLen += uiNextLeftLen;
			}
			
			if (flmAsiaGetCollation( ui16RightWPChar, ui16NextRightWPChar, 
					ui16ColVal, &ui16RightCol, &ui16RightSubCol, 
					&ucRightCaseValue, FALSE) == 2)
			{
				uiRightValueLen += uiNextRightLen;
			}
			
			// Compare all of the stuff now.

			if (ui16LeftCol == ui16RightCol)
			{
				if( (iCompareType == COMPARE_COL_AND_SUBCOL) ||
					 (piSubColCompare && (*piSubColCompare == 0)))
				{
					if( ui16LeftSubCol != ui16RightSubCol)
					{
						if( iCompareType == COMPARE_COL_AND_SUBCOL)
						{	
							iCompare = -1;
							goto Exit;
						}

						// At this point piSubColCompare cannot be NULL.

						*piSubColCompare = (ui16LeftSubCol < ui16RightSubCol) 
														? -1 
														: 1;

						// Write over the case compare value

						if( piCaseCompare )
						{
							*piCaseCompare = *piSubColCompare;
						}
					}
				}
				
				if (iCompareType != COMPARE_COL_AND_SUBCOL)
				{

					// Check case?

					if (piCaseCompare && (*piCaseCompare == 0))
					{
						if( ucLeftCaseValue != ucRightCaseValue)
						{
							*piCaseCompare = ucLeftCaseValue < ucRightCaseValue
															? -1
															: 1;
						}
					}
				}
			}
			else
			{
				iCompare = (ui16LeftCol < ui16RightCol) ? -1 : 1;
			}
			
			goto Exit;
		}

		flmAssert( !uiLeftWpChar2 && !uiRightWpChar2);

		if (uiLangId != FLM_US_LANG)
		{
			const FLMBYTE *	pucTmp;

			pucTmp = pLeftText + uiLeftValueLen;
			uiLeftWpChar2 = flmCheckDoubleCollation( &ui16LeftWPChar, 
										&bLeftTwoIntoOne, &pucTmp, uiLangId);
			uiLeftValueLen = (FLMUINT)(pucTmp - pLeftText);

			pucTmp = pRightText + uiRightValueLen;
			uiRightWpChar2 = flmCheckDoubleCollation( &ui16RightWPChar, 
										&bRightTwoIntoOne, &pucTmp, uiLangId);
			uiRightValueLen = (FLMUINT)(pucTmp - pRightText);

			// See if we got the same double character

			if (uiLeftWpChar2 == uiRightWpChar2 &&
				 ui16LeftWPChar == ui16RightWPChar)
			{
				uiLeftWpChar2 = 0;
				uiRightWpChar2 = 0;
				goto Exit;
			}
		}
		else if (ui16LeftWPChar == ui16RightWPChar)
		{

			// Same WP character

			goto Exit;
		}

		ui16LeftCol = flmGetCollation( ui16LeftWPChar, uiLangId);

		// Handle two characters collating as one.

		if (uiLeftWpChar2 && bLeftTwoIntoOne)
		{
			ui16LeftCol++;
		}

		ui16RightCol = flmGetCollation( ui16RightWPChar, uiLangId);

		// Handle two characters collating as one.

		if (uiRightWpChar2 && bRightTwoIntoOne)
		{
			ui16RightCol++;
		}

		if( ui16LeftCol == ui16RightCol)
		{
			// Should we bother to check subcollation? - don't bother with 7-bit

			if( ((iCompareType == COMPARE_COL_AND_SUBCOL) || 
				  (piSubColCompare && (*piSubColCompare == 0))) &&  
				 ((ui16LeftWPChar | ui16RightWPChar) & 0xFF00))
			{
				ui16LeftSubCol = flmTextGetSubCol( ui16LeftWPChar, 
															ui16LeftCol, uiLangId);
				ui16RightSubCol= flmTextGetSubCol( ui16RightWPChar, 
															ui16RightCol, uiLangId);

				if (!piCaseCompare)
				{

					// If the sub-collation value is the original
					// character, it means that the collation could not
					// distinguish the characters and sub-collation is being
					// used to do it.  However, this creates a problem when the
					// characters are the same character except for case.  In that
					// scenario, we incorrectly return a not-equal when we are
					// doing a case-insensitive comparison.  So, at this point,
					// we need to use the sub-collation for the upper-case of the
					// character instead of the sub-collation for the character
					// itself.

					if (ui16LeftSubCol == ui16LeftWPChar)
					{
						ui16LeftSubCol = flmTextGetSubCol(
													flmCh6Upper( ui16LeftWPChar),
													ui16LeftCol, uiLangId);
					}
					
					if (ui16RightSubCol == ui16RightWPChar)
					{
						ui16RightSubCol= flmTextGetSubCol(
													flmCh6Upper( ui16RightWPChar),
													ui16RightCol, uiLangId);
					}
				}

				// YES - go for it...
				
				if( ui16LeftSubCol != ui16RightSubCol)
				{
					if( iCompareType == COMPARE_COL_AND_SUBCOL)
					{	
						iCompare = (ui16LeftSubCol < ui16RightSubCol) ? -1 : 1;
						goto Exit;
					}
					
					// At this point piSubColCompare cannot be NULL.
					
					*piSubColCompare = (ui16LeftSubCol < ui16RightSubCol) 
														? -1 
														: 1;
					
					// Write over the case compare value
					
					if( piCaseCompare )
					{
						*piCaseCompare = *piSubColCompare;
					}
				}
			}

			if( iCompareType == COMPARE_COL_AND_SUBCOL)
			{
				goto Exit;
			}
			
			if( piCaseCompare && (*piCaseCompare == 0))
			{

				// flmIsUpper() only returns FALSE (lower) or TRUE (not-lower)

				FLMBOOL	bLeftUpper = flmIsUpper( ui16LeftWPChar);
				FLMBOOL	bRightUpper = flmIsUpper( ui16RightWPChar);

				if (bLeftUpper != bRightUpper)
				{
					*piCaseCompare = !bLeftUpper ? -1 : 1;
				}
			}
		}
		else
		{
			iCompare = (ui16LeftCol < ui16RightCol) ? -1 : 1;
		}
		
		goto Exit;

	}

	if( ui16LeftUniChar && ui16RightUniChar)
	{
		// Compare two (non-convertable) UNICODE values.
		// Check the obvious case of equal UNICODE values.
		
		if( ui16LeftUniChar == ui16RightUniChar)
		{
			goto Exit;
		}

		// Compare subcollation or compare value?
		
		if( iCompareType != COMPARE_COLLATION) 
		{
			iCompare = -1;
			goto Exit;
		}

		// For non-asian - we store these values in the sub-collcation area.
		// We should return the differece in sub-collation values - but this 
		// may not work for all compares.
		//
		// For asian compares, most values we have a collation value.  
		// This is a BIG differece in comparing asian values.
		//
		// If we want sub-collation compare then set it, otherwise set main
		// iCompare value.

		if( piSubColCompare )
		{
			if( *piSubColCompare == 0)
			{
				*piSubColCompare = ui16LeftUniChar < ui16RightUniChar 
													? -1 
													: 1;
			}
		}
		else
		{
			// Treat as the collation value - this is different than the index.

			iCompare = ui16LeftUniChar < ui16RightUniChar 
													? -1 
													: 1;
		}
		
		goto Exit;
	}

	// Compare subcollation or compare value?
	
	if( iCompareType != COMPARE_COLLATION) 
	{
		iCompare = -1;
		goto Exit;
	}

	// Check for no left character.
	
	if( !ui16LeftWPChar && !ui16LeftUniChar)
	{
		// No left character.  check if no right character.

		if( ui16RightWPChar || ui16RightUniChar)
		{
			iCompare = -1;
		}
	}

	// Check for no right character.
	
	else if( !ui16RightWPChar && !ui16RightUniChar)
	{
		iCompare = 1;
	}

	// What remains is one WP char and one Unicode char.
	// Remember the sub-collation comment above.  Some WP char may not
	// have a collation value (COLS0) so in US sort these values may be
	// equal and have different sub-collation values.  YECH!!!!
	//
	// The unicode value will always have collation value of COLS0 (0xFF)
	// and subcollation value of 11110 [unicodeValue]
	// The WP value could be anything & if collation value is COLS0 will
	// have a subcollation value os 1110 [WPValue]
	//
	// So, we have to check to see of the WP collation value is COLS0.  
	// If not iCompare is used.  If both represent high collation then
	// the WP value will always have a lower sub-collation value.
	// 
	// The (not so obvious) code would be to code up...
	// iCompare = ui16LeftWPChar ? -1 : 1;
	// if we didn't care about sub-collation (and we may not care).
	//
	// This is easier to over code than have ?: operators for the two cases.

	else if( ui16LeftWPChar)
	{
		// Remember - unicode subcol is always COLS0.

		if( flmGetCollation( ui16LeftWPChar, uiLangId) == COLS0)
		{
			if( piSubColCompare && (*piSubColCompare == 0))
			{
				*piSubColCompare = -1;
			}
		}
		else
		{
			iCompare = -1;
		}
	}
	else
	{
		// left=unicode, right=WP
		// Remember - unicode subcol is always COLS0 for non-asian.

		if( flmGetCollation( ui16RightWPChar, uiLangId) == COLS0)
		{
			if( piSubColCompare && (*piSubColCompare == 0))
			{
				*piSubColCompare = 1;
			}
		}
		else
		{
			iCompare = 1;
		}
	}
	
Exit:

	if( !iCompare)
	{
		// Position to the next values if equal

		*puiLeftLen -= uiLeftValueLen;
		*ppLeftText  = pLeftText + uiLeftValueLen;
		*puiLeftWpChar2 = uiLeftWpChar2;
		*puiRightLen -= uiRightValueLen;
		*ppRightText = pRightText + uiRightValueLen;
		*puiRightWpChar2 = uiRightWpChar2;
	}
	
	return( iCompare);
}

/**************************************************************************
Desc:		Get the Flaim collating string and convert back to a text string
Ret: 		Length of new wpStr
Notes:	Allocates the area for the word string buffer if will be over 256.
***************************************************************************/
FLMUINT FColStrToText(
	FLMBYTE *	fColStr,				// Points to the Flaim collated string
	FLMUINT *	fcStrLenRV,			// Length of the Flaim collated string
	FLMBYTE *	textStr,				// Output string to build - TEXT string
	FLMUINT		fWPLang,				// FLAIM WP language number
	FLMBYTE *	postBuf,				// Lower/upper POST buffer or NULL
	FLMUINT *	postBytesRV,		// Return next position to use in postBuf
	FLMBOOL *	pbDataTruncated,	// Sets to TRUE if data had been truncated
	FLMBOOL *	pbFirstSubstring) // Sets to TRUE if first substring
{
	#define LOCAL_CHARS	150

	FLMBYTE		wordStr[LOCAL_CHARS * 2 + LOCAL_CHARS / 5];	// Sample + 20%
	FLMBYTE *	wsPtr = NULL;
	FLMBYTE *	wsAllocatedWsPtr = NULL;
	FLMUINT		wsLen;
	FLMUINT		textLen;
	FLMBYTE *	textPtr;

	if (*fcStrLenRV > LOCAL_CHARS)					// If won't fit allocate 1280
	{
		if (RC_BAD( f_alloc( MAX_KEY_SIZ * 2, &wsPtr)))
		{
			return (0);
		}

		wsAllocatedWsPtr = wsPtr;
	}
	else
	{
		wsPtr = wordStr;
	}

	if ((fWPLang >= FLM_FIRST_DBCS_LANG) && (fWPLang <= FLM_LAST_DBCS_LANG))
	{
		wsLen = AsiaConvertColStr( fColStr, fcStrLenRV, wsPtr, pbDataTruncated,
										  pbFirstSubstring);
		if (postBuf)
		{
			FLMUINT	postBytes = *postBytesRV + 2; // Skip past marker

			// may change wsLen

			postBytes += AsiaParseCase( wsPtr, &wsLen, &postBuf[postBytes]);
			*postBytesRV = postBytes;
		}
	}
	else
	{
		wsLen = FWWSGetColStr( fColStr, fcStrLenRV, wsPtr, fWPLang,
									 pbDataTruncated, pbFirstSubstring);

		// If a post buffer is sent - turn unflagged chars to lower case

		if (postBuf)
		{
			FLMUINT	postBytes = *postBytesRV;

			// Check if mixed case chars follow and always increment
			// postBytes
			//

			if (postBuf[postBytes++] == (COLL_MARKER | SC_MIXED))
			{
				postBytes += FWWSToMixed( wsPtr, wsLen, &postBuf[postBytes], fWPLang);
			}

			*postBytesRV = postBytes;
		}
	}

	// Copy word string to TEXT string area

	wsLen >>= 1;							// Convert # of bytes to # of words
	textPtr = textStr;

	while (wsLen--)
	{
		register FLMBYTE	ch;

		register FLMBYTE	cSet;

		// Put the character in a local variable for speed

		ch = *wsPtr++;
		cSet = *wsPtr++;

		if ((!cSet) && (ch <= 127))
		{

			// Character set zero only needs one byte if the character is <=
			// 127. Otherwise, it is handled like all other extended
			// characters below.
			//

			*textPtr++ = ch;
		}

		// If the character set is > 63 it takes three bytes to store,
		// otherwise only two bytes are needed.

		else if (cSet < 63)
		{
			*textPtr++ = (FLMBYTE) (CHAR_SET_CODE | cSet);
			*textPtr++ = ch;
		}
		else if (cSet == 0xFF && ch == 0xFF)
		{
			*textPtr++ = UNICODE_CODE;
			*textPtr++ = *(wsPtr + 1); // Character set
			*textPtr++ = *wsPtr;			// Character
			wsPtr += 2;
			wsLen--;							// Skip past 4 bytes for UNICODE
		}
		else
		{
			*textPtr++ = EXT_CHAR_CODE;
			*textPtr++ = cSet;
			*textPtr++ = ch;
		}
	}

	textLen = (textPtr - textStr);	// Compute total length

	if (wsAllocatedWsPtr != NULL)
	{
		f_free( &wsAllocatedWsPtr);
	}

	return (textLen);
}

/****************************************************************************
Desc:  	Compare two entire strings.  There is some debate how this routine
			should compare the sub-collation values when wild cards are used.
			THIS DOES NOT ALLOW WILD CARDS.
Return:	Signed value of compare.  
			<0 if less than, 0 if equal, >0 if greater than
			The case of returning 1 may be in using wild cards which
			only need to return a does not match value.
****************************************************************************/
FLMINT flmTextCompare(										
	FLMBYTE *	pLeftBuf,
	FLMUINT		uiLeftLen,
	FLMBYTE *	pRightBuf,
	FLMUINT		uiRightLen,
	FLMUINT		uiFlags,
	FLMUINT		uiLang)
{
	FLMINT		iCompare = 0;
	FLMINT		iSubColCompare = 0;
	FLMINT *		pSubColCompare;
	FLMINT		iCaseCompare = 0;
	FLMINT *		pCaseCompare;
	FLMUINT		uiLeadingSpace;
	FLMUINT		uiTrailingSpace;
	FLMUINT16	ui16ColVal = 0;
	FLMUINT16	ui16WPChar;
	FLMUINT16	ui16UniChar;
	FLMUINT		uiLeftWpChar2 = 0;
	FLMUINT		uiRightWpChar2 = 0;
	
	uiTrailingSpace = uiLeadingSpace = 
		(uiFlags & FLM_COMP_COMPRESS_WHITESPACE) ? FLM_COMP_NO_WHITESPACE : 0;
	pCaseCompare = (uiFlags & FLM_COMP_CASE_INSENSITIVE) ? NULL : &iCaseCompare;
	pSubColCompare = &iSubColCompare;

	// Handle NULL buffers first.

	if (!pLeftBuf)
	{
		if (pRightBuf)
		{
			iCompare = -1;
		}
		goto Exit;
	}

	while ((uiLeftLen || uiLeftWpChar2) &&
			 (uiRightLen || uiRightWpChar2))
	{
		if ((iCompare = flmTextCompareSingleChar(
								&pLeftBuf, &uiLeftLen, &uiLeftWpChar2,
								&pRightBuf, &uiRightLen, &uiRightWpChar2,
								pSubColCompare, pCaseCompare, NULL_WILD_CARD_CHECK,
								COMPARE_COLLATION, &ui16ColVal, 
								uiFlags | uiLeadingSpace, uiLang)) != 0)
		{
			goto Exit;
		}
		uiLeadingSpace = 0;
	}

	// EQUAL - as far as the collation values are concerned and one
	// or both of the strings is at the end.

	if (uiLeftLen || uiLeftWpChar2)
	{
		uiLeftLen -= flmTextGetValue( pLeftBuf, uiLeftLen, &uiLeftWpChar2,
								uiFlags | uiTrailingSpace, &ui16WPChar, &ui16UniChar);

		if (uiLeftLen || ui16WPChar || ui16UniChar)
		{
			iCompare = 1;
		}
	}
	else if (uiRightLen || uiRightWpChar2)
	{
		uiRightLen -= flmTextGetValue( pRightBuf, uiRightLen, &uiRightWpChar2,
			uiFlags | uiTrailingSpace, &ui16WPChar, &ui16UniChar);
		if (uiRightLen || ui16WPChar || ui16UniChar)
		{
			iCompare = -1;
		}
	}
	if (iCompare == 0)
	{

		// All collation bytes equal - return subcollation/case difference.

		iCompare = (iSubColCompare != 0) ? iSubColCompare : iCaseCompare;
	}

Exit:

	return iCompare;
}

/****************************************************************************
Desc:  	Match two entire strings.  
Return:	FLM_TRUE or FLM_FALSE
Notes:	This code calls the collation routine because in the future there
			will be equal conditions with different unicode characters.

DOCUMENTATION DEALING WITH WILD CARDS AND SPACE RULES.

	The space rules are not obvious when dealing with wild cards.  
	This will outline the rules that are being applied so that we can
	do a regression test when this code changes.

	Rule #1:	Return same result if leading or trailing wild card is added.
				The underscore is also the space character in these examples
				and the MIN_SPACES rule is being applied.

	Format:	DataString Operator SearchString

	Example:	if     A == A      A_ == A      A == A_     A_ == A_  
				then   A == A*     A_ == A*     A == A_*    A_ == A_*
				and    A == *A     A_ == *A     A == *A_    A_ == *A_
				and    A == *A*    A_ == *A*    A == *A_*   A_ == *A_*
				where 'A' represent a string of any characters.

	Strictly put, the query Field == A_* can be broken down to
		Field == A || Field == A_*
	where the space after 'A' should not be treated as a trailing space.

	In addition we can apply the space before the string with the same results,
	but we are not going to handle the case of *_A correctly.
	This is because the query *_A should be expanded to 
		Field == A || Field == *_A
	where the space before 'A' should not be treated as a leading space.
	When we need to find "_A" in a search string then we will expand the
	query to handle this.


	Rule #2:	The spaces before a trailing truncation are NOT to be treated
				as trailing spaces if there are remaining bytes in the data string.

	Example:	(A_B == A_*) but (AB != A_*)


	Rule #3:	Space value(s) without anything other value are equal to no values.
	Example:	(" " == "")


	Rule #4: Trim leading/trailing spaces before and after wild cards. 
				SMI does this when formatting.

		_* and *_ same as *			so A == _* and A = *_ but A != *_*


	Additional wildcard cases to test for:

	Wildcard cases to handle.
		(ABBBBC == A*BC)			Hits the goto Compare_Again case three times.
		(ABBBBD != A*B)			Stuff still remains in dataString
		(ABBBBC != A*BCD)			Stuff still remains in searchString

****************************************************************************/
FLMUINT flmTextMatch(										
	FLMBYTE *	pLeftBuf,
	FLMUINT		uiLeftLen,
	FLMBYTE *	pRightBuf,
	FLMUINT		uiRightLen,
	FLMUINT		uiFlags,
	FLMBOOL		bLeadingWildCard,
	FLMBOOL		bTrailingWildCard,
	FLMUINT		uiLang)
{
	FLMINT		iCompare = 0;
	FLMUINT		uiLeadingSpace;
	FLMUINT		uiTrailingSpace;
	FLMBOOL		bHitWildCard;
	FLMBOOL		bHasWildCardPos;
	FLMBOOL *	pbHitWildCard;
	FLMUINT		uiValueLen;
	FLMUINT16	ui16WPChar;
	FLMUINT16	ui16UniChar;
	FLMUINT16	ui16Tmp1;
	FLMUINT16	ui16Tmp2;
	FLMINT		iCompareType;
	FLMUINT		uiLeftWpChar2 = 0;
	FLMUINT		uiRightWpChar2 = 0;
	// LWCP = Last Wild Card Position - used for wild card state
	FLMBYTE *	pLWCPLeftBuf = NULL;
	FLMBYTE *	pLWCPRightBuf = NULL;
	FLMUINT		uiLWCPLeftLen = 0;
	FLMUINT		uiLWCPRightLen = 0;
	FLMUINT		uiLWCPLeftWpChar2 = 0;
	FLMUINT		uiLWCPRightWpChar2 = 0;

	if( uiFlags & FLM_COMPARE_COLLATED_VALUES)
	{
		iCompareType = COMPARE_COLLATION;
	}
	else
	{
		iCompareType = (uiFlags & FLM_COMP_CASE_INSENSITIVE) 
								? COMPARE_COL_AND_SUBCOL : COMPARE_VALUE;
	}

	// Handle NULL buffers first - don't test for zero length values yet.

	if (!pLeftBuf)
	{
		if (pRightBuf)
		{
			iCompare = -1;
		}
		goto Exit;
	}

	bHitWildCard = bHasWildCardPos = FALSE;
	uiLeadingSpace = uiTrailingSpace = 
		(uiFlags & FLM_COMP_COMPRESS_WHITESPACE) ? FLM_COMP_NO_WHITESPACE : 0;
	pbHitWildCard = (uiFlags & FLM_COMP_WILD) ? &bHitWildCard : NULL;

	if (bLeadingWildCard)
	{
		goto Leading_Wild_Card;
	}

	while (!iCompare &&
			(uiLeftLen || uiLeftWpChar2) &&
			(uiRightLen || uiRightWpChar2))
	{
		iCompare = flmTextCompareSingleChar(
								&pLeftBuf, &uiLeftLen, &uiLeftWpChar2,
								&pRightBuf, &uiRightLen, &uiRightWpChar2,
								NULL_SUB_COL_CHECK, NULL_CASE_CHECK, pbHitWildCard, 
								iCompareType, NULL, 
								uiFlags | uiLeadingSpace, uiLang);

		uiLeadingSpace = 0;
		if (bHitWildCard)
		{
			
Leading_Wild_Card:

			bHitWildCard = FALSE;
			bHasWildCardPos = FALSE;		// Turn off last wildcard.

			// If right side is done, we are done.

			if (!uiRightLen && !uiRightWpChar2)
			{
				uiLeftLen = 0;
				uiLeftWpChar2 = 0;
				break;
			}

			// Save state on the RIGHT to handle the sick case of search key 
			// "b*aH" being able to match "baaaaaaaaaH" (Lambda Case)
			// LWCP = LastWildCardPosition

			pLWCPRightBuf = pRightBuf;
			uiLWCPRightLen = uiRightLen;
			uiLWCPRightWpChar2 = uiRightWpChar2;

			// Find first matching character on the left side.

Compare_Again:

			iCompare = -1;
			while (iCompare && (uiLeftLen || uiLeftWpChar2))
			{
				iCompare = flmTextCompareSingleChar(
								&pLeftBuf, &uiLeftLen, &uiLeftWpChar2,
								&pRightBuf, &uiRightLen, &uiRightWpChar2,
								NULL_SUB_COL_CHECK, NULL_CASE_CHECK, NULL_WILD_CARD_CHECK,
								iCompareType, NULL, 
								uiFlags | uiLeadingSpace, uiLang);
				
				uiLeadingSpace = 0;

				// Done with the right side?  Return iCompare value.

				if (!uiRightLen && !uiRightWpChar2)
				{
					break;
				}

				// Values different and still have stuff on left?

				if (iCompare && (uiLeftLen || uiLeftWpChar2))
				{
					// Advance the left if there is anything left
					uiValueLen = flmTextGetValue( pLeftBuf, uiLeftLen,
													&uiLeftWpChar2,
													uiFlags, &ui16Tmp1, &ui16Tmp2);
					pLeftBuf += uiValueLen;
					uiLeftLen -= uiValueLen;
				}
			}

			// Save state on the LEFT 

			if (uiLeftLen || uiLeftWpChar2)
			{
				pLWCPLeftBuf = pLeftBuf;
				uiLWCPLeftLen = uiLeftLen;
				uiLWCPLeftWpChar2 = uiLeftWpChar2;
				bHasWildCardPos = TRUE;
			}

			// EQUAL - as far as the collation values are concerned.
		}
	}

	if (iCompare == 0)
	{
		// In here because LEFT and/or RIGHT are out of bytes.
		// Check for trailing spaces if MIN_SPACES.

		if (uiLeftLen || uiLeftWpChar2)
		{
			if (!bTrailingWildCard)
			{
				uiLeftLen -= flmTextGetValue( pLeftBuf, uiLeftLen,
										&uiLeftWpChar2,
										uiFlags | uiTrailingSpace, &ui16WPChar,
										&ui16UniChar);
				
				if (uiLeftLen || ui16WPChar || ui16UniChar)
				{
					iCompare = 1;
				}
			}
		}
		else if (uiRightLen || uiRightWpChar2)
		{
			uiRightLen -= flmTextGetValue( pRightBuf, uiRightLen, &uiRightWpChar2,
									uiFlags | uiTrailingSpace, &ui16WPChar, &ui16UniChar);

			// Equals if right just had a trailing wild card. (else case)

			if (uiRightLen || !pbHitWildCard || ui16WPChar != '*')
			{				
				if (uiRightLen || ui16WPChar || ui16UniChar)
				{
					iCompare = -1;
				}
			}
		}
	}

	// Handle the embedded wild card case.

	if (iCompare != 0 && bHasWildCardPos)
	{

		// Restore wild card state.

		pLeftBuf = pLWCPLeftBuf;
		uiLeftLen = uiLWCPLeftLen;
		uiLeftWpChar2 = uiLWCPLeftWpChar2;
		pRightBuf = pLWCPRightBuf;
		uiRightLen = uiLWCPRightLen;
		uiRightWpChar2 = uiLWCPRightWpChar2;
		bHasWildCardPos = FALSE;

		goto Compare_Again;
	}

Exit:

	return (!iCompare ? FLM_TRUE : FLM_FALSE);
}

/****************************************************************************
Desc:		Check for double characters that sort as 1 (like ch in Spanish) or
			1 character that should sort as 2 (like � sorts as ae in French).
Return:	0 = nothing changes.  Otherwise, *pui16WpChar is the first
			character, and the return value contains the 2nd character.
			In addition, *pbTwoIntoOne will be TRUE if we should take two
			characters and treat as one (i.e, change the collation on the
			outside to one more than the collation of the first character).
****************************************************************************/
FLMUINT16 flmCheckDoubleCollation(
	FLMUINT16 *			pui16WpChar,
	FLMBOOL *			pbTwoIntoOne,
	const FLMBYTE **	ppucInputStr,
	FLMUINT				uiLanguage)
{
	FLMUINT16	ui16CurState;
	FLMUINT16	ui16WpChar;
	FLMUINT16	ui16SecondChar;
	FLMUINT16	ui16LastChar = 0;
	FLMUINT		uiInLen;
	FLMBOOL		bUpperFlag;

	ui16WpChar = *pui16WpChar;
	bUpperFlag = flmIsUpper( ui16WpChar);

	uiInLen = 0;
	ui16SecondChar = 0;

	// Primer read

	if ((ui16CurState = getNextCharState( 0, uiLanguage)) == 0)
	{
		goto Exit;
	}

	for (;;)
	{
		switch (ui16CurState)
		{
			case INSTSG:
			{
				*pui16WpChar = ui16SecondChar = (FLMUINT16) f_toascii( 's');
				*pbTwoIntoOne = FALSE;
				goto Exit;
			}
			
			case INSTAE:
			{
				if (bUpperFlag)
				{
					*pui16WpChar = (FLMUINT16) f_toascii( 'A');
					ui16SecondChar = (FLMUINT16) f_toascii( 'E');
				}
				else
				{
					*pui16WpChar = (FLMUINT16) f_toascii( 'a');
					ui16SecondChar = (FLMUINT16) f_toascii( 'e');
				}

				*pbTwoIntoOne = FALSE;
				goto Exit;
			}
			
			case INSTIJ:
			{
				if (bUpperFlag)
				{
					*pui16WpChar = (FLMUINT16) f_toascii( 'I');
					ui16SecondChar = (FLMUINT16) f_toascii( 'J');
				}
				else
				{
					*pui16WpChar = (FLMUINT16) f_toascii( 'i');
					ui16SecondChar = (FLMUINT16) f_toascii( 'j');
				}

				*pbTwoIntoOne = FALSE;
				goto Exit;
			}
			
			case INSTOE:
			{
				if (bUpperFlag)
				{
					*pui16WpChar = (FLMUINT16) f_toascii( 'O');
					ui16SecondChar = (FLMUINT16) f_toascii( 'E');
				}
				else
				{
					*pui16WpChar = (FLMUINT16) f_toascii( 'o');
					ui16SecondChar = (FLMUINT16) f_toascii( 'e');
				}

				*pbTwoIntoOne = FALSE;
				goto Exit;
			}
			
			case WITHAA:
			{
				*pui16WpChar = (FLMUINT16) (bUpperFlag 
														? (FLMUINT16) 0x122 
														: (FLMUINT16) 0x123);
				(*ppucInputStr)++;
				break;
			}
			
			case AFTERC:
			{
				*pui16WpChar = (FLMUINT16) (bUpperFlag 
														? (FLMUINT16) f_toascii( 'C') 
														: (FLMUINT16) f_toascii( 'c'));
				ui16SecondChar = ui16LastChar;
				*pbTwoIntoOne = TRUE;
				(*ppucInputStr)++;
				goto Exit;
			}
			
			case AFTERH:
			{
				*pui16WpChar = (FLMUINT16) (bUpperFlag 
														? (FLMUINT16) f_toascii( 'H') 
														: (FLMUINT16) f_toascii( 'h'));
				ui16SecondChar = ui16LastChar;
				*pbTwoIntoOne = TRUE;
				(*ppucInputStr)++;
				goto Exit;
			}
			
			case AFTERL:
			{
				*pui16WpChar = (FLMUINT16) (bUpperFlag 
														? (FLMUINT16) f_toascii( 'L') 
														: (FLMUINT16) f_toascii( 'l'));
				ui16SecondChar = ui16LastChar;
				*pbTwoIntoOne = TRUE;
				(*ppucInputStr)++;
				goto Exit;
			}
			
			default:
			{

				// Handles STATE1 through STATE11 also

				break;
			}
		}

		if ((ui16CurState = getNextCharState( ui16CurState, 
				flmCh6Lower( ui16WpChar))) == 0)
		{
			goto Exit;
		}

		ui16LastChar = ui16WpChar;
		ui16WpChar = (FLMUINT16) * ((*ppucInputStr) + (uiInLen++));
	}

Exit:

	return (ui16SecondChar);
}

/****************************************************************************
Desc:	Convert a WPChar from hankaku (single wide) to zenkaku (double wide).
Ret:	0 = no conversion
		1 = converted character to zenkaku
		2 = ui16NextWpChar dakuten or handakuten voicing got combined 
****************************************************************************/
FLMUINT16 HanToZenkaku(
	FLMUINT16		ui16WpChar,
	FLMUINT16		ui16NextWpChar,
	FLMUINT16 *		pui16Zenkaku)
{
	FLMUINT16		ui16Zenkaku = 0;
	FLMBYTE			ucCharSet = ui16WpChar >> 8;
	FLMBYTE			ucCharVal = ui16WpChar & 0xFF;
	FLMUINT			uiLoop;
	FLMUINT16		ui16CharsUsed = 1;

	switch (ucCharSet)
	{

		// Character set 0 - symbols

		case 0:
		{

			// Invalid? - all others are used.

			if (ucCharVal < 0x20)
			{
				;
			}
			else if (ucCharVal <= 0x2F)
			{

				// Symbols A

				ui16Zenkaku = 0x2400 + From0AToZen[ucCharVal - 0x20];
			}
			else if (ucCharVal <= 0x39)
			{

				// 0..9

				ui16Zenkaku = 0x2500 + (ucCharVal - 0x21);
			}
			else if (ucCharVal <= 0x40)
			{

				// Symbols B

				ui16Zenkaku = 0x2400 + From0BToZen[ucCharVal - 0x3A];
			}
			else if (ucCharVal <= 0x5A)
			{

				// A..Z

				ui16Zenkaku = 0x2500 + (ucCharVal - 0x21);
			}
			else if (ucCharVal <= 0x60)
			{

				// Symbols C

				ui16Zenkaku = 0x2400 + From0CToZen[ucCharVal - 0x5B];
			}
			else if (ucCharVal <= 0x7A)
			{

				// a..z

				ui16Zenkaku = 0x2500 + (ucCharVal - 0x21);
			}
			else if (ucCharVal <= 0x7E)
			{

				// Symbols D

				ui16Zenkaku = 0x2400 + From0DToZen[ucCharVal - 0x7B];
			}
			break;
		}

		// GREEK

		case 8:
		{
			if ((ucCharVal >= sizeof(From8ToZen)) ||
				 ((ui16Zenkaku = 0x2600 + From8ToZen[ucCharVal]) == 0x26FF))
			{
				ui16Zenkaku = 0;
			}
			break;
		}

		// CYRILLIC

		case 10:
		{

			// Check range

			ui16Zenkaku = 0x2700 + (ucCharVal >> 1);	// Uppercase value

			// Convert to lower case?

			if (ucCharVal & 0x01)
			{
				ui16Zenkaku += 0x30;
			}
			break;
		}

		// JAPANESE

		case 11:
		{
			if (ucCharVal < 5)
			{
				ui16Zenkaku = 0x2400 + From11AToZen[ucCharVal];
			}
			else if (ucCharVal < 0x3D)						// katakana?
			{
				if ((ui16Zenkaku = 0x2600 + From11BToZen[ucCharVal - 5]) == 0x26FF)
				{

					// Dash - convert to this

					ui16Zenkaku = 0x241b;
				}
				else
				{
					if (ui16NextWpChar == 0xB3D)			// dakuten? - voicing
					{

						// First check exception(s) then check if voicing
						// exists!

						if ((ui16Zenkaku != 0x2652) &&	// is not 'N'?
							 (flmKanaSubColTbl[ui16Zenkaku - 0x2600 + 1] == 3))
						{
							ui16Zenkaku++;

							// Return 2

							ui16CharsUsed++;
						}
					}
					else if (ui16NextWpChar == 0xB3E)	// handakuten? - voicing
					{

						// Check if voicing exists! - will NOT access out of
						// table

						if (flmKanaSubColTbl[ui16Zenkaku - 0x2600 + 2] == 5)
						{
							ui16Zenkaku += 2;

							// Return 2

							ui16CharsUsed++;
						}
					}
				}
			}
			else if (ucCharVal == 0x3D)					// dakuten?
			{

				// Convert to voicing symbol

				ui16Zenkaku = 0x240A;
			}
			else if (ucCharVal == 0x3E)					// handakuten?
			{

				// Convert to voicing symbol

				ui16Zenkaku = 0x240B;
			}

			// else cannot convert

			break;
		}

		// Other character sets CS 1,4,5,6 - symbols

		default:
		{

			// Instead of includes more tables from char.asm - look down the
			// Zen24Tohankaku[] table for a matching value - not much slower.

			for (uiLoop = 0;
				  uiLoop < (sizeof(Zen24ToHankaku) / sizeof(BYTE_WORD_TBL));
				  uiLoop++)
			{
				if (Zen24ToHankaku[uiLoop].WordValue == ui16WpChar)
				{
					ui16Zenkaku = 0x2400 + Zen24ToHankaku[uiLoop].ByteValue;
					break;
				}
			}
			break;
		}
	}

	if (!ui16Zenkaku)
	{

		// Change return value

		ui16CharsUsed = 0;
	}

	*pui16Zenkaku = ui16Zenkaku;
	return (ui16CharsUsed);
}

/****************************************************************************
Desc:	Convert a zenkaku (double wide) char to a hankaku (single wide) char
Ret:	Hankaku char or 0 if a conversion doesn't exist
****************************************************************************/
FLMUINT16 ZenToHankaku(
	FLMUINT16		ui16WpChar,
	FLMUINT16 *		DakutenOrHandakutenRV)
{
	FLMUINT16		ui16Hankaku = 0;
	FLMBYTE			ucCharSet = ui16WpChar >> 8;
	FLMBYTE			ucCharVal = ui16WpChar & 0xFF;
	FLMUINT			uiLoop;

	switch (ucCharSet)
	{

		// SYMBOLS

		case 0x24:
		{
			for (uiLoop = 0;
				  uiLoop < (sizeof(Zen24ToHankaku) / sizeof(BYTE_WORD_TBL));
				  uiLoop++)
			{

				// List is sorted so table entry is more you are done

				if (Zen24ToHankaku[uiLoop].ByteValue >= ucCharVal)
				{
					if (Zen24ToHankaku[uiLoop].ByteValue == ucCharVal)
					{
						ui16Hankaku = Zen24ToHankaku[uiLoop].WordValue;
					}
					break;
				}
			}
			break;
		}

		// ROMAN - 0x250F..2559 Hiragana - 0x255E..2580

		case 0x25:
		{
			if (ucCharVal >= 0x0F && ucCharVal < 0x5E)
			{
				ui16Hankaku = ucCharVal + 0x21;
			}
			break;
		}

		// Katakana - 0x2600..2655 Greek - 0x265B..2695

		case 0x26:
		{
			if (ucCharVal <= 0x55)			// Katakana range
			{
				FLMBYTE		ucCS11CharVal;
				FLMUINT16	ui16NextWpChar = 0;

				if ((ucCS11CharVal = MapCS26ToCharSet11[ucCharVal]) != 0xFF)
				{
					if (ucCS11CharVal & 0x80)
					{
						if (ucCS11CharVal & 0x40)
						{

							// Handakuten voicing

							ui16NextWpChar = 0xB3E;
						}
						else
						{

							// Dakuten voicing

							ui16NextWpChar = 0xB3D;
						}

						ucCS11CharVal &= 0x3F;
					}

					ui16Hankaku = 0x0b00 + ucCS11CharVal;
					if (ui16NextWpChar && DakutenOrHandakutenRV)
					{
						*DakutenOrHandakutenRV = ui16NextWpChar;
					}
				}
			}
			else if (ucCharVal <= 0x95)	// Greek
			{
				FLMBYTE	ucGreekChar = ucCharVal;

				// Make a zero based number.

				ucGreekChar -= 0x5E;

				// Check for lowercase

				if (ucGreekChar >= 0x20)
				{

					// Convert to upper case for now

					ucGreekChar -= 0x20;
				}

				if (ucGreekChar >= 2)
				{
					ucGreekChar++;
				}

				if (ucGreekChar >= 19)
				{
					ucGreekChar++;
				}

				// Convert to character set 8

				ui16Hankaku = (ucGreekChar << 1) + 0x800;
				if (ucCharVal >= (0x5E + 0x20))
				{

					// Adjust to lower case character

					ui16Hankaku++;
				}
			}
			break;
		}

		// Cyrillic

		case 0x27:
		{

			// Uppercase?

			if (ucCharVal <= 0x20)
			{
				ui16Hankaku = (ucCharVal << 1) + 0xa00;
			}
			else if (ucCharVal >= 0x30 && ucCharVal <= 0x50)
			{

				// Lower case

				ui16Hankaku = ((ucCharVal - 0x30) << 1) + 0xa01;
			}
			break;
		}
	}

	return (ui16Hankaku);
}
