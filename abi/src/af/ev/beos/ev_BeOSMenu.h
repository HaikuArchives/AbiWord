/* AbiSource Program Utilities
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

#ifndef EV_BEOSMENU_H
#define EV_BEOSMENU_H

#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Menu.h"

#include <MenuItem.h>

#define ABI_BEOS_MENU_EV 		'amnu'
#define ABI_BEOS_MENU_EV_NAME 	"menu_event"

class AV_View;
class XAP_BeOSApp;
class XAP_BeOSFrameImpl;
class BPopUpMenu;

/*****************************************************************/

class EV_BeOSMenu : public EV_Menu
{
public:
	EV_BeOSMenu(XAP_BeOSApp * pBeOSApp, XAP_Frame * pFrame,
				const char * szMenuLayoutName,
				const char * szMenuLabelSetName);
	~EV_BeOSMenu(void);
	bool synthesize();
	bool synthesizeMenuBar();

 	XAP_Frame * 	getFrame(); 	
protected:
	XAP_BeOSApp 	*m_pBeOSApp;
	XAP_Frame 		*m_pFrame;
	
	class BMenuBar* pMenuBar;

	virtual bool		_doAddMenuItem(UT_uint32 id) { return false; /* TODO */ }
        void _convertStringToAccel(const char *s, char &accel_key, int &ac_mods);   
	BMenuItem * s_createNormalMenuEntry(const XAP_Menu_Id id, const bool isCheckable,const bool isRadio, const char *szLabelName, const char *szMnemonicName);
};

/*****************************************************************/

class EV_BeOSMenuPopup : public EV_BeOSMenu
{
public:
	EV_BeOSMenuPopup(XAP_BeOSApp * pBeOSApp, XAP_Frame * pBeOSFrame,
					  const char * szMenuLayoutName,
					  const char * szMenuLabelSetName);
	~EV_BeOSMenuPopup(void);

	bool				synthesizeMenuPopup(XAP_BeOSFrameImpl * pFrame);
	
	BPopUpMenu* GetHandle();
	
protected:
	
	BPopUpMenu* m_pPopup;
};

#endif /* EV_BEOSMENU_H */
