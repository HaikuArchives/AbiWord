/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#include "xap_BeOSFrameImpl.h"
#include "ev_BeOSToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_BeOSGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_BeOSFrame.h"
#include "ap_BeOSFrameImpl.h"
#include "xap_BeOSApp.h"
#include "ap_BeOSTopRuler.h"
#include "ap_BeOSLeftRuler.h"
#include "ap_Prefs.h"
#include "ap_BeOSStatusBar.h"

#include "ev_BeOSKeyboard.h"
#include "ev_BeOSMouse.h"


/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

/*
 This function actually sets up the value of the scroll bar in 
 terms of it max/min and step size, and will set an initial
 value of the widget by calling the scroll event which
 will eventually call _scrollFuncX()
*/
void AP_BeOSFrame::setXScrollRange(void)
{
	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	AP_BeOSFrameImpl *pBeOSFrame = (AP_BeOSFrameImpl *)getFrameImpl();
	be_Window *pBWin = (be_Window*)pBeOSFrame->getTopLevelWindow();

	if (!pBWin->Lock()) {
		return;
	}

	int windowWidth = (int)pBWin->m_pbe_DocView->Bounds().Width();
	pBWin->Unlock();

	int newvalue = (m_pView) ? m_pView->getXScrollOffset() : 0;
	/* This is a real dilemma ... should the maximum of the
       scrollbar be set to the document width, or should it
       be set to the document width - window width since that
       is all you are going to scroll? I think it should be
       set to width - window width myself.  */
	int newmax = width - windowWidth; /* upper - page_size */ 
	int differentPosition = 0;

	/* Adjust the bounds of maximum and value as required */
	if (newmax <= 0) {
		newvalue = newmax = 0;
	}
	else if (newvalue > newmax) {
		newvalue = newmax;
	}

	BScrollBar *hscroll = pBWin->m_hScroll;
	if (hscroll->Window()->Lock()) {
    	hscroll->SetSteps(20.0, windowWidth);
    	hscroll->SetRange(0, newmax);
		differentPosition = (hscroll->Value() != newvalue);
    	hscroll->Window()->Unlock(); 
	}

	if (m_pView && differentPosition /*|| differentLimits */) {
		m_pView->sendHorizontalScrollEvent(newvalue);
	}
}

void AP_BeOSFrame::setYScrollRange(void)
{
	int height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();
	AP_BeOSFrameImpl *pBeOSFrame = (AP_BeOSFrameImpl *)getFrameImpl();

	be_Window *pBWin = (be_Window*)pBeOSFrame->getTopLevelWindow();

    if (!pBWin->Lock()) {
		return;
	}
    int windowHeight = (int)pBWin->m_pbe_DocView->Bounds().Height();
    pBWin->Unlock();

	int newvalue = (m_pView) ? m_pView->getYScrollOffset() : 0;
	int newmax = height - windowHeight; /* upper - page_size */ 
	int differentPosition = 0;

	/* Adjust the bounds of maximum and value as required */
	if (newmax <= 0) {
		newvalue = newmax = 0;
	}
	else if (newvalue > newmax) {
		newvalue = newmax;
	}

    BScrollBar *vscroll = pBWin->m_vScroll;
    if (vscroll->Window()->Lock()) {
    	vscroll->SetSteps(20.0, windowHeight);
    	vscroll->SetRange(0, newmax);
		differentPosition = (vscroll->Value() != newvalue);
    	vscroll->Window()->Unlock();        
	}

	if (m_pView && differentPosition /*|| differentLimits */) {
		m_pView->sendVerticalScrollEvent(newvalue);
	}
}


AP_BeOSFrame::AP_BeOSFrame(XAP_BeOSApp * app)
	: AP_Frame(new AP_BeOSFrameImpl(this, app),app)
{
	// TODO
}

AP_BeOSFrame::AP_BeOSFrame(AP_BeOSFrame * f)
	: AP_Frame(static_cast<AP_Frame *>(f))
{
	// TODO
}

AP_BeOSFrame::~AP_BeOSFrame()
{
	killFrameData();
}

