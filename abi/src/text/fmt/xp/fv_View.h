/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#ifndef FV_VIEW_H
#define FV_VIEW_H

#include "xap_Features.h"

//#include "ut_misc.h"
#include "ut_types.h"

#include "xav_View.h"

#include "pt_Types.h"
#include "ie_types.h"
#include "ap_types.h"
#include "fp_types.h"
#include "fl_Squiggles.h"
#include "ev_EditBits.h"


// have to include these as they are instantiated in the FV_View
// class definition
#include "fv_FrameEdit.h"
#include "fv_VisualDragText.h"
#include "fv_Selection.h"
#include "fv_InlineImage.h"

#define AUTO_SCROLL_MSECS	100

class FL_DocLayout;
class FV_Caret_Listener;

class fl_DocSectionLayout;
class fl_HdrFtrSectionLayout;
class fl_DocListener;
class fl_BlockLayout;
class fl_PartOfBlock;
class fl_AutoNum;
class fl_EndnoteLayout;

class fp_PageSize;
class fp_Page;
class fp_Run;
class fp_HyperlinkRun;
class fp_CellContainer;

class FG_Graphic;

class PD_Document;
class PP_AttrProp;
class PP_RevisionAttr;

class GR_Graphics;
struct dg_DrawArgs;

class UT_Worker;
class UT_Timer;
class UT_UTF8String;

class AP_TopRulerInfo;
class AP_LeftRulerInfo;
class AP_TopRuler;
class AP_LeftRuler;

class AP_Dialog_SplitCells;

class XAP_App;
class XAP_Prefs;

class SpellChecker;

class CellLine;

struct fv_ChangeState
{
	bool				bUndo;
	bool				bRedo;
	bool				bDirty;
	bool				bSelection;
	UT_uint32			iColumn;
	const XML_Char **	propsChar;
	const XML_Char **	propsBlock;
	const XML_Char **	propsSection;
};

struct FV_DocCount
{
	UT_uint32 word;
	UT_uint32 para;
	UT_uint32 ch_no;
	UT_uint32 ch_sp;
	UT_uint32 line;
	UT_uint32 page;
       // sometimes people want to have a word count without header/footers included
	UT_uint32 words_no_hdrftr;
};

class ABI_EXPORT fv_PropCache
{
public:
	fv_PropCache(void);
	~fv_PropCache(void);
	UT_uint32         getTick(void) const;
	void              setTick(UT_uint32 iTick);
	bool              isValid(void) const;
	const XML_Char ** getCopyOfProps(void) const;
	void              fillProps(UT_uint32 numProps, const XML_Char ** props);
	void              clearProps(void);
private:
	UT_uint32         m_iTick;
	UT_uint32         m_iNumProps;
	XML_Char **       m_pszProps;
};

enum FV_BIDI_Order
{
	FV_Order_Visual = 0,
	FV_Order_Logical_LTR = UT_BIDI_LTR,
	FV_Order_Logical_RTL = UT_BIDI_RTL
};

								
class ABI_EXPORT FV_View : public AV_View
{
	friend class fl_DocListener;
	friend class fl_BlockLayout;
	friend class FL_DocLayout;
	friend class fl_Squiggles;
	friend class fl_DocSectionLayout;
	friend class GR_Caret;
	friend class FV_FrameEdit;
	friend class FV_VisualDragText;
	friend class FV_VisualInlineImage;
	friend class FV_Selection;
	friend class CellLine;

public:
	FV_View(XAP_App*, void*, FL_DocLayout*);
	virtual ~FV_View();

	virtual inline GR_Graphics*    getGraphics(void) const { return m_pG; }
	void  setGraphics(GR_Graphics *pG);
	void  replaceGraphics(GR_Graphics *pG);
	
	virtual inline PT_DocPosition   getPoint(void) const { return m_iInsPoint; }
	PT_DocPosition	getSelectionAnchor(void) const;
	UT_uint32       getSelectionLength(void) const;

	virtual void focusChange(AV_Focus focus);

	virtual void	setXScrollOffset(UT_sint32);
	virtual void	setYScrollOffset(UT_sint32);
	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0);

	virtual void	cmdHyperlinkJump(UT_sint32 xPos, UT_sint32 yPos);
	void	        cmdHyperlinkJump(PT_DocPosition pos);
	void			cmdHyperlinkCopyLocation(PT_DocPosition pos);

	virtual void	draw(const UT_Rect* pRect=static_cast<UT_Rect*>(NULL));
	virtual void 	drawSelectionBox(UT_Rect & box, bool drawHandles);
private: 
	inline void 	_drawResizeHandle(UT_Rect & box);
	
