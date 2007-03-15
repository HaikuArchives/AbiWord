/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 William Lachance
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

#ifndef AP_BEOSFRAMEIMPL_H
#define AP_BEOSFRAMEIMPL_H
#include "xap_Frame.h"
#include "ap_Frame.h"
#include "xap_BeOSFrameImpl.h"
#include "ap_BeOSFrame.h"
#include "ie_types.h"

class XAP_BeOSApp;
class AP_BeOSFrame;

enum apufi_ScrollType { apufi_scrollX, apufi_scrollY }; // can we use namespaces yet? this is quite ugly

class AP_BeOSFrameImpl : public XAP_BeOSFrameImpl
{
 public:
	AP_BeOSFrameImpl(AP_BeOSFrame *pBeOSFrame, XAP_BeOSApp *pBeOSApp); 
	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame, XAP_App *pApp);
//	virtual XAP_Frame *			buildFrame(XAP_Frame * pF);

	virtual UT_RGBColor getColorSelBackground () const;
	virtual UT_RGBColor getColorSelForeground () const;
 protected:
	friend class AP_BeOSFrame;

	void _showOrHideStatusbar(void);
	void _showOrHideToolbars(void);

	virtual void _refillToolbarsInFrameData();

	void _createWindow();
	void _bindToolbars(AV_View * pView);
	virtual void _setWindowIcon();
//	void _setScrollRange(apufi_ScrollType scrollType, int iValue, gfloat fUpperLimit, gfloat fSize);

};
#endif
