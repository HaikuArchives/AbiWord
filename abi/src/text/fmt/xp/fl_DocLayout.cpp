/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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

#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_sleep.h"
#include "fl_DocListener.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_FootnoteLayout.h"
#include "fl_BlockLayout.h"
#include "fl_TOCLayout.h"
#include "fl_ContainerLayout.h"
#include "fl_Squiggles.h"
#include "fl_AutoNum.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_TextRun.h"
#include "fp_Run.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "gr_Graphics.h"
#include "xav_Listener.h"
#include "xap_App.h"
#include "ap_Prefs.h"
#include "fp_ContainerObject.h"
#include "fp_FootnoteContainer.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_timer.h"
#include "ut_string.h"
#include "xap_Frame.h"
#include "spell_manager.h"
#include "gr_EmbedManager.h"

#define REDRAW_UPDATE_MSECS	500

const char * s_FootnoteTypeDesc[] = {
	"1, 2, 3,...",
	"[1], [2], [3],...",
	"(1), (2), (3),...",
	"1), 2), 3),...",
	"a, b, c,...",
	"(a), (b), (c),...",
	"a), b), c)...",
	"A, B, C,...",
	"(A), (B), (C)...",
	"A), B), C),...",
	"i, ii, iii,...",
	"(i), (ii), (iii),...",
	"I, II, III,...",
	"(I), (II), (III),...",
	NULL
};

FL_DocLayout::FL_DocLayout(PD_Document* doc, GR_Graphics* pG)
  : m_pG(pG),
    m_pDoc(doc),
    m_pView(NULL),
    m_lid((PL_ListenerId)-1),
    m_pFirstSection(NULL),
    m_pLastSection(NULL),
    m_pPendingBlockForSpell(NULL),
    m_pPendingWordForSpell(NULL),
    m_bSpellCheckCaps(true),
    m_bSpellCheckNumbers(true),
    m_bSpellCheckInternet(true),
    m_bAutoSpellCheck(true),
    m_uDocBackgroundCheckReasons(0),
    m_bStopSpellChecking(false),
    m_bImSpellCheckingNow(false),
    m_pPendingBlockForSmartQuote(NULL),
    m_uOffsetForSmartQuote(0),
    m_pBackgroundCheckTimer(NULL),
    m_pPrefs(NULL),
    m_pRedrawUpdateTimer(NULL),
    m_iSkipUpdates(0),
    m_bDeletingLayout(false),
    m_bisLayoutFilling(false),
    m_iRedrawCount(0),
    m_FootnoteType(FOOTNOTE_TYPE_NUMERIC),
    m_iFootnoteVal(1),
    m_bRestartFootSection(false),
    m_bRestartFootPage(false),
    m_iEndnoteVal(1),
    m_EndnoteType(FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS),
    m_bRestartEndSection(false),
    m_bPlaceAtDocEnd(false),
    m_bPlaceAtSecEnd(true),
    m_iGraphicTick(0), 
    m_iDocSize(0),
    m_iFilled(0),
    m_bSpellCheckInProgress(false),
    m_bAutoGrammarCheck(false),
    m_PendingBlockForGrammar(NULL),
    m_iGrammarCount(0),
    m_bFinishedInitialCheck(false),
    m_iPrevPos(0)
{
#ifdef FMT_TEST
        m_pDocLayout = this;
#endif
        setLayoutIsFilling(false),
	m_pRedrawUpdateTimer = UT_Timer::static_constructor(_redrawUpdate, this, m_pG);
	if (m_pRedrawUpdateTimer)
	{
		m_pRedrawUpdateTimer->set(REDRAW_UPDATE_MSECS);
		m_pRedrawUpdateTimer->start();
	}
	
	// TODO the following (both the new() and the addListener() cause
	// TODO malloc's to occur.  we are currently inside a constructor
	// TODO and are not allowed to report failure.

	// Turn off list updating until document is formatted

	m_pDoc->disableListUpdates();

	strncpy(m_szCurrentTransparentColor,static_cast<const char *>(XAP_PREF_DEFAULT_ColorForTransparent),9);
	m_vecFootnotes.clear();
	m_vecEndnotes.clear();

}

FL_DocLayout::~FL_DocLayout()
{
	m_bDeletingLayout = true;
	if (m_pPrefs)
	{
		m_pPrefs->removeListener ( _prefsListener, this );
	}

	if (m_pDoc)
	{
		m_pDoc->removeListener(m_lid);
	}

	DELETEP(m_pDocListener);

	if (m_pBackgroundCheckTimer)
	{
		m_bStopSpellChecking = true;
		m_pBackgroundCheckTimer->stop();
	}

	DELETEP(m_pBackgroundCheckTimer);
	DELETEP(m_pPendingWordForSpell);

	if (m_pRedrawUpdateTimer)
	{
		m_pRedrawUpdateTimer->stop();
	}

	DELETEP(m_pRedrawUpdateTimer);

	UT_sint32 count = static_cast<UT_sint32>(m_vecPages.getItemCount()) -1;
	while(count >= 0)
	{
		fp_Page * pPage = static_cast<fp_Page *>(m_vecPages.getNthItem(count));
		if(pPage->getPrev())
		{
			pPage->getPrev()->setNext(NULL);
		}
		m_vecPages.deleteNthItem(count);
		delete pPage;
		count--;
	}

	while (m_pFirstSection)
	{
		fl_DocSectionLayout* pNext = m_pFirstSection->getNextDocSection();
		delete m_pFirstSection;
		m_pFirstSection = pNext;
	}
	UT_VECTOR_PURGEALL(GR_EmbedManager *,m_vecEmbedManager);
}


/*!
 * Get an embedManager of the requested Type.
 */
GR_EmbedManager * FL_DocLayout::getEmbedManager(const char * szEmbedType)
{
  // Look in the current collection first.
  UT_uint32 i = 0;
  GR_EmbedManager * pDefault = NULL;
  GR_EmbedManager * pEmbed = NULL;
  for(i=0; i< m_vecEmbedManager.getItemCount(); i++)
    {
      pEmbed = m_vecEmbedManager.getNthItem(i);
      if(UT_strcmp(pEmbed->getObjectType(),szEmbedType) == 0)
	{
	  return pEmbed;
	}
      if(UT_strcmp(pEmbed->getObjectType(),"default") == 0)
	{
	  pDefault = pEmbed;
	}
    }
  pEmbed = XAP_App::getApp()->getEmbeddableManager(m_pG,szEmbedType);
  if((UT_strcmp(pEmbed->getObjectType(),"default") == 0) && pDefault != NULL)
    {
      delete pEmbed;
      return pDefault;
    }
  UT_DEBUGMSG(("Got mamanger of type %s \n",pEmbed->getObjectType()));
  m_vecEmbedManager.addItem(pEmbed);
  pEmbed->initialize();
  
  return pEmbed;
}

/*! 
 * little helper method for lookup properties
 */
FootnoteType FL_DocLayout::FootnoteTypeFromString(const XML_Char * pszFootnoteType)
{
	FootnoteType iFootnoteType;
	if (pszFootnoteType == NULL)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC;
	}
	else if(pszFootnoteType[0] == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"numeric") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"numeric-square-brackets") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"numeric-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC_PAREN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"numeric-open-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"upper") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"upper-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER_PAREN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"upper-paren-open") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER_OPEN_PAREN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"lower") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"lower-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER_PAREN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"lower-paren-open") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER_OPEN_PAREN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"lower-roman") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER_ROMAN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"lower-roman-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_LOWER_ROMAN_PAREN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"upper-roman") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER_ROMAN;
	}
	else if(UT_XML_strcmp(pszFootnoteType,"upper-roman-paren") == 0)
	{
		iFootnoteType = FOOTNOTE_TYPE_UPPER_ROMAN_PAREN;
	}
	else
	{
		iFootnoteType = FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS;
	}
	return iFootnoteType;
}

/*!
 * This method reads the document properties and saves local versions of them
 * here.
 */