public:
	const PP_AttrProp * getAttrPropForPoint();

	virtual bool	notifyListeners(const AV_ChangeMask hint);

	virtual bool	canDo(bool bUndo) const;
	virtual UT_uint32 undoCount (bool bUndo) const;
	virtual void	cmdUndo(UT_uint32 count);
	virtual void	cmdRedo(UT_uint32 count);
	virtual UT_Error	cmdSave(void);
	virtual UT_Error	cmdSaveAs(const char * szFilename, int ieft);
	virtual UT_Error		cmdSaveAs(const char * szFilename, int ieft, bool cpy);

	UT_Error		cmdInsertField(const char* szName, const XML_Char ** extra_attrs = NULL, const XML_Char ** extra_props = NULL);
	UT_Error		cmdInsertBookmark(const char* szName);
	UT_Error		cmdDeleteBookmark(const char* szName);
	UT_Error		cmdInsertHyperlink(const char* szName);
	fp_Run *        getHyperLinkRun(PT_DocPosition pos);
	UT_Error		cmdDeleteHyperlink();
	bool                    cmdInsertMathML(const char * szFileName,
						PT_DocPosition pos);
	bool	        cmdInsertEmbed(UT_ByteBuf * pBuf,PT_DocPosition pos,const char * szMime,const char * szProps);
	bool            cmdUpdateEmbed(UT_ByteBuf * pBuf, const char * szMime, const char * szProps);
	bool	        cmdUpdateEmbed(fp_Run * pRun, UT_ByteBuf * pBuf, const char * szMime, const char * szProps);
	bool	        cmdDeleteEmbed(fp_Run * pRun);

	bool                    cmdInsertLatexMath(UT_UTF8String & sLatex,
						   UT_UTF8String & sMath);

	UT_Error		cmdInsertTOC(void);
	UT_Error		cmdHyperlinkStatusBar(UT_sint32 xPos, UT_sint32 yPos);

	UT_Error		cmdInsertGraphic(FG_Graphic*);
	UT_Error        cmdInsertGraphicAtStrux(FG_Graphic* pFG, PT_DocPosition iPos, PTStruxType iStruxType);
	virtual void	toggleCase(ToggleCase c);
	virtual void	setPaperColor(const XML_Char * clr);

	virtual bool    isDocumentPresent(void);
	virtual void	cmdCopy(bool bToClipboard = true);
	virtual void	cmdCut(void);
	virtual void	cmdPaste(bool bHonorFormatting = true);
	virtual void	cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos);

	void            pasteFromLocalTo(PT_DocPosition pos);
	void            _pasteFromLocalTo(PT_DocPosition pos);
	void            copyToLocal(PT_DocPosition pos1, PT_DocPosition pos2);
	void		copyTextToClipboard(const UT_UCS4String sIncoming, bool useClipboard=true);

	virtual void	getTopRulerInfo(AP_TopRulerInfo * pInfo);
	virtual void	getTopRulerInfo(PT_DocPosition pos, AP_TopRulerInfo * pInfo);
	virtual void	getLeftRulerInfo(AP_LeftRulerInfo * pInfo);
	virtual void	getLeftRulerInfo(PT_DocPosition pos, AP_LeftRulerInfo * pInfo);
        virtual void    setCursorWait(void);
	virtual void    clearCursorWait(void);
	virtual void    setCursorToContext(void);
	virtual EV_EditMouseContext getMouseContext(UT_sint32 xPos, UT_sint32 yPos);
	virtual EV_EditMouseContext getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos);
	virtual void        updateLayout(void);
	virtual void        rebuildLayout(void);
	virtual void        remeasureCharsWithoutRebuild();
	virtual void        fontMetricsChange();
	virtual bool		isSelectionEmpty(void) const;
	bool                isSelectAll(void)
	{ return m_Selection.isSelectAll();}
	virtual void		cmdUnselectSelection(void);
	void				getDocumentRangeOfCurrentSelection(PD_DocumentRange * pdr);
	PT_DocPosition		mapDocPos( FV_DocPos dp );
	PT_DocPosition		mapDocPosSimple( FV_DocPos dp );
	PT_DocPosition saveSelectedImage (const char * toFile );
	PT_DocPosition saveSelectedImage (const UT_ByteBuf ** outByteBuf);
	PT_DocPosition getSelectedImage(const char **dataId);

	void            getTextInCurrentBlock(UT_GrowBuf & buf);
	void            getTextInCurrentSection(UT_GrowBuf & buf);
	void            getTextInDocument(UT_GrowBuf & buf);

