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


#ifndef XAP_BeOSFrameImpl_H
#define XAP_BeOSFrameImpl_H

#include <InterfaceKit.h>

//#include "xap_Frame.h"
#include "xap_FrameImpl.h"
#include "ut_vector.h"
#include "xap_BeOSDialogFactory.h"
#include "be_BackView.h"

#include "ev_BeOSKeyboard.h"
#include "ev_BeOSMouse.h"

class XAP_BeOSApp;
class EV_BeOSMenu;
class EV_BeOSMenuPopup;
class EV_Toolbar;
class GR_Graphics;
class ap_BeOSStausBar;
/*****************************************************************
******************************************************************
** This file defines the beos-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** BeOS-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/
class XAP_BeOSFrameImpl;

class TFScrollBar: public BScrollBar {
	public:
		TFScrollBar(XAP_BeOSFrameImpl *pFrame, BRect frame, const char *name,
					BView *target, float min, float max, 
					orientation direction);
		 virtual void ValueChanged(float newValue);
		
		XAP_BeOSFrameImpl *m_pBeOSFrame;				
};

class be_DocView: public BBackView {
	public: 
		be_DocView(AV_View * pView, BRect frame, const char *name, 
			   uint32 resizeMask, uint32 flags);
//		virtual	void Draw(BRect updateRect);
		virtual void SetPrintPicture(BPicture *pic) { m_pBPicture = pic; };
		virtual	void FrameResized(float new_width, float new_height);
		virtual void WindowActivated(bool activated);

		virtual void MouseDown(BPoint p) {
			MakeFocus(true);
			fMouseHandler->mouseClick(Looper()->CurrentMessage());
		}

		virtual void MouseUp(BPoint p) {
			MakeFocus(true);
			fMouseHandler->mouseUp(Looper()->CurrentMessage());
		}

		virtual void MouseMoved(BPoint p) {
			MakeFocus(true);
			fMouseHandler->mouseUp(Looper()->CurrentMessage());
		}

		virtual void KeyDown(const char* bytes, int32 numBytes) {
			if (!fKeyHandler->keyPressEvent(Looper()->CurrentMessage()))
				BBackView::KeyDown(bytes, numBytes);
		}

		void SetMouseHandler(ev_BeOSMouse* handler) {
			fMouseHandler = handler;
		}

		void SetKeyboardHandler(ev_BeOSKeyboard* handler) {
			fKeyHandler = handler;
		}


	BPicture *	m_pBPicture;

	private:
		ev_BeOSKeyboard* fKeyHandler;
		ev_BeOSMouse* fMouseHandler;

		float		m_fOldWidth;
		float		m_fOldHeight;
		
};

class be_Window: public BWindow {
	public:
		be_Window(XAP_BeOSApp *theApp, XAP_BeOSFrameImpl *theFrame,
			  BRect r, char *name, 
			  window_look look, window_feel feel, 
			  uint32 flags = 0, 
			  uint32 workspace = B_CURRENT_WORKSPACE);

		bool _createWindow(const char *szMenuLayoutName,
				   const char *szMenuLabelSetName);

		//Located in the app specific code
		be_DocView *		_createDocumentWindow(void);
		virtual BView *		_createStatusBarWindow(void);

		
		//Be Function overrides
		virtual bool QuitRequested(void);
		virtual void MessageReceived(BMessage *pMsg);
		
		be_DocView			*m_pbe_DocView;
		EV_BeOSMenu 		*m_pBeOSMenu;
		XAP_BeOSApp 		*m_pBeOSApp;
		XAP_BeOSFrameImpl 	*m_pBeOSFrame;
		TFScrollBar		*m_hScroll, *m_vScroll;
		BRect			m_winRectAvailable;
		BView			*m_pBeOSStatusBarView; //TODO: I don't like this!!!!!!!!!!!!!
};

class be_Status;

/*****************************************************************/

class XAP_BeOSFrameImpl : public XAP_FrameImpl
{
friend class be_Window;		// HACK: to allow access to _createToolbars
public:
	friend class XAP_FrameImpl;
	XAP_BeOSFrameImpl(XAP_Frame *pFrame, XAP_BeOSApp *app);
	friend class XAP_Frame;
	virtual ~XAP_BeOSFrameImpl(void);

	BWindow *getTopLevelWindow(void) const;
	void setTopLevelWindow(BWindow * window) { m_wTopLevelWindow = window; }
	void createTopLevelWindow(void);

	virtual void _initialize();
	
	virtual void 				_setFullScreen(bool isFullScreen) {}
	virtual bool				_openURL(const char * szURL);
	virtual bool				_updateTitle(void);
	virtual void                _nullUpdate () const {}
	virtual void                _setCursor(GR_Graphics::Cursor c) {}


	be_DocView *			getBeDocView(void) const;
	void 					setBeDocView(be_DocView *);
	UT_sint32 				_setInputMode(const char * szName);

	virtual XAP_DialogFactory 	*_getDialogFactory(void);
	virtual bool 				_runModalContextMenu(AV_View * pView, const char * szMenuName, UT_sint32 x, UT_sint32 y);

	//TF Added 
	UT_Vector *				VecToolbarLayoutNames();
  	const char *            ToolbarLabelSetName();
	GR_Graphics *			Graphics();
	void					setScrollBars(TFScrollBar *h, TFScrollBar *v);

	virtual EV_Menu*			_getMainMenu();

protected:
	virtual bool _openHelpURL(const char * szURL);
//	virtual UT_String _localizeHelpUrl (bool bLocal, const char * pathBefore, 
//										const char * pathAfter);
	virtual bool _close();
	virtual bool _raise();
	virtual bool _show();
	virtual EV_Toolbar *		_newToolbar(XAP_App *app, XAP_Frame *frame, const char *, const char *);
	void _rebuildToolbar(UT_uint32 ibar);	
	virtual void _queue_resize();
	
	// TODO see why ev_BeOSKeyboard has lowercase prefix...
	XAP_BeOSApp *				m_pBeOSApp;
	EV_BeOSMenu *				m_pBeOSMenu;
	EV_BeOSMenuPopup *			m_pBeOSPopup; /* only valid while a context popup is up */
	
	//Main window and  document view 
	be_Window *					m_pBeWin;			
	be_DocView *				m_pBeDocView;						
	TFScrollBar *				m_hScroll;
	TFScrollBar *				m_vScroll;
	BWindow *				m_wTopLevelWindow;
	
/*
	GtkWidget *				m_wTopLevelWindow;
	GtkWidget *				m_wVBox;
	GtkWidget * 				m_wSunkenBox;
*/

	AP_BeOSDialogFactory		m_dialogFactory;
};

#endif /* XAP_BeOSFrameImpl_H */