bool AP_BeOSFrame::initialize(XAP_FrameMode frameMode=XAP_NormalFrame)
{
	if (!initFrameData())
		return false;

	if (!XAP_Frame::initialize(AP_PREF_KEY_KeyBindings, AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
		return false;

	AP_BeOSFrameImpl * pFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());
	pFrameImpl->_createWindow();

	//At this point in time the BeOS widgets are all
	//realized so we should be able to go and attach
	//the various input filters to them.
	ev_BeOSKeyboard * pBeOSKeyboard = static_cast<ev_BeOSKeyboard *>(pFrameImpl->m_pKeyboard);
	pBeOSKeyboard->synthesize(pFrameImpl->m_pBeOSApp, pFrameImpl);

	ev_BeOSMouse * pBeOSMouse = static_cast<ev_BeOSMouse *>(pFrameImpl->m_pMouse);
	pBeOSMouse->synthesize(pFrameImpl->m_pBeOSApp, pFrameImpl);
    printf("beos frame\n");
	UT_uint32 nrToolbars = pFrameImpl->m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{			
		// Set the initial hidden poperty to match what the FrameDate holds.
		EV_BeOSToolbar * pBeOSToolbar = (EV_BeOSToolbar *)pFrameImpl->m_vecToolbars.getNthItem(k);
		UT_ASSERT(pBeOSToolbar);
		if(!(static_cast<AP_FrameData*> (m_pData)->m_bShowBar[k]))
		{
			UT_DEBUGMSG(("Hiding toolbar %d", k));
			printf("Tool bar hiding %d",k);
			pBeOSToolbar->hide();
		}	
	}

	//Actually show the window to the world
//	pFrameImpl->m_pBeWin->Show();
 //	pFrameImpl->getTopLevelWindow()->Show();

	return true;
}

/*****************************************************************/

//bool AP_BeOSFrame::initFrameData(void)
//{
//	UT_ASSERT(!m_pData);
//
//	AP_BeOSFrameImpl * pFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());
//	m_pData = new AP_FrameData(pFrameImpl->m_pBeOSApp);
//	
//	return (m_pData ? true : false);
//}
//
bool AP_BeOSFrame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{
	AP_BeOSFrameImpl * pFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());
	GR_BeOSAllocInfo ai(pFrameImpl->getBeDocView(), getApp());
	pG = getApp()->newGraphics(ai);	
	ENSUREP(pG);
	pG->setZoomPercentage(iZoom);
	return true;
Cleanup:
	return false;
}
//
bool AP_BeOSFrame::_createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
					     ap_ViewListener *& pViewListener, ap_Scrollbar_ViewListener *& pScrollbarViewListener,
					     AV_ListenerId &lid, AV_ListenerId &lidScrollbarViewListener)
{
	// The "AV_ScrollObj pScrollObj" receives
	// send{Vertical,Horizontal}ScrollEvents
	// from both the scroll-related edit methods
	// and from the UI callbacks.
	// 
	// The "ap_ViewListener pViewListener" receives
	// change notifications as the document changes.
	// This ViewListener is responsible for keeping
	// the title-bar up to date (primarily title
	// changes, dirty indicator, and window number).
	// ON BeOS ONLY: we subclass this with ap_BeOSViewListener
	// ON BeOS ONLY: so that we can deal with X-Selections.
	//
	// The "ap_Scrollbar_ViewListener pScrollbarViewListener"
	// receives change notifications as the doucment changes.
	// This ViewListener is responsible for recalibrating the
	// scrollbars as pages are added/removed from the document.
	//
	// Each Toolbar will also get a ViewListener so that
	// it can update toggle buttons, and other state-indicating
	// controls on it.
	//
	// TODO we ***really*** need to re-do the whole scrollbar thing.
	// TODO we have an addScrollListener() using an m_pScrollObj
	// TODO and a View-Listener, and a bunch of other widget stuff.
	// TODO and its very confusing.

	pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	ENSUREP(pScrollObj);

	pViewListener = new ap_ViewListener(this);
	ENSUREP(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	ENSUREP(pScrollbarViewListener);
	
	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		return false;
	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		return false;

	return true;
Cleanup:
	return false;
}
//
void AP_BeOSFrame::_bindToolbars(AV_View *pView)
{
	static_cast<AP_BeOSFrameImpl *>(getFrameImpl())->_bindToolbars(pView);
}
//
void AP_BeOSFrame::_setViewFocus(AV_View *pView)
{
	pView->focusChange(AV_FOCUS_HERE);
}
/*
  We've been notified (via sendScrollEvent()) of a scroll (probably
  a keyboard motion or user scroll event).  push the new values into 
  the scrollbar widgets (with clamping).  Then cause the view to scroll.
*/
void AP_BeOSFrame::_scrollFuncX(void * pData, 
								UT_sint32 xoff, 
								UT_sint32 /*xrange*/)
{
    // this is a static callback function and doesn't have a 'this' pointer.
 	AP_BeOSFrame * pBeOSFrame = static_cast<AP_BeOSFrame *>(pData);
	AV_View * pView = pBeOSFrame->getCurrentView();
	AP_BeOSFrameImpl * pFrameImpl = static_cast<AP_BeOSFrameImpl *>(pBeOSFrame->getFrameImpl());

	//Actually set the scroll bar value ...
	pView->setXScrollOffset(xoff);

   	BScrollBar *hscroll = pFrameImpl->m_hScroll;
   	if (hscroll->Window()->Lock()) {
		float min, max;
		hscroll->GetRange(&min, &max);
		if (xoff < min)	xoff = (UT_sint32)min;
		if (xoff > max)	xoff = (UT_sint32)max;
		hscroll->SetValue(xoff);
   		hscroll->Window()->Unlock(); 
	}
}

void AP_BeOSFrame::_scrollFuncY(void * pData, 
								UT_sint32 yoff, 
								UT_sint32 /*yrange*/)
{
    // this is a static callback function and doesn't have a 'this' pointer.
 	AP_BeOSFrame * pBeOSFrame = static_cast<AP_BeOSFrame *>(pData);
	AV_View * pView = pBeOSFrame->getCurrentView();
	AP_BeOSFrameImpl * pFrameImpl = static_cast<AP_BeOSFrameImpl *>(pBeOSFrame->getFrameImpl());

	//Actually set the scroll bar value ...
	pView->setYScrollOffset(yoff);

   	BScrollBar *vscroll = pFrameImpl->m_vScroll;
   	if (vscroll->Window()->Lock()) {
		float min, max;
		vscroll->GetRange(&min, &max);
		if (yoff < min)	yoff = (UT_sint32)min;
		if (yoff > max)	yoff = (UT_sint32)max;
		vscroll->SetValue(yoff);
   		vscroll->Window()->Unlock(); 
	}
}

void AP_BeOSFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}                                                                        