// ----------------------
	FL_DocLayout*	getLayout() const;
	UT_uint32		getCurrentPageNumForStatusBar(void) const;
	fp_Page*		getCurrentPage(void) const;
	fl_BlockLayout* getCurrentBlock(void);

	void draw(int page, dg_DrawArgs* da);

	// TODO some of these functions should move into protected

	void	getPageScreenOffsets(const fp_Page* pPage, UT_sint32& xoff, UT_sint32& yoff);
	void	getPageYOffset(fp_Page* pPage, UT_sint32& yoff)const;
	virtual UT_sint32 getPageViewLeftMargin(void) const;
	virtual UT_sint32 getPageViewTopMargin(void) const;
	virtual UT_sint32 getPageViewSep(void) const;

	bool	setSectionFormat(const XML_Char * properties[]);
	bool	getSectionFormat(const XML_Char *** properties);

	bool	setBlockIndents(bool doLists, double indentChange, double page_size);
	bool    setCollapsedRange(PT_DocPosition posLow,PT_DocPosition posHigh, const XML_Char * properties[]);
	bool	setBlockFormat(const XML_Char * properties[]);
	bool	getBlockFormat(const XML_Char *** properties,bool bExpandStyles=true);
	bool    removeStruxAttrProps(PT_DocPosition ipos1, PT_DocPosition ipos2, PTStruxType iStrux,const XML_Char * attributes[] ,const XML_Char * properties[]);
	bool    isImageAtStrux(PT_DocPosition ipos1, PTStruxType iStrux);

	bool	processPageNumber(HdrFtrType hfType, const XML_Char ** atts);

	bool	isTextMisspelled()const ;
	bool	isTabListBehindPoint(UT_sint32 & iNumToDelete);
	bool	isTabListAheadPoint(void);
	void	processSelectedBlocks(FL_ListType listType);
	void	getBlocksInSelection(UT_GenericVector<fl_BlockLayout*> * vBlock);
	UT_sint32 getNumColumnsInSelection(void);
	UT_sint32 getNumRowsInSelection(void);
	void	getAllBlocksInList(UT_GenericVector<fl_BlockLayout *> * vBlock);
	bool	isPointBeforeListLabel(void);
	bool	isCurrentListBlockEmpty(void);
	bool	cmdStartList(const XML_Char * style);
	bool	cmdStopList(void);
	void	changeListStyle(fl_AutoNum* pAuto,
							FL_ListType lType,
							UT_uint32 startv,
							const XML_Char* pszDelim,
							const XML_Char* pszDecimal,
							const XML_Char* pszFormat,
							float Aligm,
							float Indent);

	void	setDontChangeInsPoint(void);
	void	allowChangeInsPoint(void);

	bool    getAttributes(const PP_AttrProp ** ppSpanAP, const PP_AttrProp ** ppBlockAP = NULL, PT_DocPosition posStart = 0);

	/* Experimental, for the moment; use with caution. - fjf, 24th Oct. '04
	 */
	// - begin
	bool    getAllAttrProp(const PP_AttrProp *& pSpanAP, const PP_AttrProp *& pBlockAP, const PP_AttrProp *& pSectionAP, const PP_AttrProp *& pDocAP) const;
	bool	queryCharFormat(const XML_Char * szProperty, UT_UTF8String & szValue, bool & bExplicitlyDefined, bool & bMixedSelection) const;
	bool	queryCharFormat(const XML_Char * szProperty, UT_UTF8String & szValue, bool & bExplicitlyDefined, PT_DocPosition position) const;
	// - end

	bool	setCharFormat(const XML_Char * properties[], const XML_Char * attribs[] = NULL);
	bool	resetCharFormat(bool bAll);
	bool	getCharFormat(const XML_Char *** properties,bool bExpandStyles=true);
	bool	getCharFormat(const XML_Char *** properties,bool bExpandStyles, PT_DocPosition posStart);
	fl_BlockLayout * getBlockFromSDH(PL_StruxDocHandle sdh);
	bool	setStyle(const XML_Char * style, bool bDontGeneralUpdate=false);
	bool	setStyleAtPos(const XML_Char * style, PT_DocPosition posStart, PT_DocPosition posEnd, bool bDontGeneralUpdate=false);
	bool    isNumberedHeadingHere(fl_BlockLayout * pBlock);
	bool	getStyle(const XML_Char ** style);
	bool appendStyle(const XML_Char ** style);

	UT_uint32		getCurrentPageNumber(void);

	bool	getEditableBounds(bool bEnd, PT_DocPosition & docPos, bool bOverride=false)const;

	bool    isParaBreakNeededAtPos(PT_DocPosition pos);
	bool    insertParaBreakIfNeededAtPos(PT_DocPosition pos);
	void	insertParagraphBreak(void);
	void	insertParagraphBreaknoListUpdate(void);
	void	insertSectionBreak( BreakSectionType type);
	void	insertSectionBreak(void);
	void	insertSymbol(UT_UCSChar c, XML_Char * symfont);

	// ----------------------
	bool			isLeftMargin(UT_sint32 xPos, UT_sint32 yPos);
	void			cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd);
	void			cmdSelectTOC(UT_sint32 xPos, UT_sint32 yPos);
	bool            isTOCSelected(void);
	bool            setTOCProps(PT_DocPosition pos, const char * szProps);

	void			cmdSelect(PT_DocPosition dpBeg, PT_DocPosition dpEnd);
	void			cmdCharMotion(bool bForward, UT_uint32 count);
	bool			cmdCharInsert(const UT_UCSChar * text, UT_uint32 count, bool bForce = false);
	void			cmdCharDelete(bool bForward, UT_uint32 count);
	void			delTo(FV_DocPos dp);
	void            getSelectionText(UT_UCS4Char *& text);

	UT_UCSChar *	getTextBetweenPos(PT_DocPosition pos1, PT_DocPosition pos2);
	inline PT_DocPosition  getInsPoint () const { return m_iInsPoint; }
	void			warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos, bool bClick);
	void			moveInsPtTo(FV_DocPos dp, bool bClearSelection = true);
	void			moveInsPtTo(PT_DocPosition dp);
	void			warpInsPtNextPrevPage(bool bNext);
	void			warpInsPtNextPrevLine(bool bNext);
	void            warpInsPtNextPrevScreen(bool bNext);
	void			extSelHorizontal(bool bForward, UT_uint32 count);
	void			extSelToXY(UT_sint32 xPos, UT_sint32 yPos, bool bDrag);
	void			extSelToXYword(UT_sint32 xPos, UT_sint32 yPos, bool bDrag);
	void			extSelTo(FV_DocPos dp);

	SpellChecker * getDictForSelection ();

	void			extSelNextPrevLine(bool bNext);
	void            extSelNextPrevPage(bool bNext);
	void            extSelNextPrevScreen(bool bNext);
	void			endDrag(UT_sint32 xPos, UT_sint32 yPos);

	void endDragSelection(UT_sint32 xPos, UT_sint32 yPos);

	PT_DocPosition  getDocPositionFromXY(UT_sint32 xpos, UT_sint32 ypos, bool bNotFrames = false);
	PT_DocPosition  getDocPositionFromLastXY(void);

	fl_BlockLayout* getBlockAtPosition(PT_DocPosition pos) const {return _findBlockAtPosition(pos);};
	virtual void	updateScreen(bool bDirtyRunsOnly=true);
	bool            isInDocSection(PT_DocPosition pos = 0);

