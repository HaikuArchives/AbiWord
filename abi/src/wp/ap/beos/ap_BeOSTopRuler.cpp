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
#include "ut_debugmsg.h"

#include <InterfaceKit.h>
#include <MessageFilter.h>

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_BeOSFrameImpl.h"
#include "ap_BeOSTopRuler.h"
#include "gr_BeOSGraphics.h"

#define ENSUREP(p)	do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*
 TODO: Take all of this generic ruler draw view stuff and put
 it in it's own class, rather than duplicating it!
*/

/*****************************************************************/
class TopRulerDrawView: public BBackView 
{
public:
 	TopRulerDrawView(AP_BeOSTopRuler *pRuler, AV_View*, const char *name, uint32 flags);
	virtual void FrameResized(float new_width, float new_height);
//	virtual void Draw(BRect invalid);
	AP_BeOSTopRuler *m_pAPRuler;
};

class RulerFilter: public BMessageFilter {
        public:
                RulerFilter(TopRulerDrawView *pRuler);
                filter_result Filter(BMessage *message, BHandler **target);
        private:
                TopRulerDrawView  *m_pRuler;
};

RulerFilter::RulerFilter(TopRulerDrawView *pRuler)
          : BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE) {
        m_pRuler = pRuler;
}

filter_result RulerFilter::Filter(BMessage *msg, BHandler **target) {
        switch(msg->what) {
        case B_MOUSE_MOVED:
		//Check the queue for other mouse moved events,
		//if they exists then drop this one.
        case B_MOUSE_DOWN:
        case B_MOUSE_UP:
                break;
        default:
                return(B_DISPATCH_MESSAGE);
        }

	if (!m_pRuler)
		return(B_DISPATCH_MESSAGE);
        AP_BeOSTopRuler * pBeOSTopRuler = m_pRuler->m_pAPRuler;
	if (!pBeOSTopRuler)
		return(B_DISPATCH_MESSAGE);

  	BPoint  pt;
        int32   clicks, mod, buttons;
        EV_EditModifierState ems = 0;
        EV_EditMouseButton emb = 0;

        msg->FindInt32("clicks", &clicks);
        msg->FindInt32("buttons", &buttons);
        msg->FindInt32("modifiers", &mod);

        if (mod & B_SHIFT_KEY)
                ems |= EV_EMS_SHIFT;
        if (mod & B_CONTROL_KEY)
                ems |= EV_EMS_CONTROL;
        if (mod & B_OPTION_KEY)
                ems |= EV_EMS_ALT;

        if (buttons & B_PRIMARY_MOUSE_BUTTON)
                emb = EV_EMB_BUTTON1;
        else if (buttons & B_PRIMARY_MOUSE_BUTTON)
                emb = EV_EMB_BUTTON2;
        else if (buttons & B_PRIMARY_MOUSE_BUTTON)
                emb = EV_EMB_BUTTON3;           

	if (msg->what == B_MOUSE_DOWN) {
        	msg->FindPoint("where", &pt);
		pBeOSTopRuler->mousePress(ems, emb,(long)pt.x,(long)pt.y);
	}
	else if (msg->what == B_MOUSE_UP) {
        	msg->FindPoint("where", &pt);
	        pBeOSTopRuler->mouseRelease(ems, emb, (long)pt.x, (long)pt.y);
	}
	else if (msg->what == B_MOUSE_MOVED && emb) {
		//The where position is screen centric, use the view_where
		//instead.  Also we get a -1 value when we leave the view 
        	msg->FindPoint("be:view_where", &pt);
		if (pt.x >= 0 && pt.y >=0)
 			pBeOSTopRuler->mouseMotion(ems, (long)pt.x, (long)pt.y);   
	}

        return(B_SKIP_MESSAGE);
}                                             

TopRulerDrawView::TopRulerDrawView(AP_BeOSTopRuler *pRuler, AV_View *pView, 
	const char *name, uint32 flags) 
		: BBackView(pView, name, flags)
{
	m_pAPRuler = pRuler;
	AddFilter(new RulerFilter(this));
	SetExplicitMaxSize(BSize(B_SIZE_UNSET, 32));
}

void TopRulerDrawView::FrameResized(float new_width, float new_height) 
{
//	First call inherited version
	BBackView::FrameResized(new_width, new_height);
	GR_Graphics * pG = m_pAPRuler->getGraphics();
	if(!pG) { return; }
	m_pAPRuler->setHeight((int)new_height+1);
	m_pAPRuler->setWidth((int)new_width+1);
	m_pAPRuler->draw(NULL);
	pG->flush();
}

/*****************************************************************/

AP_BeOSTopRuler::AP_BeOSTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_wTopRuler = NULL;
}

AP_BeOSTopRuler::~AP_BeOSTopRuler(void)
{
}

void AP_BeOSTopRuler::createWidget()
{
	m_wTopRuler = new TopRulerDrawView(this, NULL, "TopRuler", 
		B_WILL_DRAW | B_FRAME_EVENTS );
	
	//Attach the widget to the window ...
	BWindow *pBWin = (BWindow *)((XAP_BeOSFrameImpl *)m_pFrame->getFrameImpl())->getTopLevelWindow();
	pBWin->AddChild(m_wTopRuler);
 	setHeight(32);
	setWidth(1);
}

void AP_BeOSTopRuler::setView(AV_View * pView) 
{
	AP_TopRuler::setView(pView);

	if (m_wTopRuler && pView) 
	{
		m_wTopRuler->SetView(pView);
		// We really should allocate m_pG in createWidget(), but
		// unfortunately, the actual window (m_wTopRuler->window)
		// is not created until the frame's top-level window is
		// shown. 
		DELETEP(m_pG);
		BBackView* preview = m_wTopRuler;
		GR_BeOSAllocInfo ai(preview, m_pFrame->getApp());		
		m_pG = m_pFrame->getApp()->newGraphics(ai);
		UT_ASSERT(m_pG);
	}
}
