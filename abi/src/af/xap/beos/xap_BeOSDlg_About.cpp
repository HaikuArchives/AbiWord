/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <Window.h>
#include <TranslationUtils.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Rehydrate.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrameImpl.h"
#include "xap_Frame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_BeOSDlg_About.h"

#define DEFAULT_BUTTON_WIDTH 85

/*****************************************************************/
extern unsigned char g_pngSidebar[];            // see ap_wp_sidebar.cpp
extern unsigned long g_pngSidebar_sizeof;       // see ap_wp_sidebar.cp

class AboutWin:public BWindow
{
public:
	AboutWin(XAP_Frame * pFrame);
	virtual void DispatchMessage(BMessage *msg, BHandler *handler);
	void init();
	void initDialogFields(void);

private:
	XAP_Frame *m_pFrame;
};                                                    

AboutWin::AboutWin(XAP_Frame * pFrame)
	: BWindow(BRect(58.0, 62.0, 574.0, 439.0), "About", B_FLOATING_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_RESIZABLE | B_NOT_MINIMIZABLE)
{
	m_pFrame = pFrame;
	init();
}

void AboutWin::init()
{	
#warning TODO replace with constants
	BView * view = new BView(BRect(0, 0, 755, 555), "", B_FOLLOW_ALL, 536870912);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(view);
	
	BBox * box = new BBox(BRect(3.0, 3.0, 512.0, 369.0), "", B_FOLLOW_ALL, 738197504);
	view->AddChild(box);
	
	view = new BView(BRect(7.0, 6.0, 227.0, 356.0), "sideView", 4626, 536870912);
	box->AddChild(view);
	
	BStringView * strview = new BStringView(BRect(313.0, 61.0, 468.0, 78.0 ), "strVersion", "Version:", 4626, 536870912);
	box->AddChild(strview);
	
	BButton * btn = new BButton(BRect(297.0, 333.0, 411.0, 357.0), "", "www.abisource.com", new BMessage('gurl'), 4626, 570425344);
	box->AddChild(btn);
	
	btn = new BButton(BRect(418.0, 330.0, 499.0, 360.0), "", "OK", new BMessage('_QRQ'), 4626, 570425344);
	box->AddChild(btn);
	
	strview = new BStringView(BRect(235.0, 86.0, 501.0, 107.0), "strCopyright", "strCopyright", 4626, 536870912);
	box->AddChild(strview);
	
	BTextView * textview = new BTextView(BRect(236.0, 111.0, 498.0, 329.0), "strBodyText", BRect(1.0, 1.0, 261.0, 14.0), 4626, 910163968);
	box->AddChild(textview);
}

void AboutWin::initDialogFields(void)
{
	XAP_App * pApp = m_pFrame->getApp();
	
	char buf[1000];

	BStringView *strAppName = (BStringView*)FindView("strAppName");
	if (strAppName) strAppName->SetText(pApp->getApplicationName());
	
	BStringView *strVersion = (BStringView*)FindView("strVersion");
	sprintf(buf, XAP_ABOUT_VERSION, pApp->getBuildId());
	if (strVersion) strVersion->SetText(buf);

	BStringView * strCopyright = (BStringView*)FindView("strCopyright");
	if (strCopyright) strCopyright->SetText(XAP_ABOUT_COPYRIGHT);

	BTextView * strBodyText = (BTextView*)FindView("strBodyText");
	sprintf(buf,XAP_ABOUT_GPL_LONG_LF,pApp->getApplicationName());
	if (strBodyText) strBodyText->SetText(buf);

//	str = (BStringView*)FindView("strOptions");
//	sprintf(buf, XAP_ABOUT_BUILD, XAP_App::s_szBuild_Options);
//	if (str) str->SetText(buf);

	BView *view = FindView("sideView");
	if (view) {
		BMemoryIO mio(g_pngSidebar, g_pngSidebar_sizeof);
		BBitmap *bitmap =  BTranslationUtils::GetBitmap(&mio);
		if (bitmap)
		{
			BRect frameRect = bitmap->Bounds();
			view->ResizeTo(frameRect.Width() , frameRect.Height());
			view->SetViewBitmap(bitmap, B_FOLLOW_ALL, 0);
		}
		
	}
} 

void AboutWin::DispatchMessage(BMessage *msg, BHandler *handler)
{
	switch(msg->what)
	{
	case 'gurl':
		XAP_App::getApp()->openURL("http://www.abisource.com");
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
		break;
	}                           
}

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_About::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_BeOSDialog_About * p = new XAP_BeOSDialog_About(pFactory,id);
	return p;
}

XAP_BeOSDialog_About::XAP_BeOSDialog_About(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: XAP_Dialog_About(pDlgFactory,id)
{
}

XAP_BeOSDialog_About::~XAP_BeOSDialog_About(void)
{
}

void XAP_BeOSDialog_About::runModal(XAP_Frame * pFrame)
{
	//XAP_App* pApp = pFrame->getApp();
	//char buf[2048];
	//sprintf(buf, XAP_ABOUT_TITLE, pApp->getApplicationName());
	//sprintf(buf, XAP_ABOUT_DESCRIPTION, pApp->getApplicationName());
	AboutWin *nwin = new AboutWin(pFrame);
	if (nwin)
	{
		nwin->initDialogFields();
		nwin->Show();
	}
}