//---------
//Visual Drag stuff
//
	void            cutVisualText(UT_sint32 x, UT_sint32 y);
	void            copyVisualText(UT_sint32 x, UT_sint32 y);
	void            dragVisualText(UT_sint32 x, UT_sint32 y);
	void            pasteVisualText(UT_sint32 x, UT_sint32 y);
	void            btn0VisualDrag(UT_sint32 x, UT_sint32 y);
	FV_VisualDragText * getVisualText(void)
	  { return &m_VisualDragText;}


//---------
//Visual Inline Image Drag stuff
//
	void            btn0InlineImage(UT_sint32 x, UT_sint32 y);
	void            btn1InlineImage(UT_sint32 x, UT_sint32 y);
	void            btn1CopyImage(UT_sint32 x, UT_sint32 y);
	void            dragInlineImage(UT_sint32 x, UT_sint32 y);
	void            releaseInlineImage(UT_sint32 x, UT_sint32 y);

// -------
// Frame stuff
//
	FV_FrameEdit *  getFrameEdit(void);
	void            btn0Frame(UT_sint32 x, UT_sint32 y);
	void            btn1Frame(UT_sint32 x, UT_sint32 y);
	void            dragFrame(UT_sint32 x, UT_sint32 y);
	void            releaseFrame(UT_sint32 x, UT_sint32 y);
	bool            isInFrame(PT_DocPosition pos);
	void            deleteFrame(void);
	void            cutFrame(void);
	void            selectFrame(void);
	fl_FrameLayout * getFrameLayout(PT_DocPosition pos);
	fl_FrameLayout * getFrameLayout(void);
	void            setFrameFormat(const XML_Char ** props);
	void            setFrameFormat(const XML_Char ** attribs, const XML_Char ** props);
	void            setFrameFormat(const XML_Char ** props,FG_Graphic * pFG, UT_String & dataID);
	void            convertInLineToPositioned(PT_DocPosition pos, 
											const XML_Char ** attribs);

	bool            convertPositionedToInLine(fl_FrameLayout * pFrame);

// ----------------------

	bool			isPosSelected(PT_DocPosition pos) const;
	bool			isXYSelected(UT_sint32 xPos, UT_sint32 yPos) const;
	FV_SelectionMode getSelectionMode(void) const;
	FV_SelectionMode getPrevSelectionMode(void) const;
	PD_DocumentRange * getNthSelection(UT_sint32 i);
	UT_sint32          getNumSelections(void) const;
	void            setSelectionMode(FV_SelectionMode selMode);
// ----------------------
// Stuff for spellcheck context menu
//
	UT_UCSChar *	getContextSuggest(UT_uint32 ndx);
	void			cmdContextSuggest(UT_uint32 ndx, fl_BlockLayout * ppBL = NULL, fl_PartOfBlock * ppPOB = NULL);
	void			cmdContextIgnoreAll(void);
	void			cmdContextAdd(void);
// ----------------------
// Stuff for edittable Headers/Footers
//
	bool                isInHdrFtr(PT_DocPosition pos);
	void				setHdrFtrEdit(fl_HdrFtrShadow * pShadow);
	void				clearHdrFtrEdit(void);
	bool				isHdrFtrEdit(void);
	fl_HdrFtrShadow *	getEditShadow(void);
	void				rememberCurrentPosition(void);
	PT_DocPosition		getSavedPosition(void);
	void				clearSavedPosition(void);
	void				markSavedPositionAsNeeded(void);
	bool				needSavedPosition(void);
	void				insertHeaderFooter(HdrFtrType hfType);
	bool				insertHeaderFooter(const XML_Char ** props, HdrFtrType hfType, fl_DocSectionLayout * pDSL=NULL);

	void				cmdEditHeader(void);
	void				cmdEditFooter(void);

	void                cmdRemoveHdrFtr(bool isHeader);
	bool                isFooterOnPage(void);
	bool                isHeaderOnPage(void);

	void                SetupSavePieceTableState(void);
	void                RestoreSavedPieceTableState(void);
    void                removeThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves = false);
	void                createThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves = false);
	void                populateThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves = false);
	void                _populateThisHdrFtr(fl_HdrFtrSectionLayout * pHdrFtrSrc, fl_HdrFtrSectionLayout * pHdrFtrDest);

//
// ----------------------
// Stuff for edittable Footnote/Endnotes
//
	bool	            insertFootnote(bool bFootnote);
	bool	            insertFootnoteSection(bool bFootnote,const XML_Char * enpid);
	bool                isInFootnote(PT_DocPosition pos);
	bool                isInFootnote(void);
	bool                isInEndnote(PT_DocPosition pos);
	bool                isInEndnote(void);
	fl_FootnoteLayout * getClosestFootnote(PT_DocPosition pos);
	fl_EndnoteLayout *  getClosestEndnote(PT_DocPosition pos);
	UT_sint32           getEmbedDepth(PT_DocPosition pos);
// ----------------------

	bool		gotoTarget(AP_JumpTarget type, UT_UCSChar * data);

	void			changeNumColumns(UT_uint32 iNumColumns);