void FL_DocLayout::_lookupProperties(void)
{
	const XML_Char * pszFootnoteType = NULL;
	const PP_AttrProp* pDocAP = getDocument()->getAttrProp();
	UT_ASSERT(pDocAP);
	pDocAP->getProperty("document-footnote-type", (const XML_Char *&)pszFootnoteType);
	m_FootnoteType = FootnoteTypeFromString(pszFootnoteType);

	const XML_Char * pszEndnoteType = NULL;
	pDocAP->getProperty("document-endnote-type", (const XML_Char *&)pszEndnoteType);
	m_EndnoteType = FootnoteTypeFromString(pszEndnoteType);

	const XML_Char * pszTmp = NULL;
	pDocAP->getProperty("document-footnote-initial", (const XML_Char *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		m_iFootnoteVal =  atoi(pszTmp);
	}
	else
	{
		m_iFootnoteVal = 1;
	}

	pDocAP->getProperty("document-footnote-restart-section", (const XML_Char *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(UT_XML_strcmp(pszTmp,"1") == 0)
		{
			m_bRestartFootSection = true;
		}
		else
		{
			m_bRestartFootSection = false;
		}
	}
	else
	{
		m_bRestartFootSection = false;
	}

	pDocAP->getProperty("document-footnote-restart-page", (const XML_Char *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(UT_XML_strcmp(pszTmp,"1") == 0)
		{
			m_bRestartFootPage = true;
		}
		else
		{
			m_bRestartFootPage = false;
		}
	}
	else
	{
		m_bRestartFootPage = false;
	}

	pDocAP->getProperty("document-endnote-initial", (const XML_Char *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		m_iEndnoteVal =  atoi(pszTmp);
	}
	else
	{
		m_iEndnoteVal = 1;
	}

	pDocAP->getProperty("document-endnote-restart-section", (const XML_Char *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(UT_XML_strcmp(pszTmp,"1") == 0)
		{
			m_bRestartEndSection = true;
		}
		else
		{
			m_bRestartEndSection = false;
		}
	}
	else
	{
		m_bRestartEndSection = false;
	}

	pDocAP->getProperty("document-endnote-place-endsection", (const XML_Char *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(UT_XML_strcmp(pszTmp,"1") == 0)
		{
			m_bPlaceAtDocEnd = false;
		}
		else
		{
			m_bPlaceAtDocEnd = true;
		}
	}
	else
	{
		m_bPlaceAtDocEnd = false;
	}

	pDocAP->getProperty("document-endnote-place-enddoc", (const XML_Char *&)pszTmp);
	if(pszTmp && pszTmp[0])
	{
		if(UT_XML_strcmp(pszTmp,"1") == 0)
		{
			m_bPlaceAtSecEnd = false;
		}
		else
		{
			m_bPlaceAtSecEnd = true;
		}
	}
	else
	{
		m_bPlaceAtSecEnd = true;
	}

}

void FL_DocLayout::updatePropsNoRebuild(void)
{
	_lookupProperties();
}

/*!
 * Change the graphics pointer for this layout.Needed for quick zoom.
 */
void FL_DocLayout::setGraphics(GR_Graphics * pG)
{
	m_pG = pG;
	m_iGraphicTick++;

	// we need to at least re-acquire a new copy of the fonts
	// since the last ones' lives are attached to the old
	// graphics class
	updatePropsRebuild();
}

void FL_DocLayout::updatePropsRebuild(void)
{
	_lookupProperties();
	rebuildFromHere( m_pFirstSection);
}

FootnoteType FL_DocLayout::getFootnoteType(void) const
{
	return m_FootnoteType;
}

/*!
 * This Method fills the layout structures from the PieceTable.
 */
void FL_DocLayout::fillLayouts(void)
{
	_lookupProperties();
	setLayoutIsFilling(true);
	if(m_pView)
	{
		m_pView->setPoint(0);
		m_pView->setLayoutIsFilling(true);
	}
	m_pDoc->getBounds(true,m_iDocSize);
//
// Make a document listner to get info pumped into the layouts.
//
	m_pDocListener = new fl_DocListener(m_pDoc, this);
	UT_ASSERT(m_pDocListener);
//
// The act of adding the listner to the document also causes the
// the document to pump it's content into the layout classes.
//
	//m_pDoc->setDontImmediatelyLayout(true);
	//m_pDocListener->setHoldTableLayout(false);
	m_pDoc->addListener(static_cast<PL_Listener *>(m_pDocListener),&m_lid);
	//m_pDoc->setDontImmediatelyLayout(false);
	UT_ASSERT(m_lid != (PL_ListenerId)-1);
	GR_Graphics * pG = getGraphics();
	formatAll(); // Do we keep this or not?
	m_bFinishedInitialCheck = false;
	m_iPrevPos = 0;
	m_iGrammarCount = 0;
	if(m_pView)
	{
		m_pView->setLayoutIsFilling(false);
		setLayoutIsFilling(false);
		m_pView->moveInsPtTo(FV_DOCPOS_BOD);
		m_pView->clearCursorWait();
		m_pView->updateLayout();
		if(!pG->queryProperties(GR_Graphics::DGP_PAPER))
		{
			m_pView->updateScreen(false);
			XAP_Frame * pFrame = static_cast<XAP_Frame *>(m_pView->getParentData());
			if(pFrame)
			{
				pFrame->setYScrollRange();
			}
		}
	}
	setLayoutIsFilling(false);
	if(!m_pView)
	{
		updateLayout();
	}

	// Layout of any TOC that is built only from a restricted document range is tentative, because
	// the information required by the TOC might not have been available during the incremental
	// load.  In that case the TOCs made made certain assumptions about the presence of a given
	// bookmark in the doc during the fill. These assumptions now need to be verified and, if
	// required, fixed
	
	fl_TOCLayout* pBadTOC = NULL;
	
	for (UT_sint32 i = 0; i < getNumTOCs(); ++i)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(!pTOC)
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			continue;
		}

		// because the incremental load is sequential, the TOCs are in the order they have in the
		// document, so we just need to remember the first one.
		if(pTOC->verifyBookmarkAssumptions() && !pBadTOC)
		{
			pBadTOC = pTOC;
		}
	}

	if(pBadTOC)
	{
		// hard luck -- we need to redo the layout, since the TOC probably changed size
		fl_SectionLayout * pSL = pBadTOC->getSectionLayout();
		fl_DocSectionLayout * pDSL = NULL;
		
		if(pSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			pDSL = static_cast<fl_DocSectionLayout*>(pSL);
		}
		else
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		}

		if(!pDSL)
		{
			formatAll();
		}
		else
		{
			while (pSL)
			{
				pSL->format();
				if(pSL->getContainerType() == FL_CONTAINER_DOCSECTION)
				{
					static_cast<fl_DocSectionLayout *>(pSL)->completeBreakSection();
					static_cast<fl_DocSectionLayout *>(pSL)->checkAndRemovePages();
				}
				pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
			}
		}
		
		if(m_pView)
		{
			m_pView->updateLayout();
			if(!pG->queryProperties(GR_Graphics::DGP_PAPER))
			{
				m_pView->updateScreen(false);
				XAP_Frame * pFrame = static_cast<XAP_Frame *>(m_pView->getParentData());
				if(pFrame)
				{
					pFrame->setYScrollRange();
				}
			}
		}
	}
	if(m_pView)
	{
	    fl_DocSectionLayout * pLastSec = getLastSection();
		fl_ContainerLayout * pCL = pLastSec->getLastLayout();
		fl_BlockLayout * pBL = NULL;
		bool bRebreak = false;
		if(pCL->getContainerType() == FL_CONTAINER_BLOCK)
		{
	              pBL = static_cast<fl_BlockLayout *>(pCL);
		}
		else
		{
	              pBL = pCL->getPrevBlockInDocument();
		}
		if(pBL)
		{
		      fp_Line * pLine = static_cast<fp_Line *>(pBL->getLastContainer());
		      if(pLine == NULL)
		      {
			    bRebreak = true;
		      }
		      else if(pLine->getPage() == NULL)
		      {
			    bRebreak = true;
		      }
		      else
		      {
			    fp_Page * pPage = getFirstPage();
			    while(pPage && pPage != pLine->getPage())
			    {
			         pPage = pPage->getNext();
			    }
			    if(pLine->getPage() != pPage)
			    {
			         bRebreak = true;
			    }
			    if(pLine->getPage() == getFirstPage())
			    {
				bRebreak = true;
			    }
		      }
		}
		if(bRebreak)
		{
		      getFirstSection()->completeBreakSection();
		}
	}
}


void FL_DocLayout::setView(FV_View* pView)
{
	m_pView = pView;

	fp_Page* pPage = getFirstPage();

	while (pPage)
	{
		pPage->setView(pView);

		pPage = pPage->getNext();
	}

	if (m_pView && !m_pPrefs )
	{
		XAP_App * pApp = m_pView->getApp();
		UT_ASSERT(pApp);
		XAP_Prefs *pPrefs= pApp->getPrefs();
		UT_ASSERT(pPrefs);

		if (pPrefs)
		{
			// remember this so we can remove the listener later
			m_pPrefs = pPrefs;

			// initialize the vars here
			_prefsListener( pApp, pPrefs, NULL, this );

			// keep updating itself
			pPrefs->addListener ( _prefsListener, this );
			bool b;
			if (m_pPrefs->getPrefsValueBool(static_cast<const XML_Char *>("DebugFlash"),&b)  &&  b == true)
			{
				addBackgroundCheckReason(bgcrDebugFlash);
			}
			m_pPrefs->getPrefsValueBool(static_cast<const XML_Char *>("AutoGrammarCheck"),&b);
			if (b)
			{
				addBackgroundCheckReason(bgcrGrammar);
				m_bAutoGrammarCheck = true;
				m_iGrammarCount = 0;
				m_iPrevPos = 0;
			}
		}
	}
}

/*!
 * This method fills the referenced string with the character representation
 * of the decimal footnote value based on the FootnoteType passed as a 
 * parameter
 */
void FL_DocLayout::getStringFromFootnoteVal(UT_String & sVal, UT_sint32 iVal, FootnoteType iFootType)
{
	fl_AutoNum autoCalc(0,0,NUMBERED_LIST,0,NULL,NULL,NULL,NULL);

	switch (iFootType)
	{
	case FOOTNOTE_TYPE_NUMERIC:
		UT_String_sprintf (sVal,"%d", iVal);
		break;
	case FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS:
		UT_String_sprintf (sVal,"[%d]", iVal);
		break;
	case FOOTNOTE_TYPE_NUMERIC_PAREN:
		UT_String_sprintf (sVal,"(%d)", iVal);
		break;
	case FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN:
		UT_String_sprintf (sVal,"%d)", iVal);
		break;
	case FOOTNOTE_TYPE_LOWER:
	{
		char * val = autoCalc.dec2ascii(iVal,96);
		UT_String_sprintf (sVal,"%s",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_LOWER_PAREN:
	{
		char * val = autoCalc.dec2ascii(iVal,96);
		UT_String_sprintf (sVal,"(%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_LOWER_OPEN_PAREN:
	{
		char * val = autoCalc.dec2ascii(iVal,96);
		UT_String_sprintf (sVal,"%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER:
	{
		char * val = autoCalc.dec2ascii(iVal,64);
		UT_String_sprintf (sVal,"%s",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER_PAREN:
	{
		char * val = autoCalc.dec2ascii(iVal,64);
		UT_String_sprintf (sVal,"(%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER_OPEN_PAREN:
	{
		char * val = autoCalc.dec2ascii(iVal,64);
		UT_String_sprintf (sVal,"%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_LOWER_ROMAN:
	{
		char * val = autoCalc.dec2roman(iVal,true);
		UT_String_sprintf (sVal,"%s",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_LOWER_ROMAN_PAREN:
	{
		char * val = autoCalc.dec2roman(iVal,true);
		UT_String_sprintf (sVal,"(%s)",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER_ROMAN:
	{
		char * val = autoCalc.dec2roman(iVal,false);
		UT_String_sprintf (sVal,"%s",val);
		FREEP(val);
		break;
	}
	case FOOTNOTE_TYPE_UPPER_ROMAN_PAREN:
	{
		char * val = autoCalc.dec2roman(iVal,false);
		UT_String_sprintf (sVal,"(%s)",val);
		FREEP(val);
		break;
	}
	default:
		UT_String_sprintf (sVal,"%d", iVal);
	}
}


/*!
 * This simply returns the number of footnotes in the document.
 */
UT_uint32 FL_DocLayout::countFootnotes(void)
{
	return m_vecFootnotes.getItemCount();
}
/*!
 * Add a footnote layout to the vector remembering them.
 */
void FL_DocLayout::addFootnote(fl_FootnoteLayout * pFL)
{
	m_vecFootnotes.addItem(pFL);
}

/*!
 * get a pointer to the Nth footnote layout in the vector remembering them.
 */
fl_FootnoteLayout * FL_DocLayout::getNthFootnote(UT_sint32 i)
{
	UT_ASSERT(i>=0);
	if(i >= static_cast<UT_sint32>(m_vecFootnotes.getItemCount()))
	{
		return NULL;
	}
	else
	{
		return m_vecFootnotes.getNthItem(i);
	}
}

/*!
 * Remove a foonote layout from the Vector.
 */
void FL_DocLayout::removeFootnote(fl_FootnoteLayout * pFL)
{
	UT_sint32 i = m_vecFootnotes.findItem(pFL);
	if(i< 0)
	{
		return;
	}
	m_vecFootnotes.deleteNthItem(i);
}

/*!
 * This method returns the footnote layout associated with the input PID
 */
fl_FootnoteLayout * FL_DocLayout::findFootnoteLayout(UT_uint32 footpid)
{
	UT_sint32 i = 0;
	fl_FootnoteLayout * pTarget = NULL;
 	fl_FootnoteLayout * pFL = NULL;
	for(i=0; i<static_cast<UT_sint32>(m_vecFootnotes.getItemCount()); i++)
	{
		pFL = getNthFootnote(i);
		if(pFL->getFootnotePID() == footpid)
		{
			pTarget = pFL;
			break;
		}
	}
	return pTarget;
}
/*!
 * This returns the position of the footnote in the document. This is useful
 * for calculating the footnote's value and positioning it in a footnote 
 * section
 */
UT_sint32 FL_DocLayout::getFootnoteVal(UT_uint32 footpid)
{
	UT_sint32 i =0;
	UT_sint32 pos = m_iFootnoteVal;
	fl_FootnoteLayout * pTarget = findFootnoteLayout(footpid);
 	fl_FootnoteLayout * pFL = NULL;
	if(pTarget== NULL)
	{
		return 0;
	}
	PT_DocPosition posTarget = pTarget->getDocPosition();
	fl_DocSectionLayout * pDocSecTarget = pTarget->getDocSectionLayout();
	fp_Container * pCon = pTarget->getFirstContainer();
	fp_Page * pPageTarget = NULL;
	if(pCon)
	{
		pPageTarget = pCon->getPage();
	}
	for(i=0; i<static_cast<UT_sint32>(m_vecFootnotes.getItemCount()); i++)
	{
		pFL = getNthFootnote(i);
		if(!m_bRestartFootSection && !m_bRestartFootPage)
		{
			if(pFL->getDocPosition() < posTarget)
			{
				pos++;
			}
		}
		else if(m_bRestartFootSection)
		{
			if((pDocSecTarget == pFL->getDocSectionLayout()) && (pFL->getDocPosition() < posTarget))
			{
				pos++;
			}
		}
		else if(m_bRestartFootPage)
		{
			pCon = pFL->getFirstContainer();
			fp_Page * pPage = NULL;
			if(pCon)
			{
				pPage = pCon->getPage();
			}
			if((pPage == pPageTarget) && (pFL->getDocPosition() < posTarget))
			{
				pos++;
			} 
		}
	}
	return pos;
}


/*!
 * The method returns the doc section layout before which the endnotes are 
 * inserted.
 */
fl_DocSectionLayout * FL_DocLayout::getDocSecForEndnote(fp_EndnoteContainer * pECon)
{
	fl_DocSectionLayout *pDSL = NULL;
	if(getPlaceEndAtSecEnd())
	{
		fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(pECon->getSectionLayout());
		pDSL = pEL->getDocSectionLayout();
		return pDSL;
	}
	pDSL = getLastSection();
	return pDSL;
}

/*!
 * This method checks to too if the endnote container to be removed is the 
 * first or last of the section. If it is the first/last pointers are updated.
 */
void FL_DocLayout::removeEndnoteContainer(fp_EndnoteContainer * pECon)
{
	UT_DEBUGMSG(("Remove endnote container %x \n",pECon));
	fl_DocSectionLayout * pDSL = getDocSecForEndnote(pECon);
	if(pDSL->getFirstEndnoteContainer() == static_cast<fp_Container *>(pECon))
	{
		pDSL->setFirstEndnoteContainer(static_cast<fp_EndnoteContainer *>(pECon->getNext()));
	}
	if(pDSL->getLastEndnoteContainer() == static_cast<fp_Container *>(pECon))
	{
		pDSL->setLastEndnoteContainer(static_cast<fp_EndnoteContainer *>(pECon->getPrev()));
	}
//
// Remove from list
//
	if(pECon->getPrev())
	{
		pECon->getPrev()->setNext(pECon->getNext());
	}
	if(pECon->getNext())
	{
		pECon->getNext()->setPrev(pECon->getPrev());
	}
//	fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(pECon->getSectionLayout());
//	pDSL = static_cast<fl_DocSectionLayout *>(pEL->myContainingLayout());
//	if(!pDSL->isCollapsing())
	{
		fp_Column * pCol = static_cast<fp_Column *>(pECon->getContainer());

		if(pCol)
		{
			pCol->removeContainer(pECon);
		}
	}
}

/*!
 * This method inserts the endnote container into the list of containers held
 * held by the appropriate DocSection.
 */
void FL_DocLayout::insertEndnoteContainer(fp_EndnoteContainer * pECon)
{
	fl_DocSectionLayout * pDSL = getDocSecForEndnote(pECon);
	fp_Container * pCon = pDSL->getFirstEndnoteContainer();
	if(pCon == NULL)
	{
		pDSL->setFirstEndnoteContainer(pECon);
		pDSL->setLastEndnoteContainer(pECon);
		pECon->setNext(NULL);
		pECon->setPrev(NULL);
		fp_Column * pCol =  static_cast<fp_Column *>(pDSL->getLastContainer());
		if(pCol)
		{
			pCol->addContainer(pECon);
//
// No height defined yet. Can't layout
//
//			pCol->layout();
		}
		else
		{
			fp_Column * pCol = static_cast<fp_Column *>(pDSL->getNewContainer(NULL));
			pCol->addContainer(pECon);
//
// No height defined yet. Can't layout
//
//			pCol->layout();
		}
		return;
	}
	fp_EndnoteContainer * pETmp = static_cast<fp_EndnoteContainer *>(pCon);
	fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(pECon->getSectionLayout());
	fl_EndnoteLayout * pETmpL = static_cast<fl_EndnoteLayout *>(pETmp->getSectionLayout());
	bool bBefore = (pEL->getPosition() < pETmpL->getPosition());
	while(!bBefore && pETmp)
	{
		pETmp = static_cast<fp_EndnoteContainer *>(pETmp->getNext());
		if(pETmp)
		{
			pETmpL = static_cast<fl_EndnoteLayout *>(pETmp->getSectionLayout());
			UT_ASSERT(pETmpL);
			bBefore = (pEL->getPosition() < pETmpL->getPosition());
		}
	}
	if(bBefore)
	{
		fp_EndnoteContainer * pOldPrev = static_cast<fp_EndnoteContainer *>(pETmp->getPrev());
		pETmp->setPrev(pECon);
		if(pDSL->getFirstEndnoteContainer() == pETmp)
		{
			pDSL->setFirstEndnoteContainer(pECon);
		}
		else
		{
			pOldPrev->setNext(pECon);
	
		}
		fp_Column * pCol = static_cast<fp_Column *>(pETmp->getContainer());
		pECon->setNext(pETmp);
		pECon->setPrev(pOldPrev);
		if(pOldPrev)
		{
			pCol->insertContainerAfter(pECon, pOldPrev);
		}
		else
		{
			pCol->insertContainer(pECon);
		}
		pCol->layout();
	}
	else
	{
		pETmp = static_cast<fp_EndnoteContainer *>(pDSL->getLastEndnoteContainer());
		pETmp->setNext(pECon);
		pECon->setPrev(pETmp);
		pECon->setNext(NULL);
		pDSL->setLastEndnoteContainer(pECon);
		fp_Column * pCol = static_cast<fp_Column *>(pETmp->getContainer());
		if(!pCol)
		{
			pCol = static_cast<fp_Column *>(pDSL->getLastContainer());
			if(pCol == NULL)
			{
				pCol = static_cast<fp_Column *>(pDSL->getNewContainer());
			}
		}
		pCol->addContainer(pECon);
		pCol->layout();
	}
}

/*!
 * This simply returns the number of footnotes in the document.
 */
UT_uint32 FL_DocLayout::countEndnotes(void)
{
	return m_vecEndnotes.getItemCount();
}
/*!
 * Add a footnote layout to the vector remembering them.
 */
void FL_DocLayout::addEndnote(fl_EndnoteLayout * pFL)
{
	m_vecEndnotes.addItem(pFL);
}

/*!
 * get a pointer to the Nth footnote layout in the vector remembering them.
 */
fl_EndnoteLayout * FL_DocLayout::getNthEndnote(UT_sint32 i)
{
	UT_ASSERT(i>=0);
	if(i >= static_cast<UT_sint32>(m_vecEndnotes.getItemCount()))
	{
		return NULL;
	}
	else
	{
		return m_vecEndnotes.getNthItem(i);
	}
}

/*!
 * Remove a foonote layout from the Vector.
 */
void FL_DocLayout::removeEndnote(fl_EndnoteLayout * pFL)
{
	UT_sint32 i = m_vecEndnotes.findItem(pFL);
	if(i< 0)
	{
		return;
	}
	m_vecEndnotes.deleteNthItem(i);
}

/*!
 * This method returns the footnote layout associated with the input PID
 */
fl_EndnoteLayout * FL_DocLayout::findEndnoteLayout(UT_uint32 footpid)
{
	UT_sint32 i = 0;
	fl_EndnoteLayout * pTarget = NULL;
 	fl_EndnoteLayout * pFL = NULL;
	for(i=0; i<static_cast<UT_sint32>(m_vecEndnotes.getItemCount()); i++)
	{
		pFL = getNthEndnote(i);
		if(pFL->getEndnotePID() == footpid)
		{
			pTarget = pFL;
			break;
		}
	}
	return pTarget;
}
/*!
 * This returns the position of the Endnote in the document. This is useful
 * for calculating the Endnote's value and positioning it in a footnote 
 * section
 */
UT_sint32 FL_DocLayout::getEndnoteVal(UT_uint32 footpid)
{
	UT_sint32 i =0;
	UT_sint32 pos = m_iEndnoteVal;
	fl_EndnoteLayout * pTarget = findEndnoteLayout(footpid);
 	fl_EndnoteLayout * pFL = NULL;
	if(pTarget== NULL)
	{
		return 0;
	}
	PT_DocPosition posTarget = pTarget->getDocPosition();
	fl_DocSectionLayout * pDocSecTarget = pTarget->getDocSectionLayout();
	fp_Container * pCon = pTarget->getFirstContainer();
	fp_Page * pPageTarget = NULL;
	if(pCon)
	{
		pPageTarget = pCon->getPage();
	}
	for(i=0; i<static_cast<UT_sint32>(m_vecEndnotes.getItemCount()); i++)
	{
		pFL = getNthEndnote(i);
		if(!m_bRestartEndSection)
		{
			if(pFL->getDocPosition() < posTarget)
			{
				pos++;
			}
		}
		else if(m_bRestartEndSection)
		{
			if((pDocSecTarget == pFL->getDocSectionLayout()) && (pFL->getDocPosition() < posTarget))
			{
				pos++;
			}
		}
	}
	return pos;
}


//
//--------------------------------------------------------------------
// Table of content Functions.
//--------------------------------------------------------------------
//

UT_sint32 FL_DocLayout::getNumTOCs(void)
{
	return static_cast<UT_sint32 >(m_vecTOC.getItemCount());
}

fl_TOCLayout * FL_DocLayout::getNthTOC(UT_sint32 i)
{
	if( i >= getNumTOCs())
	{
		return NULL;
	}
	return static_cast<fl_TOCLayout *>(m_vecTOC.getNthItem(i));
}

void FL_DocLayout::recalculateTOCFields(void)
{
	UT_sint32 num = getNumTOCs();
	UT_sint32 i =0;
	for (i=0; i<num; i++)
	{
		fl_TOCLayout * pTOCL = getNthTOC(i);
		pTOCL->recalculateFields(i);
	}
}

/*!
 * This method scans all the TOC in the document and adds or removes the
 * supplied block if it needs to be either added or removed from a TOC.
 * This method returns true if pBlock is in at least one TOC
 */
bool FL_DocLayout::addOrRemoveBlockFromTOC(fl_BlockLayout * pBlock)
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_UTF8String sStyle;
	pBlock->getStyle(sStyle);
	UT_sint32 i = 0;
	UT_sint32 inTOC = count;
	UT_sint32 addTOC = 0;

	for(i=0; i<count; i++)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(pTOC->isBlockInTOC(pBlock))
		{
			if(!pTOC->isStyleInTOC(sStyle))
			{
				pTOC->removeBlock(pBlock);
				inTOC--;
			}
			else
			{
//
// Style changed so delete the old shadow of the block and make a new shadow.
//
				pTOC->removeBlock(pBlock);
				pTOC->addBlock(pBlock);
			}
		}
		else
		{
			if(pTOC->isStyleInTOC(sStyle))
			{
				pTOC->addBlock(pBlock);
				addTOC++;
			}
		}
	}
	if((inTOC <= 0) && (addTOC == 0))
	{
		return false;
	}
	return true;
}

/*!
 * Remove pBlock from all the TOC's it's in.
 * Return false if there are no TOC's in the Doc.
 * return true otherwise.
 */
bool FL_DocLayout::removeBlockFromTOC(fl_BlockLayout *pBlock)
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_sint32 i = 0;
	for(i=0; i<count; i++)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(pTOC->isBlockInTOC(pBlock))
		{
			pTOC->removeBlock(pBlock);
		}
	}
	return true;
}

/*!
 * returns true if the block is in at least one TOC.
 */
bool FL_DocLayout::isBlockInTOC(fl_BlockLayout * pBlock)
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_sint32 i = 0;
	for(i=0; i<count; i++)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(pTOC->isBlockInTOC(pBlock))
		{
			return true;
		}
	}
	return false;;
}

/*!
 * Fill the supplied vector with pointers to the blocks matching the supplied
 * Block.
 * Return false if no matching block were found.
 * true otherwise
 */
bool FL_DocLayout::getMatchingBlocksFromTOCs(fl_BlockLayout * pBlock, UT_GenericVector<fl_BlockLayout*>* pVecBlocks)
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_sint32 i = 0;
	for(i=0; i<count; i++)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		if(pTOC->isBlockInTOC(pBlock))
		{
			fl_BlockLayout * pMatch = pTOC->getMatchingBlock(pBlock);
			pVecBlocks->addItem(pMatch);
		}
	}
	return (pVecBlocks->getItemCount() > 0);
}
	
bool FL_DocLayout::addTOC(fl_TOCLayout * pTOC)
{
	m_vecTOC.addItem(pTOC);
	return true;
}

bool FL_DocLayout::removeTOC(fl_TOCLayout * pTOC)
{
	UT_sint32 count = getNumTOCs();
	if(count == 0)
	{
		return false;
	}
	UT_sint32 i = m_vecTOC.findItem(pTOC);
	if(i < 0)
	{
		return false;
	}
	m_vecTOC.deleteNthItem(i);
	return true;
}

bool FL_DocLayout::fillTOC(fl_TOCLayout * pTOC)
{
	fl_DocSectionLayout * pDSL = m_pFirstSection;
	fl_BlockLayout * pBlock = NULL;
	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pDSL);
	while(pCL && pCL->getContainerType() != FL_CONTAINER_BLOCK)
	{
		pCL = pCL->getFirstLayout();
	}
	if(pCL == NULL)
	{
		return false;
	}
	if(pCL->getContainerType() != FL_CONTAINER_BLOCK)
	{
		return false;
	}
	UT_UTF8String sStyle;
	pBlock = static_cast<fl_BlockLayout *>(pCL);
	bool filled = false;

	const XML_Char * pBookmark = pTOC->getRangeBookmarkName().size() ? pTOC->getRangeBookmarkName().utf8_str() : NULL;
	
	if(pBookmark)
	{
		if(m_pDoc->isBookmarkUnique(pBookmark))
		{
			// bookmark does not exist
			pBookmark = NULL;
		}
	}

	fl_BlockLayout * pBlockLast = NULL;
	
	if(pBookmark)
	{
		UT_uint32 i = 0;
		fp_BookmarkRun * pB[2] = {NULL,NULL};
		fp_Run * pRun;
		fl_BlockLayout * pBlockStart = pBlock;
		bool bFound = false;
		
		while(pBlock)
		{
			pRun = pBlock->getFirstRun();

			while(pRun)
			{
				if(pRun->getType()== FPRUN_BOOKMARK)
				{
					fp_BookmarkRun * pBR = static_cast<fp_BookmarkRun*>(pRun);
					if(!strcmp(pBR->getName(),pBookmark))
					{
						pB[i] = pBR;
						i++;
						if(i>1)
						{
							bFound = true;
							break;
						}
					}
				}

				pRun = pRun->getNextRun();
			}
			
			if(bFound)
				break;
			
			pBlock = pBlock->getNextBlockInDocument();
		}

		if(pB[0] && pB[1])
		{
			pBlockLast = pB[1]->getBlock();

			pBlock = pB[0]->getBlock();
			PT_DocPosition pos1 = pB[0]->getBookmarkedDocPosition(false);

			if(pBlock->getPosition(true) < pos1)
				pBlock = pBlock->getNextBlockInDocument();
		}
		else
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			pBlock = pBlockStart;
		}
	}

	// clear any existing contents
	pTOC->purgeLayout();

	while(pBlock)
	{
		pBlock->getStyle(sStyle);
		if(pTOC->isStyleInTOC(sStyle))
		{
			filled = true;
			pTOC->addBlock(pBlock, false);
		}

		if(pBlockLast && pBlockLast == pBlock)
			break;
		
		pBlock = pBlock->getNextBlockInDocument();
	}
	return filled;
}

/*
   updates affected TOCs in response to bookmark operation
   returns true if operation resulted in change, false otherwise
*/
bool FL_DocLayout::updateTOCsOnBookmarkChange(const XML_Char * pBookmark)
{
	UT_return_val_if_fail( pBookmark && !isLayoutFilling(), false );
	bool bChange = false;
	
	for(UT_sint32 i = 0; i < getNumTOCs(); ++i)
	{
		fl_TOCLayout * pTOC = getNthTOC(i);
		UT_return_val_if_fail( pTOC, false );

		if(pTOC->getRangeBookmarkName().size() && !UT_strcmp(pTOC->getRangeBookmarkName().utf8_str(), pBookmark))
		{
			// this TOC depends on the given bookmark, update ...
			fillTOC(pTOC);
			bChange = true;
		}
	}

	return bChange;
}


//------------------------------------------------------------------
UT_sint32 FL_DocLayout::getHeight()
{
	UT_sint32 iHeight = 0;
	int count = m_vecPages.getItemCount();

//
// restore when we support different page heights per document.
//
	for (int i=0; i<count; i++)
	{
		fp_Page* p = m_vecPages.getNthItem(i);

		iHeight += p->getHeight();
		if(getView() && (getView()->getViewMode() != VIEW_PRINT))
		{
			iHeight = iHeight - p->getOwningSection()->getTopMargin() - p->getOwningSection()->getBottomMargin();
		}
	}

	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		// add page view dimensions
		FV_View * pView = getView();
		if(pView)
		{
			iHeight += pView->getPageViewSep() * count; // Not count - 1, since we want a nice gray border at the very bottom of the document as well
			iHeight += pView->getPageViewTopMargin();
		}
		else
		{
			iHeight += fl_PAGEVIEW_PAGE_SEP * count; // Not count - 1, since we want a nice gray border at the very bottom of the document as well
			iHeight += fl_PAGEVIEW_MARGIN_Y;
		}
	}
	if(iHeight < 0)
	{
		iHeight = 0;
	}
	xxx_UT_DEBUGMSG(("returned height %d \n",iHeight));
	return iHeight;
}

UT_sint32 FL_DocLayout::getWidth()
{
	UT_sint32 iWidth = 0;
	int count = m_vecPages.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_Page* p = m_vecPages.getNthItem(i);

		// we layout pages vertically, so this is max, not sum
		if (static_cast<UT_sint32>(iWidth) < p->getWidth())
			iWidth = p->getWidth();
	}

	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		// add page view dimensions
		if(getView())
			iWidth += getView()->getPageViewLeftMargin();
		else
			iWidth += fl_PAGEVIEW_MARGIN_X * 2;
	}
	return iWidth;
}

const GR_Font* FL_DocLayout::findFont(const PP_AttrProp * pSpanAP,
									  const PP_AttrProp * pBlockAP,
									  const PP_AttrProp * pSectionAP,
									  bool isField)
{
	const char* pszFamily	= PP_evalProperty("font-family",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszField	= PP_evalProperty("field-font",NULL,pBlockAP,NULL, m_pDoc, true);
	const char* pszStyle	= PP_evalProperty("font-style",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszVariant	= PP_evalProperty("font-variant",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszWeight	= PP_evalProperty("font-weight",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszStretch	= PP_evalProperty("font-stretch",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszSize		= PP_evalProperty("font-size",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);
	const char* pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, m_pDoc, true);

	if (pszField != NULL && isField && UT_strcmp(pszField, "NULL"))
		pszFamily = pszField;

	// for superscripts and subscripts, we'll automatically shrink the font size
	if ((0 == UT_strcmp(pszPosition, "superscript")) ||
		(0 == UT_strcmp(pszPosition, "subscript")))
	{
		double newSize = UT_convertToPoints(pszSize) * 2.0 / 3.0;
		pszSize = UT_formatDimensionedValue(newSize,"pt",".0");
	}

	return m_pG->findFont(pszFamily, pszStyle, pszVariant, pszWeight, pszStretch, pszSize);
}

void FL_DocLayout::changeDocSections(const PX_ChangeRecord_StruxChange * pcrx, fl_DocSectionLayout * pDSL)
{
	fl_DocSectionLayout * pCur = pDSL;
	pDSL->doclistener_changeStrux(pcrx);
	while(pCur != NULL)
	{
		if(m_pDoc->isMarginChangeOnly())
		{
			pCur->doMarginChangeOnly();
		}
		else
		{
			pCur->collapse();
		}
		pCur = pCur->getNextDocSection();
	}
	if(m_pDoc->isMarginChangeOnly())
	{
		return;
	}
	pCur = pDSL;
	while(pCur != NULL)
	{
		pCur->updateDocSection();
		pCur = pCur->getNextDocSection();
	}
}


UT_uint32 FL_DocLayout::countPages()
{
	return m_vecPages.getItemCount();
}

UT_sint32 FL_DocLayout::findPage(fp_Page * pPage)
{
	UT_sint32 count = m_vecPages.getItemCount();
	if(count < 1)
	{
		return -1;
	}
	return m_vecPages.findItem(pPage);
}

fp_Page* FL_DocLayout::getNthPage(int n)
{
	UT_ASSERT(m_vecPages.getItemCount() > 0);

	return m_vecPages.getNthItem(n);
}

fp_Page* FL_DocLayout::getFirstPage()
{
	if (m_vecPages.getItemCount() == 0)
	{
		return NULL;
	}

	return m_vecPages.getNthItem(0);
}

fp_Page* FL_DocLayout::getLastPage()
{
	if (m_vecPages.getItemCount() == 0)
	{
		return NULL;
	}

	return m_vecPages.getNthItem(m_vecPages.getItemCount()-1);
}

void FL_DocLayout::deletePage(fp_Page* pPage, bool bDontNotify /* default false */)
{
	UT_sint32 ndx = m_vecPages.findItem(pPage);
	UT_ASSERT(ndx >= 0);

	if (pPage->getPrev())
	{
		pPage->getPrev()->setNext(pPage->getNext());
	}

	if (pPage->getNext())
	{
		pPage->getNext()->setPrev(pPage->getPrev());
	}
	pPage->setPrev(NULL);
	pPage->setNext(NULL);
	m_vecPages.deleteNthItem(ndx);
	delete pPage;

	// let the view know that we deleted a page,
	// so that it can update the scroll bar ranges
	// and whatever else it needs to do.
    //
    // Check for point > 0 to allow multi-threaded loads
    //
	if (m_pView && !bDontNotify && (m_pView->getPoint() > 0) && !m_pDoc->isPieceTableChanging())
	{
		m_pView->notifyListeners(AV_CHG_PAGECOUNT);
	}
}

fp_Page* FL_DocLayout::addNewPage(fl_DocSectionLayout* pOwner, bool bNoUpdate)
{
	fp_Page*		pLastPage;

	if (countPages() > 0)
	{
		pLastPage = getLastPage();
	}
	else
	{
		pLastPage = NULL;
	}

	fp_Page* pPage = new fp_Page(	this,
									m_pView,
									m_pDoc->m_docPageSize,
									pOwner);
	if (pLastPage)
	{
		UT_ASSERT(pLastPage->getNext() == NULL);

		pLastPage->setNext(pPage);
	}
	pPage->setPrev(pLastPage);
	m_vecPages.addItem(pPage);
	pOwner->addOwnedPage(pPage);

	// let the view know that we created a new page,
	// so that it can update the scroll bar ranges
	// and whatever else it needs to do.

	if (m_pView && m_pView->shouldScreenUpdateOnGeneralUpdate() && m_pView->getPoint() > 0 && !bNoUpdate) // skip this if rebuilding or if we're loading a document
	{
		m_pView->notifyListeners(AV_CHG_PAGECOUNT);
	}

	return pPage;
}

/*!
  Find block at document position
  \param pos Document position
  \return Block at specified posistion, or the first block to the
          rigth of that position. May return NULL.

*/
fl_BlockLayout* FL_DocLayout::findBlockAtPosition(PT_DocPosition pos) const
{
	fl_BlockLayout* pBL = NULL;
	PL_StruxFmtHandle sfh = 0;

	PT_DocPosition posEOD;
	bool bRes;
	xxx_UT_DEBUGMSG(("Pos at entry %d \n",pos));
	bRes = m_pDoc->getBounds(true, posEOD);
	UT_ASSERT(bRes);
	if(m_pDoc->isEndFootnoteAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("End footnote found at %d \n",pos));
		pos--;
	}
	if(m_pDoc->isFootnoteAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("Start footnote found at %d \n",pos));
		pos+=2;
	}
	if(m_pDoc->isFootnoteAtPos(pos-1))
	{
		xxx_UT_DEBUGMSG(("Start footnote found at %d \n",pos));
		pos+=1;
	}

	bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	// If block wasn't found at position, try finding it to the right,
	// limited only by the EOD.
	while(!bRes && (pos < posEOD))
	{
		pos++;
		bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	}

	if (bRes)
	{
		fl_Layout * pL = const_cast<fl_Layout *>(static_cast<const fl_Layout *>(sfh));
		if(!pL)
			return NULL;

		switch (pL->getType())
		{
		case PTX_Block:
			pBL = static_cast<fl_BlockLayout *>(pL);
			while(pBL && !pBL->canContainPoint())
			{
				pBL = pBL->getPrevBlockInDocument();
			}
				  
			break;

		case PTX_Section:
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// We asked for a block, and we got a section.  Bad
			return NULL;
		}
	}
	else
	{
		UT_ASSERT(0);
		return NULL;
	}

	UT_return_val_if_fail( pBL, NULL );
	
	fl_ContainerLayout * pMyC = pBL->myContainingLayout();
	while(pMyC && (pMyC->getContainerType() != FL_CONTAINER_DOCSECTION)
	      && (pMyC->getContainerType() != FL_CONTAINER_HDRFTR)
	      && (pMyC->getContainerType() != FL_CONTAINER_SHADOW))
	{
	  pMyC = pMyC->myContainingLayout();
	}
	if((pMyC->getContainerType() == FL_CONTAINER_HDRFTR)
	      || (pMyC->getContainerType() == FL_CONTAINER_SHADOW))
	{
		fl_HdrFtrShadow * pShadow = NULL;
		FV_View * pView = getView();
		if(pView && pView->isHdrFtrEdit())
		{
			pShadow = pView->getEditShadow();
//
// We might actually be in the other HdrFtr is the point got here from an undo!
// Check for this.
//
			if(!pShadow->getHdrFtrSectionLayout()->isPointInHere(pos))
			{
			        fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(pBL->getSectionLayout());
				while(pCL && pCL->getContainerType() != FL_CONTAINER_HDRFTR && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
				{
				  if(pCL == pCL->myContainingLayout())
				  {
				    break;
				  }
				  pCL = pCL->myContainingLayout();
				}
				fl_HdrFtrSectionLayout * pHF = NULL;
				if(pCL && pCL->getContainerType() == FL_CONTAINER_HDRFTR)
				{ 
				     pHF = static_cast<fl_HdrFtrSectionLayout *>(pCL);
				}
				if(pHF && pHF->isPointInHere(pos))
				{
					pShadow = pHF->getFirstShadow();
					if(pShadow)
					{
						pView->clearHdrFtrEdit();
						pView->setHdrFtrEdit(pShadow);
						pBL = static_cast<fl_BlockLayout *>(pShadow->findBlockAtPosition(pos));
						return pBL;
					}
					else
					{
						return NULL;
					}
				}
				// Ok, we're really confused now, point is nowhere to be found.
				// It might be OK if pos-1 is in here, though...
				if (pShadow && !pShadow->getHdrFtrSectionLayout()->isPointInHere(pos-1))
				{
					//			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
			}

		}
		else if(pMyC->getContainerType() == FL_CONTAINER_SHADOW)
		{
		        pShadow = static_cast<fl_HdrFtrShadow *>(pMyC);
		}
		else
		{
			pShadow = static_cast<fl_HdrFtrSectionLayout *>(pMyC)->getFirstShadow();
		}
		fl_BlockLayout * ppBL = NULL;
		if(pShadow != NULL)
		{
			ppBL = static_cast<fl_BlockLayout *>(pShadow->findMatchingContainer(pBL));
		}
		else
		{
			UT_DEBUGMSG(("No Shadow! But there should be ! \n"));
			//	UT_ASSERT(0);
		}
//
// FIXME: Header/Footers
// some failsafe code should not trigger. Header/footer still not perfect.
//
		if(!ppBL)
		{
			if(!isLayoutFilling())
			{
				//			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
		else
		{
			pBL = ppBL;
		}
	}
	UT_ASSERT(pBL);
	return pBL;
}

fl_BlockLayout* FL_DocLayout::findBlockAtPositionReverse(PT_DocPosition pos)
{
	fl_BlockLayout* pBL = NULL;
	PL_StruxFmtHandle sfh = 0;

	PT_DocPosition posBOD;
	bool bRes;

	bRes = m_pDoc->getBounds(false, posBOD);
	UT_ASSERT(bRes);
	if(m_pDoc->isEndFootnoteAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("End footnote found at %d \n",pos));
		pos--;
	}
	if(m_pDoc->isFootnoteAtPos(pos))
	{
		xxx_UT_DEBUGMSG(("Start footnote found at %d \n",pos));
		pos+=2;
	}
	bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	// If block wasn't found at position, try finding it to the right,
	// limited only by the EOD.
	while(!bRes && (pos > posBOD))
	{
		pos--;
		bRes = m_pDoc->getStruxOfTypeFromPosition(m_lid, pos, PTX_Block, &sfh);
	}

	if (bRes)
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		if(!pL)
			return NULL;

		switch (pL->getType())
		{
		case PTX_Block:
			pBL = static_cast<fl_BlockLayout *>(pL);
			break;

		case PTX_Section:
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// We asked for a block, and we got a section.  Bad
			return NULL;
		}
	}
	else
	{
		UT_ASSERT(0);
		return NULL;
	}

	if(pBL->getSectionLayout()->getType() == FL_SECTION_HDRFTR)
	{
		fl_HdrFtrShadow * pShadow = NULL;
		FV_View * pView = getView();
		if(pView && pView->isHdrFtrEdit())
		{
			pShadow = pView->getEditShadow();
//
// We might actually be in the other HdrFtr is the point got here from an undo!
// Check for this.
//
			if(!pShadow->getHdrFtrSectionLayout()->isPointInHere(pos))
			{
				fl_HdrFtrSectionLayout * pHF = (fl_HdrFtrSectionLayout *) pBL->getSectionLayout();
				if(pHF->isPointInHere(pos))
				{
					pShadow = pHF->getFirstShadow();
					pView->clearHdrFtrEdit();
					pView->setHdrFtrEdit(pShadow);
					pBL = (fl_BlockLayout *) pShadow->findBlockAtPosition(pos);
					return pBL;
				}
				// Ok, we're really confused now, point is nowhere to be found.
				// It might be OK if pos-1 is in here, though...
				if (!pShadow->getHdrFtrSectionLayout()->isPointInHere(pos-1))
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				}
			}

		}
		else
		{
			pShadow = ((fl_HdrFtrSectionLayout *) pBL->getSectionLayout())->getFirstShadow();
		}
		fl_BlockLayout * ppBL = NULL;
		if(pShadow != NULL)
			ppBL = (fl_BlockLayout *) pShadow->findMatchingContainer(pBL);
		else
		{
			if(!isLayoutFilling())
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
		
		if(ppBL) {
			pBL = ppBL;
			}
	}
	UT_ASSERT(pBL);
	return pBL;
}

void FL_DocLayout::deleteEmptyColumnsAndPages(void)
{
	fl_DocSectionLayout* pSL = m_pFirstSection;
	while (pSL)
	{
		pSL->deleteEmptyColumns();
		pSL = pSL->getNextDocSection();
	}

	deleteEmptyPages();
}

void FL_DocLayout::deleteEmptyPages( bool bDontNotify /* default false */)
{
	int i;

	int iCountPages = m_vecPages.getItemCount();
	for (i=iCountPages - 1; i>=0; i--)
	{
		fp_Page* p = m_vecPages.getNthItem(i);
		UT_ASSERT(p);
		if (p && p->isEmpty())
		{
			deletePage(p, bDontNotify);
		}
	}
}

void FL_DocLayout::formatAll()
{
	UT_ASSERT(m_pDoc);
	m_pDoc->enableListUpdates();
	fl_SectionLayout* pSL = m_pFirstSection;
//  	while (pSL)
//  	{
//  		if(pSL->getType() == FL_SECTION_DOC)
//  		{
//  			static_cast<fl_DocSectionLayout *>(pSL)->collapseDocSection();
//  		}
//  		pSL = pSL->getNext();
//  	}
	while (pSL)
	{
		pSL->format();
		if(pSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			static_cast<fl_DocSectionLayout *>(pSL)->completeBreakSection();
			static_cast<fl_DocSectionLayout *>(pSL)->checkAndRemovePages();
		}
		pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
	}
}


void FL_DocLayout::rebuildFromHere( fl_DocSectionLayout * pFirstDSL)
{
	UT_ASSERT(m_pDoc);
	if(isLayoutFilling())
	{
//		UT_ASSERT(0);
		return;
	}
	if(m_pDoc->isMarginChangeOnly())
	{
		return;
	}
//
	fl_DocSectionLayout * pStart = pFirstDSL;
//	fl_DocSectionLayout * pStart = pFirstDSL->getPrevDocSection();
//	if(pStart == NULL)
//	{
//		pStart = pFirstDSL;
//	}
	fl_DocSectionLayout * pDSL = pStart;
	// add page view dimensions
#if 1
	UT_DEBUGMSG(("SEVIOR: Rebuild from section %x \n",pFirstDSL));
	for(UT_uint32 k=0; k< m_vecPages.getItemCount(); k++)
	{
		fp_Page * pPage = m_vecPages.getNthItem(k);
		if(pPage->getOwningSection() == pFirstDSL)
		{
			UT_DEBUGMSG(("SEVIOR: Rebuilding from page %d \n",k));
			break;
		}
	}
#endif
	while (pDSL)
	{
		pDSL->collapse();
		pDSL = pDSL->getNextDocSection();
	}
	deleteEmptyColumnsAndPages();
//
// Clear out rebuild marks from this collapse
//
	pDSL = static_cast<fl_DocSectionLayout *>(m_pFirstSection);
	while(pDSL)
	{
		pDSL->clearRebuild();
		pDSL = pDSL->getNextDocSection();
	}

	deleteEmptyColumnsAndPages();
	pDSL= pStart;
	while (pDSL)
	{
		UT_DEBUGMSG(("SEVIOR: Building section %x \n",pDSL));
		pDSL->updateDocSection();
		pDSL->clearRebuild();
		pDSL = pDSL->getNextDocSection();
	}
//
// Clear out rebuild marks from the rebuild
//
	pDSL = static_cast<fl_DocSectionLayout *>(m_pFirstSection);
	while(pDSL)
	{
		pDSL->clearRebuild();
		pDSL = pDSL->getNextDocSection();
	}
}

void FL_DocLayout::updateLayout()
{
	/*
	  TODO the following routine checks every paragraph in the
	  document to see if it needs a reformat.  How is this going
	  to perform on a 50-page document?


	  Very good point. We need a isOnScreen() method!!!
	*/
	UT_ASSERT(m_pDoc);

	fl_SectionLayout* pSL = m_pFirstSection;
	while (pSL)
	{
		pSL->updateLayout(false);
		if(pSL->getType() == FL_SECTION_DOC)
		{
			if(static_cast<fl_DocSectionLayout *>(pSL)->needsRebuild())
			{
				break;
			}
		}
		pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
	}
	if(pSL == NULL)
	{
		deleteEmptyColumnsAndPages();
		return;
	}
	if(m_pDoc->isPieceTableChanging())
	{
		static_cast<fl_DocSectionLayout *>(pSL)->clearRebuild();
		return;
	}
	rebuildFromHere(static_cast<fl_DocSectionLayout *>(pSL));
}


void FL_DocLayout::updateColor()
{
	UT_ASSERT(m_pDoc);
	FV_View * pView = getView();
	if(pView)
	{
		XAP_App * pApp = pView->getApp();
		XAP_Prefs * pPrefs = pApp->getPrefs();
		const XML_Char * pszTransparentColor = NULL;
		pPrefs->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForTransparent),&pszTransparentColor);
//
// Save the new preference color
//
		strncpy(m_szCurrentTransparentColor,pszTransparentColor,9);
	}
//
// Now loop through the document and update the Background color
//
	fl_DocSectionLayout* pDSL = static_cast<fl_DocSectionLayout *>(m_pFirstSection);
	while (pDSL)
	{
		pDSL->setPaperColor();
		pDSL = pDSL->getNextDocSection();
	}
	fp_Page * pPage = NULL;
	UT_uint32 i =0;
	for(i=0; i<m_vecPages.getItemCount(); i++)
	{
		pPage = m_vecPages.getNthItem(i);
		pPage->getFillType()->setTransColor(m_szCurrentTransparentColor);
		pPage->getFillType()->markTransparentForPrint();
	}

//
// Redraw the view associated with this document.
//
	if(pView)
	{
		pView->updateScreen(false);
	}

}

#define BACKGROUND_CHECK_MSECS 100

/*!
 Toggle auto spell-checking state
 \param bSpell True if spell-checking should be enabled, false otherwise
 When disabling spelling, all squiggles are deleted.
 When enabling spelling, force a full check of the document.
*/
void
FL_DocLayout::_toggleAutoSpell(bool bSpell)
{
	bool bOldAutoSpell = getAutoSpellCheck();
	UT_DEBUGMSG(("_toggleAutoSpell %d \n",bSpell));
	// Add reason to background checker
	if (bSpell)
	{
		UT_DEBUGMSG(("Adding Auto SpellCheck  \n"));
		addBackgroundCheckReason(bgcrSpelling);
	}
	else
	{
		UT_DEBUGMSG(("Removing Auto SpellCheck  \n"));
		removeBackgroundCheckReason(bgcrSpelling);
	}

	xxx_UT_DEBUGMSG(("FL_DocLayout::_toggleAutoSpell (%s)\n",
					 bSpell ? "true" : "false" ));

	if (bSpell)
	{
		UT_DEBUGMSG(("Rechecking spelling in blocks \n"));
		queueAll(bgcrSpelling);
	}
	else
	{
		// Disabling, so remove the squiggles too
		fl_DocSectionLayout * pSL = getFirstSection();
		if(pSL)
		{
			fl_ContainerLayout* b = pSL->getFirstLayout();
			while (b)
			{
				if(b->getContainerType() == FL_CONTAINER_BLOCK)
				{
					static_cast<fl_BlockLayout *>(b)->removeBackgroundCheckReason(bgcrSpelling);
					static_cast<fl_BlockLayout *>(b)->getSpellSquiggles()->deleteAll();
					b = static_cast<fl_BlockLayout *>(b)->getNextBlockInDocument();
				}
				else
				{
					b = b->getNext();
				}
			}
		}
		if (bOldAutoSpell)
		{
			// If we're here, it was set to TRUE before but now it is
			// being set to FALSE. This means that it is the user
			// setting it. That's good.
			m_pView->draw(NULL);
			// A pending word would be bad. Not sure why it's not
			// ignored once autospell is off, but for now it should
			// definitely be annulled.
			setPendingWordForSpell(NULL, NULL);
		}
	}
}


/*!
 Toggle auto spell-checking state
 \param bGrammar True if grammar-checking should be enabled, false otherwise
 When disabling grammar checking, all squiggles are deleted.
 When enabling grammar, force a full check of the document.
*/
void
FL_DocLayout::_toggleAutoGrammar(bool bGrammar)
{
	bool bOldAutoGrammar = getAutoGrammarCheck();
	UT_DEBUGMSG(("_toggleAutoGrammar %d \n",bGrammar));
	// Add reason to background checker
	if (bGrammar)
	{
		UT_DEBUGMSG(("Adding Auto GrammarCheck  \n"));
		addBackgroundCheckReason(bgcrGrammar);
		m_bAutoGrammarCheck = true;
	}
	else
	{
		UT_DEBUGMSG(("Removing Auto Grammar  \n"));
		removeBackgroundCheckReason(bgcrGrammar);
		m_bAutoGrammarCheck = false;
	}

	xxx_UT_DEBUGMSG(("FL_DocLayout::_toggleAutoGrammar (%s)\n",
					 bGrammar ? "true" : "false" ));

	if (bGrammar)
	{
		UT_DEBUGMSG(("Rechecking Grammar in blocks \n"));
		queueAll(bgcrGrammar);
	}
	else
	{
		// Disabling, so remove the squiggles too
		fl_DocSectionLayout * pSL = getFirstSection();
		if(pSL)
		{
			fl_ContainerLayout* b = pSL->getFirstLayout();
			while (b)
			{
				if(b->getContainerType() == FL_CONTAINER_BLOCK)
				{
					static_cast<fl_BlockLayout *>(b)->removeBackgroundCheckReason(bgcrGrammar);
					static_cast<fl_BlockLayout *>(b)->getGrammarSquiggles()->deleteAll();
					b = static_cast<fl_BlockLayout *>(b)->getNextBlockInDocument();
				}
				else
				{
					b = b->getNext();
				}
			}
		}
		if (bOldAutoGrammar)
		{
			// If we're here, it was set to TRUE before but now it is
			// being set to FALSE. This means that it is the user
			// setting it. That's good.
			m_pView->draw(NULL);
		}
	}
}

void FL_DocLayout::_toggleAutoSmartQuotes(bool bSQ)
{
	setPendingSmartQuote(NULL, 0);  // avoid surprises
	if (bSQ)
	{
		addBackgroundCheckReason(bgcrSmartQuotes);
	}
	else
	{
		removeBackgroundCheckReason(bgcrSmartQuotes);
	}

	UT_DEBUGMSG(("FL_DocLayout::_toggleAutoSmartQuotes(%s)\n", bSQ ? "true" : "false" ));
}

/*!
 Do background spell-check
 \param pWorker Worker object
 \note This is a static callback method and does not have a 'this' pointer.
*/
void
FL_DocLayout::_backgroundCheck(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);

	// Get the doclayout
	FL_DocLayout * pDocLayout = static_cast<FL_DocLayout *>(pWorker->getInstanceData());
	UT_ASSERT(pDocLayout);

	// Win32 timers can fire prematurely on asserts (the dialog's
	// message pump releases the timers)
	if (!pDocLayout->m_pView)
	{
		return;
	}

//
// Don't redraw on selections.
//
//	if (!pDocLayout->m_pView->isSelectionEmpty())
//	{
//		return;
//	}
	xxx_UT_DEBUGMSG(("BAckground check called. \n"));
	// Don't spell check while printing!
	if(pDocLayout->m_pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		return;
	}

	// Don't spell check if disabled, or already happening
	if(pDocLayout->m_bStopSpellChecking || pDocLayout->m_bImSpellCheckingNow || pDocLayout->isLayoutFilling())
	{
	  xxx_UT_DEBUGMSG(("Already spellchecking!!!!!!!! \n"));
		return;
	}

	// Code added to hold spell checks during block insertions
	if(pDocLayout->m_pDoc->isPieceTableChanging())
	{
		return;
	}

	// Don't spell check while a redrawupdate is happening either...
	PD_Document * pDoc = pDocLayout->getDocument();
	if(pDoc->isRedrawHappenning())
	{
		return;
	}

	// Flag that spell checking is in action.
	// Note: this is not a good way to do mutual exclusion!
	pDocLayout->m_bImSpellCheckingNow = true;

	// Find vector of blocks to check.
	UT_GenericVector<fl_BlockLayout *>* vecToCheck = &pDocLayout->m_vecUncheckedBlocks;
	UT_ASSERT(vecToCheck);

	UT_uint32 i = vecToCheck->getItemCount();
	if (i > 0)
	{
		// Check each block in the queue
		fl_BlockLayout *pB = vecToCheck->getFirstItem();
		xxx_UT_DEBUGMSG(("Spellchecking block %x \n",pB));
		if (pB != NULL)
		{
			// This looping seems like a lot of wasted effort when we
			// don't define meaning for most of the bits, but it's
			// small effort compared to all that squiggle stuff that
			// goes on for the spelling stuff.
			if(pB->getContainerType() == FL_CONTAINER_BLOCK)
			{
				for (UT_uint32 bitdex = 0;
					 bitdex < 8*sizeof(pB->m_uBackgroundCheckReasons);
					 bitdex++)
				{
					UT_uint32 mask;
					mask = (1 << bitdex);
					if (pB->hasBackgroundCheckReason(mask))
					{
					        if(!pDocLayout->m_bFinishedInitialCheck && pDocLayout->m_iPrevPos > pB->getPosition())
						{
						     pDocLayout->m_bFinishedInitialCheck = true;
						}
						pDocLayout->m_iPrevPos = pB->getPosition();

					// Note that we remove this reason from queue
					// before checking it (otherwise asserts could
					// trigger redundant recursive calls)
						switch (mask)
						{
						case bgcrNone:
							pB->removeBackgroundCheckReason(mask);
							break;
						case bgcrDebugFlash:
							pB->debugFlashing();
							pB->removeBackgroundCheckReason(mask);
							break;
						case bgcrSpelling:
						{
							xxx_UT_DEBUGMSG(("Spelling checking block %x directly \n",pB));
							bool b = pB->checkSpelling();
							if(b)
							{
								pB->removeBackgroundCheckReason(mask);
							}
							break;
						}
						case bgcrGrammar:
						{
						        if(!pDocLayout->m_bFinishedInitialCheck)
							{
							      if(pDocLayout->m_iGrammarCount < 4)
							      {
								   pDocLayout->m_iGrammarCount++;
								   pDocLayout->m_bImSpellCheckingNow = false;
								   return;
							      }
							      pDocLayout->m_iGrammarCount = 0;
							}

							UT_DEBUGMSG(("Grammar checking block %x directly \n",pB));
							XAP_App * pApp = pDocLayout->getView()->getApp();
     //
     // If a grammar checker plugin is loaded it will check the block now.
     //
							pApp->notifyListeners(pDocLayout->getView(),AV_CHG_BLOCKCHECK,reinterpret_cast<void *>(pB));
							pB->removeBackgroundCheckReason(mask);
							pB->drawGrammarSquiggles();
							break;
						}

						case bgcrSmartQuotes:
						default:
							pB->removeBackgroundCheckReason(mask);
							break;
						}
					}
				}
			}
			// Delete block from queue if there are no more reasons
			// for checking it.
			if(pB->getContainerType() != FL_CONTAINER_BLOCK)
			{
				vecToCheck->deleteNthItem(0);
				i--;
			}
			else if (!pB->m_uBackgroundCheckReasons)
			{
				vecToCheck->deleteNthItem(0);
				i--;
			}
		}
	}
	else
	{
		// No blocks to spellcheck so stop the idle/timer. Otherwise
		// we consume 100% CPU.
		pDocLayout->m_pBackgroundCheckTimer->stop();
	}

	pDocLayout->m_bImSpellCheckingNow = false;
}

/*!
 Enqueue block for background spell-checking
 \param iReason Reason for checking the block FIXME - enum?
 \param pBlock Block to enqueue
 \param bHead When true, insert block at head of queue

 This routine queues up blocks for timer-driven spell checking, etc.
 By default, this is a FIFO queue, but it can be explicitly
 reprioritized by setting bHead to true.
*/
void
FL_DocLayout::queueBlockForBackgroundCheck(UT_uint32 iReason,
										   fl_BlockLayout *pBlock,
										   bool bHead)
{
	// If there's no timer running, start one
	if (!m_pBackgroundCheckTimer)
	{
	    int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	    if(getView() && getView()->isGrammarLoaded() && m_bAutoGrammarCheck)
	    {
	         inMode = UT_WorkerFactory::TIMER;
	    }
	    UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	    
	    m_pBackgroundCheckTimer = UT_WorkerFactory::static_constructor (_backgroundCheck, this, inMode, outMode, m_pG);

	    UT_ASSERT(m_pBackgroundCheckTimer);
	    UT_ASSERT(outMode != UT_WorkerFactory::NONE);

		// If the worker is working on a timer instead of in the idle
		// time, set the frequency of the checks.
	    if ( UT_WorkerFactory::TIMER == outMode )
		{
			// this is really a timer, so it's safe to static_cast it
			static_cast<UT_Timer*>(m_pBackgroundCheckTimer)->set(BACKGROUND_CHECK_MSECS);
		}
#if 1
	    m_bStopSpellChecking = false;
	    m_pBackgroundCheckTimer->start();
#endif

	}
#if 1 // We need this to restart the idle handler.
	else
	{
		//		m_pBackgroundCheckTimer->stop();
		m_bStopSpellChecking = false;
		m_pBackgroundCheckTimer->start();
	}
#endif

	// Set debug flash reason on block if it is set
	if (hasBackgroundCheckReason(bgcrDebugFlash))
	{
		pBlock->addBackgroundCheckReason(bgcrDebugFlash);
	}
	pBlock->addBackgroundCheckReason(iReason);

	UT_sint32 i = m_vecUncheckedBlocks.findItem(pBlock);
	if (i < 0)
	{
		// Add block if it's not already in the queue. Add it either
		// at the head, or at the tail.
		if (bHead)
			m_vecUncheckedBlocks.insertItemAt(pBlock, 0);
		else
			m_vecUncheckedBlocks.addItem(pBlock);
	}
	else if (bHead)
	{
		// Block is already in the queue, bubble it to the start
		m_vecUncheckedBlocks.deleteNthItem(i);
		m_vecUncheckedBlocks.insertItemAt(pBlock, 0);
	}
}

void FL_DocLayout::dequeueAll(void)
{
	UT_sint32 i =0;
	for(i= m_vecUncheckedBlocks.getItemCount()-1; i>= 0; i--)
	{
		m_vecUncheckedBlocks.deleteNthItem(i);	
	}
	UT_DEBUGMSG(("Dequeue all \n"));

	m_PendingBlockForGrammar = NULL;
	m_bStopSpellChecking = true;
	if(m_pBackgroundCheckTimer)
	{
		m_pBackgroundCheckTimer->stop();
		// Wait for checking to complete before returning.
		while(m_bImSpellCheckingNow == true)
		{
			// TODO shouldn't we have a little sleep here?
		}
	}
}

void FL_DocLayout::queueAll(UT_uint32 iReason)
{
	fl_DocSectionLayout * pSL = getFirstSection();
	if(pSL)
	{
		// We will place the block that contains the ins point and its immediate neigbours
		// at the top of the queue, this will make the check look faster to the user
		FV_View * pView = getView();
		UT_GenericVector<fl_BlockLayout*> vBL;
		const UT_sint32 iLimit = 5;
		
		fl_BlockLayout * pCurBL = pView->getBlockAtPosition(pView->getPoint());

		if(pCurBL)
		{
			fl_BlockLayout * pBL = pCurBL;

			UT_sint32 i = 0;
			for(i = 0; i < iLimit/2 + iLimit%2 && pBL; ++i, pBL = pBL->getPrevBlockInDocument())
			{
				vBL.addItem(pBL);
			}

			pBL = pCurBL->getNextBlockInDocument();
			for(i = iLimit/2 + iLimit%2; i < iLimit && pBL; ++i, pBL = pBL->getNextBlockInDocument())
			{
				vBL.addItem(pBL);
			}
		}
		
		fl_ContainerLayout* b = pSL->getFirstLayout();
		while (b)
		{
			// TODO: just check and remove matching squiggles
			// for now, destructively recheck the whole thing
			if(b->getContainerType() == FL_CONTAINER_BLOCK)
			{
				bool bHead = (vBL.findItem(static_cast<fl_BlockLayout *>(b)) >= 0);
				queueBlockForBackgroundCheck(iReason, static_cast<fl_BlockLayout *>(b), bHead);
				b = static_cast<fl_BlockLayout *>(b)->getNextBlockInDocument();
			}
			else
			{
				b = b->getNext();
			}
		}
	}
}

/*!
 * Set the next block to be grammar checked. It won't actually get checked
 * until the insertPoint leaves this block.
 */
void FL_DocLayout::setPendingBlockForGrammar(fl_BlockLayout * pBL)
{
  xxx_UT_DEBUGMSG(("Pending called with block %x pending %x \n",pBL,m_PendingBlockForGrammar));
  if(!m_bAutoGrammarCheck)
    return;
  if((m_PendingBlockForGrammar != NULL) && (m_PendingBlockForGrammar != pBL))
    {
      xxx_UT_DEBUGMSG(("Block %x queued \n",m_PendingBlockForGrammar));
      queueBlockForBackgroundCheck(bgcrGrammar,m_PendingBlockForGrammar,true);
    }
  m_PendingBlockForGrammar = pBL;
}


/*!
 * This is called from fv_View::_fixPointCoords to actually queue a grammar 
 * check a pending block.
 */
void FL_DocLayout::triggerPendingBlock(fl_BlockLayout * pBL)
{
  xxx_UT_DEBUGMSG(("Trigger called with block %x pending %x \n",pBL,m_PendingBlockForGrammar));
  if(!m_bAutoGrammarCheck)
    return;
  if((m_PendingBlockForGrammar != NULL) && (m_PendingBlockForGrammar != pBL))
    {
      queueBlockForBackgroundCheck(bgcrGrammar,m_PendingBlockForGrammar,true);
      m_PendingBlockForGrammar = NULL;
     }
}

/*!
 Remove block from background checking queue
 \param pBlock Block to remove

 When the last block is removed from the queue, the background timer
 is stopped. The function does not return before the background
 spell-checking timer has stopped.
*/
bool
FL_DocLayout::dequeueBlockForBackgroundCheck(fl_BlockLayout *pBlock)
{
	bool bRes = false;

	// Remove block from queue if it's found there
	UT_sint32 i = m_vecUncheckedBlocks.findItem(pBlock);
	if (i >= 0)
	{
		m_vecUncheckedBlocks.deleteNthItem(i);
		bRes = true;
	}
	if(pBlock == m_PendingBlockForGrammar)
	  {
	    xxx_UT_DEBUGMSG(("Dequeue block %x in dequeue \n",pBlock));
	    m_PendingBlockForGrammar = NULL;
	  }
	// When queue is empty, kill timer
	if (m_vecUncheckedBlocks.getItemCount() == 0)
	{
		m_bStopSpellChecking = true;
		if(m_pBackgroundCheckTimer)
		{
			m_pBackgroundCheckTimer->stop();
			// Wait for checking to complete before returning.
			while(m_bImSpellCheckingNow == true)
			{
				// TODO shouldn't we have a little sleep here?
			}
		}
	}

	return bRes;
}

/*!
  Mark a region of a block to be spell checked
  \param pBlock Block
  \param pWord  Region

  If called with NULL arguments, any prior marked region will be
  freed. Callers must reuse pWord (by calling getPendingWordForSpell)
  when set.
*/
void
FL_DocLayout::setPendingWordForSpell(fl_BlockLayout *pBlock,
									 fl_PartOfBlock* pWord)
{
	// Return if matching the existing marked region
	if ((pBlock == m_pPendingBlockForSpell) &&
		(pWord == m_pPendingWordForSpell))
		return;

	// Assert an existing pWord allocation is reused
	UT_ASSERT(!m_pPendingBlockForSpell || !pBlock
			  || m_pPendingWordForSpell == pWord);

	// Check for valid arguments
	if (pBlock && m_pPendingBlockForSpell && m_pPendingWordForSpell)
	{
		UT_ASSERT(pWord);
	}

	if (m_pPendingWordForSpell && (m_pPendingWordForSpell != pWord))
	{
		// When clobbering prior POB, make sure we don't leak it
		DELETEP(m_pPendingWordForSpell);
	}

	m_pPendingBlockForSpell = pBlock;
	m_pPendingWordForSpell = pWord;
}

/*!
 Spell-check pending word
 \result True if word checked, false otherwise
 If a word is pending, spell-check it.

 \note This function used to exit if PT was changing - but that
       prevents proper squiggle behavior during undo, so the check has
       been removed. This means that the pending word POB must be
       updated to reflect the PT changes before the IP is moved.
*/
bool
FL_DocLayout::checkPendingWordForSpell(void)
{
	// do not attempt to check a word if check is already in progress (see 7197)
	if(m_bSpellCheckInProgress)
		return false;

	bool bUpdate = false;

	xxx_UT_DEBUGMSG(("FL_DocLayout::checkPendingWordForSpell\n"));

	if (!m_pPendingBlockForSpell)
		return bUpdate;

	m_bSpellCheckInProgress = true;
	
	// Check pending word
	UT_ASSERT(m_pPendingWordForSpell);
	bUpdate = m_pPendingBlockForSpell->checkWord(m_pPendingWordForSpell);

	m_pPendingWordForSpell = NULL;	// NB: already freed by checkWord

	// Not pending any more
	setPendingWordForSpell(NULL, NULL);

	m_bSpellCheckInProgress = false;
	
	return bUpdate;
}

/*!
 Is a word pending for spelling
 \return True if a word is pending, false otherwise
*/
bool
FL_DocLayout::isPendingWordForSpell(void) const
{
	return (m_pPendingBlockForSpell ? true : false);
}

/*!
 Determine if position touches the pending word for spelling
 \param pBLock Block of position
 \param iOffset Offset in block
 \param chg  FIXME
 \return True if position touches pending word, false otherwise

 FIXME: why this function/chg? Caller uses result for what?
*/
bool
FL_DocLayout::touchesPendingWordForSpell(fl_BlockLayout *pBlock,
										 UT_sint32 iOffset,
										 UT_sint32 chg) const
{
	UT_uint32 len = (chg < 0) ? -chg : 0;

	UT_ASSERT(pBlock);

	if (!m_pPendingBlockForSpell)
		return false;

	// Are we in the same block?
	if (m_pPendingBlockForSpell != pBlock)
		return false;

	UT_ASSERT(m_pPendingWordForSpell);

	return m_pPendingWordForSpell->doesTouch(iOffset, len);
}

/*!
 * This method appends a DocSectionLayout onto the linked list of SectionLayout's
 * and updates the m_pFirstSection and m_pLastSection member variables
 * accordingly.
 * The structure of this linked list is as follows.
 *    pDSL->pDSL->....pDSL->pEndnoteSL->pHdrFtrSL->pHdrFtrSL->NULL
 *     ^                ^
 *m_pFirstSection   m_pLastSection
 *ie we have all the DocSections in a linked list followed by all the Header/
 * Footer sections. This reflects the locations in the piece table where
 * the header/footer sections are located at the end of the document.
\param  fl_DocSectionLayout * pSL the DocSectionLayout to be appended.
\param  fl_DocSectionLayout* pAfter the DocSectionLayout after which our new
        DocSectionLayout is inserted.
*/
void FL_DocLayout::addSection(fl_DocSectionLayout* pSL)
{
	if (m_pLastSection)
	{
		UT_ASSERT(m_pFirstSection);
		insertSectionAfter(m_pLastSection,pSL);
	}
	else
	{
		pSL->setPrev(NULL);
		pSL->setNext(NULL);
		m_pFirstSection = pSL;
		m_pLastSection = m_pFirstSection;
	}
}

/*!
 * This method inserts a DocSectionLayout into the linked list of SectionLayout's
 * and updates the m_pFirstSection and m_pLastSection member variables
 * accordingly
\param  fl_DocSectionLayout * pNewSL the DocSectionLayout to be inserted.
\param  fl_DocSectionLayout* pAfter the DocSectionLayout after which our new
        DocSectionLayout is inserted.
*/
void FL_DocLayout::insertSectionAfter(fl_DocSectionLayout* pAfter, fl_DocSectionLayout* pNewSL)
{
	pNewSL->setNext(pAfter->getNext());
	pNewSL->setPrev(pAfter);
	if (pAfter->getNext())
	{
		pAfter->getNext()->setPrev(pNewSL);
	}
	pAfter->setNext(pNewSL);

	if (m_pLastSection == pAfter)
	{
		m_pLastSection = pNewSL;
	}
}

/*!
 * This method removes a DocSectionLayout from the linked list of SectionLayout's
 * and updates the m_pFirstSection and m_pLastSection member variables
 * accordingly
\param  fl_DocSectionLayout * pSL the DocSectionLayout to be removed.
*/

void FL_DocLayout::removeSection(fl_DocSectionLayout * pSL)
{
	UT_ASSERT(pSL);
	UT_ASSERT(m_pFirstSection);

	if (pSL->getPrev())
	{
		pSL->getPrev()->setNext(pSL->getNext());
	}

	if (pSL->getNext())
	{
		pSL->getNext()->setPrev(pSL->getPrev());
	}

	if (pSL == m_pFirstSection)
	{
		m_pFirstSection = m_pFirstSection->getNextDocSection();
		if (!m_pFirstSection)
		{
			m_pLastSection = NULL;
		}
	}

	if (pSL == m_pLastSection)
	{
		m_pLastSection = m_pLastSection->getPrevDocSection();
		if (!m_pLastSection)
		{
			m_pFirstSection = NULL;
		}
	}

	pSL->setNext(NULL);
	pSL->setPrev(NULL);
}

/*!
 * Include the header/footer section layouts AFTER the last DocSection in the
 * the getNext, getPrev list. This will ensure that the headers/footers will be
 * formatted and updated correctly.
 \param fl_SectionLayout * pHdrFtrSL the header/footer layout to be inserted
        into the sectionlayout linked list.
 * The structure of this linked list is as follows.
 *    pDSL->pDSL->pDSL....pDSL->pEndnoteSL->pHdrFtrSL->pHdrFtrSL->NULL
 *     ^                   ^
 *m_pFirstSection   m_pLastSection
 *
 *ie we have all the DocSections in a linked list followed by all the Header/
 * Footer sections. This reflects the locations in the piece table where
 * the header/footer sections are located at the end of the document.
*/
void FL_DocLayout::addHdrFtrSection(fl_SectionLayout* pHdrFtrSL)
{
	UT_ASSERT(m_pLastSection);

	fl_SectionLayout * pLSL = static_cast<fl_SectionLayout *>(m_pLastSection);
	fl_SectionLayout * pnext = static_cast<fl_SectionLayout *>(pLSL->getNext());

	while (pnext && pnext->getType() == FL_SECTION_ENDNOTE)
		pnext = static_cast<fl_SectionLayout *>(pnext->getNext());

	if(pnext)
	{
		pnext->setPrev(pHdrFtrSL);
		pLSL->setNext(pHdrFtrSL);
		pHdrFtrSL->setPrev(pLSL);
		pHdrFtrSL->setNext(pnext);
	}
	else
	{
		pLSL->setNext(pHdrFtrSL);
		pHdrFtrSL->setPrev(pLSL);
		pHdrFtrSL->setNext(pnext);
	}
}

/*!
 *  This method removes a header/footer layout from the section linked list.
 \param fl_SectionLayout * pHdrFtrSL is the header/footer section to be removed
*/
void FL_DocLayout::removeHdrFtrSection(fl_SectionLayout * pHdrFtrSL)
{
	UT_ASSERT(pHdrFtrSL);

	if(pHdrFtrSL->getPrev())
	{
		pHdrFtrSL->getPrev()->setNext(pHdrFtrSL->getNext());
	}
	if (pHdrFtrSL->getNext())
	{
		pHdrFtrSL->getNext()->setPrev(pHdrFtrSL->getPrev());
	}
	pHdrFtrSL->setNext(NULL);
	pHdrFtrSL->setPrev(NULL);
}

fl_DocSectionLayout* FL_DocLayout::findSectionForHdrFtr(const char* pszHdrFtrID) const
{
	const char* pszAtt = NULL;

	fl_DocSectionLayout* pDocSL = m_pFirstSection;
	while (pDocSL)
	{
		pszAtt = pDocSL->getAttribute("header");
		if ( pszAtt	&& (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pszAtt = pDocSL->getAttribute("footer");
		if (pszAtt && (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}
		pszAtt = pDocSL->getAttribute("header-even");
		if ( pszAtt	&& (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pszAtt = pDocSL->getAttribute("footer-even");
		if (pszAtt && (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}
		pszAtt = pDocSL->getAttribute("header-last");
		if ( pszAtt	&& (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pszAtt = pDocSL->getAttribute("footer-last");
		if (pszAtt && (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}
		pszAtt = pDocSL->getAttribute("header-first");
		if ( pszAtt	&& (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pszAtt = pDocSL->getAttribute("footer-first");
		if (pszAtt && (0 == strcmp(pszAtt, pszHdrFtrID)))
		{
			return pDocSL;
		}

		pDocSL = pDocSL->getNextDocSection();
	}

	return NULL;
}

/*static*/ void FL_DocLayout::_prefsListener (
	XAP_App				*pApp,
	XAP_Prefs			*pPrefs,
	UT_StringPtrMap	* /*phChanges*/,  // not used
	void				*data
	)
{
	bool b;
	FL_DocLayout *pDocLayout = static_cast<FL_DocLayout *>(data);

	xxx_UT_DEBUGMSG(("spell_prefsListener %p\n", pDocLayout));
	UT_ASSERT( pApp && pPrefs && data );

	// caps/number/internet

    // Note that these options are now "ignore..." in the prefs pane,
    // so the opton settings are reverted for use in the doclayout
    // (b = !b)
	bool changed = false;
	pPrefs->getPrefsValueBool(static_cast<const XML_Char *>(AP_PREF_KEY_SpellCheckCaps), &b );
    b = !b;
	changed = changed || (b != pDocLayout->getSpellCheckCaps());
	pDocLayout->m_bSpellCheckCaps = b;

	pPrefs->getPrefsValueBool(static_cast<const XML_Char *>(AP_PREF_KEY_SpellCheckNumbers), &b );
    b = !b;
	changed = changed || (b != pDocLayout->getSpellCheckNumbers());
	pDocLayout->m_bSpellCheckNumbers = b;

	pPrefs->getPrefsValueBool(static_cast<const XML_Char *>(AP_PREF_KEY_SpellCheckInternet), &b );
    b = !b;
	changed = changed || (b != pDocLayout->getSpellCheckInternet());
	pDocLayout->m_bSpellCheckInternet = b;

	// auto spell
	pPrefs->getPrefsValueBool(static_cast<const XML_Char *>(AP_PREF_KEY_AutoSpellCheck), &b );
	changed = changed || (b != pDocLayout->m_bAutoSpellCheck);
	if(b != pDocLayout->m_bAutoSpellCheck || (pDocLayout->m_iGraphicTick < 2))
	{
		pDocLayout->m_bAutoSpellCheck = b;
		pDocLayout->_toggleAutoSpell( b );
	}

	// grammar check
	pPrefs->getPrefsValueBool(static_cast<const XML_Char *>(AP_PREF_KEY_AutoGrammarCheck), &b );
	changed = changed || (b != pDocLayout->m_bAutoSpellCheck);
	if(b != pDocLayout->m_bAutoGrammarCheck || (pDocLayout->m_iGraphicTick < 2))
	{
		pDocLayout->m_bAutoGrammarCheck = b;
		pDocLayout->_toggleAutoGrammar( b );
	}

// autosave

	UT_String stTmp;
	FV_View * pView = pDocLayout->getView();
	if(pView)
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
		if(pFrame)
		{
			pPrefs->getPrefsValueBool(static_cast<const XML_Char *>(XAP_PREF_KEY_AutoSaveFile), &b );
			changed = (b != pFrame->isBackupRunning());
			if(changed)
			{
				pFrame->setAutoSaveFile(b);
			}

// autosave period

			pPrefs->getPrefsValue(XAP_PREF_KEY_AutoSaveFilePeriod, stTmp);
			UT_sint32 iPeriod = atoi(stTmp.c_str());
			changed = (iPeriod != pFrame->getAutoSavePeriod());
			if(changed)
			{
				pFrame->setAutoSaveFilePeriod(iPeriod);
				if(pFrame->isBackupRunning())
				{
					pFrame->setAutoSaveFile(false);
					pFrame->setAutoSaveFile(true);
				}
			}
		}
	}


	if ( changed )
	{
		// TODO: recheck document
		;
	}

	pPrefs->getPrefsValueBool( static_cast<const XML_Char *>(XAP_PREF_KEY_SmartQuotesEnable), &b );
	pDocLayout->_toggleAutoSmartQuotes( b );

	const XML_Char * pszTransparentColor = NULL;
	pPrefs->getPrefsValue(static_cast<const XML_Char *>(XAP_PREF_KEY_ColorForTransparent),&pszTransparentColor);
	if(UT_strcmp(pszTransparentColor,pDocLayout->m_szCurrentTransparentColor) != 0)
	{
		if(pDocLayout->getView() && (pDocLayout->getView()->getPoint() > 0))
		{
			pDocLayout->updateColor();
		}
	}
}

void FL_DocLayout::recheckIgnoredWords()
{
	// recheck the whole doc
	fl_DocSectionLayout * pSL = getFirstSection();
	if(pSL)
	{
		fl_ContainerLayout* b = pSL->getFirstLayout();
		while (b)
		{
			if(b->getContainerType() == FL_CONTAINER_BLOCK)
			{
				static_cast<fl_BlockLayout *>(b)->recheckIgnoredWords();
				b = static_cast<fl_BlockLayout *>(b)->getNextBlockInDocument();
			}
			else
			{
				b = b->getNext();
			}
		}
	}
}

void FL_DocLayout::_redrawUpdate(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);

	// this is a static callback method and does not have a 'this' pointer.

	FL_DocLayout * pDocLayout = static_cast<FL_DocLayout *>(pWorker->getInstanceData());
	UT_ASSERT(pDocLayout);

	if (!pDocLayout->m_pView || pDocLayout->isLayoutFilling())
	{
		// Win32 timers can fire prematurely on asserts
		// (the dialog's message pump releases the timers)
		return;
	}
	xxx_UT_DEBUGMSG(("SEVIOR: _redraw update \n"));
//
// Check if we're in the middle of a PieceTable change. If so don't redraw!
//
	PD_Document * pDoc = pDocLayout->getDocument();
	if(pDoc->isPieceTableChanging())
	{
		UT_DEBUGMSG(("PieceTable changing don't redraw \n"));
		return;
	}
//
// Don't redraw while the selection is active
//
//	if(!pDocLayout->m_pView->isSelectionEmpty())
//	{
//		return;
//	}
//
// Lock out PieceTable changes till we finished.
//
	pDoc->setRedrawHappenning(true);
//
// Check if we've been asked to wait for a while..
//
	UT_uint32 skip = pDocLayout->getSkipUpdates();
	if(skip > 0)
	{
		skip--;
		pDocLayout->setSkipUpdates(skip);
		pDoc->setRedrawHappenning(false);
		return;
	}
//
// Check if we're printing. If so Bail out
//
	if(pDocLayout->m_pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		pDoc->setRedrawHappenning(false);
		return;
	}
	bool bStopOnRebuild = false;
	fl_SectionLayout* pSL = pDocLayout->m_pFirstSection;
//
// This bit is to make sure the insertionPoint is always on screen.
//
	FV_View * pView = pDocLayout->getView();
	bool bEnd,bDir;
	bEnd = false;
	fl_BlockLayout * pBlock = NULL;
	fp_Run *pRun = NULL;
	UT_sint32 x1,x2,y1,y2;
	UT_uint32 height;
	UT_sint32 origY;
	pView->_findPositionCoords(pView->getPoint(),bEnd,x1,y1,x2,y2,height,bDir,&pBlock,&pRun);
	origY = y1;
	while (pSL && !bStopOnRebuild)
	{
		if(pDoc->isPieceTableChanging())
		{
			pDoc->setRedrawHappenning(false);
			return;
		}
		pSL->redrawUpdate();
//
// Might need some code here to check if we need a rebuild. In principle we should not need it.
//
		if(pSL->getType() == FL_SECTION_DOC)
		{
			if(static_cast<fl_DocSectionLayout *>(pSL)->needsRebuild())
			{
				bStopOnRebuild = true;
			}
		}
		if(!bStopOnRebuild)
		{
			pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
		}
	}
	pDocLayout->deleteEmptyColumnsAndPages();
	if(bStopOnRebuild)
	{
		UT_DEBUGMSG(("SEVIOR: Rebuilding from docLayout \n"));
		pDocLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(pSL));
	}
	pView->_findPositionCoords(pView->getPoint(),bEnd,x1,y1,x2,y2,height,bDir,&pBlock,&pRun);
//
// If Y location has changed make sure it's still on screen
//
	if(y1 != origY)
	{
		UT_DEBUGMSG(("Line pos changed \n"));
//		UT_ASSERT(0);
		pView->_ensureInsertionPointOnScreen();
	}

//
// we've finished
//
	pDoc->setRedrawHappenning(false);
	pDocLayout->m_iRedrawCount++;
}

void FL_DocLayout::setPendingSmartQuote(fl_BlockLayout *bl, UT_uint32 of)
{
	xxx_UT_DEBUGMSG(("FL_DocLayout::setPendingSmartQuote(%x, %d)\n", bl, of));
	m_pPendingBlockForSmartQuote = bl;
	m_uOffsetForSmartQuote = of;
}

/* wjc sez....

This algorithm is based on my observation of how people actually use
quotation marks, sometimes in contravention of generally accepted
principles of punctuation.  It is certainly also true that my
observations are overwhelmingly of American English text, with a
smattering of various other languages observed from time to time.  I
don't believe that any algorithm for this can ever be perfect.  There
are too many infrequently-occurring but legitimate cases where a user
might want something else.  FWIW, I haven't tested out the specifics
of the smart quote algorithm in ThatOtherWordProcessor.

Some terms for the purpose of this discussion (I'm open to plenty of
advice on what specific items should fit in each of these classes):

sqBREAK  A structural break in a document.  For example, a paragraph
  break, a column break, a page break, the beginning or end of a
  document, etc.  Does not include font, size, bold/italic/underline
  changes (which are completely ignored for the purposes of this
  algorithm).

sqFOLLOWPUNCT A subset of layman's "punctuation".  I include only
  things that can normally occur after a quote mark with no intervening
  white space.  Includes period, exclamation point, question mark,
  semi-colon, colon, comma (but not parentheses, square and curly
  brackets, which are treated specially below).  There may be a few
  others that aren't on the kinds of keyboards I use, and there are
  certainly Latin1 and other locale-specific variants, but the point
  is that there are lots of random non-alphanumerics which aren't
  included in *PUNCT for this algorithm.

sqOPENPUNCT  The opening half of pairwise, non-quote punctuation.  Open
  parenthesis, open square bracket, open curly brace.

sqCLOSEPUNCT  The closing half of pairwise, non-quote punctuation.  Close
  parenthesis, close square bracket, close curly brace.

[[The idea about open and close punctuation was found in a mid-1980s
note by Dave Dunham, brought to my attention by Leonard Rosenthol
<leonardr@lazerware.com>.]]

sqOTHERPUNCT  Punctuation which is not sqFOLLOWPUNCT, sqOPENPUNCT, or
  sqCLOSEPUNCT.

sqALPHA  Alphabetic characters in the C isalpha() sense, but there are
  certainly some non-ASCII letter characters which belong in this
  bucket, too.

sqWHITE  White speace haracters in the C isspace() sense.

QUOTE  Any of ASCII double quote, ASCII quote (which many people call
  the ASCII single quote or the ASCII apostrophe), or ASCII backquote.
  I take it as given that a significant minority of people randomly or
  systematically interchange their use of ASCII quote and ASCII
  backquote, so I treat them the same in the algorithm.  The majority
  of people use ASCII quote for both opening and closing single quote.

PARITY  Whether a quote is single or double.  For ease of description,
  I'll say that the parity of single and double quotes are opposites
  of each other.  When QUOTEs are converted to curly form, the parity
  never changes.

================================================================

Given a QUOTE character, these conditions/rules are logically tested in
order:

0.  OK, first an easy exception case: If ASCII (single) quote (but not
ASCII backquote) appears between two sqALPHAs, it may be treated as an
apostrophe and converted to its curly form.  Otherwise, it is treated
like all other QUOTEs and follows the normal algorithm.

1.  If a QUOTE is immediately preceded by a curly quote of opposite
parity, it is converted to a curly quote in the same direction.

2.  If a QUOTE is immediately preceded by a curly quote of the same
parity, it is converted to a curly quote of opposite direction.

3.  If a QUOTE is immediately followed by a curly quote of opposite
parity, it is converted to a curly quote in the same direction.

4.  If a QUOTE is immediately followed by a curly quote of the same
parity, it is converted to a curly quote of opposite direction.

[[The above cases are intended to handle normal nested quotes or cases
where quotes enclose empty strings.  Different cultures use different
parities as start points for nested quotes, but the algorithm doesn't
care.]]

5.  If a QUOTE is immediately preceded by an sqOPENPUNCT, it is
converted to a curly quote in the open direction.

6.  If a QUOTE is immediately followed by a sqCLOSEPUNCT, it is
converted to a curly quote in the close direction.

7.  If a QUOTE is in isolation, it is not converted.  It is in
isolation if it is immediately preceded and followed by either a sqBREAK
or sqWHITE.  The things before and after it don't have to be of
the same type.

8.  If a QUOTE is immediately preceded by a sqBREAK or sqWHITE and
is immediately followed by anything other than a sqBREAK or sqWHITE,
it is converted to the opening form of curly quote.

9.  If a QUOTE is immediately followed by a sqBREAK, sqWHITE, or
sqFOLLOWPUNCT and is immediately preceded by anything other than sqBREAK
or sqWHITE, it is converted to the closing form of curly quote.

10.  Any other QUOTE is not converted.

================================================================

The algorithm doesn't make a special case of using ASCII double quote
as an inches indicator (there are other uses, like lat/long minutes;
ditto for the ASCII quote) because it is tough to tell if some numbers
with an ASCII double quote after them are intended to be one of those
"other things" or is just the end of a very long quote.  So, the
algorithm will be wrong sometimes in those cases.

It is otherwise sort of conservative, preferring to not convert things
it doesn't feel confident about.  The reason for that is that there is
a contemplated on-the-fly conversion to smart quotes, but there is no
contemplated on-the-fly conversion to ASCII QUOTEs.  So, if the
algorithm makes a mistake by not converting, the user can correct it
by directly entering the appropriate smart quote character or by
heuristically tricking AbiWord into converting it for him/her and then
fixing things up.  (That heuristic step shouldn't be necessary, you
know, but I think we all use software for which we have become
accustomed to such things.)

What about the occasions when this algorithm (or any alternative
algorithm) makes a mistake and converts a QUOTE to the curly form when
it really isn't wanted, in a particular case, by the user?  Although
the user can change it back, some contemplated implementation details
might run around behind the barn and re-convert it when the user isn't
looking.  I think we need a mechanism for dealing with that, but I
want to save proposals for that to be separate from the basic
algorithm.
*/
#if 0
// The following are descriptions of the thing before or after a
// character being considered for smart quote promotion.  The thing
// is either a structural break in a document, or it is a literal
// character that is part of some class (in some cases the class is
// so small it has only one possible member).  The classes should
// look familar from the algorithm above.  There is a special class
// used only for the coding of rule:  sqDONTCARE in a rule means it
// doesn't matter what occurs in that position.
enum sqThingAt
{
	sqDONTCARE     = 1,
	sqQUOTEls      = 2,
	sqQUOTErs      = 3,
	sqQUOTEld      = 4,
	sqQUOTErd      = 5,// the smart quotes, left/right single/double
	sqBREAK        = 6,
	sqFOLLOWPUNCT  = 7,
	sqOPENPUNCT    = 8,
	sqCLOSEPUNCT   = 9,
	sqOTHERPUNCT   =10,
	sqALPHA        =11,
	sqWHITE        =12
};

// TODO:  This function probably needs tuning for non-Anglo locales.
static enum sqThingAt whatKindOfChar(UT_UCSChar thing)
{
	xxx_UT_DEBUGMSG(("what sort of character is %d 0x%x |%c|\n", thing, thing, thing));
	switch (thing)
	{
	case UCS_LQUOTE:     return sqQUOTEls;
	case UCS_RQUOTE:     return sqQUOTErs;
	case UCS_LDBLQUOTE:  return sqQUOTEld;
	case UCS_RDBLQUOTE:  return sqQUOTErd;

	case '(': case '{': case '[':  return sqOPENPUNCT;
	case ')': case '}': case ']':  return sqCLOSEPUNCT;

	case '.': case ',': case ';': case ':': case '!': case '?':  return sqFOLLOWPUNCT;

		// see similar control characters in fl_BlockLayout.cpp
	case UCS_FF:	// form feed, forced page break
	case UCS_VTAB:	// vertical tab, forced column break
	case UCS_LF:	// newline
	case UCS_TAB:	// tab
		return sqBREAK;
	}
	if (UT_UCS4_isalpha(thing)) return sqALPHA;
	if (UT_UCS4_ispunct(thing)) return sqOTHERPUNCT;
	if (UT_UCS4_isspace(thing)) return sqWHITE;

	return sqBREAK;  // supposed to be a character, but...!
}

struct sqTable
{
	enum sqThingAt before;
	UT_UCSChar thing;
	enum sqThingAt after;
	UT_UCSChar replacement;
};
// The idea of the table is to drive the algorithm without lots of
// cluttery code.  Something using this table pre-computes what the
// things are before and after the character in question, and then
// dances through this table looking for a match on all three.
// The final item in each row is the character to use to replace
// the candidate character.
//
// (Yeah, this table is big, but it is only used when a quote character
// shows up in typing or in a paste, and it goes pretty fast.)
//
// sqDONTCARE is like a wild card for the thing before or after, and
// UCS_UNKPUNK in the replacement position means don't do a replacement.
static struct sqTable sqTable_en[] =
{
	{sqALPHA,     '\'', sqALPHA,      UCS_RQUOTE},          // rule 0
	{sqALPHA,     '`',  sqALPHA,      UCS_RQUOTE},          // rule 0

	{sqQUOTEld,   '\'', sqDONTCARE,   UCS_LQUOTE},          // rule 1
	{sqQUOTErd,   '\'', sqDONTCARE,   UCS_RQUOTE},          // rule 1

	{sqQUOTEld,   '`',  sqDONTCARE,   UCS_LQUOTE},          // rule 1
	{sqQUOTErd,   '`',  sqDONTCARE,   UCS_RQUOTE},          // rule 1

	{sqQUOTEls,   '"',  sqDONTCARE,   UCS_LDBLQUOTE},       // rule 1
	{sqQUOTErs,   '"',  sqDONTCARE,   UCS_RDBLQUOTE},       // rule 1

	{sqQUOTEls,   '\'', sqDONTCARE,   UCS_RQUOTE},          // rule 2
	{sqQUOTErs,   '\'', sqDONTCARE,   UCS_LQUOTE},          // rule 2

	{sqQUOTEls,   '`',  sqDONTCARE,   UCS_RQUOTE},          // rule 2
	{sqQUOTErs,   '`',  sqDONTCARE,   UCS_LQUOTE},          // rule 2

	{sqQUOTEld,   '"',  sqDONTCARE,   UCS_RDBLQUOTE},       // rule 2
	{sqQUOTErd,   '"',  sqDONTCARE,   UCS_LDBLQUOTE},       // rule 2

	{sqDONTCARE,   '\'', sqQUOTEld,   UCS_LQUOTE},          // rule 3
	{sqDONTCARE,   '\'', sqQUOTErd,   UCS_RQUOTE},          // rule 3

	{sqDONTCARE,   '`',  sqQUOTEld,   UCS_LQUOTE},          // rule 3
	{sqDONTCARE,   '`',  sqQUOTErd,   UCS_RQUOTE},          // rule 3

	{sqDONTCARE,   '"',  sqQUOTEls,   UCS_LDBLQUOTE},       // rule 3
	{sqDONTCARE,   '"',  sqQUOTErs,   UCS_RDBLQUOTE},       // rule 3

	{sqDONTCARE,   '\'', sqQUOTEls,   UCS_RQUOTE},          // rule 4
	{sqDONTCARE,   '\'', sqQUOTErs,   UCS_LQUOTE},          // rule 4

	{sqDONTCARE,   '`',  sqQUOTEls,   UCS_RQUOTE},          // rule 4
	{sqDONTCARE,   '`',  sqQUOTErs,   UCS_LQUOTE},          // rule 4

	{sqDONTCARE,   '"',  sqQUOTEld,   UCS_RDBLQUOTE},       // rule 4
	{sqDONTCARE,   '"',  sqQUOTErd,   UCS_LDBLQUOTE},       // rule 4

	{sqOPENPUNCT,  '\'', sqDONTCARE,  UCS_LQUOTE},          // rule 5
	{sqOPENPUNCT,  '`',  sqDONTCARE,  UCS_LQUOTE},          // rule 5
	{sqOPENPUNCT,  '"',  sqDONTCARE,  UCS_LDBLQUOTE},       // rule 5

	{sqDONTCARE, '\'', sqCLOSEPUNCT,  UCS_RQUOTE},          // rule 6
	{sqDONTCARE, '`',  sqCLOSEPUNCT,  UCS_RQUOTE},          // rule 6
	{sqDONTCARE, '"',  sqCLOSEPUNCT,  UCS_RDBLQUOTE},       // rule 6

	{sqBREAK,      '\'', sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '\'', sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqBREAK,      '\'', sqWHITE,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '\'', sqWHITE,     UCS_UNKPUNK},         // rule 7

	{sqBREAK,      '`',  sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '`',  sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqBREAK,      '`',  sqWHITE,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '`',  sqWHITE,     UCS_UNKPUNK},         // rule 7

	{sqBREAK,      '"',  sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '"',  sqBREAK,     UCS_UNKPUNK},         // rule 7
	{sqBREAK,      '"',  sqWHITE,     UCS_UNKPUNK},         // rule 7
	{sqWHITE,      '"',  sqWHITE,     UCS_UNKPUNK},         // rule 7

	{sqBREAK,      '\'', sqDONTCARE,  UCS_LQUOTE},          // rule 8
	{sqWHITE,      '\'', sqDONTCARE,  UCS_LQUOTE},          // rule 8

	{sqBREAK,      '`',  sqDONTCARE,  UCS_LQUOTE},          // rule 8
	{sqWHITE,      '`',  sqDONTCARE,  UCS_LQUOTE},          // rule 8

	{sqBREAK,      '"',  sqDONTCARE,  UCS_LDBLQUOTE},       // rule 8
	{sqWHITE,      '"',  sqDONTCARE,  UCS_LDBLQUOTE},       // rule 8

	{sqDONTCARE,   '\'', sqBREAK,     UCS_RQUOTE},          // rule 9
	{sqDONTCARE,   '\'', sqWHITE,     UCS_RQUOTE},          // rule 9
	{sqDONTCARE,   '\'', sqFOLLOWPUNCT, UCS_RQUOTE},          // rule 9

	{sqDONTCARE,   '`',  sqBREAK,     UCS_RQUOTE},          // rule 9
	{sqDONTCARE,   '`',  sqWHITE,     UCS_RQUOTE},          // rule 9
	{sqDONTCARE,   '`',  sqFOLLOWPUNCT, UCS_RQUOTE},          // rule 9

	{sqDONTCARE,   '"',  sqBREAK,     UCS_RDBLQUOTE},       // rule 9
	{sqDONTCARE,   '"',  sqWHITE,     UCS_RDBLQUOTE},       // rule 9
	{sqDONTCARE,   '"',  sqFOLLOWPUNCT, UCS_RDBLQUOTE},       // rule 9

	// following rules are the same as falling off the end of the list...

	//{sqDONTCARE,   '\'', sqDONTCARE,   UCS_UNKPUNK},        // rule 10
	//{sqDONTCARE,   '`',  sqDONTCARE,   UCS_UNKPUNK},        // rule 10
	//{sqDONTCARE,   '"',  sqDONTCARE,   UCS_UNKPUNK},        // rule 10

	{sqDONTCARE, 0, sqDONTCARE, UCS_UNKPUNK}  // signals end of table
};

static void s_swapQuote(UT_UCSChar & c)
{
    if(c == UCS_LQUOTE)
        c = UCS_RQUOTE;
    else if(c == UCS_RQUOTE)
        c = UCS_LQUOTE;
    else if(c == UCS_LDBLQUOTE)
        c = UCS_RDBLQUOTE;
    else if(c == UCS_RDBLQUOTE)
        c = UCS_LDBLQUOTE;
}
#endif
void FL_DocLayout::considerSmartQuoteCandidateAt(fl_BlockLayout *block, UT_uint32 offset)
{
#if 0
	if (!block)
		return;
	if (m_pView->isHdrFtrEdit())
		return;

	setPendingSmartQuote(NULL, 0);  // avoid recursion
	UT_GrowBuf pgb(1024);
	block->getBlockBuf(&pgb);
	// this is for the benefit of the UT_DEBUGMSG and should be changed to
	// something other than '?' if '?' ever shows up as UT_isSmartQuotableCharacter()
	UT_UCSChar c = '?';
	if (pgb.getLength() > offset) c = *pgb.getPointer(offset);
	UT_DEBUGMSG(("FL_DocLayout::considerSmartQuoteCandidateAt(%x, %d)  |%c|\n", block, offset, c));

	//  there are some operations that leave a dangling pending
	//  smart quote, so just double check before plunging onward
	if (UT_isSmartQuotableCharacter(c))
	{
		enum sqThingAt before = sqBREAK, after = sqBREAK;
		if (offset > 0)
		{
			// TODO: is there a need to see if this is on a run boundary?
			// TODO: Within a block, are there runs that are significant
			// TODO: breaks or whatever?
			before = whatKindOfChar(*pgb.getPointer(offset - 1));
		}
		else
		{
			// candidate was the first character in the block, so
			// see what was at the end of the previous block, if any
			fl_BlockLayout *ob = static_cast<fl_BlockLayout *>(block->getPrev());
			if (ob)
			{
				fp_Run *last, *r = ob->getFirstRun();
				do
				{
					last = r;
				} while ((r = r->getNextRun()));  // assignment
				if (last  &&  (FPRUN_TEXT == last->getType())  &&  last->getLength() > 0)
				{
					fp_Line *blocks_line, *lasts_line;
					blocks_line = block->getFirstRun()->getLine();
					lasts_line = last->getLine();
					if (blocks_line == lasts_line)
					{
						// last run of previous block was a text run on the same line
						// so find out what the final character was
						UT_GrowBuf pgb_b(1024);
						ob->getBlockBuf(&pgb_b);
						if (pgb_b.getLength())
						{
							before = whatKindOfChar(*pgb_b.getPointer(pgb_b.getLength()-1));
						}
					}
				}
			}
		}

		if (offset+1 < pgb.getLength())
		{
			// TODO: is there a need to see if this is on a run boundary?
			// TODO: Within a block, are there runs that are significant
			// TODO: breaks or whatever?
			after = whatKindOfChar(*pgb.getPointer(offset + 1));
		}
		else
		{
			// candidate was the last character in a block, so see
			// what's at the beginning of the next block, if any
			fl_BlockLayout *ob = static_cast<fl_BlockLayout *>(block->getNextInDocument());
			if (ob)
			{
				fp_Run *r = ob->getFirstRun();
				if (r  &&  (FPRUN_TEXT == r->getType()))
				{
					// first run of next block is a text run, so
					// see what the first character was
					UT_GrowBuf pgb_a(1024);
					ob->getBlockBuf(&pgb_a);
					if (pgb_a.getLength())
					{
						after = whatKindOfChar(*pgb_a.getPointer(0));
					}
				}
			}
		}

		// we now know what the before and after things are, so
		// spin through the table.
		UT_UCSChar replacement = UCS_UNKPUNK;  // means don't replace
		// TODO:  select a table based on default locale or on the locale
		// TODO:  of the fragment of text we're working in (locale tagging
		// TODO:  of text doesn't exist in Abi as of this writing)
		struct sqTable *table = sqTable_en;
		for (unsigned int tdex=0; table[tdex].thing; ++tdex)
		{
			if (c != table[tdex].thing) continue;
			if (table[tdex].before == sqDONTCARE  ||  table[tdex].before == before)
			{
				if (table[tdex].after == sqDONTCARE  ||  table[tdex].after == after)
				{
					replacement = table[tdex].replacement;
					break;
				}
			}
		}
		UT_DEBUGMSG(("before %d, after %d, replace %x\n", before, after, replacement));
		if (replacement != UCS_UNKPUNK)
		{
            UT_sint32 s1,s2,s3,s4,s5;
            bool b1;
            fp_Run * pThisRun = block->findPointCoords(block->getPosition() + offset,false,s1,s2,s3,s4,s5,b1);

            xxx_UT_DEBUGMSG(("pThisRun [0x%x], vis dir. %d\n", pThisRun, pThisRun->getVisDirection()));

            if(pThisRun && pThisRun->getVisDirection() == UT_BIDI_RTL)
                s_swapQuote(replacement);
			// your basic emacs (save-excursion...)  :-)
			PT_DocPosition saved_pos, quotable_at;
			saved_pos = m_pView->getPoint();
			quotable_at = block->getPosition(false) + offset;

			m_pView->moveInsPtTo(quotable_at);
			// delete/insert create change records for UNDO
			m_pView->cmdCharDelete(true, 1);
			m_pView->cmdCharInsert(&replacement, 1);
			m_pView->moveInsPtTo(saved_pos);

			// Alas, Abi undo moves the insertion point, so you can't
			// just UNDO right after a smart quote pops up to force
			// an ASCII quote.  For an open quote, you could type
			// " backspace to get it (in other words, quote, space,
			// backspace.  The space will prevent the smart quote
			// promotion (no magic ... just following the rules).
			// For a close quote, type "/backspace (quote, slash, backspace)
			// for similar reasons.
		}
	}
#else
	return;
#endif
}

void FL_DocLayout::notifyBlockIsBeingDeleted(fl_BlockLayout *pBlock)
{
	if(pBlock == m_pPendingBlockForSpell)
	{
		m_pPendingBlockForSpell = NULL;
	}

	if(pBlock == m_pPendingBlockForSmartQuote)
	{
		m_pPendingBlockForSmartQuote = NULL;
	}
	UT_sint32 loc = m_vecUncheckedBlocks.findItem(pBlock);
	if(loc >= 0)
	{
		m_vecUncheckedBlocks.deleteNthItem(loc);
	}
}

inline fl_AutoNum * FL_DocLayout::getListByID(UT_uint32 id) const
{
	return m_pDoc->getListByID(id);
}

inline fl_AutoNum * FL_DocLayout::getNthList(UT_uint32 i) const
{
	return m_pDoc->getNthList(i);
}

inline UT_uint32 FL_DocLayout::getListsCount(void) const
{
	return m_pDoc->getListsCount();
}

inline void FL_DocLayout::addList(fl_AutoNum * pAutoNum)
{
	m_pDoc->addList(pAutoNum);
}

void FL_DocLayout::notifyListeners(AV_ChangeMask mask)
{
	if (m_pView)
		m_pView->notifyListeners(mask);
}
