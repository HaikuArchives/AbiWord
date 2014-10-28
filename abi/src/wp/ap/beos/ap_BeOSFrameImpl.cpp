#include <Cursor.h>

#include "ap_BeOSFrameImpl.h"
#include "ap_BeOSApp.h"
#include "ev_BeOSToolbar.h"
#include "ap_FrameData.h"
#include "ap_BeOSTopRuler.h"
#include "ap_BeOSLeftRuler.h"
#include "xap_BeOSApp.h"
//#include "xap_BeOSDialogHelper.h"
#include "ap_BeOSStatusBar.h"
#include "gr_BeOSGraphics.h"
#include "ut_debugmsg.h"

#ifdef ABISOURCE_LICENSED_TRADEMARKS
#include "abiword_48_tm.xpm"
#else
#include "abiword_48.xpm"
#endif

AP_BeOSFrameImpl::AP_BeOSFrameImpl(AP_BeOSFrame *pBeOSFrame, XAP_BeOSApp *pBeOSApp) :
	XAP_BeOSFrameImpl(static_cast<XAP_Frame *>(pBeOSFrame), static_cast<AP_App *>(pBeOSApp))
{
	UT_DEBUGMSG(("Created AP_BeOSFrameImpl %x \n",this));
}

XAP_FrameImpl * AP_BeOSFrameImpl::createInstance(XAP_Frame *pFrame, XAP_App *pApp)
{
	XAP_FrameImpl *pFrameImpl = new AP_BeOSFrameImpl(static_cast<AP_BeOSFrame *>(pFrame), static_cast<XAP_BeOSApp *>(pApp));
	return pFrameImpl;
}

// Does the initial show/hide of toolbars (based on the user prefs).
// This is needed because toggleBar is called only when the user
// (un)checks the show {Stantandard,Format,Extra} toolbar checkbox,
// and thus we have to manually call this function at startup.

/*!
 * Refills the framedata class with pointers to the current toolbars. We 
 * need to do this after a toolbar icon and been dragged and dropped.
 */
void AP_BeOSFrameImpl::_refillToolbarsInFrameData()
{
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();
	UT_DEBUGMSG(("Refill tool bar\n"));
	for (UT_uint32 i = 0; i < cnt; i++)
	{
		EV_BeOSToolbar * pBeOSToolbar = static_cast<EV_BeOSToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*>(getFrame()->getFrameData())->m_pToolbar[i] = pBeOSToolbar;
	}
}

/*****
 This is here as an abstraction of the Be _createDocumentWindow() code
 which sits in the xap code.  In the worse case we just create a
 plain view, in the best case we actually put neat stuff here
*****/
be_DocView *be_Window::_createDocumentWindow() 
{
	const BCursor *tc=B_CURSOR_I_BEAM;	
	UT_DEBUGMSG(("create Window fn\n"));

	//Set up the scroll bars on the outer edges of the document area
	m_vScroll = new TFScrollBar(m_pBeOSFrame, "VertScroll", NULL, 0, 100, B_VERTICAL);

	m_hScroll = new TFScrollBar(m_pBeOSFrame, "HortScroll", NULL, 0, 100, B_HORIZONTAL);
	m_pBeOSFrame->setScrollBars(m_hScroll, m_vScroll);

	//Create the Top and Left Rulers (need a width here)
#define TOP_HEIGHT 32
#define LEFT_WIDTH 32
	// create the top ruler
	XAP_Frame* pFrame = m_pBeOSFrame->getFrame();	
	AP_BeOSTopRuler * pBeOSTopRuler = new AP_BeOSTopRuler(pFrame);
	UT_ASSERT(pBeOSTopRuler);
	pBeOSTopRuler->createWidget();
	((AP_FrameData*)m_pBeOSFrame->getFrame()->getFrameData())->m_pTopRuler = pBeOSTopRuler;

	// create the left ruler
	AP_BeOSLeftRuler * pBeOSLeftRuler = new AP_BeOSLeftRuler(pFrame);
	UT_ASSERT(pBeOSLeftRuler);

	BGroupView* v = new BGroupView(B_HORIZONTAL);
	v->GroupLayout()->SetSpacing(0);
	v->AddChild(pBeOSLeftRuler->createWidget());
	v->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	((AP_FrameData*)m_pBeOSFrame->getFrame()->getFrameData())->m_pLeftRuler = pBeOSLeftRuler;

	// get the width from the left ruler and stuff it into the top ruler.
	pBeOSTopRuler->setOffsetLeftRuler(pBeOSLeftRuler->getWidth());

	//Add the document view in the remaining space
	m_pbe_DocView = new be_DocView(m_pBeOSFrame->getFrame()->getCurrentView(),
		"MainDocView", B_WILL_DRAW | B_NAVIGABLE);
	m_pbe_DocView->SetViewColor(0,120, 255);
	m_pbe_DocView->SetViewColor(B_TRANSPARENT_32_BIT);
	//Add the view to both frameworks (Be and Abi)

	v->AddChild(m_pbe_DocView);
	v->AddChild(m_vScroll);

	AddChild(v);
	AddChild(m_hScroll);

	m_pBeOSFrame->setBeDocView(m_pbe_DocView);
	
	//Without this we never get any key inputs
	m_pbe_DocView->WindowActivated(true); // So the cursor shows up.
	return(m_pbe_DocView);
}

