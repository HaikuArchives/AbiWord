#! gmake

## AbiSource Application Framework
## Copyright (C) 1998 AbiSource, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
## 02111-1307, USA.

ABI_ROOT:=$(shell pwd)/../../../..

#include $(ABI_ROOT)/src/config/abi_defs.mk
include $(ABI_ROOT)/src/config/abi_defs_wp.mk

CPPSRCS=		xap_BeOSApp.cpp			\
			xap_BeOSAppImpl.cpp		\
			xap_BeOSClipboard.cpp		\
			xap_BeOSDlg_WindowMore.cpp	\
			xap_BeOSDlg_About.cpp		\
			xap_BeOSDlg_Image.cpp		\
			xap_BeOSDlg_ClipArt.cpp		\
			xap_BeOSDlg_FileOpenSaveAs.cpp	\
			xap_BeOSDlg_FontChooser.cpp	\
			xap_BeOSDlg_Language.cpp		\
			xap_BeOSDlg_MessageBox.cpp	\
			xap_BeOSDlg_Password.cpp		\
			xap_BeOSDlg_PluginManager.cpp	\
			xap_BeOSDlg_Print.cpp			\
			xap_BeOSDlg_PrintPreview.cpp	\
			xap_BeOSEncodingManager.cpp	\
			xap_BeOSModule.cpp			\
			xap_BeOSDlg_Zoom.cpp		\
			xap_BeOSDlg_Insert_Symbol.cpp	\
			xap_BeOSFrameImpl.cpp		\
			xap_BeOSToolbar_Icons.cpp		\
			xap_BeOSDlg_Encoding.cpp		\
			xap_BeOSDlg_HTMLOptions.cpp	\
			xap_BeOSFontPreview.cpp \
			xap_BeOSDlg_ListDocuments.cpp \
			xap_BeOSDlg_History.cpp \
			xap_BeOSDlg_DocComparison.cpp


#CPPSRCS+=		be_GRDrawView.cpp

TARGETS=		$(OBJS)

include $(ABI_ROOT)/src/config/abi_rules.mk

build:: $(TARGETS)