// ----------------------

	// find and replace

	void			findSetFindString	(const UT_UCSChar* string);
	void			findSetReplaceString(const UT_UCSChar* string);
	void			findSetReverseFind	(bool newValue);
	void			findSetMatchCase	(bool newValue);
	void			findSetWholeWord	(bool newValue);
	UT_UCSChar *	findGetFindString   (void);
	UT_UCSChar *	findGetReplaceString(void);
	bool			findGetReverseFind	();
	bool			findGetMatchCase	();
	bool			findGetWholeWord	();

	bool			findAgain(void);

	void			findSetStartAtInsPoint(void);

	bool			findNext(bool& bDoneEntireDocument);
	bool			findNext(const UT_UCSChar* pFind, bool& bDoneEntireDocument);
	
	UT_uint32*		_computeFindPrefix(const UT_UCSChar* pFind);

	bool			_findNext(UT_uint32* pPrefix,
							 bool& bDoneEntireDocument);

	bool			findPrev(bool& bDoneEntireDocument);
	bool			findPrev(const UT_UCSChar* pFind, bool& bDoneEntireDocument);

	bool			_findPrev(UT_uint32* pPrefix,
							  bool& bDoneEntireDocument);

	bool			findReplaceReverse(bool& bDoneEntireDocument);

	bool			_findReplaceReverse(UT_uint32* pPrefix,
										bool& bDoneEntireDocument,
										bool bNoUpdate);

	bool			_findReplace(UT_uint32* pPrefix,
								 bool& bDoneEntireDocument,
								 bool bNoUpdate);


	bool			findReplace(bool& bDoneEntireDocument);

	UT_uint32		findReplaceAll();

// ----------------------

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	void			Test_Dump(void);
#endif

// ----------------------

	FV_DocCount 		countWords(void);

// -----------------------

	bool				insertPageNum(const XML_Char ** props, HdrFtrType hfType);
	void				setPoint(PT_DocPosition pt);
	void                ensureInsertionPointOnScreen(void);