BView * be_Window::_createStatusBarWindow() 
{
	AP_BeOSStatusBar *pStatusBar = new AP_BeOSStatusBar(m_pBeOSFrame->getFrame());
	BView *pStatusBarView;
	UT_ASSERT(pStatusBar);
	static_cast<AP_FrameData*>(m_pBeOSFrame->getFrame()->getFrameData())->m_pStatusBar = pStatusBar;
	BRect r;
	r = Bounds();
	r.top = r.bottom - STATUS_BAR_HEIGHT;
	pStatusBarView = pStatusBar->createWidget(r);
	AddChild(pStatusBarView);	
	return pStatusBarView;	
}	


void AP_BeOSFrameImpl::_createWindow()
{
	UT_DEBUGMSG(("create top levelwindow\n"));
	createTopLevelWindow();
	getTopLevelWindow()->Show();	
	_showOrHideToolbars();	
	UT_DEBUGMSG(("show toolbars\n"));
	// _showOrHideStatusbar();
	// we let our caller decide when to show m_wTopLevelWindow.
	return;
}

UT_RGBColor AP_BeOSFrameImpl::getColorSelBackground () const
{
	return UT_RGBColor (0, 0, 0);
}

UT_RGBColor AP_BeOSFrameImpl::getColorSelForeground () const
{
	return UT_RGBColor (255, 255, 255);
}
//
void AP_BeOSFrameImpl::_setWindowIcon()
{
}
//
void AP_BeOSFrameImpl::_bindToolbars(AV_View * pView)
{
	int nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (int k = 0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.		
		EV_BeOSToolbar * pBeOSToolbar = (EV_BeOSToolbar *)m_vecToolbars.getNthItem(k);
		pBeOSToolbar->bindListenerToView(pView);
	}	
}
//
void AP_BeOSFrameImpl::_showOrHideToolbars()
{
	XAP_Frame* pFrame = getFrame();
	bool *bShowBar = static_cast<AP_FrameData*>(pFrame->getFrameData())->m_bShowBar;
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();
    	UT_DEBUGMSG(("show or hide toolbars\n"));
	for (UT_uint32 i = 0; i < cnt; i++)
	{
		// TODO: The two next lines are here to bind the EV_Toolbar to the
		// AP_FrameData, but their correct place are next to the toolbar creation (JCA)
		EV_BeOSToolbar * pBeOSToolbar = static_cast<EV_BeOSToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (pFrame->getFrameData())->m_pToolbar[i] = pBeOSToolbar;
		static_cast<AP_BeOSFrame *>(pFrame)->toggleBar(i, bShowBar[i]);
	}
}
