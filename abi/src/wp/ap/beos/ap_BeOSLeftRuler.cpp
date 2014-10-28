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
#include "ap_BeOSLeftRuler.h"
#include "gr_BeOSGraphics.h"

#define ENSUREP(p)	do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

class LeftRulerDrawView: public BBackView 
{
public:
	LeftRulerDrawView(AP_BeOSLeftRuler *pRuler, AV_View *pView, 
		const char *name, uint32 flags);
		
	virtual void FrameResized(float new_width, float new_height);
	AP_BeOSLeftRuler *m_pAPRuler;
};

class LeftRulerFilter: public BMessageFilter 
{
        public:
                LeftRulerFilter(LeftRulerDrawView *pRuler);
                filter_result Filter(BMessage *message, BHandler **target);
        private:
                LeftRulerDrawView  *m_pRuler;
};

LeftRulerFilter::LeftRulerFilter(LeftRulerDrawView *pRuler)
          : BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE) 
{
        m_pRuler = pRuler;
}

filter_result LeftRulerFilter::Filter(BMessage *msg, BHandler **target) 
{
/*
  There currently aren't any events 
  defined for the Left Ruler yet.
  We are ready when they do happen!
*/
	return(B_DISPATCH_MESSAGE);
#if 0
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
        AP_BeOSLeftRuler * pBeOSLeftRuler = m_pRuler->m_pAPRuler;
	if (!pBeOSLeftRuler)
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
		pBeOSLeftRuler->mousePress(ems, emb,(long)pt.x,(long)pt.y);
	}
	else if (msg->what == B_MOUSE_UP) {
        	msg->FindPoint("where", &pt);
	        pBeOSLeftRuler->mouseRelease(ems, emb, (long)pt.x, (long)pt.y);
	}
	else if (msg->what == B_MOUSE_MOVED && emb) {
		//The where position is screen centric, use the view_where
		//instead.  Also we get a -1 value when we leave the view 
        	msg->FindPoint("be:view_where", &pt);
		if (pt.x >= 0 && pt.y >=0)
 			pBeOSLeftRuler->mouseMotion(ems, (long)pt.x, (long)pt.y);   
	}

        return(B_SKIP_MESSAGE);
#endif
}                                             

LeftRulerDrawView::LeftRulerDrawView(AP_BeOSLeftRuler *pRuler, AV_View *pView, 
	const char *name, uint32 flags) :
		BBackView(pView, name, flags) 
{
	m_pAPRuler = pRuler;
	AddFilter(new LeftRulerFilter(this));
}

void LeftRulerDrawView::FrameResized(float new_width, float new_height) 
{
	//First call the inherited version
	BBackView::FrameResized(new_width, new_height);
	GR_Graphics * pG = m_pAPRuler->getGraphics();
	if(!pG) { return; }
	m_pAPRuler->setHeight(new_height+1);
	m_pAPRuler->setWidth(new_width+1);
	m_pAPRuler->draw(NULL);
	pG->flush();
}

/*****************************************************************/

AP_BeOSLeftRuler::AP_BeOSLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame)
{
	m_wLeftRuler = NULL;
}

AP_BeOSLeftRuler::~AP_BeOSLeftRuler(void)
{
}

BView* AP_BeOSLeftRuler::createWidget()
{
	m_wLeftRuler = new LeftRulerDrawView(this, NULL, "LeftRuler",
		B_WILL_DRAW);
	m_wLeftRuler->SetExplicitMaxSize(BSize(32, B_SIZE_UNSET));
	setHeight(1);
	setWidth(32);

	return m_wLeftRuler;
}

void AP_BeOSLeftRuler::setView(AV_View * pView) 
{
	AP_LeftRuler::setView(pView);

	if (m_wLeftRuler && pView) 
	{
		m_wLeftRuler->SetView(pView);
		// We really should allocate m_pG in createWidget(), but
		// unfortunately, the actual window (m_wLeftRuler->window)
		// is not created until the frame's top-level window is
		// shown.
		DELETEP(m_pG);
		BBackView * preview = m_wLeftRuler;
		GR_BeOSAllocInfo ai(preview, m_pFrame->getApp());
		m_pG = m_pFrame->getApp()->newGraphics(ai);
		UT_ASSERT(m_pG);
	}
}

void AP_BeOSLeftRuler::show(void)
{
	m_wLeftRuler->Show();
}

void AP_BeOSLeftRuler::hide(void) 
{
	m_wLeftRuler->Hide();
}