// -----------------------
	void                killBlink(void);
	void				setShowPara(bool);
	inline bool 	getShowPara(void) const { return m_bShowPara; };

	const fp_PageSize&	getPageSize(void) const;
	UT_uint32			calculateZoomPercentForPageWidth();
	UT_uint32			calculateZoomPercentForPageHeight();
	UT_uint32			calculateZoomPercentForWholePage();
	inline void 			setViewMode (ViewMode vm) {m_viewMode = vm;}
	inline ViewMode 		getViewMode (void) const  {return m_viewMode;}
	bool				isPreview(void) const {return VIEW_PREVIEW == m_viewMode;}
	void				setPreviewMode(PreViewMode pre) {m_previewMode = pre;}
	PreViewMode 		getPreviewMode(void) { return m_previewMode;}

	void				setScreenUpdateOnGeneralUpdate( bool bDoit)
		{m_bDontUpdateScreenOnGeneralUpdate = !bDoit;}
	bool				shouldScreenUpdateOnGeneralUpdate(void) const
		{ return !m_bDontUpdateScreenOnGeneralUpdate;}

	inline PD_Document * getDocument (void) const {return m_pDoc;}

	/* Revision related functions */
	void                toggleMarkRevisions();
	void                cmdAcceptRejectRevision(bool bReject, UT_sint32 x, UT_sint32 y);
	// NB: 'mark revisions' state is document-wide
	bool                isMarkRevisions() const;
	// NB: 'show revisions' state is view-specific
	bool                isShowRevisions() const {return m_bShowRevisions;}
	void                toggleShowRevisions();
	void                setShowRevisions(bool bShow);
	
	void                cmdSetRevisionLevel(UT_uint32 i);
	UT_uint32           getRevisionLevel()const;
	void                setRevisionLevel(UT_uint32 i);

	bool                cmdFindRevision(bool bNext, UT_sint32 xPos, UT_sint32 yPos);
	bool                doesSelectionContainRevision() const;

	void                updateRevisionMode();
  protected:
	void                _fixInsertionPointAfterRevision();
	bool                _makePointLegal(void);
  public:
	
	/* Table related functions */
	bool                isPointLegal(PT_DocPosition pos);
	bool                isPointLegal(void);
	bool				isInTable();
	fl_TableLayout *    getTableAtPos(PT_DocPosition);
	bool				isInTable(PT_DocPosition pos);
	bool				isInTableForSure(PT_DocPosition pos);
	bool                cmdAutoSizeCols(void);
	bool                cmdTextToTable(bool bIgnoreSpaces);
	bool                cmdAutoSizeRows(void);
	bool                cmdAdvanceNextPrevCell(bool bGoNext);
	fp_CellContainer *  getCellAtPos(PT_DocPosition pos);
	PT_DocPosition      findCellPosAt(PT_DocPosition posTable, UT_sint32 row, UT_sint32 col);
	bool                _deleteCellAt(PT_DocPosition posTable,UT_sint32 row, UT_sint32 col);
	bool                _restoreCellParams(PT_DocPosition posTable, UT_sint32 iLineWidth);
	UT_sint32           _changeCellParams(PT_DocPosition posTable,PL_StruxDocHandle tableSDH );
	bool                deleteCellAt(PT_DocPosition posTable,UT_sint32 row, UT_sint32 col);
	bool                cmdDeleteCell(PT_DocPosition pos);
	bool                cmdDeleteCol(PT_DocPosition pos);
	bool                cmdDeleteRow(PT_DocPosition pos);
	bool                cmdDeleteTable(PT_DocPosition pos, bool bDontNotify=false);
	bool                cmdInsertRow(PT_DocPosition posTable, bool bBfore);
	bool                cmdInsertCol(PT_DocPosition posTable, bool bBefore);
	bool                cmdSplitCells(AP_CellSplitType iSplitType);
	bool                cmdSelectColumn(PT_DocPosition posOfColumn);
	bool                cmdAutoFitTable(void);
	bool                cmdMergeCells(PT_DocPosition posSource, PT_DocPosition posDestination);
	bool                cmdTableToText(PT_DocPosition posSource,UT_sint32 iSepType);

	bool                _MergeCells( PT_DocPosition posDestination,PT_DocPosition posSource, bool bBefore);
	bool                getCellParams(PT_DocPosition posCol, UT_sint32 *iLeft, 
									  UT_sint32 *iRight,UT_sint32 *iTop, UT_sint32 *iBot);
	bool				getCellLineStyle(PT_DocPosition posCell, UT_sint32 * pLeft, UT_sint32 * pRight,
										 UT_sint32 * pTop, UT_sint32 * pBot);
	bool				setCellFormat(const XML_Char * properties[], FormatTable applyTo, FG_Graphic * pFG, UT_String & sDataID);
	bool				getCellProperty(XML_Char * szPropName, XML_Char * &szPropValue);
	bool	            setTableFormat(const XML_Char * properties[]);
	bool	            setTableFormat(PT_DocPosition pos,const XML_Char * properties[]);
	bool                getCellFormat(PT_DocPosition pos, UT_String & sCellProps);

	UT_Error            cmdInsertTable(UT_sint32 numRows, UT_sint32 numCols, 
									   const XML_Char * pPropsArray[]);
	bool                _changeCellTo(PT_DocPosition posTable,UT_sint32 rowOld, 
									  UT_sint32 colOld, UT_sint32 left, UT_sint32 right,
									  UT_sint32 top, UT_sint32 bot);
	bool                changeCellTo(PT_DocPosition posTable,UT_sint32 rowOld, 
									 UT_sint32 colOld, UT_sint32 left, UT_sint32 right,
		                             UT_sint32 top, UT_sint32 bot);
	bool                _insertCellAfter(PT_DocPosition posTable, UT_sint32 row, 
										 UT_sint32 col,UT_sint32 left,UT_sint32 right,
										 UT_sint32 top, UT_sint32 bot);
	bool                _insertCellBefore(PT_DocPosition posTable, UT_sint32 row, 
										  UT_sint32 col, UT_sint32 left, UT_sint32 right,
										  UT_sint32 top, UT_sint32 bot);
	void				_generalUpdate(void);
	
	UT_RGBColor			getColorShowPara(void) const { return m_colorShowPara; }
	UT_RGBColor			getColorSquiggle(FL_SQUIGGLE_TYPE iSquiggleType) const;
	UT_RGBColor			getColorMargin(void) const { return m_colorMargin; }
	UT_RGBColor			getColorSelBackground(void);
	UT_RGBColor			getColorSelForeground(void);
	UT_RGBColor			getColorFieldOffset(void) const { return m_colorFieldOffset; }
	UT_RGBColor			getColorImage(void) const { return m_colorImage; }
	UT_RGBColor			getColorImageResize(void) const { return m_colorImageResize; }
	UT_RGBColor			getColorHyperLink(void) const { return m_colorHyperLink; }
	UT_RGBColor			getColorRevisions(int rev) const { 
		if (rev < 0) rev = 9;
		if (rev > 9) rev = 9;
		return m_colorRevisions[rev]; }
	UT_RGBColor			getColorHdrFtr(void) const { return m_colorHdrFtr; }
	UT_RGBColor			getColorColumnLine(void) const { return m_colorColumnLine; }

	void                getVisibleDocumentPagesAndRectangles(UT_GenericVector<UT_Rect*> &vRect, 
															 UT_GenericVector<fp_Page*> &vPages) const;

	//
	// image selection && resizing && dragging functions
	//
	void				setImageSelRect(UT_Rect r);
	UT_Rect				getImageSelRect();
	UT_sint32			getImageSelInfo();
	GR_Graphics::Cursor getImageSelCursor();
	void				setCurImageSel(UT_Rect r);
	UT_Rect				getCurImageSel();
#if XAP_DONTUSE_XOR
	void				setCurImageSelCache(GR_Image* cache);
	GR_Image* 			getCurImageSelCache();
#endif
	bool				isOverImageResizeBox(GR_Graphics::Cursor &cur, UT_uint32 xPos, UT_uint32 yPos);
	void				startImageResizing(UT_sint32 xPos, UT_sint32 yPos);
	void				stopImageResizing();
	bool				isResizingImage();
	void				getResizeOrigin(UT_sint32 &xOrigin, UT_sint32 &yOrigin);
	bool				isDraggingImage();
	void				startImageDrag(fp_Run * pRun, UT_sint32 xPos, UT_sint32 yPos);
	void				drawDraggedImage(UT_sint32 xPos, UT_sint32 yPos);
	void				stopImageDrag(UT_sint32 xPos, UT_sint32 yPos);

	bool                isImageSelected(void);