/*UT_Error AP_BeOSFrame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}*/


UT_sint32 AP_BeOSFrame::_getDocumentAreaWidth()
{
	AP_BeOSFrameImpl * pFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());
	be_DocView *pView = pFrameImpl->getBeDocView();
	if(pView->Window()->Lock())
	{
		UT_sint32 ww = static_cast<UT_sint32>(pView->Bounds().Width());
		pView->Window()->Unlock();
		return ww;
	} else 
		return 0;
}

UT_sint32 AP_BeOSFrame::_getDocumentAreaHeight()
{
	AP_BeOSFrameImpl * pFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());
	be_DocView *pView = pFrameImpl->getBeDocView();
	if(pView->Window()->Lock())
	{
		UT_sint32 hh = static_cast<UT_sint32>(pView->Bounds().Height());
		pView->Window()->Unlock();
		return hh;
	} else 
		return 0;
}

/*XAP_Frame * AP_BeOSFrame::buildFrame(XAP_Frame * pF)
{
	UT_Error error = UT_OK;
	AP_BeOSFrame * pFrame = static_cast<AP_BeOSFrame *>(pF);
	ENSUREP(pFrame);
	return pFrame;

	if (!pFrame->initialize())
		goto Cleanup;

	error = pFrame->_showDocument();
	if (error)
		goto Cleanup;

	pFrame->show();
	return static_cast<XAP_Frame *>(pFrame);
								                             
Cleanup:
	// clean up anything we created here
	if (pFrame)
	{
		m_pApp->forgetFrame(pFrame);
		delete pFrame;
	}

	return NULL;
}*/

// Does the initial show/hide of statusbar (based on the user prefs).
// Idem.
void AP_BeOSFrame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleBar %d\n", bBarOn));

	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
	UT_ASSERT(pFrameData);
	UT_ASSERT(pFrameData->m_pToolbar);
	
	EV_Toolbar *pToolbar = pFrameData->m_pToolbar[iBarNb];
	
	UT_ASSERT(pToolbar);

	if (bBarOn)
		pToolbar->show();
	else	// turning toolbar off
		pToolbar->hide();
}

void AP_BeOSFrame::toggleStatusBar(bool bStatusBarOn)
{
        UT_DEBUGMSG(("AP_BeOSFrame::toggleStatusBar %d\n", bStatusBarOn));

        AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
        UT_ASSERT(pFrameData);

        if (bStatusBarOn)
                pFrameData->m_pStatusBar->show();
        else    // turning status bar off
                pFrameData->m_pStatusBar->hide();
}

void AP_BeOSFrame::toggleRuler(bool bRulerOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleRuler %d", bRulerOn));	
 
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);
}

void AP_BeOSFrame::toggleTopRuler(bool bRulerOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleRuler %d", bRulerOn));	
 
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);
}

void AP_BeOSFrame::toggleLeftRuler(bool bRulerOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleRuler %d", bRulerOn));	
 
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);
}

XAP_Frame * AP_BeOSFrame::cloneFrame(void)
{
	AP_BeOSFrame * pClone = new AP_BeOSFrame(this);
	ENSUREP(pClone);
	return pClone;

Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		m_pApp->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}
//
void AP_BeOSFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{ 
	// translate the given document mouse coordinates into absolute screen coordinates.

	BPoint pt(x,y);// = { x, y };

	AP_BeOSFrameImpl * pFrameImpl = static_cast<AP_BeOSFrameImpl *>(getFrameImpl());
	pFrameImpl->m_pBeWin->Lock();
	
	be_DocView *pView = pFrameImpl->getBeDocView();
	
	if(pView)
	{
		pView->ConvertToScreen(&pt);
	}
	
	pFrameImpl->m_pBeWin->Unlock();
	
	x = (UT_sint32)pt.x;
	y = (UT_sint32)pt.y;
}
//
