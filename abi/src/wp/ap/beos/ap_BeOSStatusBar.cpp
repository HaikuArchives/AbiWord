/* AbiWord
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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_debugmsg.h"

#include <InterfaceKit.h>
#include <String.h>

#include "xap_Frame.h"
#include "xap_BeOSFrameImpl.h"
#include "gr_BeOSGraphics.h"
#include "ap_BeOSStatusBar.h"

//////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

rgb_color dark = { 154, 154, 154, 255 };
rgb_color light = { 241, 241, 241, 255 };
//
class StatusBarField: public BView {
public: 
	StatusBarField(const char * name, const char * atext);
	virtual void Draw(BRect invalid);
	void SetText(const char *text);
private:
	BString * m_text;
};

StatusBarField::StatusBarField(const char * name, const char * atext)
	:BView(name, B_WILL_DRAW | B_FRAME_EVENTS)
{		
	m_text = new BString("");
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));	
	SetDrawingMode(B_OP_OVER);
}

void StatusBarField::Draw(BRect invalid)
{
	BRect r = Bounds();
	ClearViewBitmap();
	SetHighColor(light);
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom));
	StrokeLine(BPoint(r.left+1, r.bottom-1), BPoint(r.right-1, r.bottom-1));
	StrokeLine(BPoint(r.right, r.top), BPoint(r.right, r.bottom));
	StrokeLine(BPoint(r.right-1, r.top+1), BPoint(r.right-1, r.bottom-1));

	SetHighColor(dark);
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.left, r.top));
	StrokeLine(BPoint(r.left+1, r.bottom-1), BPoint(r.left+1, r.top+1));
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	StrokeLine(BPoint(r.left+1, r.top+1), BPoint(r.right-1, r.top+1));

	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));	
	DrawString(m_text->String(), BPoint(4, Bounds().bottom-3));
}

void StatusBarField::SetText(const char * text)
{
	m_text->SetTo(text);
}

class ap_bsb_TextListener : public AP_StatusBarFieldListener
{
public:
	ap_bsb_TextListener(AP_StatusBarField *pStatusBarField, StatusBarField *sbf) : AP_StatusBarFieldListener(pStatusBarField) { sbfield = sbf;}
	virtual void notify(); 
private:
	StatusBarField * sbfield;
};

void ap_bsb_TextListener::notify()
{
	AP_StatusBarField_TextInfo * textInfo = ((AP_StatusBarField_TextInfo *)m_pStatusBarField);
	if(sbfield->Window()->Lock())
	{
		sbfield->SetText(textInfo->getBuf().utf8_str());
		sbfield->Draw(sbfield->Bounds());
		sbfield->Window()->Unlock();
	}
}

class StatusBarDrawView: public BBackView
{
public:
 	StatusBarDrawView(AP_BeOSStatusBar *pBar, AV_View *pView, 
		const char *name, uint32 flags);
	virtual void FrameResized(float new_width, float new_height);
	
private:
	float				m_fOldWidth;
	float				m_fOldHeight;
	AP_BeOSStatusBar  *m_pAPStatusBar;

};

StatusBarDrawView::StatusBarDrawView(AP_BeOSStatusBar *pBar, AV_View *pView, 
	const char *name, uint32 flags) 
		: BBackView(pView, name, flags) 
{
	m_pAPStatusBar = pBar;
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetExplicitMaxSize(BSize(B_SIZE_UNSET, 16));
}

void StatusBarDrawView::FrameResized(float new_width, float new_height) 
{
	BBackView::FrameResized(new_width, new_height);
//TODO does this goes well?? umm.. we'll see later
//	BRect r;
//	if (new_width > m_fOldWidth)
//	{
//		r.left=m_fOldWidth-5;
//		r.right=new_width-1;
//		r.top=Bounds().top;
//		r.bottom=Bounds().bottom-1;
//		UT_DEBUGMSG(("Actually invalidating StatusBar\n"));
//		Invalidate(r);
//	}
//	m_fOldWidth=new_width;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_BeOSStatusBar::AP_BeOSStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
	m_pG = NULL;
}

AP_BeOSStatusBar::~AP_BeOSStatusBar(void)
{
	DELETEP(m_pG);
}

BBackView * AP_BeOSStatusBar::createWidget(BRect r)
{
	BWindow *pBWin = (BWindow*)((XAP_BeOSFrameImpl *)(m_pFrame->getFrameImpl()))->getTopLevelWindow();
	UT_ASSERT(!m_pG && !m_wStatusBar);
	m_wStatusBar = new StatusBarDrawView(this,m_pView,"StatusBar", B_WILL_DRAW | B_FRAME_EVENTS);
	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	
	m_wStatusBar->SetLayout(layout);
	BGroupLayoutBuilder builder(layout);
	float cleft = 1;
	for (UT_uint32 k=0; k<getFields()->getItemCount(); k++) 
	{
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT(pf); // we should NOT have null elements
		AP_StatusBarField_TextInfo *pf_TextInfo = static_cast<AP_StatusBarField_TextInfo*>(pf); 		
		const char * rs = pf_TextInfo->getRepresentativeString();
		BRect fr(cleft+r.left, r.top+2, cleft+r.left+m_wStatusBar->StringWidth(rs)+4, r.bottom-2);
		cleft+=fr.Width()+2;
		char sbfn[20];
		sprintf(sbfn, "StatusBarField%i", k);
		StatusBarField * pSBField = new StatusBarField(sbfn, sbfn);		
		pf->setListener((AP_StatusBarFieldListener *)(new ap_bsb_TextListener(pf_TextInfo, pSBField)));		
		builder.Add(pSBField);
//		pSBField->SetAlignment(B_ALIGN_LEFT);
//		if (pf_TextInfo->getAlignmentMethod() == CENTER)
//			pSBField->SetAlignment(B_ALIGN_CENTER);
//		else
//			pSBField->SetAlignment(B_ALIGN_RIGHT);
	}	

	builder.AddGlue();

 	return m_wStatusBar;
}

void AP_BeOSStatusBar::setView(AV_View * pView)
{
	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wStatusBar->window)
	// is not created until the frame's top-level window is
	// shown.

	UT_DEBUGMSG(("StatusBar setView"));
	DELETEP(m_pG);	
	UT_ASSERT(m_wStatusBar);

	//GR_BeOSGraphics *pG = new GR_BeOSGraphics(m_wStatusBar , m_pFrame->getApp());
	GR_BeOSAllocInfo ai(m_wStatusBar, m_pFrame->getApp());
	m_pG = (GR_BeOSGraphics*)XAP_App::getApp()->newGraphics(ai);
	UT_ASSERT(m_pG);

	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	// Now that we've initialized the graphics context and
	// installed the GUI font, let the base class do it's
	// think and layout the fields.
	
	AP_StatusBar::setView(pView);
}


void AP_BeOSStatusBar::show(void)
{
	m_wStatusBar->Show();
}

void AP_BeOSStatusBar::hide(void)
{
	m_wStatusBar->Hide();
}