//
// Table resizing
//
	void                setDragTableLine(bool bSet) 
                        { m_bDragTableLine = bSet;}
	bool                getDragTableLine(void) const 
		                { return m_bDragTableLine;}
	void                setTopRuler(AP_TopRuler * pRuler) 
                        { m_pTopRuler = pRuler;}
	AP_TopRuler *       getTopRuler(void) const 
		                { return m_pTopRuler;}
	void                setLeftRuler(AP_LeftRuler * pRuler) 
                        { m_pLeftRuler = pRuler;}
	AP_LeftRuler *       getLeftRuler(void) const 
		                { return m_pLeftRuler;}
	

	const XML_Char **   getViewPersistentProps();
	FV_BIDI_Order	    getBidiOrder()const {return m_eBidiOrder;}
	void                setBidiOrder(FV_BIDI_Order o) {m_eBidiOrder = o;}

	bool                isMathSelected(UT_sint32 x, UT_sint32 y, PT_DocPosition & pos);
	// -- plugins
        bool                isMathLoaded(void);
	bool                isGrammarLoaded(void);

protected:
	void				_saveAndNotifyPieceTableChange(void);
	void				_restorePieceTableState(void);
	

	void				_draw(UT_sint32, UT_sint32, UT_sint32, UT_sint32, bool bDirtyRunsOnly, bool bClip=false);

	void				_drawBetweenPositions(PT_DocPosition left, PT_DocPosition right);
	bool				_clearBetweenPositions(PT_DocPosition left, PT_DocPosition right, bool bFullLineHeightRect);
	bool                		_drawOrClearBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2, bool bClear, bool bFullLineHeight);
	bool				_ensureInsertionPointOnScreen();
	void				_moveInsPtNextPrevPage(bool bNext);
	void				_moveInsPtNextPrevScreen(bool bNext);
	void				_moveInsPtNextPrevLine(bool bNext);
	fp_Page *			_getCurrentPage(void);
	void				_moveInsPtNthPage(UT_uint32 n);
	void				_moveInsPtToPage(fp_Page *page);
	void				_insertSectionBreak(void);

	PT_DocPosition		_getDocPosFromPoint(PT_DocPosition iPoint, FV_DocPos dp, bool bKeepLooking=true);
	PT_DocPosition		_getDocPos(FV_DocPos dp, bool bKeepLooking=true);
	void				_findPositionCoords(PT_DocPosition pos,
											bool b,
											UT_sint32& x,
											UT_sint32& y,
											UT_sint32& x2, //these are needed for BiDi split carret
											UT_sint32& y2,

											UT_uint32& height,
											bool& bDirection,
											fl_BlockLayout** ppBlock,
											fp_Run** ppRun)const;

	fl_BlockLayout* 	_findBlockAtPosition(PT_DocPosition pos) const;

	fp_Page*			_getPageForXY(UT_sint32 xPos,
									  UT_sint32 yPos,
									  UT_sint32& xClick,
									  UT_sint32& yClick) const;
	bool                _insertField(const char* szName, 
									 const XML_Char ** extra_attrs = NULL, 
									 const XML_Char ** extra_props = NULL);
	void				_moveToSelectionEnd(bool bForward);
	void				_eraseSelection(void);
	void				_clearSelection(void);
	void				_resetSelection(void);
	void				_setSelectionAnchor(void);
	void				_deleteSelection(PP_AttrProp *p_AttrProp_Before = NULL,
							 bool bNoUpdate = false,
							 bool bCaretLeft = false);
	bool				_insertFormatPair(const XML_Char * szName, const XML_Char * properties[]);
	void				_updateInsertionPoint();
	void				_fixInsertionPointCoords();
	void				_drawSelection();
	void				_swapSelectionOrientation(void);
	void				_extSel(UT_uint32 iOldPoint);
	void				_extSelToPos(PT_DocPosition pos);
	UT_Error			_insertGraphic(FG_Graphic*, const char*);

	UT_UCSChar *		_lookupSuggestion(fl_BlockLayout* pBL, fl_PartOfBlock* pPOB, UT_uint32 ndx);

	static void 		_autoScroll(UT_Worker * pTimer);

	// localize handling of insertion point logic
	void				_setPoint(PT_DocPosition pt, bool bEOL = false);
	UT_uint32			_getDataCount(UT_uint32 pt1, UT_uint32 pt2);
	bool				_charMotion(bool bForward,UT_uint32 countChars, bool bSkipCannotContainPoint = true);
	void				_doPaste(bool bUseClipboard, bool bHonorFormatting = true);
	void				_clearIfAtFmtMark(PT_DocPosition dpos);

	void				_checkPendingWordForSpell(void);

	bool				_isSpaceBefore(PT_DocPosition pos);
	void				_removeThisHdrFtr(fl_HdrFtrSectionLayout * pHdrFtr);
	void 				_cmdEditHdrFtr(HdrFtrType hfType);

	UT_Error			_deleteBookmark(const char* szName, bool bSignal, PT_DocPosition * pos1 = NULL, PT_DocPosition * pos2 = NULL);
	UT_Error			_deleteHyperlink(PT_DocPosition &i, bool bSignal);
	fp_HyperlinkRun *   _getHyperlinkInRange(PT_DocPosition &posStart,
											 PT_DocPosition &posEnd);
	bool			    _charInsert(const UT_UCSChar * text, UT_uint32 count, bool bForce = false);

	void                _adjustDeletePosition(UT_uint32 &iDocPos, UT_uint32 &iCount);
	
private:
	PT_DocPosition		m_iInsPoint;
	UT_sint32			m_xPoint;
	UT_sint32			m_yPoint;
	//the followingare BiDi specific, but need to be in place because of the
	//change to the signature of findPointCoords
	UT_sint32			m_xPoint2;
	UT_sint32			m_yPoint2;
	bool			    m_bPointDirection;
	bool				m_bDefaultDirectionRtl;
	bool				m_bUseHebrewContextGlyphs;
	UT_uint32			m_iPointHeight;
	UT_sint32			m_xPointSticky; 	// used only for _moveInsPtNextPrevLine()

	bool				m_bPointVisible;
	bool				m_bPointEOL;
	FL_DocLayout*		m_pLayout;
	PD_Document*		m_pDoc;
	GR_Graphics*		m_pG;
	void*				m_pParentData;

	// autoscroll stuff
	UT_Timer *			m_pAutoScrollTimer;
	UT_sint32			m_xLastMouse;
	UT_sint32			m_yLastMouse;

	bool				m_bCursorIsOn;
	bool				m_bEraseSaysStopBlinking;
	bool				m_bCursorBlink;

	bool				m_bdontSpellCheckRightNow;
	fv_ChangeState		m_chg;

	// find and replace stuff
	bool				m_wrappedEnd;
	PT_DocPosition		m_startPosition;

	bool				m_doneFind;

	bool				m_bEditHdrFtr;
	fl_HdrFtrShadow *	m_pEditShadow;
	PT_DocPosition		m_iSavedPosition;
	bool				m_bNeedSavedPosition;
	PT_DocPosition		_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset);

	fl_BlockLayout *	_findGetCurrentBlock(void);
	PT_DocPosition		_findGetCurrentOffset(void);
	UT_UCSChar *		_findGetNextBlockBuffer(fl_BlockLayout ** block, PT_DocPosition *offset);
	UT_UCSChar *		_findGetPrevBlockBuffer(fl_BlockLayout ** block, PT_DocPosition *offset);

	bool				m_bReverseFind;
	bool				m_bWholeWord;
	bool				m_bMatchCase;
	UT_UCSChar *		m_sFind;
	UT_UCSChar *		m_sReplace;

	UT_sint32			_findBlockSearchRegexp(const UT_UCSChar * haystack, const UT_UCSChar * needle);

	// prefs listener - to change cursor blink on/off (and possibly others)
	static void _prefsListener( XAP_App *, XAP_Prefs *, UT_StringPtrMap *, void *);

	bool				 m_bShowPara;
	ViewMode			 m_viewMode;
	PreViewMode 		 m_previewMode;
	bool				 m_bDontUpdateScreenOnGeneralUpdate;
	//#TF had to change the whole logic of storing PT state, since
	//the earlier implementation did not work with nested calls to
	//_saveAndNotifyPieceTableChange();
	UT_uint32			m_iPieceTableState;

	UT_sint32           m_iMouseX;
	UT_sint32           m_iMouseY;

	UT_uint32           m_iViewRevision;

	bool				m_bWarnedThatRestartNeeded;

	// properties for image selection
	UT_Rect				m_selImageRect;
	UT_uint32			m_iImageSelBoxSize;  // in device units!
	GR_Graphics::Cursor	m_imageSelCursor;
	UT_sint32			m_ixResizeOrigin;
	UT_sint32			m_iyResizeOrigin;
	bool				m_bIsResizingImage;
	UT_Rect				m_curImageSel;
#if XAP_DONTUSE_XOR
	GR_Image*			m_curImageSelCache;
#endif
	// properties for image dragging
	bool				m_bIsDraggingImage;
	fp_Run *			m_pDraggedImageRun;
	UT_Rect				m_dragImageRect;
	UT_sint32			m_ixDragOrigin;
	UT_sint32			m_iyDragOrigin;
	
	// default color values
	UT_RGBColor			m_colorShowPara;
	UT_RGBColor			m_colorSpellSquiggle;
	UT_RGBColor			m_colorGrammarSquiggle;
	UT_RGBColor			m_colorMargin;
	UT_RGBColor			m_colorFieldOffset;
	UT_RGBColor			m_colorImage;
	UT_RGBColor			m_colorImageResize;
	UT_RGBColor			m_colorHyperLink;
	UT_RGBColor         m_colorRevisions[10];
	UT_RGBColor			m_colorHdrFtr;
	UT_RGBColor			m_colorColumnLine;

	UT_uint32 m_countDisable; // cursor disable count
	bool                m_bDragTableLine;
	EV_EditMouseContext m_prevMouseContext;
	AP_TopRuler *       m_pTopRuler;
	AP_LeftRuler *      m_pLeftRuler;
	bool                m_bInFootnote;

	FV_Caret_Listener * m_caretListener;
	bool m_bgColorInitted;
	PT_DocPosition      m_iLowDrawPoint;
	PT_DocPosition      m_iHighDrawPoint;
	fv_PropCache        m_CharProps;
	fv_PropCache        m_BlockProps;
	fv_PropCache        m_SecProps;
	AV_ListenerId       m_CaretListID;
	FV_FrameEdit        m_FrameEdit;
	FV_VisualDragText   m_VisualDragText;
	FV_Selection        m_Selection;
	bool                m_bShowRevisions;

	FV_BIDI_Order       m_eBidiOrder;
	UT_uint32           m_iFreePass;
	bool                m_bDontNotifyListeners;
	UT_ByteBuf *        m_pLocalBuf;
	UT_sint32           m_iGrabCell;
	FV_VisualInlineImage  m_InlineImage;
	bool                m_bInsertAtTablePending;
	PT_DocPosition      m_iPosAtTable;
};

#endif /* FV_VIEW_H */
